#include "sim.h"
#include <stdio.h>

void SimI2c::addSlave(int addr, SimI2cSlave& Lslave) {
  slaves.emplace(addr,&Lslave);
}

//The real hardware is fairly complicated and driven by a state machine. The state variable is I2CSTAT.
//State transitions are triggered by writes to I2CCONSET and I2CCONCLR. Depending on the state,
//the real hardware will take some time, perhaps transmit or receive on the bus, and when it is done, it will
//set the SI bit in I2CSTAT and the state variable to the appropriate value. We simulate this pretty much
//as-is, except that no time is taken in transmitting or recieving.

void SimI2c::printConStatus(int port) {
  ::printf("I2CCON(%d) %02x (%d) AA=%d SI=%d STO=%d STA=%d I2EN=%d\n",port,I2CCONSET[port],I2CCONSET[port],AA(port),SI(port),STO(port),STA(port),I2EN(port));
}

void SimI2c::runStateMachine(int port) {
  if(SI(port)) return; //Don't run anything here unless the int bit has been cleared by software already. Keep us from going off half-cocked.
  bool ack=false;
  SI(port,1); //Since the transfer takes no time, immediately set the interrupt (hardware needs attention) flag
  ::printf("I2C running state 0x%02x\n",I2CSTAT[port]);
  switch(I2CSTAT[port]) {
    case 0xF8:
      //Idle
	  if(STA(port)) {
        ::printf("Transmission on I2C%d started\n",port);
        I2CSTAT[port]=0x08;
	  }
      break;
    case 0x08:
      //Start condition has been transmitted. Software should load I2CDAT with SLA+W and
      //clear the STA bit, in that order, so that when STA is cleared, I2CDAT already has
      //the appropriate addr+r/w and the slave can be addressed
      if(~STA(port)) {
    	//Check if there is a slave with this address on this port
        int addr=(I2CDAT[port] >> 1) & 0x7f;
        bool R=(I2CDAT[port] & 0x01)>0; //If lowest bit is 1, this is a read  (slave->master)
        bool W=~R;                      //         otherwise, this is a write (master->slave)
        if(slaves.count(addr)>0) {
          slave=slaves[addr];
        } else {
          slave=nullptr;
        }
    	if(slave!=nullptr) {
    	  slave->start();
    	  //There is a slave and we have addressed it
    	  if(R) {
    		I2CSTAT[port]=0x40; //Slave addressed for read
    	  } else {
    	    I2CSTAT[port]=0x18; //Slave addressed for write
    	  }
    	} else {
      	  if(R) {
      		I2CSTAT[port]=0x48; //Slave not addressed for read
      	  } else {
      	    I2CSTAT[port]=0x20; //Slave not addressed for write
      	  }
    	}
    	//In reality, we would have to handle arbitration lost. We are currently
    	//only simulating slaves, so arbitration can't be lost.
      }
      break;
    case 0x18:
      //SLA+W has been transmitted, slave has been addressed.
      //Intentional case statement fall-through, as handling is the same
      //for both cases
    case 0x28:
      //Data has been transmitted
	  if(STA(port)&&STO(port)) {
		//Stop followed by start. STO is cleared by hardware.
		slave->stop();
		slave->start();
		STO(port,false);
		I2CSTAT[port]=0x08;
	  } else if(STA(port)) {
		//Repeated start
		slave->repeatStart();
		I2CSTAT[port]=0x10;
	  } else if(STO(port)) {
		//Stop. STO cleared by hardware
		slave->stop();
		STO(port,false);
		I2CSTAT[port]=0xF8; //Back to idle state
	  } else {
        //Send a(nother) byte
	    slave->writeByte(I2CDAT[port],ack);
	    if(ack) {
	      I2CSTAT[port]=0x28; //Ready to send the next byte
	    } else {
	  	  I2CSTAT[port]=0x30; //Data byte NAK
	    }
	  }
      break;
    case 0x40:
      //SLA+R has been transmitted, slave has been addressed.
      //Intentional case fall-through
    case 0x50:
      ack=AA(port)!=0;
  	  I2CDAT[port]=slave->readByte(ack);
      if(ack) {
    	I2CSTAT[port]=0x50;
  	  } else {
  		I2CSTAT[port]=0x58;
  	  }
      break;
    case 0x58:
        //Data has been received, NAK sent back
  	  if(STA(port)&&STO(port)) {
  		//Stop followed by start. STO is cleared by hardware.
  		slave->stop();
  		slave->start();
  		STO(port,false);
  		I2CSTAT[port]=0x08;
  	  } else if(STA(port)) {
  		//Repeated start
  		slave->repeatStart();
  		I2CSTAT[port]=0x10;
  	  } else if(STO(port)) {
  		//Stop. STO cleared by hardware
  		slave->stop();
  		STO(port,false);
  		I2CSTAT[port]=0xF8; //Back to idle state
  	  }
      break;
    default:
      ::printf("Unhandled state 0x%02x\n",I2CSTAT[port]);
      exit(1);
      break;
  }
}

void SimI2c::write_I2CCONSET(int port, uint32_t write) {
  I2CCONSET[port]|=write;
  printConStatus(port);
  runStateMachine(port);
}

void SimI2c::write_I2CCONCLR(int port, uint32_t write) {
  I2CCONSET[port]&=~write;
  printConStatus(port);
  runStateMachine(port);
}

#include "HardTwoWire.h"
#include "gpio.h"
#include "Time.h"

//HardTwoWire: Use LPC214x hardware I2C port to implement TwoWire object
HardTwoWire::HardTwoWire(int Lport):port(Lport) {}

//Initialize I2C peripheral 
void HardTwoWire::twi_init(unsigned int freq) {
  //Turn on appropriate I2C peripheral
  PCONP |= (1<<(7+port));
  //Set the clock rate
  unsigned int rate=(PCLK/freq)/2;
  I2CSCLL(port)=rate;
  I2CSCLH(port)=rate;
  //Grab the pins needed
  if(port==0) {
    set_pin( 2,1); //Pin 0.2 is SCL0
    set_pin( 3,1); //Pin 0.3 is SDA0
  } else {
    set_pin(11,3); //Pin 0.11 is SCL1
    set_pin(14,3); //Pin 0.14 is SDA1
  }
}

uint8_t HardTwoWire::twi_readFrom(uint8_t address, char* data, uint8_t length) {
  //Go to master transmit mode
  I2CCONCLR(port)=STA | STO | SI | AA;
  //Send a start condition
  I2CCONSET(port)=EN | STA;
  wait_si(); //wait for start condition to be sent

  I2CDAT(port)=(address << 1)+1;  //Send slave address and read (low bit set)
  I2CCONCLR(port)=SI;  //Clear SI, allow protocol engine to continue
  wait_si(); //wait for address to be sent

  //Ask for each byte except the last, send ACK each time
  for(int i=0;i<length-1;i++) {
    I2CCONSET(port)=AA;
    I2CCONCLR(port)=STA | STO | SI;
    wait_si(); //Wait for data to arrive
    data[i]=I2CDAT(port);
  }

  //Get the final byte and send a NAK
  I2CCONCLR(port)=STA | STO | SI | AA; 

  wait_si(); //wait for data arrive

  data[length-1]=I2CDAT(port);

  twi_stop();

  return length;
}

uint8_t HardTwoWire::twi_writeTo(uint8_t address, const char* data, uint8_t length, uint8_t wait) {
  //Go to master transmit mode and send the start condition
  I2CCONCLR(port)=STA | STO | SI | AA; //Clear STA, STO, SI, AA
  I2CCONSET(port)=EN | STA; //set I2EN and STA

  wait_si(); //wait for start condition to be sent
  I2CDAT(port)=address << 1;  //Send slave address and write (low bit cleared)
  I2CCONCLR(port)=SI;  //Clear SI, allow protocol engine to continue
  wait_si(); //wait for address to be sent

  for(int i=0;i<length;i++) {
    I2CDAT(port)=data[i];
    //Clear SI, allow protocol engine to continue
    I2CCONCLR(port)=STA | STO | SI;

    wait_si(); //wait for data to be sent
  }

  twi_stop();

  return 0;
}

void HardTwoWire::twi_stop(void) {
  //Send the stop condition
  I2CCONSET(port)=STO;
  I2CCONCLR(port)=STA | SI;

}

void HardTwoWire::twi_releaseBus() {
  //LPC2148 automatically releases bus after sending STOP
}


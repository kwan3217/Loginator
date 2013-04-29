#include "StateTwoWire.h"
#include "gpio.h"
#include "Time.h"
#include "irq.h"
#include "Serial.h"

//Define this to use interrupts - really doesn't gain 
#undef I2C_IRQ

//StateTwoWire: Use LPC214x hardware I2C port to implement TwoWire object
StateTwoWire::StateTwoWire(int Lport):port(Lport) {
  thisPtr[port]=this;
}

StateTwoWire *StateTwoWire::thisPtr[2];


void StateTwoWire::IntHandler0() {
  thisPtr[0]->stateDriver();
}

void StateTwoWire::IntHandler1() {
  thisPtr[1]->stateDriver();
}

void stateIn(StateTwoWire* that) {
  Serial.print("Invalid state -- Port ");
  Serial.print(that->port,DEC);
  Serial.print(", State ");
  Serial.println(I2CSTAT(that->port),HEX,2);
}

void stateSl(StateTwoWire* that) {
  Serial.print("Slave state -- Port ");
  Serial.print(that->port,DEC);
  Serial.print(", State ");
  Serial.println(I2CSTAT(that->port),HEX,2);
}

void state00(StateTwoWire* that) {
  //Bus error - send stop
  I2CCONSET(that->port)=StateTwoWire::STO;
}
void state08(StateTwoWire* that) {
  //Start sent
  //I2CDAT<-SLA+R/W
  //I2CCON<-X00X
  I2CDAT(that->port)=that->address;
  I2CCONSET(that->port)=StateTwoWire::AA;
  I2CCONCLR(that->port)=StateTwoWire::STA | StateTwoWire::STO;
}
void state10(StateTwoWire* that) {
  //Repeated start sent
  //I2CDAT<-SLA+R/W
  //I2CCON<-X00X
  I2CDAT(that->port)=that->address;
  I2CCONSET(that->port)=StateTwoWire::AA;
  I2CCONCLR(that->port)=StateTwoWire::STA | StateTwoWire::STO;
}
void state18(StateTwoWire* that) {
  //SLA+W sent, ACK received
  //Could send repeated start (100X), stop (010X), stop then start (110X)
  //but normal action is to send first data byte
  //I2CDAT<-SLA+R/W
  //I2CCON<-000X
  I2CDAT(that->port)=*that->dataWrite;
  I2CCONSET(that->port)=StateTwoWire::AA;
  I2CCONCLR(that->port)=StateTwoWire::STA | StateTwoWire::STO;
}
void state20(StateTwoWire* that) {
  //SLA+W sent, NOT ACK received
  //Could send repeated start (100X), stop then start (110X),
  //resend last byte (000X) and I2CDAT-<data
  //but normal action is to stop and give up
  I2CCONSET(that->port)=StateTwoWire::STO;
  that->done=true;
}
void state28(StateTwoWire* that) {
  //Data sent, ACK received
  //Count that byte as sent
  that->lengthWrite--;
  that->dataWrite++;
  if(that->lengthWrite==0) {
    //If no more data to be sent, I2CCON<-010X for stop or I2CCON<-100X for repeated start
    I2CCONCLR(that->port)=StateTwoWire::STA;
    I2CCONSET(that->port)=StateTwoWire::STO;
    that->done=true;
  } else {
    //If more data to be sent, load next byte I2CDAT<-data
    //and just send it I2CCON<-(000X)
    I2CDAT(that->port)=*that->dataWrite;
    I2CCONSET(that->port)=StateTwoWire::AA;
    I2CCONCLR(that->port)=StateTwoWire::STA | StateTwoWire::STO;
  }
}
void state30(StateTwoWire* that) {
  I2CCONSET(that->port)=StateTwoWire::STO;
}
void state38(StateTwoWire* that) {
  I2CCONSET(that->port)=StateTwoWire::STA;
}
void state40(StateTwoWire* that) {
  if(that->lengthRead==1) {
    //Only one byte to be read, so NAK it immediately
    I2CCONCLR(that->port)=StateTwoWire::AA;
  } else {
    I2CCONSET(that->port)=StateTwoWire::AA;
  }
}
void state48(StateTwoWire* that) {
  that->done=true;
  I2CCONSET(that->port)=StateTwoWire::AA|StateTwoWire::STO;
}
void state50(StateTwoWire* that) {
  *that->dataRead=I2CDAT(that->port);
  that->dataRead++;
  that->lengthRead--;
  if(that->lengthRead==1) {
    //Only one more byte to read, so NAK it
    I2CCONCLR(that->port)=StateTwoWire::AA;
  } else {
    I2CCONSET(that->port)=StateTwoWire::AA;
  }
}
void state58(StateTwoWire* that) {
  *that->dataRead=I2CDAT(that->port);
  that->lengthRead--;
  that->done=true;
  I2CCONSET(that->port)=StateTwoWire::AA|StateTwoWire::STO;
}

void stateF8(StateTwoWire* that) {}

typedef void (*State)(StateTwoWire*);
static const State state[32]={&state00,&state08,
                              &state10,&state18,
                              &state20,&state28,
                              &state30,&state38,
                              &state40,&state48,
                              &state50,&state58,
                              &stateSl,&stateSl,
                              &stateSl,&stateSl,
                              &stateSl,&stateSl,
                              &stateSl,&stateSl,
                              &stateSl,&stateSl,
                              &stateSl,&stateSl,
                              &stateSl,&stateSl,
                              &stateIn,&stateIn,
                              &stateIn,&stateIn,
                              &stateIn,&stateF8};

void StateTwoWire::stateDriver() {
#ifdef I2C_IRQ
  int s=I2CSTAT(port);
  state[s>>3](this);
  I2CCONCLR(port)=SI;
  VICVectAddr=0;
#else
  if(I2CCONSET(port) & SI) {
    int s=I2CSTAT(port);
    state[s>>3](this);
    I2CCONCLR(port)=SI;
  }
#endif
}

//Initialize I2C peripheral 
void StateTwoWire::twi_init(unsigned int freq) {
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
  I2CCONSET(port)=EN;
  I2CCONCLR(port)=SI;
#ifdef I2C_IRQ
  if(port==0) { 
    IRQHandler::install(IRQHandler::I2C0,IntHandler0);
  } else {
    IRQHandler::install(IRQHandler::I2C1,IntHandler1);
  }
#endif
}

uint8_t StateTwoWire::twi_readFrom(uint8_t Laddress, char* Ldata, uint8_t Llength) {
  //Go to master transmit mode and send the start condition
  if(Llength==0) return 0;
  address=(Laddress<<1)|1;
  dataRead=Ldata;
  lengthRead=Llength;
  I2CCONSET(port)=STA; //set I2EN and STA
  done=false;
  while(!done) 
#ifdef I2C_IRQ
  ;
#else
  stateDriver();
#endif
  return Llength;
}

uint8_t StateTwoWire::twi_writeTo(uint8_t Laddress, const char* Ldata, uint8_t Llength, uint8_t wait) {
  //Go to master transmit mode and send the start condition
  if(Llength==0) return 0;
  address=(Laddress<<1)|0;
  dataWrite=Ldata;
  lengthWrite=Llength;
  I2CCONSET(port)=STA; //set I2EN and STA
  done=false;
#ifdef I2C_IRQ
  if(wait) {
#endif
  while(!done) 
#ifdef I2C_IRQ
  ;
#else
  stateDriver();
#endif
#ifdef I2C_IRQ
  }
#endif
  return 0;
}



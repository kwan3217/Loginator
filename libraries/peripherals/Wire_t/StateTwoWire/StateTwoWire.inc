#include "StateTwoWire.h"
#include "gpio.h"
#include "Time.h"
#include "irq.h"
#include "Serial.h"

//StateTwoWire: Use LPC214x hardware I2C port to implement TwoWire object
template<class T, int port> inline
StateTwoWire<T,port>::StateTwoWire() {

}

template<class T, int port>
void stateIn(StateTwoWire<T,port>& that) {
  Serial.print("Invalid state -- Port ");
  Serial.print(port,DEC);
  Serial.print(", State ");
  Serial.println(I2CSTAT(port),HEX,2);
}

template<class T, int port>
void stateSl(StateTwoWire<T,port>& that) {
  Serial.print("Slave state -- Port ");
  Serial.print(port,DEC);
  Serial.print(", State ");
  Serial.println(I2CSTAT(port),HEX,2);
}

/**
  Bus error - send stop
*/
template<class T, int port>
void state00(StateTwoWire<T,port>& that) {
  I2CCONSET(port)=StateTwoWire<T,port>::STO;
}

  //Start sent
template<class T, int port>
void state08(StateTwoWire<T,port>& that) {
  //I2CDAT<-SLA+R/W
  //I2CCON<-X00X
  I2CDAT(port)=that.address;
  I2CCONSET(port)=StateTwoWire<T,port>::AA;
  I2CCONCLR(port)=StateTwoWire<T,port>::STA | StateTwoWire<T,port>::STO;
}

  //Repeated start sent
template<class T, int port>
void state10(StateTwoWire<T,port>& that) {
  //I2CDAT<-SLA+R/W
  //I2CCON<-X00X
  I2CDAT(port)=that.address;
  I2CCONSET(port)=StateTwoWire<T,port>::AA;
  I2CCONCLR(port)=StateTwoWire<T,port>::STA | StateTwoWire<T,port>::STO;
}

  //SLA+W sent, ACK received
template<class T, int port>
void state18(StateTwoWire<T,port>& that) {
  //Could send repeated start (100X), stop (010X), stop then start (110X)
  //but normal action is to send first data byte
  //I2CDAT<-SLA+R/W
  //I2CCON<-000X
  I2CDAT(port)=*that.dataWrite;
  I2CCONSET(port)=StateTwoWire<T,port>::AA;
  I2CCONCLR(port)=StateTwoWire<T,port>::STA | StateTwoWire<T,port>::STO;
}

  //SLA+W sent, NOT ACK received
template<class T, int port>
void state20(StateTwoWire<T,port>& that) {
  //Could send repeated start (100X), stop then start (110X),
  //resend last byte (000X) and I2CDAT-<data
  //but normal action is to stop and give up
  I2CCONSET(port)=StateTwoWire<T,port>::STO;
  that.done=true;
}

  //Data sent, ACK received
template<class T, int port>
void state28(StateTwoWire<T,port>& that) {
  //Count that byte as sent
  that.lengthWrite--;
  that.dataWrite++;
  if(that.lengthWrite==0) {
    //If no more data to be sent, I2CCON<-010X for stop or I2CCON<-100X for repeated start
    I2CCONCLR(port)=StateTwoWire<T,port>::STA;
    I2CCONSET(port)=StateTwoWire<T,port>::STO;
    that.done=true;
  } else {
    //If more data to be sent, load next byte I2CDAT<-data
    //and just send it I2CCON<-(000X)
    I2CDAT(port)=*that.dataWrite;
    I2CCONSET(port)=StateTwoWire<T,port>::AA;
    I2CCONCLR(port)=StateTwoWire<T,port>::STA | StateTwoWire<T,port>::STO;
  }
}

template<class T, int port>
void state30(StateTwoWire<T,port>& that) {
  I2CCONSET(port)=StateTwoWire<T,port>::STO;
}

template<class T, int port>
void state38(StateTwoWire<T,port>& that) {
  I2CCONSET(port)=StateTwoWire<T,port>::STA;
}

template<class T, int port>
void state40(StateTwoWire<T,port>& that) {
  if(that.lengthRead==1) {
    //Only one byte to be read, so NAK it immediately
    I2CCONCLR(port)=StateTwoWire<T,port>::AA;
  } else {
    I2CCONSET(port)=StateTwoWire<T,port>::AA;
  }
}

template<class T, int port>
void state48(StateTwoWire<T,port>& that) {
  that.done=true;
  I2CCONSET(port)=StateTwoWire<T,port>::AA|StateTwoWire<T,port>::STO;
}

template<class T, int port>
void state50(StateTwoWire<T,port>& that) {
  *that.dataRead=I2CDAT(port);
  that.dataRead++;
  that.lengthRead--;
  if(that.lengthRead==1) {
    //Only one more byte to read, so NAK it
    I2CCONCLR(port)=StateTwoWire<T,port>::AA;
  } else {
    I2CCONSET(port)=StateTwoWire<T,port>::AA;
  }
}

template<class T, int port>
void state58(StateTwoWire<T,port>& that) {
  *that.dataRead=I2CDAT(port);
  that.lengthRead--;
  that.done=true;
  I2CCONSET(port)=StateTwoWire<T,port>::AA|StateTwoWire<T,port>::STO;
}

template<class T, int port>
void stateF8(StateTwoWire<T,port>& that) {}

template<class T, int port>
void StateTwoWire<T,port>::stateDriver() {
  if(I2CCONSET(port) & SI) {
    int s=I2CSTAT(port);
    state[s>>3](*this);
    I2CCONCLR(port)=SI;
  }
}

//Initialize I2C peripheral
template<class T, int port>
void StateTwoWire<T,port>::twi_init(unsigned int freq) {
  //Turn on appropriate I2C peripheral
  PCONP() |= (1<<((0==port)?7:19));
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
}

template<class T, int port>
uint8_t StateTwoWire<T,port>::twi_readFrom(uint8_t Laddress, char* Ldata, uint8_t Llength) {
  //Go to master transmit mode and send the start condition
  if(Llength==0) return 0;
  address=(Laddress<<1)|1;
  dataRead=Ldata;
  lengthRead=Llength;
  I2CCONSET(port)=STA; //set I2EN and STA
  done=false;
  while(!done) stateDriver();
  return Llength;
}

template<class T, int port>
uint8_t StateTwoWire<T,port>::twi_writeTo(uint8_t Laddress, const char* Ldata, uint8_t Llength, uint8_t wait) {
  //Go to master transmit mode and send the start condition
  if(Llength==0) return 0;
  address=(Laddress<<1)|0;
  dataWrite=Ldata;
  lengthWrite=Llength;
  I2CCONSET(port)=STA; //set I2EN and STA
  done=false;
  while(!done) stateDriver();
  return 0;
}



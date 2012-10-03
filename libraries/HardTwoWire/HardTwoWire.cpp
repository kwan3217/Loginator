#include "HardTwoWire.h"
#include "LPC214x.h"
#include "gpio.h"
#include "Time.h"

HardTwoWire Wire(0);
HardTwoWire Wire1(1);

//HardTwoWire: Use LPC214x hardware I2C port to implement TwoWire object
HardTwoWire::HardTwoWire(int Lport):port(Lport) {}

#define I2CFREQ 400000

#define I2CCON_AA   (1 << 2)
#define I2CCON_SI   (1 << 3)
#define I2CCON_STO  (1 << 4)
#define I2CCON_STA  (1 << 5)
#define I2CCON_I2EN (1 << 6)

//Initialize I2C peripheral 
void HardTwoWire::twi_init() {
  //Turn on appropriate I2C peripheral
  PCONP |= (1<<(7+port));
  //Set the clock rate
  int rate=(PCLK/I2CFREQ)/2;
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

static inline void wait_si(int port) {
  while(!(I2CCONSET(port) & I2CCON_SI)) ;
}

uint8_t HardTwoWire::twi_readFrom(uint8_t address, char* data, uint8_t length) {
  //Go to master transmit mode and send the start condition
  I2CCONCLR(port)=I2CCON_STA | I2CCON_STO | I2CCON_SI | I2CCON_AA; //Clear STA, STO, SI, AA
  I2CCONSET(port)=I2CCON_I2EN | I2CCON_STA; //set I2EN and STA
  wait_si(port); //wait for start condition to be sent

  I2CDAT(port)=(address << 1)+1;  //Send slave address and read (low bit set)
  I2CCONCLR(port)=I2CCON_SI;  //Clear SI, allow protocol engine to continue
  wait_si(port); //wait for address to be sent

  //Ask for each byte except the last, send ACK each time
  for(int i=0;i<length-1;i++) {
    I2CCONSET(port)= I2CCON_AA;
    I2CCONCLR(port)=I2CCON_STA | I2CCON_STO | I2CCON_SI;
    wait_si(port); //Wait for data to arrive
    data[i]=I2CDAT(port);
  }

  //Get the final byte and send a NAK
  I2CCONCLR(port)=I2CCON_STA | I2CCON_STO | I2CCON_SI | I2CCON_AA; 

  wait_si(port); //wait for data arrive

  data[length-1]=I2CDAT(port);

  twi_stop();

  return length;
}

uint8_t HardTwoWire::twi_writeTo(uint8_t address, const char* data, uint8_t length, uint8_t wait) {
  //Go to master transmit mode and send the start condition
  I2CCONCLR(port)=I2CCON_STA | I2CCON_STO | I2CCON_SI | I2CCON_AA; //Clear STA, STO, SI, AA
  I2CCONSET(port)=I2CCON_I2EN | I2CCON_STA; //set I2EN and STA

  wait_si(port); //wait for start condition to be sent
  I2CDAT(port)=address << 1;  //Send slave address and write (low bit cleared)
  I2CCONCLR(port)=I2CCON_SI;  //Clear SI, allow protocol engine to continue
  wait_si(port); //wait for address to be sent

  for(int i=0;i<length;i++) {
    I2CDAT(port)=data[i];
    //Clear SI, allow protocol engine to continue
    I2CCONCLR(port)=I2CCON_STA | I2CCON_STO | I2CCON_SI;

    wait_si(port); //wait for data to be sent
  }

  twi_stop();

  return 0;
}

void HardTwoWire::twi_stop(void) {
  //Send the stop condition
  I2CCONSET(port)=I2CCON_STO;
  I2CCONCLR(port)=I2CCON_STA | I2CCON_SI;

}

void HardTwoWire::twi_releaseBus() {
  //LPC2148 automatically releases bus after sending STOP
}

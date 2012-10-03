//I2C bitbang interface
#include <stdlib.h>
#include "LPC214x.h"
#include "i2c.h"
#include "setup.h"
#include "pktwrite.h"

#define CLOCK_STRETCH

//Was a start condition sent most recently? Used to do double-start
unsigned char i2c_start = 0;

//==== Lowest level functions to manage the pins -- All LPC-specific code is here 

//Number of times around the delay loop to go
int I2CSPEED;
#define I2CDELAY(clock_period) {for(unsigned int i=clock_period;i>0;i--) asm volatile ("nop");}

//SDA pin constants
unsigned int sda; //P0.XX number of SDA pin

//SCL pin constants
unsigned int scl; //P0.XX number of SCL pin

//Set SDA line to read and return the value on it. 
//Since the line has a pull-up on it, this pulls the 
//SDA line high if no one else is driving it
unsigned char READSDA(void) {
  IODIR0&=~(1<<sda);
  return (IOPIN0>>sda) & 1;
}

//Set SDA line to write and pull it low
void CLRSDA(void) {
  IODIR0|=(1<<sda);
  IOCLR0=(1<<sda);
}

//Set SCL line to read and return the value on it. 
//Since the line has a pull-up on it, this pulls the 
//SCL line high if no one else is driving it
unsigned char READSCL(void) {
  IODIR0&=~(1<<scl);
  return (IOPIN0>>scl) & 1;
}

//Set SCL line to write and pull it low
void CLRSCL(void) {
  IODIR0|=(1<<scl);
  IOCLR0=(1<<scl);
}

#ifdef ARBITRATION
static void ARBITRATION_LOST(void) {
  //Can't happen, only slaves on the bus
}
#endif

//==== Set up and tear down - Public interface, but LPC-specific code

int i2c_setup(int PSCL, int PSDA, int freq) {
  scl=PSCL;
  sda=PSDA;
  //clear the pin select bits for the chosen pins, changing them to GPIO
  if(PSCL<16) {
    unsigned int pinmask=3<<(PSCL*2);
    PINSEL0&=(~pinmask);
  } else {
    unsigned int pinmask=3<<((PSCL-16)*2);
    PINSEL1&=(~pinmask);
  }
  if(PSDA<16) {
    unsigned int pinmask=3<<(PSDA*2);
    PINSEL0&=(~pinmask);
  } else {
    unsigned int pinmask=3<<((PSDA-16)*2);
    PINSEL1&=(~pinmask);
  }
  //Say 10 cycles per empty for loop, so it runs at 6MHz
  //To get the wait to run at 400kHz, loop 6M/400k=15 times
  I2CSPEED=(CCLK/6)/freq;
  return 0;
}

int i2c_stop() {
  READSDA();
  READSCL();
  sda=-1;
  scl=-1;
  I2CSPEED=-1;
  return 0;
}

//==== Bit-level functions
//Translated from Wikipedia pseudocode - http://en.wikipedia.org/wiki/I2C

unsigned char read_bit(void) {
	unsigned char bit;
 
	// Let the slave drive data
	READSDA();
	I2CDELAY(I2CSPEED/2);
	/* Clock stretching */
  #ifdef CLOCK_STRETCH
	while (READSCL() == 0);
  #else
  READSCL();
  #endif
	/* SCL is high, now data is valid */
	bit = READSDA();
	I2CDELAY(I2CSPEED/2);
	CLRSCL();
	return bit;
}
 
void write_bit(unsigned char bit) {
  //Put the bit on SDA by either letting it rise or pulling it down
	if (bit) {
		READSDA();
	} else {
		CLRSDA();
	}
	I2CDELAY(I2CSPEED/2);
	/* Clock stretching - Let SCL rise and wait for slave to let it rise */
  #ifdef CLOCK_STRETCH
	while (READSCL() == 0);
  #else
  READSCL();
  #endif
	/* SCL is high, now data is being read */
	/* If SDA is high, check that nobody else is driving SDA */
	if (bit) {
    #ifdef ARBITRATION
    if (READSDA() == 0) { //Oops, someone else pulled SDA down
		  ARBITRATION_LOST();
		}
    #else
    READSDA();
    #endif
	}
	I2CDELAY(I2CSPEED/2);
	CLRSCL();
}
 
void start_cond(void) {
	if (i2c_start) {
		/* Let SDA rise */
		READSDA();
		I2CDELAY(I2CSPEED/2);
		/* Clock stretching */
    #ifdef CLOCK_STRETCH
		while (READSCL() == 0);
    #else
    READSCL();
    #endif
	}
  #ifdef ARBITRATION
	if (READSDA() == 0) {
		ARBITRATION_LOST();
	}
  #else
  READSDA();
  #endif
	/* SCL is high, we waited for slave to raise it, so pull SDA down */
	CLRSDA();
	I2CDELAY(I2CSPEED/2);
  // Now pull SCL down
	CLRSCL();
	i2c_start = 1;
}
 
void stop_cond(void) {
	/* Pull SDA down */
	CLRSDA();
	I2CDELAY(I2CSPEED/2);
	/* Clock stretching - wait for slave to raise it*/
  #ifdef CLOCK_STRETCH
	while (READSCL() == 0);
  #else
  READSCL();
  #endif
	/* SCL is high, set SDA from 0 to 1 */
  #ifdef ARBITRATION
	if (READSDA() == 0) {
		ARBITRATION_LOST();
	}
  #else
  READSDA();
  #endif
	I2CDELAY(I2CSPEED/2);
	i2c_start = 0;
}

//==== Byte-level protocol
//Translated from Wikipedia pseudocode - http://en.wikipedia.org/wiki/I2C

unsigned char tx(int send_start, int send_stop, unsigned char byte) {
	unsigned char bit;
	unsigned char nack;
	if (send_start) {
		start_cond();
	}
	for (bit = 0; bit < 8; bit++) {
		write_bit(byte & 0x80);
		byte <<= 1;
	}
	nack = read_bit();
	if (send_stop) {
		stop_cond();
	}
	return nack;
}
 
unsigned char rx (int nak, int send_stop) {
	unsigned char byte = 0;
	unsigned char bit;

	for(bit = 0; bit < 8; bit++) {
    byte <<= 1;		
    byte |= read_bit();		
	}
	write_bit(nak);
	if (send_stop) {
		stop_cond();
	}
	return byte;
}

//==== Multi-byte protocol - Public interface to I2C bus

int i2c_tx_string(unsigned char addr, const char* buf, int len) {
  if(tx(1,0,addr << 1 | 0)) return 1;
  for(int i=0;i<len-1;i++) if(tx(0,0,buf[i])) return 1;
  tx(0,1,buf[len-1]);
  return 0;
}

int i2c_rx_string(unsigned char addr, char* buf, int len) {
  tx(1,0,addr << 1 | 1);
  for(int i=0;i<len-1;i++) buf[i]=rx(0,0);
  buf[len-1]=rx(1,1);
  return 0;
}

int i2c_txrx_string(unsigned char addr, const char* txbuf, int txlen, char* rxbuf, int rxlen) {
  //Send the slave address with the start condition and write bit
  if(tx(1,0,addr << 1 | 0))  return 1;
  //Send the message, no stop condition on the last byte
  for(int i=0;i<txlen;i++) if(tx(0,0,txbuf[i]))  return 1;
  //Send the slave address with the start condition (again) and read bit
  if(tx(1,0,addr << 1 | 1)) return 1;
  //Read the slave n-1 times, sending ack each time
  for(int i=0;i<rxlen-1;i++) rxbuf[i]=rx(0,0);
  //Read the slave one more time, with a nak and stop condition
  rxbuf[rxlen-1]=rx(1,1);
  return 0;
}

void writeI2Csetup(circular& buf, int PCL, int PDA, int freq) {
  i2c_setup(PCL,PDA,freq);

  fillPktStart(buf,PT_I2CSETUP);
  fillPktByte(buf,PCL);
  fillPktByte(buf,PDA);
  fillPktInt(buf,freq);
  fillPktFinish(buf);
}

#ifndef i2c_int_h
#define i2c_int_h

#include "circular.h"
#include "setup.h"

//A bit-banging implementation of the master part of the I2C two-wire protocol using any
//two general purpose I/O pins in P0.XX. Intended for the Sparkfun Logomatic since they 
//wired the actual I2C port to the lights and buttons :(

//This implementation uses (will use) timer 0 interrupts to do timing. When
//an I2C transaction is in progress, no other process can use timer 0 interrupts.

class i2c_int {
private:
  const unsigned int scl; //P0.XX number of SCL pin
  const unsigned int sclmask;
  const unsigned int sda; //P0.XX number of SDA pin
  const unsigned int sdamask;
  const unsigned int I2CSPEED;
//Was a start condition sent most recently? Used to do double-start
  unsigned char i2c_start;
public:

  //Setup a bit-bang I2C port using two digital I/O pins
  //input
  //  PCL - Pin P0.XX number to use as SCL clock line
  //  PDA - Pin P0.XX number to use as SDA data line
  //  freq - Approximate bus frequency in Hz
  //return
  //  0 is success
  //  some other number for failure
  i2c_int(unsigned int LPCL, unsigned int LPDA, unsigned int Lfreq):
    scl(LPCL),sclmask(1<<LPCL),
    sda(LPDA),sdamask(LPDA),
    I2CSPEED((designCCLK/6)/Lfreq),i2c_start(0) {}

  void writeSetup(circular& buf);

  void start(void);

  //Release the bit-bang I2C port, setting the used pins to hi-Z
  //return
  //  0 is success
  //  some other number for failure
  void stop(void);

  //Transmit a string on the I2C port
  //input
  //  addr - slave address to send to
  //  buf - pointer to byte string to send
  //  len - number of bytes to send
  //return
  //  0 is success
  //  some other number for failure
  int tx_string(unsigned char addr, const char* buf, int len);

  //Receive a string on the I2C port
  //input
  //  addr - slave address to receive from
  //  buf - pointer to byte buffer to receive
  //  len - number of bytes to receive
  //return
  //  0 is success
  //  some other number for failure
  int rx_string(unsigned char addr, char* buf, int len);

  //Send then receive a string on the I2C port, with only a start condition in between
  //input
  //  addr - slave address to receive from
  //  txbuf - pointer to byte buffer to receive
  //  txlen - number of bytes to receive
  //  rxbuf - pointer to byte buffer to receive
  //  rxlen - number of bytes to receive
  //return
  //  0 is success
  //  some other number for failure
  int txrx_string(unsigned char addr, const char* txbuf, int txlen, char* rxbuf, int rxlen);
  void tx1rx_string(unsigned char addr, char tx1, int txlen, char* rxbuf, int rxlen);
private:
  //These were static
  unsigned char READSDA(void);
  void CLRSDA(void);
  unsigned char READSCL(void);
  void CLRSCL(void);
  unsigned char read_bit(void);
  void write_bit(unsigned char bit);
  void start_cond(void);
  void stop_cond(void);
  unsigned char tx(int send_start, int send_stop, unsigned char byte);
  unsigned char rx (int nak, int send_stop);
  //Number of times around the delay loop to go
  void I2CDELAY(unsigned int clock_period) {for(unsigned int i=clock_period;i>0;i--) asm volatile ("nop");}
};

extern i2c_int i2c0;

#endif

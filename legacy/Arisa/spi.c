//SPI interface
#include <stdlib.h>
#include "LPC214x.h"
#include "spi.h"
#include "setup.h"

//Setup the SPI1 port, including claiming the pins
//input
//  freq - Approximate bus frequency in Hz
//return
//  0 is success
//  some other number for failure

int spi1_setup(unsigned int freq, unsigned char CPOL, unsigned char CPHA) {
           //SSP disabled for the moment
  SSPCR1&=~(1 << 1);

  //claim the pins
  set_pin(17,2);
  set_pin(18,2);
  set_pin(19,2);
  set_pin(20,0); //Set this to GPIO, so that we control it
  IODIR0 |= ((1<<20)); //Set pin 20 to output
  IOSET0 = (1<<20); //Raise the pin (active low)

         // 8 bit frame
             //SPI mode
                        //CPOL
                                            //CPHA
                                                                //SCR 1 (write SCR-1 to the register)
  SSPCR0=7 | (0 << 4) | ((CPOL & 1) << 6) | ((CPHA & 1) << 7) | (0 << 8);
         //No loopback
                    //SSP disabled (still)
                               //SSP master
                                          //SSP Slave bit (doesn't matter)
  SSPCR1=(0 << 0) | (0 << 1) | (0 << 2) | (0 << 3);
  //SSP Prescale register (Scale register SCR set to 1 above)
  SSPCPSR=PCLK/freq;
          //SSP enabled
  SSPCR1|= (1 << 1);
  return 0;
}

//Release the SPI1 port, setting the used pins back to GPIO input
//return
//  0 is success
//  some other number for failure
int spi1_stop(void) {
           //SSP disabled
  SSPCR1&=~(1 << 1);
  set_pin(17,0);
  set_pin(18,0);
  set_pin(19,0);
  set_pin(20,0);
  //Set these pins to input
  IODIR0 &= ~((1<<17) | (1<<18) | (1<<19) | (1<<20));
  return 0;
}

//Trade a string with the slave, block until finished
//input
//  txb - pointer to bytes to send
//  tx_count - number of bytes to send
//  rxb - pointer to receive buffer
//  rx_count - number of bytes to receive
//return
//  0 is success
//  some other number for failure
//note - first receive byte will almost always be garbage
int spi1_txrx_string_block(char* txb, int tx_count, char* rxb, int rx_count) {
  return 0;
}

//Send a string to the slave, throw away the returned data, block until finished
//input
//  txb - pointer to bytes to send
//  tx_count - number of bytes to send
//return
//  0 is success
//  some other number for failure
int spi1_tx_string_block(char* txb, int tx_count) {
  IOCLR0 = (1<<20); //Lower the CS pin (active low)
  while(tx_count>0) {
    while(!(SSPSR & (1 << 0))); //Wait for Tx not full
    SSPDR=*txb;
    while(!(SSPSR & (1 << 2))); //Wait for Rx not empty
    volatile char rx=SSPDR;
    txb++;
    tx_count--;
  }
  IOSET0 = (1<<20); //Raise the CS pin (active low)
  return 0;
}

//Send a byte to the slave, receive a string, block until finished
//input
//  tx - byte to send
//  rxb - pointer to bytes to send
//  rx_count - number of bytes to receive, incuding dummy byte at beginning
//return
//  0 is success
//  some other number for failure
//note - first receive byte will almost always be garbage
int spi1_rx_string_block(unsigned char tx, char* rxb, int rx_count) {
  IOCLR0 = (1<<20); //Lower the CS pin (active low)
  while(rx_count>0) {
    while(!(SSPSR & (1 << 0))); //Wait for Tx not full
    SSPDR=tx;
    while(!(SSPSR & (1 << 2))); //Wait for Rx not empty
    *rxb=SSPDR;
    rxb++;
    rx_count--;
  }
  IOSET0 = (1<<20); //Raise the CS pin (active low)
  return 0;
}




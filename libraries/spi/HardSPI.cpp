//SPI interface
#include <stdlib.h>
#include "LPC214x.h"
#include "gpio.h"
#include "Time.h"
#include "HardSPI.h"

HardSPI0 SPI;
HardSPI1 SPI1;

void HardSPI::claim_cs(int p0) {
  set_pin(p0,0); //Set this to GPIO, so that we control it
  IODIR0 |= ((1<<p0)); //Set pin to output
  IOSET0 = (1<<p0); //Raise the pin (active low)
}

void HardSPI::release_cs(int p0) {
  set_pin(p0,0); //Set this to GPIO, so that we control it
  IODIR0&=~((1<<p0)); //Set pin to input
}

void HardSPI::select_cs(int p0) {
  IOCLR0 = (1<<p0); //Lower the CS pin (active low)
}

void HardSPI::deselect_cs(int p0) {
  IOSET0 = (1<<p0); //Raise the CS pin (active low)
}

//Setup the SPI0 port, including claiming the pins
void HardSPI0::begin(unsigned int freq, unsigned char CPOL, unsigned char CPHA) {

  //claim the pins
  set_pin(4,1);
  set_pin(5,1);
  set_pin(6,1);

          //BitEnable CPHA               CPOL                MSTR
  S0SPCR= (0 << 2) | ((CPHA & 1) << 3) | ((CPOL & 1) << 4) | (1 << 5) |
          //LSBF     //SPIE     //BITS
          (0 << 6) | (0 << 7) | (0 << 8);
  setfreq(freq);
}

void HardSPI0::setfreq(unsigned int freq) {
  //SSP Prescale register (Scale register SCR set to 1 above)
  unsigned int scale=PCLK/freq;
  if(scale>254)scale=254;
  scale&=0x7F;
  S0SPCCR=scale;
}

//Release the SPI0 port, setting the used pins back to GPIO input
void HardSPI0::stop(void) {
           //SSP disabled
  SSPCR1&=~(1 << 1);
  set_pin(4,0);
  set_pin(5,0);
  set_pin(6,0);
  //Set these pins to input
  IODIR0 &= ~((1<<4) | (1<<5) | (1<<6));
}

//Send a string to the slave, throw away the returned data, block until finished
//input
//  txb - pointer to bytes to send
//  tx_count - number of bytes to send
void HardSPI0::tx_block(const char* txb, int count) {
  for(;count>0;count--) {
    S0SPDR=*txb;
    while(!(S0SPSR & (1 << 7))); //Wait for transfer to happen
    volatile char rx __attribute__ ((unused))=S0SPDR;
    txb++;
  }
}

void HardSPI0::send_byte(uint8_t tx) {
  S0SPDR=tx;
  while(!(S0SPSR & (1 << 7))); //Wait for transfer to happen
  volatile char rx __attribute__ ((unused))=S0SPDR;
}

//Send a byte to the slave, receive a string, block until finished
//input
//  tx - byte to send
//  rxb - pointer to bytes to send
//  rx_count - number of bytes to receive, incuding dummy byte at beginning
//side effect
//  buffer pointed to by rxb will hold received data
void HardSPI0::rx_block(uint8_t tx, char* rxb, int count) {
  for(;count>0;count--) {
    S0SPDR=tx;
    while(!(S0SPSR & (1 << 7))); //Wait for transfer to happen
    *rxb=S0SPDR;
    rxb++;
  }
}

uint8_t HardSPI0::rec_byte() {
  S0SPDR=0xff;
  while(!(S0SPSR & (1 << 7))); //Wait for transfer to happen
  return S0SPDR;
}

//Setup the SPI1 port, including claiming the pins
void HardSPI1::begin(unsigned int freq, unsigned char CPOL, unsigned char CPHA) {
           //SSP disabled for the moment
  SSPCR1&=~(1 << 1);

  //claim the pins
  set_pin(17,2);
  set_pin(18,2);
  set_pin(19,2);

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
  setfreq(freq);
          //SSP enabled
  SSPCR1|= (1 << 1);
}

void HardSPI1::setfreq(unsigned int freq) {
  //SSP Prescale register (Scale register SCR set to 1 above)
  SSPCPSR=PCLK/freq;
}

//Release the SPI1 port, setting the used pins back to GPIO input
void HardSPI1::stop(void) {
           //SSP disabled
  SSPCR1&=~(1 << 1);
  set_pin(17,0);
  set_pin(18,0);
  set_pin(19,0);
  //Set these pins to input
  IODIR0 &= ~((1<<17) | (1<<18) | (1<<19));
}

//Send a string to the slave, throw away the returned data, block until finished
//input
//  txb - pointer to bytes to send
//  tx_count - number of bytes to send
void HardSPI1::tx_block(const char* txb, int count) {
  for(;count>0;count--) {
    while(!(SSPSR & (1 << 0))); //Wait for Tx not full
    SSPDR=*txb;
    while(!(SSPSR & (1 << 2))); //Wait for Rx not empty
    volatile char rx __attribute__ ((unused))=SSPDR;
    txb++;
  }
}

void HardSPI1::send_byte(uint8_t tx) {
  while(!(SSPSR & (1 << 0))); //Wait for Tx not full
  SSPDR=tx;
  while(!(SSPSR & (1 << 2))); //Wait for Rx not empty
  volatile char rx __attribute__ ((unused))=SSPDR;
}

//Send a byte to the slave, receive a string, block until finished
//input
//  tx - byte to send
//  rxb - pointer to bytes to send
//  rx_count - number of bytes to receive, incuding dummy byte at beginning
//side effect
//  buffer pointed to by rxb will hold received data
void HardSPI1::rx_block(uint8_t tx, char* rxb, int count) {
  for(;count>0;count--) {
    while(!(SSPSR & (1 << 0))); //Wait for Tx not full
    SSPDR=tx;
    while(!(SSPSR & (1 << 2))); //Wait for Rx not empty
    *rxb=SSPDR;
    rxb++;
  }
}

uint8_t HardSPI1::rec_byte() {
  while(!(SSPSR & (1 << 0))); //Wait for Tx not full
  SSPDR=0xFF;
  while(!(SSPSR & (1 << 2))); //Wait for Rx not empty
  return SSPDR;
}


/*
  HardwareSerial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "Serial.h"
#include "Time.h"
#include "gpio.h"
#ifndef NOINT
#include "irq.h"

//Actual hardware interaction

static void UARTISR(int n, HardwareSerial& port) {
  int iir=(UIIR(n)>>1) & 0x07;
  if(0x01==iir) {//If it's a THRE int...
    int i=0;
    while(i<16 && port.txBuf.readylen()>0) {
      UTHR(0)=port.txBuf.get();
      i++;
    }
  } else { //presume it's a read int
    port.rxBuf.fill(URBR(n));
    port.rxBuf.mark();
  }  
  //Acknowledge the VIC
  VICVectAddr = 0;
}

static void UARTISR0() {UARTISR(0,Serial);}
static void UARTISR1() {UARTISR(1,Serial1);}

#endif

// Constructors ////////////////////////////////////////////////////////////////

HardwareSerial::HardwareSerial(int Lport):
#ifndef NOINT
txBuf(bufSize,txBuf_loc),rxBuf(bufSize,rxBuf_loc),
#endif
port(Lport) {

}

// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(unsigned int baud) {
  measurePCLK();

  //Set up the pins
  if(port==0) {
    gpio_set_write(0);
    gpio_set_read(1);
    set_pin(0,1); //TX0
    set_pin(1,1); //RX0
  } else {
    gpio_set_write(8);
    gpio_set_read(9);
    set_pin(8,1); //TX1
    set_pin(9,1); //RX1
  }
  ULCR(port) = (3 << 0) | //8 data bits
               (0 << 2) | //1 stop bit
               (0 << 3) | //No parity
               (0 << 4) | //I said, no parity!
               (0 << 6) | //No break transmission
               (1 << 7);  //DLAB = 1
  //DLAB - Divisor Latch Access bit. When set, a certain memory address
  //       maps to the divisor latches, which control the baud rate. When
  //       cleared, those same addresses correspond to the processor end
  //       of the FIFOs. In other words, set the DLAB to change the baud
  //       rate, and clear it to use the FIFOs.

  unsigned int Denom=PCLK/baud;
  unsigned int UDL=Denom/16;

  UDLM(port)=(UDL >> 8) & 0xFF;
  UDLL(port)=(UDL >> 0) & 0xFF;

  UFCR(port) = (1 << 0) |  //FIFOs on
               (1 << 1) |  //Clear rx FIFO
               (1 << 2) |  //Clear tx FIFO
               (3 << 6);   //Rx watermark=14 bytes
  ULCR(port) = ULCR(port) & ~(1<<7); //Turn of DLAB - FIFOs accessable
#ifdef NOINT
  UIER(port)=0;
#else
  if(port==0) {
    IRQHandler::install(IRQHandler::UART0,UARTISR0);
  } else {
    IRQHandler::install(IRQHandler::UART1,UARTISR1);
  }
  UIER(port) = (1 << 0) | //Int on Rx ready
#ifdef NOBLOCK_TX    
               (1 << 1) | //Int on Tx empty
#endif    
               (0 << 2) | //No int on line status
               (0 << 8) | //No int on autobaud timeout
               (0 << 9);  //No int on end of autobaud
#endif
}

void HardwareSerial::end() {
  //set the pins to read (high Z)
  if(port==0) {
    set_pin(0,0); //TX0->GPIO
    set_pin(1,0); //RX0->GPIO
    gpio_set_read(0);
    gpio_set_read(1);
  } else {
    set_pin(8,0); //TX1->GPIO
    set_pin(9,0); //RX1->GPIO
    gpio_set_read(8);
    gpio_set_read(9);
  }
}

void HardwareSerial::write(uint8_t c) {
#ifdef NOINT
  while (!(ULSR(port) & (1<<5))); //do nothing, wait for Tx FIFO to become empty;
  UTHR(port)=c;
#else
  if(!txBuf.fill(c)) blinklock(1);
  txBuf.mark();
#ifdef NOBLOCK_TX  
  if(ULSR(port) & (1<<5)) UTHR(port)=txBuf.get(); //If transmit fifo is empty, kick off transfer by writing first byte
#else
  while(!txBuf.isEmpty()) {
    while (!(ULSR(port) & (1<<5))); //do nothing, wait for Tx FIFO to become empty;
    UTHR(port)=txBuf.get();
  }
#endif
#endif
}

#ifdef NOINT
int HardwareSerial::available(void) {return ULSR(port) & 0x01;}; //Returns 0 or 1 if none or some bytes $
int HardwareSerial::peek(void) {return -1;};                      //Not supported
int HardwareSerial::read(void) {return URBR(port);};            
void HardwareSerial::flush(void) {UFCR(port)=((1 << 0) | (1 << 1) | (1 << 2));}; //Enable FIFOs and rese$
#else
int HardwareSerial::available(void) {return rxBuf.readylen();};
int HardwareSerial::peek(void) {return rxBuf.peekTail();};
int HardwareSerial::read(void) {return rxBuf.get();};
void HardwareSerial::flush(void) {rxBuf.empty();};
#endif
// Preinstantiate Objects //////////////////////////////////////////////////////

HardwareSerial  Serial(0);
HardwareSerial  Serial1(1);
HardwareSerial* SerialA[] {&Serial,&Serial1};


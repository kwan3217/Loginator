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
#include "LPC214x.h"

#include "Serial.h"
#include "Time.h"
#include "gpio.h"

//Actual hardware interaction

static inline int putc_serial(int port,int ch) {
  while (!(ULSR(port) & 0x20));
  return (UTHR(port) = ch);
}

// Constructors ////////////////////////////////////////////////////////////////

HardwareSerial::HardwareSerial(int Lport):port(Lport) {

}

// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(unsigned int baud) {
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
  ULCR(port) = 0x83;   // 8 bits, no parity, 1 stop bit, DLAB = 1
  //DLAB - Divisor Latch Access bit. When set, a certain memory address
  //       maps to the divisor latches, which control the baud rate. When
  //       cleared, those same addresses correspond to the processor end
  //       of the FIFOs. In other words, set the DLAB to change the baud
  //       rate, and clear it to use the FIFOs.

  unsigned int Denom=PCLK/baud;
  unsigned int UDL=Denom/16;

  UDLM(port)=(UDL >> 8) & 0xFF;
  UDLL(port)=(UDL >> 0) & 0xFF;

  UFCR(port) = 0xC7; //Enable and clear both FIFOs, Rx trigger level=14 chars
  ULCR(port) = 0x03; //Turn of DLAB - FIFOs accessable
  UIER(port)=0;

/*
   enableIRQ();
  if(port==0) {
    install_irq(UART0_INT,uart0_isr);
    U0IER=0x03; //Rx ready, Tx empty, not line status, not autobaud ints
  } else {
    install_irq(UART1_INT,uart1_isr);
    U1IER=0x03; //Rx ready, Tx empty, not line status, not autobaud ints
  }
 */
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

int HardwareSerial::available(void) {

}

int HardwareSerial::peek(void) {

}

int HardwareSerial::read(void) {

}

void HardwareSerial::flush() {

}

void HardwareSerial::write(uint8_t c) {
  putc_serial(port,c);
}

// Preinstantiate Objects //////////////////////////////////////////////////////

HardwareSerial Serial(0);
HardwareSerial Serial1(1);


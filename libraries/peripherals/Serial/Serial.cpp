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

// Constructors ////////////////////////////////////////////////////////////////

HardwareSerial::HardwareSerial(int Lport):
port(Lport) {

}

// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(unsigned int baud) {
// for now, do nothing. We will use whatever the ISP had set up.
  measurePCLK();

#if MCU == MCU_ARM7TDMI
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
#else
  //This only knows how to set up UART0.
  IODIR(0)|= (1<<2); //TX set to output
  IODIR(0)&=~(1<<3); //RX set to input
  IOCON(0,2)=(0b010 << 0) |  //Function TX0
             (0b00  << 3) |  //No pullup/pulldown
             (0b0   << 5) |  //No hysteresis
             (0b0   << 6) |  //No inversion
             (0b0   << 9) |  //Standard slew
             (0b0   <<10) ;  //Not open-drain
  IOCON(0,3)=(0b010 << 0) |  //Function RX0
             (0b00  << 3) |  //No pullup/pulldown
             (0b0   << 5) |  //No hysteresis
             (0b0   << 6) |  //No inversion
             (0b0   << 9) |  //Standard slew
             (0b0   <<10) ;  //Not open-drain
#endif
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
  UFDR(port)=0x10; //reset nilpotent value

  UFCR(port) = (1 << 0) |  //FIFOs on
               (1 << 1) |  //Clear rx FIFO
               (1 << 2) |  //Clear tx FIFO
               (3 << 6);   //Rx watermark=14 bytes
  ULCR(port) = ULCR(port) & ~(1<<7); //Turn of DLAB - FIFOs accessable
  UIER(port)=0;

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

int HardwareSerial::available(void) {return ULSR(port) & 0x01;}; //Returns 0 or 1 if none or some bytes can be read immediately
int HardwareSerial::peek(void) {return -1;};                      //Not supported
int HardwareSerial::read(void) {return URBR(port);};            
void HardwareSerial::flush(void) {UFCR(port)=((1 << 0) | (1 << 1) | (1 << 2));}; //Enable FIFOs and reset
// Preinstantiate Objects //////////////////////////////////////////////////////

HardwareSerial  Serial(0);
HardwareSerial  Serial1(1);
HardwareSerial* SerialA[] {&Serial,&Serial1};


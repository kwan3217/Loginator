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
#include "robot.h"
#include "Time.h"

// Constructors ////////////////////////////////////////////////////////////////

HardwareSerial::HardwareSerial(int Lport):
#ifndef NOINT
txBuf(bufSize,txBuf_loc),rxBuf(bufSize,rxBuf_loc),
#endif
port(Lport) {

}

// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(unsigned int baud) {

}

void HardwareSerial::end() {

}

void HardwareSerial::write(uint8_t c) {
  ::printf("%c",c);
}

int HardwareSerial::available(void) {return state->hasGPS();}; //Returns 0 or 1 if none or some bytes are present
int HardwareSerial::peek(void) {return state->gpsBuf[state->gpsTransPointer];};                      //Not supported
int HardwareSerial::read(void) {
  char result=state->gpsBuf[state->gpsTransPointer];
  state->gpsTransPointer++;
  return result;
};
void HardwareSerial::flush(void) {}; //Enable FIFOs and reset both

// Preinstantiate Objects //////////////////////////////////////////////////////

HardwareSerial Serial(0);
HardwareSerial Serial1(1);


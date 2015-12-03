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


HardwareSerial<0> Serial;
HardwareSerial<1> Serial1;


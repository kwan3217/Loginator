/*SimUART.cpp - simulated Serial port. This code just prints transmitted 
                characters to stdout, and always reports no received characters
                available.

  Originally derived from:
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
  Repurposed 2013 by Chris Jeppesen
*/

#include <stdio.h>
#include <inttypes.h>
#include "sim.h"

const uint32_t PCLK=60'000'000;

void SimUart::write_UTHR(int Lport, uint32_t write) {
  ::printf("%c",(uint8_t)write); ///< Implement simulated serial write
}

int SimUart::baud(int Lport) {
  if(UDLM[Lport]==0 && UDLL[Lport]==0) return -1;
  return PCLK/(16*(256*UDLM[Lport]+UDLL[Lport]));
}

void SimUart::write_UDLL(int Lport, uint32_t write) {
  if(DLAB[Lport]) {
    UDLL[Lport]=write & 0xFF;
    ::fprintf(stderr,"Changed UDLL(%d) to %d, current baud rate is %d\n",Lport,write,baud(Lport));
  } else {
    ::fprintf(stderr,"Attempt to write to DLL failed, DLAB=%d\n",DLAB[Lport]);
  };
}

void SimUart::write_UDLM(int Lport, uint32_t write) {
  if(DLAB[Lport]) {
    UDLM[Lport]=write & 0xFF;
    ::fprintf(stderr,"Changed UDLM(%d) to %d, current baud rate is %d\n",Lport,write,baud(Lport));
  } else {
    ::fprintf(stderr,"Attempt to write to DLM failed, DLAB=%d\n",DLAB[Lport]);
  };
}

uint32_t SimUart::read_ULCR(int Lport) {
  return ((dataBits  [Lport]-5)   & ((1<<2)-1)) <<0 |
         ((stopBits  [Lport]-1)   & ((1<<1)-1)) <<2 |
         ((parityOn  [Lport]?1:0) & ((1<<2)-1)) <<3 |
         ((parityMode[Lport])     & ((1<<2)-1)) <<4 |
         ((DLAB      [Lport]?1:0) & ((1<<1)-1)) <<7;
}

void SimUart::write_ULCR(int Lport, uint32_t write) {
  dataBits  [Lport]=    ((write>>0) & ((1<<2)-1))+5 ;
  stopBits  [Lport]=    ((write>>2) & ((1<<1)-1))+1 ;
  parityOn  [Lport]=(1==((write>>3) & ((1<<1)-1))  );
  parityMode[Lport]=    ((write>>4) & ((1<<2)-1))   ;
  DLAB      [Lport]=(1==((write>>7) & ((1<<1)-1))  );
  const char modeLetter[]="OEMS";
  ::fprintf(stderr,"ULCR(%d) written as %d %02x, mode %d%c%d, DLAB=%d\n",Lport,write,write,
   dataBits[Lport],
   parityOn[Lport]?modeLetter[parityMode[Lport]]:'N',
   stopBits[Lport],
   DLAB[Lport]
  );
}



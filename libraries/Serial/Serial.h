/*
  HardwareSerial.h - Hardware serial library for Wiring
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

  Modified 28 September 2010 by Mark Sproul
*/

#ifndef Serial_h
#define Serial_h

#define NOINT

#include <inttypes.h>
#include "Stream.h"
#ifndef NOINT
#include "Circular.h"
#endif
#include "LPC214x.h"

class HardwareSerial: public Stream {
  private:
#ifndef NOINT
    static const int bufSize=512;
    char txBuf_loc[bufSize], rxBuf_loc[bufSize];
#endif
  public:
#ifndef NOINT
    Circular txBuf,rxBuf;
#endif
    unsigned int port;
    HardwareSerial(int port);
    void begin(unsigned int baud);
    void end();
#ifdef NOINT
    virtual int available(void) {return ULSR(port) & 0x01;}; //Returns 0 or 1 if none or some bytes are present
    virtual int peek(void) {return -1;};                      //Not supported
    virtual int read(void) {return URBR(port);};            
    virtual void flush(void) {UFCR(port)=((1 << 0) | (1 << 1) | (1 << 2));}; //Enable FIFOs and reset both
#else
    virtual int available(void) {return rxBuf.readylen();};
    virtual int peek(void) {return rxBuf.peekTail();};
    virtual int read(void) {return rxBuf.get();};
    virtual void flush(void) {rxBuf.empty();};
#endif
    virtual void write(uint8_t);
    using Print::write; // pull in write(str) and write(buf, size) from Print
};

extern HardwareSerial Serial;
extern HardwareSerial Serial1;

#endif

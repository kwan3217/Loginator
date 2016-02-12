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

#include <inttypes.h>
#include "Stream.h"
#include "registers.h"

class HardwareSerial: public Stream {
  private:
  public:
    unsigned int port;
    HardwareSerial(int port);
    void begin(unsigned int baud);
    void end();
    virtual int available(void); //Returns 0 or 1 if none or some bytes are present
    virtual int peek(void);                      //Not supported
    virtual int read(void);
    virtual void flush(void); //Enable FIFOs and reset both
    virtual void write(uint8_t);
    using Print::write; // pull in write(str) and write(buf, size) from Print
};

inline void HardwareSerial::write(uint8_t c) {
  while (!(ULSR(port) & (1<<5))); //do nothing, wait for Tx FIFO to become empty;
  UTHR(port)=c;
}


//Ambient serial ports
extern HardwareSerial  Serial;
extern HardwareSerial  Serial1;
extern HardwareSerial* SerialA[];

#endif

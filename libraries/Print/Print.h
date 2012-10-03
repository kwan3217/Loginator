/*
  Print.h - Base class that provides print() and println()
  Copyright (c) 2008 David A. Mellis.  All right reserved.

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
*/

#ifndef Print_h
#define Print_h

#include <inttypes.h>
#include <stdio.h> // for size_t
#include "float.h"
//#include "WString.h"

//#define U64

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2
#define BYTE 0

class Print {
  private:
    void printNumber(unsigned int, uint8_t);
#ifdef U64
    void printNumber(unsigned long long, uint8_t);
#endif
    void printFloat(fp, uint8_t);
  public:
    virtual void write(uint8_t) = 0;
    virtual void write(const char *str);
    virtual void write(const uint8_t *buffer, size_t size);
    
    void print(const char[]);
    void print(char, int = BYTE);
    void print(unsigned char, int = BYTE);
    void print(short,  int = DEC);
    void print(unsigned short, int = DEC);
    void print(int,  int = DEC);
    void print(unsigned int, int = DEC);
#ifdef U64
    void print(long long,  int = DEC);
    void print(unsigned long long, int = DEC);
#endif
    void print(fp, int = 2);

    void println(const char[]);
    void println(char, int = BYTE);
    void println(unsigned char, int = BYTE);
    void println(short,  int = DEC);
    void println(unsigned short, int = DEC);
    void println(int,  int = DEC);
    void println(unsigned int, int = DEC);
#ifdef U64
    void println(long long,  int = DEC);
    void println(unsigned long long, int = DEC);
#endif
    void println(fp, int = 2);
    void println(void);
    void printf(char const *format, ... );
};

#endif

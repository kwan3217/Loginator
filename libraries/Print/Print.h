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
#include <stddef.h> // for size_t
#include "float.h"
//#include "WString.h"
#include <streambuf>
#include <ostream>

//#define U64

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2
#define BYTE 0

class Print: public std::streambuf, public std::ostream {
  private:
    void printNumber(unsigned int, int,int);
#ifdef U64
    void printNumber(unsigned long long, int,int);
#endif
    void printFloat(fp, unsigned char);
  public:
    Print() {rdbuf(this);};
    virtual void write(unsigned char) = 0;
    virtual void write(const char *str);
    virtual void write(const char *buffer, size_t size);
    
    void print(const char *str){write(str);};
    void print(char c, int base=BYTE, int digits=0){print((int) c, base,digits);};
    void print(unsigned char b, int base=BYTE,int digits=0){print((unsigned int) b, base,digits);};
    void print(int,  int = DEC, int=0);
    void print(unsigned int, int = DEC,int=0);
#ifdef U64
    void print(long long int,  int = DEC,int=0);
    void print(unsigned long long int, int = DEC,int=0);
#endif
    void print(fp n, int digits=2) {printFloat(n, digits);};

    void println(const char *c){print(c);println();};
    void println(char c, int base=BYTE, int digits=0){print((char)c, base,digits);println();}
    void println(unsigned char b, int base=BYTE, int digits=0){print((unsigned char)b, base,digits);println();}
    void println(int n, int base=DEC, int digits=0){print(n, base,digits);println();}
    void println(unsigned int n, int base=DEC, int digits=0){print(n, base, digits);println();}
    void println(fp n, int digits=2){print(n, digits);println();}
#ifdef U64
    void Print::println(long long int n, int base=DEC, int digits=0){print(n, base,digits); println();}
    void Print::println(unsigned long long n, int base=DEC,int digits=0){print(n, base,digits);println();}
#endif
    void println(void) {print('\r');print('\n');};
    void printf(char const *format, ... );
};

#endif





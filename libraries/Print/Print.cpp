/*
 Print.cpp - Base class that provides print() and println()
 Copyright (c) 2008 David A. Mellis.  All right reserved.
 Modifications Copyright (c) 2011 Chrisj Jeppesen. All rights reserved
 
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
 Modified for LPC2000 September 2011 by Chris Jeppesen, derived from
 arduino-0022\hardware\arduino\cores\arduino\Print.cpp
  
 Changes:
 No String class - this uses dynamic memory allocation, a no-no for embedded
                   programs.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "float.h"
//#include "wiring.h"

#include "Print.h"

#ifdef U64

extern "C" {
void __exidx_start(void) {}
void __exidx_end(void) {}
void _exit(void) {}
void _kill(void) {}
void _getpid(void) {}
void _sbrk(void) {}
}

#endif



// Public Methods //////////////////////////////////////////////////////////////

/* default implementation: may be overridden */
void Print::write(const char *str) {
  while (*str) write(*str++);
}

/* default implementation: may be overridden */
void Print::write(const char *buffer, size_t size) {
  while (size--) write(*buffer++);
}

void Print::print(int n, int base, int digits) {
  if (base == 0) {
    write(n);
  } else if (base == 10) {
    if (n < 0) {
      print('-');
      n = -n;
    }
    printNumber((unsigned int)n, 10, digits);
  } else {
    printNumber((unsigned int)n, base,digits);
  }
}

void Print::print(unsigned int n, int base, int digits) {
  if (base == 0) write(n);
  else printNumber(n, base, digits);
}

#ifdef U64
void Print::print(long long n, int base)
{
  if (base == 0) {
    write(n);
  } else if (base == 10) {
    if (n < 0) {
      print('-');
      n = -n;
    }
    printNumber((unsigned long long)n, 10);
  } else {
    printNumber((unsigned long long)n, base);
  }
}

void Print::print(unsigned long long n, int base){
  if (base == 0) write(n);
  else printNumber(n, base);
}
#endif

// Private Methods /////////////////////////////////////////////////////////////

void Print::printNumber(unsigned int n, int base, int digits) {
  unsigned char buf[8 * sizeof(unsigned int)]; 
  unsigned int i = 0;

  if (n == 0) {
    for(i=0;i<(digits>0?digits:1);i++) print('0');
    return;
  } 

  while (n > 0||digits>0) {
    buf[i] = n % base;
    i++;
    digits--;
    n /= base;
  }

  for (; i > 0; i--) print((char) (buf[i - 1] < 10 ?'0' + buf[i - 1]:'A' + buf[i - 1] - 10));
}

#ifdef U64
void Print::printNumber(unsigned long long n, int base, int digits) {
  unsigned char buf[8 * sizeof(unsigned long long)]; 
  unsigned int i = 0;

  if (n == 0) {
    for(i=0;i<(digits>0?digits:1);i++) print('0');
    return;
  } 

  while (n > 0||digits>0) {
    buf[i] = n % base;
    i++;
    digits--;
    n /= base;
  }

  for (; i > 0; i--) print((char) (buf[i - 1] < 10 ?'0' + buf[i - 1]:'A' + buf[i - 1] - 10));
}

#endif

void Print::printFloat(fp number, unsigned char digits) 
{ 
  // Handle negative numbers
  if (number < fp(0.0))
  {
     print('-');
     number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  fp rounding = fp(0.5);
  for (unsigned char i=0; i<digits; ++i)
    rounding /= fp(10.0);
  
  number += rounding;

  // Extract the integer part of the number and print it
  unsigned int int_part = (unsigned int)number;
  fp remainder = number - (fp)int_part;
  print(int_part);

  // Print the decimal point, but only if there are digits beyond
  if (digits > 0)
    print("."); 

  // Extract digits from the remainder one at a time
  while (digits-- > 0) {
    remainder *= fp(10.0);
    unsigned int toPrint = (unsigned int)(remainder);
    print(toPrint);
    remainder -= toPrint; 
  } 
}

//#########################################################################
// printf.c
//
// *** printf() based on sprintf() from gcctest9.c Volker Oth
//
// *** Changes made by Holger Klabunde
// Now takes format strings from FLASH (was into RAM ! before)
// Fixed bug for %i, %I. u_val was used before it had the right value
// Added %d, %D (is same as %i, %I)
// Support for long variables %li, %ld, %Lu, %LX ....
// %x, %X now gives upper case hex characters A,B,C,D,E,F
// Output can be redirected in a single function: myputchar()
// Make printf() smaller by commenting out a few #defines
// Added some SPACE and ZERO padding %02x or % 3u up to 9 characters
//
// Todo:
// %f, %F for floating point numbers
//
// *** Changes made by Martin Thomas for the efsl debug output:
// - removed AVR "progmem"
// - added function pointer for "putchar"
// - devopen function
//
//#########################################################################

#include <stdarg.h>

#define SCRATCH 12  //32Bits go up to 4GB + 1 Byte for \0

//Spare some program space by making a comment of all not used format flag lines
#define USE_OCTAL // %o, %O Octal output. Who needs this ?
#define USE_STRING      // %s, %S Strings as parameters
#define USE_CHAR    // %c, %C Chars as parameters
#define USE_INTEGER // %i, %I Remove this format flag. %d, %D does the same
#define USE_HEX     // %x, %X Hexadezimal output
#define USE_UPPERHEX    // %x, %X outputs A,B,C... else a,b,c...
#ifndef USE_HEX
    #undef USE_UPPERHEX    // ;)
#endif
#define USE_UPPER   // uncommenting this removes %C,%D,%I,%O,%S,%U,%X and %L..
// only lowercase format flags are used
#define PADDING         //SPACE and ZERO padding

void Print::printf(char const *format, ...) {
    unsigned char scratch[SCRATCH];
    unsigned char format_flag;
    unsigned short base;
    unsigned char *ptr;
    unsigned char issigned=0;
    va_list ap;

    unsigned int u_val=0;
    int s_val=0;

    unsigned char fill;
    unsigned char width;

    va_start (ap, format);
    for (;;)
    {
		
        while ((format_flag = *(format++)) != '%')
        {      // Until '%' or '\0'
            if (!format_flag){va_end (ap); return;}
                write(format_flag);
        }

        issigned=0; //default unsigned
        base = 10;

        format_flag = *format++; //get char after '%'

        #ifdef PADDING
            width=0; //no formatting
            fill=0;  //no formatting
            if(format_flag=='0' || format_flag==' ') //SPACE or ZERO padding  ?
            {
                fill=format_flag;
                format_flag = *format++; //get char after padding char
                if(format_flag>='0' && format_flag<='9')
                {
                    width=format_flag-'0';
                    format_flag = *format++; //get char after width char
                }
            }
        #endif

        switch (format_flag) {
            #ifdef USE_CHAR
                case 'c':
                    #ifdef USE_UPPER
                    case 'C':
                #endif
                format_flag = va_arg(ap,int);
                // no break -> run into default
            #endif

            default:
                write(format_flag);
                continue;

                #ifdef USE_STRING
                    #ifdef USE_UPPER
                    case 'S':
                #endif
                case 's':
                ptr = (unsigned char*)va_arg(ap,char *);
                while(*ptr) { write(*ptr); ptr++; }
                    continue;
                #endif

                #ifdef USE_OCTAL
                    case 'o':
                    #ifdef USE_UPPER
                    case 'O':
                #endif
                base = 8;
                write('0');
                goto CONVERSION_LOOP;
                #endif

                #ifdef USE_INTEGER //don't use %i, is same as %d
                    case 'i':
                    #ifdef USE_UPPER
                    case 'I':
                #endif
                #endif
                case 'd':
                #ifdef USE_UPPER
                    case 'D':
                #endif
                issigned=1;
                // no break -> run into next case
            case 'u':
                #ifdef USE_UPPER
                    case 'U':
                #endif

                //don't insert some case below this if USE_HEX is undefined !
                //or put       goto CONVERSION_LOOP;  before next case.
                #ifdef USE_HEX
                    goto CONVERSION_LOOP;
                    case 'x':
                    #ifdef USE_UPPER
                    case 'X':
                #endif
                base = 16;
                #endif

                CONVERSION_LOOP:

                if(issigned) //Signed types
                {
                    s_val = va_arg(ap,int);

                    if(s_val < 0) //Value negative ?
                    {
                        s_val = - s_val; //Make it positive
                        write('-');    //Output sign
                    }

                    u_val = (unsigned int)s_val;
                }
                else //Unsigned types
                {
                    u_val = va_arg(ap,unsigned int);
                }

                ptr = scratch + SCRATCH;
                *--ptr = 0;
                do
                {
                    char ch = u_val % base + '0';
                    #ifdef USE_HEX
                        if (ch > '9')
                        {
                            ch += 'a' - '9' - 1;
                            #ifdef USE_UPPERHEX
                            ch-=0x20;
                        #endif
                    }
                    #endif
                    *--ptr = ch;
                    u_val /= base;

                    #ifdef PADDING
                        if(width) width--; //calculate number of padding chars
                    #endif
                }
                while (u_val);

                #ifdef PADDING
                    while(width--) *--ptr = fill; //insert padding chars
                #endif

                while(*ptr) { write(*ptr); ptr++; }
                    }
        }
    }

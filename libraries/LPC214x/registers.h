/**
   \file registers.h
   \brief Header file for Philips LPC214x Family Microprocessors.

 The header file is the super set of all hardware definition of the
 peripherals for the LPC214x family microprocessor. It defines a series of
 functions which return references to unsigned ints, which can be used
 as lvalues as needed. It is the C++ way to define a reference to an absolute
 location in memory, and since almost all peripheral registers are
 memory-mapped, that is how you need to get to them. C++ will inline and
 optimize these calls to the assembly code you think you would need. In most
 cases, the arguments to these routines are constant (if there are any) so the
 full address can be calculated at compile-time and completely optimized away.

 Read-only registers (which cannot be used as lvalues) are identified with the
 signature 

     static inline volatile unsigned int

 while read/write and write-only registers are identified with the signature
 
     static inline volatile unsigned int&

 so that they can be used as lvalues.

    Copyright(C) 2006, Philips Semiconductor
    All rights reserved.

    History
    2005.10.01  ver 1.00    Prelimnary version, first Release
    2005.10.13  ver 1.01    Removed CSPR and DC_REVISION register.
                            CSPR can not be accessed at the user level,
                            DC_REVISION is no long available.
                            All registers use "volatile unsigned int".
    29 Jan 2015 CDJ         Changed to C++, make everything a static inline
                              instead of a #define
*/

#ifndef registers_h
#define registers_h

/** Return a reference to an absolute location in memory. Since it is a reference,
   it can be used as an lvalue. This is used to reference memory-mapped hardware
   registers.
 @param addr Address to reference
 */
static inline volatile unsigned int& AbsRef(int addr) {return (*(volatile unsigned int *)addr);}
/** Return a reference to an absolute location in memory. Since it is a reference,
   it can be used as an lvalue. This form is used when a related block of registers,
   for example related to one peripheral, is being set up.
 @param base Base Address to reference
 @param ofs Offset from base in bytes
 */
static inline volatile unsigned int& AbsRef(int base, int ofs) {return AbsRef(base+ofs);}
/** Return a reference to an absolute location in memory. Since it is a reference,
   it can be used as an lvalue. This form is used when several related blocks of
   registers, for example related to several instances of one type of peripheral,
   is being set up.  
 @param base Base Address of all blocks to reference
 @param ofs Offset from base of this block in bytes
 @param delta Offset between blocks in bytes
 @param port block number to use 
 */
static inline volatile unsigned int& AbsRef(int base, int ofs, int delta, int port) {return AbsRef(base,delta*port+ofs);}
/** Return a reference to an absolute location in memory. Since it is a reference,
   it can be used as an lvalue. This form is used when several related blocks of
   registers, for example related to several instances of one type of peripheral,
   is being set up.  
 @param base0 Base Address of starting block to reference
 @param base1 Base Address of next block to reference
 @param port block number to use  
 @param ofs Offset from base of this block in bytes
 */
static inline volatile unsigned int& AbsRefBlock(int base0, int base1, int port, int ofs) {return AbsRef(base0,ofs,base1-base0,port);}
/** Return a reference to an absolute location in memory. Since it is a reference,
   it can be used as an lvalue. This form is used when several related peripherals,
   each with several channels, is being set up. This can also be used with a single
   periphereal with several channels.  
 @param base Base Address of all blocks to reference
 @param ofs Offset from base of block to channel 0 in bytes
 @param delta_port Offset between blocks in bytes
 @param port block number to use 
 @param delta_chan Offset between channels in bytes
 @param chan channel number to use 
 */
static inline volatile unsigned int& AbsRef(int base, int ofs, int delta_port, int port, int delta_chan, int chan) {return AbsRef(base,ofs+delta_chan*chan,delta_port,port);}
/** Return a reference to an absolute location in memory. Since it is a reference,
   it can be used as an lvalue. This form is used when several related peripherals,
   each with several channels, is being set up. This can also be used with a single
   periphereal with several channels.  
 @param base0 Base Address of starting block to reference
 @param base1 Base Address of next block to reference
 @param port block number to use  
 @param delta_chan Offset between channels in bytes
 @param chan channel number to use 
 @param ofs Offset from base of this block to channel 0 in bytes
 */
static inline volatile unsigned int& AbsRefBlock(int base0, int base1, int port, int delta_chan, int chan, int ofs) {return AbsRefBlock(base0,base1,port,ofs+delta_chan*chan);}

#define ro0(part,name    ,addr) static inline volatile unsigned int  name()             {return AbsRef(addr);}
#define rw0(part,name    ,addr) static inline volatile unsigned int& name()             {return AbsRef(addr);}
#define wo0(part,name    ,addr) static inline volatile unsigned int& name()             {return AbsRef(addr);}
#define ro1(part,name  ,N,addr) static inline volatile unsigned int  name(int i)        {return AbsRef(addr);}
#define rw1(part,name  ,N,addr) static inline volatile unsigned int& name(int i)        {return AbsRef(addr);}
#define wo1(part,name  ,N,addr) static inline volatile unsigned int& name(int i)        {return AbsRef(addr);}
#define ro2(part,name,M,N,addr) static inline volatile unsigned int  name(int i, int j) {return AbsRef(addr);}
#define rw2(part,name,M,N,addr) static inline volatile unsigned int& name(int i, int j) {return AbsRef(addr);}
#define wo2(part,name,M,N,addr) static inline volatile unsigned int& name(int i, int j) {return AbsRef(addr);}

#include "registers.inc"

#undef ro0
#undef rw0
#undef wo0
#undef ro1
#undef rw1
#undef wo1
#undef ro2
#undef rw2
#undef wo2

#endif  // registers_h


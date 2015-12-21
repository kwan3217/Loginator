/**
   \file LPC214x.h
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

#ifndef __LPC214x_H
#define __LPC214x_H

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

#include "vic_registers.inc"
#include "gpio_registers.inc"

/* Fast I/O setup */
const unsigned int FIO_BASE_ADDR=0x3FFFC000;
static inline volatile unsigned int& FIODIR (int port) {return AbsRef(FIO_BASE_ADDR,0x00,0x20,port);}
static inline volatile unsigned int& FIOMASK(int port) {return AbsRef(FIO_BASE_ADDR,0x10,0x20,port);}
static inline volatile unsigned int& FIOPIN (int port) {return AbsRef(FIO_BASE_ADDR,0x14,0x20,port);}
static inline volatile unsigned int& FIOSET (int port) {return AbsRef(FIO_BASE_ADDR,0x18,0x20,port);}
static inline volatile unsigned int& FIOCLR (int port) {return AbsRef(FIO_BASE_ADDR,0x1C,0x20,port);}

#include "scb_registers.inc"
#include "timer_registers.inc"
#include "pwm_registers.inc"
#include "uart_registers.inc"
#include "i2c_registers.inc"
#include "spi_registers.inc"

/* SSP Controller (usable as SPI1 but different map) */
const unsigned int SSP_BASE_ADDR=0xE0068000;
/*R/W*/static inline volatile unsigned int& SSPCR0 () {return AbsRef(SSP_BASE_ADDR,0x00);}
/*R/W*/static inline volatile unsigned int& SSPCR1 () {return AbsRef(SSP_BASE_ADDR,0x04);}
/*R/W*/static inline volatile unsigned int& SSPDR  () {return AbsRef(SSP_BASE_ADDR,0x08);}
/*RO */static inline volatile unsigned int  SSPSR  () {return AbsRef(SSP_BASE_ADDR,0x0C);}
/*R/W*/static inline volatile unsigned int& SSPCPSR() {return AbsRef(SSP_BASE_ADDR,0x10);}
/*R/W*/static inline volatile unsigned int& SSPIMSC() {return AbsRef(SSP_BASE_ADDR,0x14);}
/*R/W*/static inline volatile unsigned int& SSPRIS () {return AbsRef(SSP_BASE_ADDR,0x18);}
/*RO */static inline volatile unsigned int  SSPMIS () {return AbsRef(SSP_BASE_ADDR,0x1C);}
/*WO */static inline volatile unsigned int& SSPICR () {return AbsRef(SSP_BASE_ADDR,0x20);}

#include "rtc_registers.inc"
#include "adc_registers.inc"

/* D/A Converter */
const unsigned int DAC_BASE_ADDR=0xE006C000;
static inline volatile unsigned int& DACR() {return AbsRef(DAC_BASE_ADDR,0x00);}

/* Hardware ID */
//We write hardware identifiers here, in its own memory page.
const unsigned int HW_ID_BASE_ADDR=0x0007C000;
//The first describes the hardware type:
// 0 for Logomatic
// 1 for Loginator
// 2 for Rocketometer
// 3 for simulator
// remainder to 0xFFFFFFFE are reserved
// 0xFFFFFFFF is unknown (memory block never written to)
//The second word is a serial number, unique to hardware type, so there
//can be a Loginator 0 and Rocketometer 0, but no two rocketometers are
//both labeled 0.
static inline volatile unsigned int& HW_TYPE  () {return AbsRef(HW_ID_BASE_ADDR,0x00);}
static inline volatile unsigned int& HW_SERIAL() {return AbsRef(HW_ID_BASE_ADDR,0x04);}
const unsigned int HW_TYPE_LOGOMATIC   =0;
const unsigned int HW_TYPE_LOGINATOR   =1;
const unsigned int HW_TYPE_ROCKETOMETER=2;
const unsigned int HW_TYPE_SIMULATOR   =3;
/* Watchdog */
const unsigned int WDG_BASE_ADDR=0xE0000000;
static inline volatile unsigned int& WDMOD () {return AbsRef(WDG_BASE_ADDR,0x00);}
static inline volatile unsigned int& WDTC  () {return AbsRef(WDG_BASE_ADDR,0x04);}
static inline volatile unsigned int& WDFEED() {return AbsRef(WDG_BASE_ADDR,0x08);}
static inline volatile unsigned int& WDTV  () {return AbsRef(WDG_BASE_ADDR,0x0C);}

/* USB Controller */
const unsigned int USB_BASE_ADDR=0xE0090000;			/* USB Base Address */
/* Device Interrupt Registers */
static inline volatile unsigned int& USBDevIntSt () {return AbsRef(USB_BASE_ADDR,0x00);}
static inline volatile unsigned int& USBDevIntEn () {return AbsRef(USB_BASE_ADDR,0x04);}
static inline volatile unsigned int& USBDevIntClr() {return AbsRef(USB_BASE_ADDR,0x08);}
static inline volatile unsigned int& DEV_INT_SET () {return AbsRef(USB_BASE_ADDR,0x0C);}
static inline volatile unsigned int& USBDevIntPri() {return AbsRef(USB_BASE_ADDR,0x2C);}

/* Endpoint Interrupt Registers */
static inline volatile unsigned int& USBEpIntSt () {return AbsRef(USB_BASE_ADDR,0x30);}
static inline volatile unsigned int& USBEpIntEn () {return AbsRef(USB_BASE_ADDR,0x34);}
static inline volatile unsigned int& USBEpIntClr() {return AbsRef(USB_BASE_ADDR,0x38);}
static inline volatile unsigned int& EP_INT_SET () {return AbsRef(USB_BASE_ADDR,0x3C);}
static inline volatile unsigned int& USBEpIntPri() {return AbsRef(USB_BASE_ADDR,0x40);}

/* Endpoint Realization Registers */
static inline volatile unsigned int& USBReEp    () {return AbsRef(USB_BASE_ADDR,0x44);}
static inline volatile unsigned int& USBEpInd   () {return AbsRef(USB_BASE_ADDR,0x48);}
static inline volatile unsigned int& USBMaxPSize() {return AbsRef(USB_BASE_ADDR,0x4C);}

/* Command Reagisters */
static inline volatile unsigned int& USBCmdCode() {return AbsRef(USB_BASE_ADDR,0x10);}
static inline volatile unsigned int& USBCmdData() {return AbsRef(USB_BASE_ADDR,0x14);}

/* Data Transfer Registers */
static inline volatile unsigned int& USBRxData() {return AbsRef(USB_BASE_ADDR,0x18);}
static inline volatile unsigned int& USBTxData() {return AbsRef(USB_BASE_ADDR,0x1C);}
static inline volatile unsigned int& USBRxPLen() {return AbsRef(USB_BASE_ADDR,0x20);}
static inline volatile unsigned int& USBTxPLen() {return AbsRef(USB_BASE_ADDR,0x24);}
static inline volatile unsigned int& USBCtrl  () {return AbsRef(USB_BASE_ADDR,0x28);}

/* DMA Registers */
static inline volatile unsigned int& DMA_REQ_STAT    () {return AbsRef(USB_BASE_ADDR,0x50);}
static inline volatile unsigned int& DMA_REQ_CLR     () {return AbsRef(USB_BASE_ADDR,0x54);}
static inline volatile unsigned int& DMA_REQ_SET     () {return AbsRef(USB_BASE_ADDR,0x58);}
static inline volatile unsigned int& UDCA_HEAD       () {return AbsRef(USB_BASE_ADDR,0x80);}
static inline volatile unsigned int& EP_DMA_STAT     () {return AbsRef(USB_BASE_ADDR,0x84);}
static inline volatile unsigned int& EP_DMA_EN       () {return AbsRef(USB_BASE_ADDR,0x88);}
static inline volatile unsigned int& EP_DMA_DIS      () {return AbsRef(USB_BASE_ADDR,0x8C);}
static inline volatile unsigned int& DMA_INT_STAT    () {return AbsRef(USB_BASE_ADDR,0x90);}
static inline volatile unsigned int& DMA_INT_EN      () {return AbsRef(USB_BASE_ADDR,0x94);}
static inline volatile unsigned int& EOT_INT_STAT    () {return AbsRef(USB_BASE_ADDR,0xA0);}
static inline volatile unsigned int& EOT_INT_CLR     () {return AbsRef(USB_BASE_ADDR,0xA4);}
static inline volatile unsigned int& EOT_INT_SET     () {return AbsRef(USB_BASE_ADDR,0xA8);}
static inline volatile unsigned int& NDD_REQ_INT_STAT() {return AbsRef(USB_BASE_ADDR,0xAC);}
static inline volatile unsigned int& NDD_REQ_INT_CLR () {return AbsRef(USB_BASE_ADDR,0xB0);}
static inline volatile unsigned int& NDD_REQ_INT_SET () {return AbsRef(USB_BASE_ADDR,0xB4);}
static inline volatile unsigned int& SYS_ERR_INT_STAT() {return AbsRef(USB_BASE_ADDR,0xB8);}
static inline volatile unsigned int& SYS_ERR_INT_CLR () {return AbsRef(USB_BASE_ADDR,0xBC);}
static inline volatile unsigned int& SYS_ERR_INT_SET () {return AbsRef(USB_BASE_ADDR,0xC0);}
static inline volatile unsigned int& MODULE_ID       () {return AbsRef(USB_BASE_ADDR,0xFC);}

#undef ro0
#undef rw0
#undef wo0
#undef ro1
#undef rw1
#undef wo1
#undef ro2
#undef rw2
#undef wo2

#endif  // __LPC214x_H


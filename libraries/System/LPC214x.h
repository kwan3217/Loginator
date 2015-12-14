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

/* Vectored Interrupt Controller (VIC) */
const unsigned int VIC_BASE_ADDR=0xFFFFF000;
/*RO */static inline volatile unsigned int  VICIRQStatus()            {return AbsRef(VIC_BASE_ADDR,0x000);}
/*RO */static inline volatile unsigned int  VICFIQStatus()            {return AbsRef(VIC_BASE_ADDR,0x004);}
/*RO */static inline volatile unsigned int  VICRawIntr()              {return AbsRef(VIC_BASE_ADDR,0x008);}
/*R/W*/static inline volatile unsigned int& VICIntSelect()            {return AbsRef(VIC_BASE_ADDR,0x00C);}
/*R/W*/static inline volatile unsigned int& VICIntEnable()            {return AbsRef(VIC_BASE_ADDR,0x010);}
/*WO */static inline volatile unsigned int& VICIntEnClr()             {return AbsRef(VIC_BASE_ADDR,0x014);}
/*R/W*/static inline volatile unsigned int& VICSoftInt()              {return AbsRef(VIC_BASE_ADDR,0x018);}
/*WO */static inline volatile unsigned int& VICSoftIntClr()           {return AbsRef(VIC_BASE_ADDR,0x01C);}
/*R/W*/static inline volatile unsigned int& VICProtection()           {return AbsRef(VIC_BASE_ADDR,0x020);}
/*R/W*/static inline volatile unsigned int& VICVectAddr()             {return AbsRef(VIC_BASE_ADDR,0x030);}
/*R/W*/static inline volatile unsigned int& VICDefVectAddr()          {return AbsRef(VIC_BASE_ADDR,0x034);}
/*R/W*/static inline volatile unsigned int& VICVectAddrSlot(int slot) {return AbsRef(VIC_BASE_ADDR,0x100,4,slot);}
/*R/W*/static inline volatile unsigned int& VICVectCntlSlot(int slot) {return AbsRef(VIC_BASE_ADDR,0x200,4,slot);}

/* Pin Connect Block */
const unsigned int PINSEL_BASE_ADDR=0xE002C000;
static inline volatile unsigned int& PINSEL(int channel) {return AbsRef(PINSEL_BASE_ADDR,channel==0?0x00:(channel==1?0x04:0x14));}

/* General Purpose Input/Output (GPIO) */
const unsigned int GPIO_BASE_ADDR=0xE0028000;
/*R/W*/static inline volatile unsigned int& IOPIN(int port) {return AbsRef(GPIO_BASE_ADDR,0x00,0x10,port);}
/*R/W*/static inline volatile unsigned int& IOSET(int port) {return AbsRef(GPIO_BASE_ADDR,0x04,0x10,port);}
/*R/W*/static inline volatile unsigned int& IODIR(int port) {return AbsRef(GPIO_BASE_ADDR,0x08,0x10,port);}
/*WO */static inline volatile unsigned int& IOCLR(int port) {return AbsRef(GPIO_BASE_ADDR,0x0C,0x10,port);}

/* Fast I/O setup */
const unsigned int FIO_BASE_ADDR=0x3FFFC000;
static inline volatile unsigned int& FIODIR (int port) {return AbsRef(FIO_BASE_ADDR,0x00,0x20,port);}
static inline volatile unsigned int& FIOMASK(int port) {return AbsRef(FIO_BASE_ADDR,0x10,0x20,port);}
static inline volatile unsigned int& FIOPIN (int port) {return AbsRef(FIO_BASE_ADDR,0x14,0x20,port);}
static inline volatile unsigned int& FIOSET (int port) {return AbsRef(FIO_BASE_ADDR,0x18,0x20,port);}
static inline volatile unsigned int& FIOCLR (int port) {return AbsRef(FIO_BASE_ADDR,0x1C,0x20,port);}

/* System Control Block(SCB) modules include Memory Accelerator Module,
Phase Locked Loop, VPB divider, Power Control, External Interrupt,
Reset, and Code Security/Debugging */

const unsigned int SCB_BASE_ADDR=0xE01FC000;

/* Memory Accelerator Module (MAM) */
static inline volatile unsigned int& MAMCR () {return AbsRef(SCB_BASE_ADDR,0x000);}
static inline volatile unsigned int& MAMTIM() {return AbsRef(SCB_BASE_ADDR,0x004);}
/*R/W*/static inline volatile unsigned int& MEMMAP() {return AbsRef(SCB_BASE_ADDR,0x040);}

/* Phase Locked Loop (PLL) */
/*R/W*/static inline volatile unsigned int& PLLCON (int port) {return AbsRef(SCB_BASE_ADDR,0x080,0x20,port);}
/*R/W*/static inline volatile unsigned int& PLLCFG (int port) {return AbsRef(SCB_BASE_ADDR,0x084,0x20,port);}
/*RO */static inline volatile unsigned int  PLLSTAT(int port) {return AbsRef(SCB_BASE_ADDR,0x088,0x20,port);}
/*WO */static inline volatile unsigned int& PLLFEED(int port) {return AbsRef(SCB_BASE_ADDR,0x08C,0x20,port);}

/* Power Control */
/*R/W*/static inline volatile unsigned int& PCON()  {return AbsRef(SCB_BASE_ADDR,0x0C0);}
/*R/W*/static inline volatile unsigned int& PCONP() {return AbsRef(SCB_BASE_ADDR,0x0C4);}

/* VPB Divider (APB Divider in 2015 version of user manual) */
/*R/W*/static inline volatile unsigned int& VPBDIV() {return AbsRef(SCB_BASE_ADDR,0x100);}

/* External Interrupts */
/*R/W*/static inline volatile unsigned int& EXTINT  () {return AbsRef(SCB_BASE_ADDR,0x140);}
/*R/W*/static inline volatile unsigned int& INTWAKE () {return AbsRef(SCB_BASE_ADDR,0x144);}
/*R/W*/static inline volatile unsigned int& EXTMODE () {return AbsRef(SCB_BASE_ADDR,0x148);}
/*R/W*/static inline volatile unsigned int& EXTPOLAR() {return AbsRef(SCB_BASE_ADDR,0x14C);}

/* Reset (RSID in 2015 version of user manual)*/
/*R/W*/static inline volatile unsigned int& RSIR() {return AbsRef(SCB_BASE_ADDR,0x180);}

/* Code Security/Debugging */
/*RO */static inline volatile unsigned int  CSPR() {return AbsRef(SCB_BASE_ADDR,0x184);}

/* System Controls and Status */
/*R/W*/static inline volatile unsigned int& SCS() {return AbsRef(SCB_BASE_ADDR,0x1A0);}

/* Timer */
const unsigned int TMR0_BASE_ADDR=0xE0004000;
const unsigned int TMR1_BASE_ADDR=0xE0008000;
/** Timer Interrupt Register. TIR can be written to clear interrupts. TIR can be
read to identify which of eight possible interrupt sources are pending. */
/*R/W*/static inline volatile unsigned int& TIR  (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x00);}
/** Timer Control Register. TCR is used to control the Timer Counter
functions. The Timer Counter can be disabled or reset through TCR. */
/*R/W*/static inline volatile unsigned int& TTCR (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x04);}
/** Timer Counter. The 32-bit TTC is incremented every TPR+1 cycles of PCLK. The
TC is controlled through the TCR. If the timer is configured as a counter with
TCTCR, then TTC is incremented every TPR+1 count events instead.*/
/*R/W*/static inline volatile unsigned int& TTC  (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x08);}
/** Prescale Register. When the Prescale Counter (TPC) is equal to this value,
 *the next clock increments the TC and clears the PC. */
/*R/W*/static inline volatile unsigned int& TPR  (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x0C);}
/** Prescale Counter. The 32-bit TPC is a counter which is incremented to the
value stored in TPR. When the value in TPR is reached, the TTC is incremented
and the TPC is cleared. The TPC is observable and controllable through the bus
interface. The effect is that if TPC is zero, TTC counts at PCLK rate. If it
is 1, TTC counts every second PCLK tick. If it is 2, TTC counts every third
PCLK, and in general if it is N, TTC increments every N+1 ticks of PCLK. If the
timer is configured as a counter with TCTCR, then TTC is incremented every TPR+1
count events instead.*/
/*R/W*/static inline volatile unsigned int& TPC  (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x10);}
/** Match Control Register. TMCR is used to control on a per-match-channel basis
if an interrupt is generated and if the TC is reset or stopped when a match
occurs */
/*R/W*/static inline volatile unsigned int& TMCR (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x14);}
/** Match register. TMR can be enabled through TMCR to reset TTC, stop
both TTC and TPC, and/or generate an interrupt every time TMR matches
TTC. */
/*R/W*/static inline volatile unsigned int& TMR  (int timer, int channel) {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,4,channel,0x18);}
/** Capture control register. TCCR controls on a per-capture-channel basis which
edge of the capture inputs are used to load TCR and whether or not an interrupt
is generated when a capture takes place. */
/*R/W*/static inline volatile unsigned int& TCCR (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x28);}
/** Capture register. TCR(m,n) is loaded with the value of TTC when there is an
event on the CAPm.n input, where m is the timer used, 0 or 1, and n is the
capture channel, 0-3. Datasheet claims that this is read-only, but used as 
read/write in the code (to set to 0).*/
/*R/W*/static inline volatile unsigned int& TCR  (int timer, int channel) {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,4,channel,0x2C);}
/** External Match Register. The EMR controls on a per-channel-basis the
external match pins MATm.n where m is the timer used, 0 or 1, and n is the
capture channel, 0-3. */
/*R/W*/static inline volatile unsigned int& TEMR (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x3C);}
/** Count Control Register. The TCTCR selects between Timer and Counter mode,
and in Counter mode selects the signal and edge(s) for counting. */
/*R/W*/static inline volatile unsigned int& TCTCR(int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x70);}

/* Pulse Width Modulator (PWM) */
const unsigned int PWM_BASE_ADDR=0xE0014000;
/*R/W*/static inline volatile unsigned int& PWMIR () {return AbsRef(PWM_BASE_ADDR,0x00);}
/*R/W*/static inline volatile unsigned int& PWMTCR() {return AbsRef(PWM_BASE_ADDR,0x04);}
/*R/W*/static inline volatile unsigned int& PWMTC () {return AbsRef(PWM_BASE_ADDR,0x08);}
/*R/W*/static inline volatile unsigned int& PWMPR () {return AbsRef(PWM_BASE_ADDR,0x0C);}
/*R/W*/static inline volatile unsigned int& PWMPC () {return AbsRef(PWM_BASE_ADDR,0x10);}
/*R/W*/static inline volatile unsigned int& PWMMCR() {return AbsRef(PWM_BASE_ADDR,0x14);}
//We are trying to match the registers of a normal Timer, but a PWM timer has
//more channels than a normal timer. We write these extra channels in a hole in
//the map of a timer. Channel 3 is at 0x18+4*3=0x24, but channel 4 falls at
//(0x18+0x18)+4*4=0x40. This is a difference of 0x18 from the 0x28 we would
//otherwise, equivalent to 6 channels.
/*R/W*/static inline volatile unsigned int& PWMMR(int channel) {return AbsRef(PWM_BASE_ADDR,0x18,4,channel+((channel>3)?6:0));}
/*R/W*/static inline volatile unsigned int& PWMEMR() {return AbsRef(PWM_BASE_ADDR,0x3C);}
/*R/W*/static inline volatile unsigned int& PWMPCR() {return AbsRef(PWM_BASE_ADDR,0x4C);}
/*R/W*/static inline volatile unsigned int& PWMLER() {return AbsRef(PWM_BASE_ADDR,0x50);}

/* Universal Asynchronous Receiver Transmitter */
const unsigned int UART0_BASE_ADDR=0xE000C000;
const unsigned int UART1_BASE_ADDR=0xE0010000;
/** UART Receive Buffer Register. This register holds the next byte received by 
the UART. Reading it will make it ready to receive the next byte, filling it from
the Rx FIFO if it is enabled. Read only, has the same address as UTHR. Only 
available when the DLAB (bit 7 in ULCR) is 0, UDLL is visible at this address
when DLAB=1. */
static inline volatile unsigned int  URBR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x00);}
/** UART Transmit Holding Register. This register holds the next byte to be
transmitted by the UART. Writing it will either start sending the byte
immediately, or put it in the Tx FIFO if that is enabled. Write only, has the 
same address as URBR */
static inline volatile unsigned int& UTHR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x00);}
/** UART Divisor Latch LSB. This is used to control the baud rate, in
combination with UDLM. The baud rate controlled by the 16-bit value stored in 
UDLL+UDLM<<8, and fine tuned by UFDR. Only available when
DLAB=1, URBR is visible at this address when DLAB=0 */
static inline volatile unsigned int& UDLL(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x00);}
/** UART Divisor Latch MSB. This is used to control the baud rate, in
combination with UDLL. Only available when DLAB=1, UIER is visible at this
address when DLAB=0*/
static inline volatile unsigned int& UDLM(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x04);}
/** UART Interrupt Enable Register. 
*    * Bit 0 controls Receive Data Available and Character Receive Timeout
*        interrupts
*    * Bit 1 enables the threshold interrupt, which is identified in ULSR
*        bit 5
*    * Bit 2 enables the RX line status interrupts, which are identified in ULSR
*        bits 1-4
*
*Only available when DLAB=0, UDLM is visible at this address when DLAB=1*/
static inline volatile unsigned int& UIER(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x04);}
/** UART Interrupt ID Register. Reading this register clears the interrupt,
*  so this register must be read before the service routine finishes. Read-only,
*  shares address with UFCR.
*    * Bit 0 (active low) indicates an interrupt is pending
*    * Bits 1-3 indicate the source of the interrupt
*     + 011 (3) - Receive Line Status
*     + 010 (2) - Receive Data Available
*     + 110 (6) - Character Timeout Indicator
*     + 001 (1) - Threshold
*     + 000 (0) - Modem (only available on UART1, and only on LPC2144/6/8)
*    * Bits 6 and 7 mirror the FIFO enable bit in UFCR bit 0
*    * Bit 8 indicates successful end of auto-baud
*    * Bit 9 indicates auto-baud timeout
*/
static inline volatile unsigned int  UIIR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x08);}
/** UART FIFO Control Register. Write-only, shares address with UIIR
*    * Bit 0 enables both FIFOs
*    * Bit 1 resets the Rx FIFO
*    * Bit 2 resets the Tx FIFO
*    * Bits 6-7 controlt the RX trigger level
*     + 00 (0) - 1 character
*     + 01 (1) - 4 characters
*     + 10 (2) - 8 characters
*     + 11 (3) - 14 characters
*/
static inline volatile unsigned int& UFCR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x08);}
/** UART Line Control Register
*    * Bits 0-1 identify character length (=5+bits)
*    * Bit 2 controls stop bits (=1+bit)
*    * Bit 3 enables parity generation and checking
*    * Bits 4-5 select parity
*     + 00 (0) - Odd Parity - bit is set such that number of 1s transmitted is odd
*     + 01 (1) - Even Parity - bit is set such that number of 1s transmitted is even
*     + 10 (2) - Mark Parity - bit is always 1
*     + 11 (3) - Space Parity - bit is always 0
*    * Bit 6 controls break transmission
*    * Bit 7 is the divisor latch access bit. Setting to 0 makes URBR, UTHR, and
*      UIER visible. Setting to 1 makes UDLL and UDLM visisble
*/
static inline volatile unsigned int& ULCR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x0C);}
/** UART Modem Control Register. Only available on UART1, and only available
* on LPC2144/6/8.
*    * Bit 0 - DTR control
*    * Bit 1 - RTS control
*    * Bit 4 - Loopback mode select. If set, the TX and RX will be internally 
*        connected and no activity will be visible on the external pins
*    * Bit 6 controls Auto-RTS
*    * Bit 7 controls Auto-CST
*/
static inline volatile unsigned int& UMCR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x10);}
/** UART Line Status Register. Read-only
*    * Bit 0 - Receiver Data Ready
*    * Bit 1 - Overrun Error, cleared on read
*    * Bit 2 - Parity Error, cleared on read
*    * Bit 3 - Framing Error, cleared on read
*    * Bit 4 - Break Interrupt, cleared on read
*    * Bit 5 - Trasmistter Holding Register Empty
*    * Bit 6 - Transmitter Empty
*    * Bit 7 - Error in Rx FIFO, cleared on read
*/
static inline volatile unsigned int  ULSR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x14);}
/** UART Modem Status Register. Only available on UART1, and only available
* on LPC2144/6/8.
*    * Bit 0 - Input CTS status changed, cleared on read
*    * Bit 1 - Input DSR status changed, cleared on read
*    * Bit 2 - Trailing edge RI, cleared on read
*    * Bit 3 - Input DCD status changed, cleared on read
*    * Bit 4 - Clear to Send state. Bits 4-7 are active high, while the pins
*                 themselves are active low.
*    * Bit 5 - Data Set Ready state
*    * Bit 6 - Ring Indicator state
*    * Bit 7 - Data Carrier Detect state
*/
static inline volatile unsigned int  UMSR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x18);}
/** UART scratch register. Not used by UART, but holds its value and is read/write */
static inline volatile unsigned int& USCR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x1C);}
/** UART Auto-baud control register */
static inline volatile unsigned int& UACR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x20);}
/** UART Fractional divider register. Used to fine-tune the baud rate. */
static inline volatile unsigned int& UFDR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x28);}
/** UART Transmit Enable Register. Bit 7 controls whether the transmitter is enabled. */
static inline volatile unsigned int& UTER(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x30);}

/* I2C Interface */
const unsigned int I2C0_BASE_ADDR=0xE001C000;
const unsigned int I2C1_BASE_ADDR=0xE005C000;
/*R/W*/static inline volatile unsigned int& I2CCONSET(int port) {return AbsRefBlock(I2C0_BASE_ADDR,I2C1_BASE_ADDR,port,0x00);}
/*RO */static inline volatile unsigned int  I2CSTAT  (int port) {return AbsRefBlock(I2C0_BASE_ADDR,I2C1_BASE_ADDR,port,0x04);}
/*R/W*/static inline volatile unsigned int& I2CDAT   (int port) {return AbsRefBlock(I2C0_BASE_ADDR,I2C1_BASE_ADDR,port,0x08);}
/*R/W*/static inline volatile unsigned int& I2CADR   (int port) {return AbsRefBlock(I2C0_BASE_ADDR,I2C1_BASE_ADDR,port,0x0C);}
/*R/W*/static inline volatile unsigned int& I2CSCLH  (int port) {return AbsRefBlock(I2C0_BASE_ADDR,I2C1_BASE_ADDR,port,0x10);}
/*R/W*/static inline volatile unsigned int& I2CSCLL  (int port) {return AbsRefBlock(I2C0_BASE_ADDR,I2C1_BASE_ADDR,port,0x14);}
/*WO */static inline volatile unsigned int& I2CCONCLR(int port) {return AbsRefBlock(I2C0_BASE_ADDR,I2C1_BASE_ADDR,port,0x18);}

/* SPI0 (Serial Peripheral Interface 0) */
const unsigned int SPI0_BASE_ADDR=0xE0020000;
/*R/W*/static inline volatile unsigned int& S0SPCR () {return AbsRef(SPI0_BASE_ADDR,0x00);}
/*RO */static inline volatile unsigned int  S0SPSR () {return AbsRef(SPI0_BASE_ADDR,0x04);}
/*R/W*/static inline volatile unsigned int& S0SPDR () {return AbsRef(SPI0_BASE_ADDR,0x08);}
/*R/W*/static inline volatile unsigned int& S0SPCCR() {return AbsRef(SPI0_BASE_ADDR,0x0C);}
/*R/W*/static inline volatile unsigned int& S0SPINT() {return AbsRef(SPI0_BASE_ADDR,0x1C);}

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

/* Real Time Clock */
const unsigned int RTC_BASE_ADDR=0xE0024000;
/*R/W*/static inline volatile unsigned int& ILR     () {return AbsRef(RTC_BASE_ADDR,0x00);}
/*RO */static inline volatile unsigned int  CTC     () {return AbsRef(RTC_BASE_ADDR,0x04);}
/*R/W*/static inline volatile unsigned int& CCR     () {return AbsRef(RTC_BASE_ADDR,0x08);}
/*R/W*/static inline volatile unsigned int& CIIR    () {return AbsRef(RTC_BASE_ADDR,0x0C);}
/*R/W*/static inline volatile unsigned int& AMR     () {return AbsRef(RTC_BASE_ADDR,0x10);}
/*RO */static inline volatile unsigned int  CTIME0  () {return AbsRef(RTC_BASE_ADDR,0x14);}
/*RO */static inline volatile unsigned int  CTIME1  () {return AbsRef(RTC_BASE_ADDR,0x18);}
/*RO */static inline volatile unsigned int  CTIME2  () {return AbsRef(RTC_BASE_ADDR,0x1C);}
/*R/W*/static inline volatile unsigned int& RTCSEC  () {return AbsRef(RTC_BASE_ADDR,0x20);}
/*R/W*/static inline volatile unsigned int& RTCMIN  () {return AbsRef(RTC_BASE_ADDR,0x24);}
/*R/W*/static inline volatile unsigned int& RTCHOUR () {return AbsRef(RTC_BASE_ADDR,0x28);}
/*R/W*/static inline volatile unsigned int& RTCDOM  () {return AbsRef(RTC_BASE_ADDR,0x2C);}
/*R/W*/static inline volatile unsigned int& RTCDOW  () {return AbsRef(RTC_BASE_ADDR,0x30);}
/*R/W*/static inline volatile unsigned int& RTCDOY  () {return AbsRef(RTC_BASE_ADDR,0x34);}
/*R/W*/static inline volatile unsigned int& RTCMONTH() {return AbsRef(RTC_BASE_ADDR,0x38);}
/*R/W*/static inline volatile unsigned int& RTCYEAR () {return AbsRef(RTC_BASE_ADDR,0x3C);}
/*R/W*/static inline volatile unsigned int& ALSEC   () {return AbsRef(RTC_BASE_ADDR,0x60);}
/*R/W*/static inline volatile unsigned int& ALMIN   () {return AbsRef(RTC_BASE_ADDR,0x64);}
/*R/W*/static inline volatile unsigned int& ALHOUR  () {return AbsRef(RTC_BASE_ADDR,0x68);}
/*R/W*/static inline volatile unsigned int& ALDOM   () {return AbsRef(RTC_BASE_ADDR,0x6C);}
/*R/W*/static inline volatile unsigned int& ALDOW   () {return AbsRef(RTC_BASE_ADDR,0x70);}
/*R/W*/static inline volatile unsigned int& ALDOY   () {return AbsRef(RTC_BASE_ADDR,0x74);}
/*R/W*/static inline volatile unsigned int& ALMON   () {return AbsRef(RTC_BASE_ADDR,0x78);}
/*R/W*/static inline volatile unsigned int& ALYEAR  () {return AbsRef(RTC_BASE_ADDR,0x7C);}
/*R/W*/static inline volatile unsigned int& PREINT  () {return AbsRef(RTC_BASE_ADDR,0x80);}
/*R/W*/static inline volatile unsigned int& PREFRAC () {return AbsRef(RTC_BASE_ADDR,0x84);}

/* A/D Converter 0 (AD0) */
const unsigned int AD0_BASE_ADDR=0xE0034000;
const unsigned int AD1_BASE_ADDR=0xE0060000;
/** A/D Control register. The ADCR register must be written to select the
 *  operating mode before A/D conversion can occur. */
/*R/W*/static inline volatile unsigned int& ADCR   (int adc)              {return AbsRefBlock(AD0_BASE_ADDR,AD1_BASE_ADDR,adc,          0x00);}
/** A/D Global Data Register. This register contains the ADC DONE bit
 *  and the result of the most recent A/D conversion. */
/*R/W*/static inline volatile unsigned int& ADGDR  (int adc)              {return AbsRefBlock(AD0_BASE_ADDR,AD1_BASE_ADDR,adc,          0x04);}
/** A/D Status Register. This register contains the DONE and OVERRUN 
 *  flags for all of the A/D channels, as well as the A/D interrupt flag */
/*RO */static inline volatile unsigned int  ADSTAT (int adc)              {return AbsRefBlock(AD0_BASE_ADDR,AD1_BASE_ADDR,adc,          0x30);}
/** A/D Global Start Register. This address can be written to start conversions
 *  in both A/D converters simultaneously. */
/*WO */static inline volatile unsigned int& ADGSR  ()                     {return AbsRef     (AD0_BASE_ADDR,                            0x08);}
/** A/D Interrupt Enable Register. This register contains enable bits that allow
 *  the DONE flag of each A/D channel to be included or excluded from 
 *  contributing to the generation of an A/D interrupt. */
/*R/W*/static inline volatile unsigned int& ADINTEN(int adc)              {return AbsRefBlock(AD0_BASE_ADDR,AD1_BASE_ADDR,adc,          0x0C);}
/** A/D Data Register. This register contains the result of the most
 *  recent conversion completed on this channel. */
/*RO */static inline volatile unsigned int ADDR   (int adc, int channel) {return AbsRefBlock(AD0_BASE_ADDR,AD1_BASE_ADDR,adc,4,channel,0x10);}

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

#endif  // __LPC214x_H


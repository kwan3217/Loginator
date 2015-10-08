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
static inline volatile unsigned int& VICIRQStatus()            {return AbsRef(VIC_BASE_ADDR,0x000);}
static inline volatile unsigned int& VICFIQStatus()            {return AbsRef(VIC_BASE_ADDR,0x004);}
static inline volatile unsigned int& VICRawIntr()              {return AbsRef(VIC_BASE_ADDR,0x008);}
static inline volatile unsigned int& VICIntSelect()            {return AbsRef(VIC_BASE_ADDR,0x00C);}
static inline volatile unsigned int& VICIntEnable()            {return AbsRef(VIC_BASE_ADDR,0x010);}
static inline volatile unsigned int& VICIntEnClr()             {return AbsRef(VIC_BASE_ADDR,0x014);}
static inline volatile unsigned int& VICSoftInt()              {return AbsRef(VIC_BASE_ADDR,0x018);}
static inline volatile unsigned int& VICSoftIntClr()           {return AbsRef(VIC_BASE_ADDR,0x01C);}
static inline volatile unsigned int& VICProtection()           {return AbsRef(VIC_BASE_ADDR,0x020);}
static inline volatile unsigned int& VICVectAddr()             {return AbsRef(VIC_BASE_ADDR,0x030);}
static inline volatile unsigned int& VICDefVectAddr()          {return AbsRef(VIC_BASE_ADDR,0x034);}
static inline volatile unsigned int& VICVectAddrSlot(int slot) {return AbsRef(VIC_BASE_ADDR,0x100,4,slot);}
static inline volatile unsigned int& VICVectCntlSlot(int slot) {return AbsRef(VIC_BASE_ADDR,0x200,4,slot);}

/* Pin Connect Block */
const unsigned int PINSEL_BASE_ADDR=0xE002C000;
static inline volatile unsigned int& PINSEL(int channel) {return AbsRef(PINSEL_BASE_ADDR,channel==0?0x00:(channel==1?0x04:0x14));}

/* General Purpose Input/Output (GPIO) */
const unsigned int GPIO_BASE_ADDR=0xE0028000;
static inline volatile unsigned int& IOPIN(int port) {return AbsRef(GPIO_BASE_ADDR,0x00,0x10,port);}
static inline volatile unsigned int& IOSET(int port) {return AbsRef(GPIO_BASE_ADDR,0x04,0x10,port);}
static inline volatile unsigned int& IODIR(int port) {return AbsRef(GPIO_BASE_ADDR,0x08,0x10,port);}
static inline volatile unsigned int& IOCLR(int port) {return AbsRef(GPIO_BASE_ADDR,0x0C,0x10,port);}

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
static inline volatile unsigned int& MEMMAP() {return AbsRef(SCB_BASE_ADDR,0x040);}

/* Phase Locked Loop (PLL) */
static inline volatile unsigned int& PLLCON (int port) {return AbsRef(SCB_BASE_ADDR,0x080,0x20,port);}
static inline volatile unsigned int& PLLCFG (int port) {return AbsRef(SCB_BASE_ADDR,0x084,0x20,port);}
static inline volatile unsigned int& PLLSTAT(int port) {return AbsRef(SCB_BASE_ADDR,0x088,0x20,port);}
static inline volatile unsigned int& PLLFEED(int port) {return AbsRef(SCB_BASE_ADDR,0x08C,0x20,port);}

/* Power Control */
static inline volatile unsigned int& PCON()  {return AbsRef(SCB_BASE_ADDR,0x0C0);}
static inline volatile unsigned int& PCONP() {return AbsRef(SCB_BASE_ADDR,0x0C4);}

/* VPB Divider */
static inline volatile unsigned int& VPBDIV() {return AbsRef(SCB_BASE_ADDR,0x100);}

/* External Interrupts */
static inline volatile unsigned int& EXTINT  () {return AbsRef(SCB_BASE_ADDR,0x140);}
static inline volatile unsigned int& INTWAKE () {return AbsRef(SCB_BASE_ADDR,0x144);}
static inline volatile unsigned int& EXTMODE () {return AbsRef(SCB_BASE_ADDR,0x148);}
static inline volatile unsigned int& EXTPOLAR() {return AbsRef(SCB_BASE_ADDR,0x14C);}

/* Reset */
static inline volatile unsigned int& RSIR() {return AbsRef(SCB_BASE_ADDR,0x180);}

/* System Controls and Status */
static inline volatile unsigned int& SCS() {return AbsRef(SCB_BASE_ADDR,0x1A0);}

/* Timer */
const unsigned int TMR0_BASE_ADDR=0xE0004000;
const unsigned int TMR1_BASE_ADDR=0xE0008000;
/** Timer Interrupt Register. TIR can be written to clear interrupts. TIR can be
read to identify which of eight possible interrupt sources are pending. */
static inline volatile unsigned int& TIR  (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x00);}
/** Timer Control Register. TCR is used to control the Timer Counter
functions. The Timer Counter can be disabled or reset through TCR. */
static inline volatile unsigned int& TTCR (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x04);}
/** Timer Counter. The 32-bit TTC is incremented every TPR+1 cycles of PCLK. The
TC is controlled through the TCR. If the timer is configured as a counter with
TCTCR, then TTC is incremented every TPR+1 count events instead.*/
static inline volatile unsigned int& TTC  (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x08);}
/** Prescale Register. When the Prescale Counter (TPC) is equal to this value,
 *the next clock increments the TC and clears the PC. */
static inline volatile unsigned int& TPR  (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x0C);}
/** Prescale Counter. The 32-bit TPC is a counter which is incremented to the
value stored in TPR. When the value in TPR is reached, the TTC is incremented
and the TPC is cleared. The TPC is observable and controllable through the bus
interface. The effect is that if TPC is zero, TTC counts at PCLK rate. If it
is 1, TTC counts every second PCLK tick. If it is 2, TTC counts every third
PCLK, and in general if it is N, TTC increments every N+1 ticks of PCLK. If the
timer is configured as a counter with TCTCR, then TTC is incremented every TPR+1
count events instead.*/
static inline volatile unsigned int& TPC  (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x10);}
/** Match Control Register. TMCR is used to control on a per-match-channel basis
if an interrupt is generated and if the TC is reset or stopped when a match
occurs */
static inline volatile unsigned int& TMCR (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x14);}
/** Match register. TMR can be enabled through TMCR to reset TTC, stop
both TTC and TPC, and/or generate an interrupt every time TMR matches
TTC. */
static inline volatile unsigned int& TMR  (int timer, int channel) {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,4,channel,0x18);}
/** Capture control register. TCCR controls on a per-capture-channel basis which
edge of the capture inputs are used to load TCR and whether or not an interrupt
is generated when a capture takes place. */
static inline volatile unsigned int& TCCR (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x28);}
/** Capture register. TCR(m,n) is loaded with the value of TTC when there is an
event on the CAPm.n input, where m is the timer used, 0 or 1, and n is the
capture channel, 0-3. */
static inline volatile unsigned int& TCR  (int timer, int channel) {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,4,channel,0x2C);}
/** External Match Register. The EMR controls on a per-channel-basis the
external match pins MATm.n where m is the timer used, 0 or 1, and n is the
capture channel, 0-3. */
static inline volatile unsigned int& TEMR (int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x3C);}
/** Count Control Register. The TCTCR selects between Timer and Counter mode,
and in Counter mode selects the signal and edge(s) for counting. */
static inline volatile unsigned int& TCTCR(int timer)              {return AbsRefBlock(TMR0_BASE_ADDR,TMR1_BASE_ADDR,timer,          0x70);}

/* Pulse Width Modulator (PWM) */
const unsigned int PWM_BASE_ADDR=0xE0014000;
static inline volatile unsigned int& PWMIR () {return AbsRef(PWM_BASE_ADDR,0x00);}
static inline volatile unsigned int& PWMTCR() {return AbsRef(PWM_BASE_ADDR,0x04);}
static inline volatile unsigned int& PWMTC () {return AbsRef(PWM_BASE_ADDR,0x08);}
static inline volatile unsigned int& PWMPR () {return AbsRef(PWM_BASE_ADDR,0x0C);}
static inline volatile unsigned int& PWMPC () {return AbsRef(PWM_BASE_ADDR,0x10);}
static inline volatile unsigned int& PWMMCR() {return AbsRef(PWM_BASE_ADDR,0x14);}
//We are trying to match the registers of a normal Timer, but a PWM timer has
//more channels than a normal timer. We write these extra channels in a hole in
//the map of a timer. Channel 3 is at 0x18+4*3=0x24, but channel 4 falls at
//(0x18+0x18)+4*4=0x40. This is a difference of 0x18 from the 0x28 we would
//otherwise, equivalent to 6 channels.
static inline volatile unsigned int& PWMMR(int channel) {return AbsRef(PWM_BASE_ADDR,0x18,4,channel+((channel>3)?6:0));}
static inline volatile unsigned int& PWMEMR() {return AbsRef(PWM_BASE_ADDR,0x3C);}
static inline volatile unsigned int& PWMPCR() {return AbsRef(PWM_BASE_ADDR,0x4C);}
static inline volatile unsigned int& PWMLER() {return AbsRef(PWM_BASE_ADDR,0x50);}

/* Universal Asynchronous Receiver Transmitter */
const unsigned int UART0_BASE_ADDR=0xE000C000;
const unsigned int UART1_BASE_ADDR=0xE0010000;
static inline volatile unsigned int& URBR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x00);}
static inline volatile unsigned int& UTHR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x00);}
static inline volatile unsigned int& UDLL(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x00);}
static inline volatile unsigned int& UDLM(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x04);}
static inline volatile unsigned int& UIER(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x04);}
static inline volatile unsigned int& UIIR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x08);}
static inline volatile unsigned int& UFCR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x08);}
static inline volatile unsigned int& ULCR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x0C);}
static inline volatile unsigned int& UMCR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x10);}
static inline volatile unsigned int& ULSR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x14);}
static inline volatile unsigned int& UMSR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x18);}
static inline volatile unsigned int& USCR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x1C);}
static inline volatile unsigned int& UACR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x20);}
static inline volatile unsigned int& UFDR(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x28);}
static inline volatile unsigned int& UTER(int port) {return AbsRefBlock(UART0_BASE_ADDR,UART1_BASE_ADDR,port,0x30);}

/* I2C Interface */
const unsigned int I2C0_BASE_ADDR=0xE001C000;
const unsigned int I2C1_BASE_ADDR=0xE005C000;
static inline volatile unsigned int& I2CCONSET(int port) {return AbsRefBlock(I2C0_BASE_ADDR,I2C1_BASE_ADDR,port,0x00);}
static inline volatile unsigned int& I2CSTAT  (int port) {return AbsRefBlock(I2C0_BASE_ADDR,I2C1_BASE_ADDR,port,0x04);}
static inline volatile unsigned int& I2CDAT   (int port) {return AbsRefBlock(I2C0_BASE_ADDR,I2C1_BASE_ADDR,port,0x08);}
static inline volatile unsigned int& I2CADR   (int port) {return AbsRefBlock(I2C0_BASE_ADDR,I2C1_BASE_ADDR,port,0x0C);}
static inline volatile unsigned int& I2CSCLH  (int port) {return AbsRefBlock(I2C0_BASE_ADDR,I2C1_BASE_ADDR,port,0x10);}
static inline volatile unsigned int& I2CSCLL  (int port) {return AbsRefBlock(I2C0_BASE_ADDR,I2C1_BASE_ADDR,port,0x14);}
static inline volatile unsigned int& I2CCONCLR(int port) {return AbsRefBlock(I2C0_BASE_ADDR,I2C1_BASE_ADDR,port,0x18);}

/* SPI0 (Serial Peripheral Interface 0) */
const unsigned int SPI0_BASE_ADDR=0xE0020000;
static inline volatile unsigned int& S0SPCR () {return AbsRef(SPI0_BASE_ADDR,0x00);}
static inline volatile unsigned int& S0SPSR () {return AbsRef(SPI0_BASE_ADDR,0x04);}
static inline volatile unsigned int& S0SPDR () {return AbsRef(SPI0_BASE_ADDR,0x08);}
static inline volatile unsigned int& S0SPCCR() {return AbsRef(SPI0_BASE_ADDR,0x0C);}
static inline volatile unsigned int& S0SPINT() {return AbsRef(SPI0_BASE_ADDR,0x1C);}

/* SSP Controller (usable as SPI1 but different map) */
const unsigned int SSP_BASE_ADDR=0xE0068000;
static inline volatile unsigned int& SSPCR0 () {return AbsRef(SSP_BASE_ADDR,0x00);}
static inline volatile unsigned int& SSPCR1 () {return AbsRef(SSP_BASE_ADDR,0x04);}
static inline volatile unsigned int& SSPDR  () {return AbsRef(SSP_BASE_ADDR,0x08);}
static inline volatile unsigned int& SSPSR  () {return AbsRef(SSP_BASE_ADDR,0x0C);}
static inline volatile unsigned int& SSPCPSR() {return AbsRef(SSP_BASE_ADDR,0x10);}
static inline volatile unsigned int& SSPIMSC() {return AbsRef(SSP_BASE_ADDR,0x14);}
static inline volatile unsigned int& SSPRIS () {return AbsRef(SSP_BASE_ADDR,0x18);}
static inline volatile unsigned int& SSPMIS () {return AbsRef(SSP_BASE_ADDR,0x1C);}
static inline volatile unsigned int& SSPICR () {return AbsRef(SSP_BASE_ADDR,0x20);}

/* Real Time Clock */
const unsigned int RTC_BASE_ADDR=0xE0024000;
static inline volatile unsigned int& ILR     () {return AbsRef(RTC_BASE_ADDR,0x00);}
static inline volatile unsigned int& CTC     () {return AbsRef(RTC_BASE_ADDR,0x04);}
static inline volatile unsigned int& CCR     () {return AbsRef(RTC_BASE_ADDR,0x08);}
static inline volatile unsigned int& CIIR    () {return AbsRef(RTC_BASE_ADDR,0x0C);}
static inline volatile unsigned int& AMR     () {return AbsRef(RTC_BASE_ADDR,0x10);}
static inline volatile unsigned int& CTIME0  () {return AbsRef(RTC_BASE_ADDR,0x14);}
static inline volatile unsigned int& CTIME1  () {return AbsRef(RTC_BASE_ADDR,0x18);}
static inline volatile unsigned int& CTIME2  () {return AbsRef(RTC_BASE_ADDR,0x1C);}
static inline volatile unsigned int& RTCSEC  () {return AbsRef(RTC_BASE_ADDR,0x20);}
static inline volatile unsigned int& RTCMIN  () {return AbsRef(RTC_BASE_ADDR,0x24);}
static inline volatile unsigned int& RTCHOUR () {return AbsRef(RTC_BASE_ADDR,0x28);}
static inline volatile unsigned int& RTCDOM  () {return AbsRef(RTC_BASE_ADDR,0x2C);}
static inline volatile unsigned int& RTCDOW  () {return AbsRef(RTC_BASE_ADDR,0x30);}
static inline volatile unsigned int& RTCDOY  () {return AbsRef(RTC_BASE_ADDR,0x34);}
static inline volatile unsigned int& RTCMONTH() {return AbsRef(RTC_BASE_ADDR,0x38);}
static inline volatile unsigned int& RTCYEAR () {return AbsRef(RTC_BASE_ADDR,0x3C);}
static inline volatile unsigned int& ALSEC   () {return AbsRef(RTC_BASE_ADDR,0x60);}
static inline volatile unsigned int& ALMIN   () {return AbsRef(RTC_BASE_ADDR,0x64);}
static inline volatile unsigned int& ALHOUR  () {return AbsRef(RTC_BASE_ADDR,0x68);}
static inline volatile unsigned int& ALDOM   () {return AbsRef(RTC_BASE_ADDR,0x6C);}
static inline volatile unsigned int& ALDOW   () {return AbsRef(RTC_BASE_ADDR,0x70);}
static inline volatile unsigned int& ALDOY   () {return AbsRef(RTC_BASE_ADDR,0x74);}
static inline volatile unsigned int& ALMON   () {return AbsRef(RTC_BASE_ADDR,0x78);}
static inline volatile unsigned int& ALYEAR  () {return AbsRef(RTC_BASE_ADDR,0x7C);}
static inline volatile unsigned int& PREINT  () {return AbsRef(RTC_BASE_ADDR,0x80);}
static inline volatile unsigned int& PREFRAC () {return AbsRef(RTC_BASE_ADDR,0x84);}

/* A/D Converter 0 (AD0) */
const unsigned int AD0_BASE_ADDR=0xE0034000;
const unsigned int AD1_BASE_ADDR=0xE0060000;
/** A/D Control register. The ADCR register must be written to select the
 *  operating mode before A/D conversion can occur. */
static inline volatile unsigned int& ADCR   (int adc)              {return AbsRefBlock(AD0_BASE_ADDR,AD1_BASE_ADDR,adc,          0x00);}
/** A/D Global Data Register. This register contains the ADC DONE bit
 *  and the result of the most recent A/D conversion. */
static inline volatile unsigned int& ADGDR  (int adc)              {return AbsRefBlock(AD0_BASE_ADDR,AD1_BASE_ADDR,adc,          0x04);}
/** A/D Status Register. This register contains the DONE and OVERRUN 
 *  flags for all of the A/D channels, as well as the A/D interrupt flag */
static inline volatile unsigned int& ADSTAT (int adc)              {return AbsRefBlock(AD0_BASE_ADDR,AD1_BASE_ADDR,adc,          0x30);}
/** A/D Global Start Register. This address can be written to start conversions
 *  in both A/D converters simultaneously. */
static inline volatile unsigned int& ADGSR  ()                     {return AbsRef     (AD0_BASE_ADDR,                            0x08);}
/** A/D Interrupt Enable Register. This register contains enable bits that allow
 *  the DONE flag of each A/D channel to be included or excluded from 
 *  contributing to the generation of an A/D interrupt. */
static inline volatile unsigned int& ADINTEN(int adc)              {return AbsRefBlock(AD0_BASE_ADDR,AD1_BASE_ADDR,adc,          0x0C);}
/** A/D Data Register. This register contains the result of the most
 *  recent conversion completed on this channel. */
static inline volatile unsigned int& ADDR   (int adc, int channel) {return AbsRefBlock(AD0_BASE_ADDR,AD1_BASE_ADDR,adc,4,channel,0x10);}

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


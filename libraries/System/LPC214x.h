/******************************************************************************
 *   LPC214X.h:  Header file for Philips LPC214x Family Microprocessors
 *   The header file is the super set of all hardware definition of the 
 *   peripherals for the LPC214x family microprocessor.
 *
 *   Copyright(C) 2006, Philips Semiconductor
 *   All rights reserved.

 *   History
 *   2005.10.01  ver 1.00    Prelimnary version, first Release
 *   2005.10.13  ver 1.01    Removed CSPR and DC_REVISION register.
 *                           CSPR can not be accessed at the user level,
 *                           DC_REVISION is no long available.
 *                           All registers use "volatile unsigned int". 
******************************************************************************/

#ifndef __LPC214x_H
#define __LPC214x_H

/* Vectored Interrupt Controller (VIC) */
#define VIC_BASE_ADDR	0xFFFFF000

#define VICIRQStatus   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x000))
#define VICFIQStatus   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x004))
#define VICRawIntr     (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x008))
#define VICIntSelect   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x00C))
#define VICIntEnable   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x010))
#define VICIntEnClr    (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x014))
#define VICSoftInt     (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x018))
#define VICSoftIntClr  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x01C))
#define VICProtection  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x020))
#define VICVectAddr    (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x030))
#define VICDefVectAddr (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x034))
#define VICVectAddrSlot(slot) (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x100+((slot)*4)))
#define VICVectAddr0   VICVectAddrSlot(0)
#define VICVectAddr1   VICVectAddrSlot(1)
#define VICVectAddr2   VICVectAddrSlot(2)
#define VICVectAddr3   VICVectAddrSlot(3)
#define VICVectAddr4   VICVectAddrSlot(4)
#define VICVectAddr5   VICVectAddrSlot(5)
#define VICVectAddr6   VICVectAddrSlot(6)
#define VICVectAddr7   VICVectAddrSlot(7)
#define VICVectAddr8   VICVectAddrSlot(8)
#define VICVectAddr9   VICVectAddrSlot(9)
#define VICVectAddr10  VICVectAddrSlot(10)
#define VICVectAddr11  VICVectAddrSlot(11)
#define VICVectAddr12  VICVectAddrSlot(12)
#define VICVectAddr13  VICVectAddrSlot(13)
#define VICVectAddr14  VICVectAddrSlot(14)
#define VICVectAddr15  VICVectAddrSlot(15)
#define VICVectCntlSlot(slot) (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x200+((slot)*4)))
#define VICVectCntl0   VICVectCntlSlot(0)
#define VICVectCntl1   VICVectCntlSlot(1)
#define VICVectCntl2   VICVectCntlSlot(2)
#define VICVectCntl3   VICVectCntlSlot(3)
#define VICVectCntl4   VICVectCntlSlot(4)
#define VICVectCntl5   VICVectCntlSlot(5)
#define VICVectCntl6   VICVectCntlSlot(6)
#define VICVectCntl7   VICVectCntlSlot(7)
#define VICVectCntl8   VICVectCntlSlot(8)
#define VICVectCntl9   VICVectCntlSlot(9)
#define VICVectCntl10  VICVectCntlSlot(10)
#define VICVectCntl11  VICVectCntlSlot(11)
#define VICVectCntl12  VICVectCntlSlot(12)
#define VICVectCntl13  VICVectCntlSlot(13)
#define VICVectCntl14  VICVectCntlSlot(14)
#define VICVectCntl15  VICVectCntlSlot(15)

/* Pin Connect Block */
#define PINSEL_BASE_ADDR	0xE002C000
#define PINSEL0        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x00))
#define PINSEL1        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x04))
#define PINSEL2        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x14))

/* General Purpose Input/Output (GPIO) */
#define GPIO_BASE_ADDR		0xE0028000
#define IOPIN(port)    (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x00+0x10*port))
#define IOSET(port)    (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x04+0x10*port))
#define IODIR(port)    (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x08+0x10*port))
#define IOCLR(port)    (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x0C+0x10*port))
#define IOPIN0         IOPIN(0)
#define IOSET0         IOSET(0)
#define IODIR0         IODIR(0)
#define IOCLR0         IOCLR(0)
#define IOPIN1         IOPIN(1)
#define IOSET1         IOSET(1)
#define IODIR1         IODIR(1)
#define IOCLR1         IOCLR(1)

/* Fast I/O setup */
#define FIO_BASE_ADDR		0x3FFFC000
#define FIO0DIR        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x00)) 
#define FIO0MASK       (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x10))
#define FIO0PIN        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x14))
#define FIO0SET        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x18))
#define FIO0CLR        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x1C))
#define FIO1DIR        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x20)) 
#define FIO1MASK       (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x30))
#define FIO1PIN        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x34))
#define FIO1SET        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x38))
#define FIO1CLR        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x3C))

/* System Control Block(SCB) modules include Memory Accelerator Module,
Phase Locked Loop, VPB divider, Power Control, External Interrupt, 
Reset, and Code Security/Debugging */

#define SCB_BASE_ADDR	0xE01FC000

/* Memory Accelerator Module (MAM) */
#define MAMCR          (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x000))
#define MAMTIM         (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x004))
#define MEMMAP         (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x040))

/* Phase Locked Loop (PLL) */
#define PLLCON(port)   (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x080+0x20*port))
#define PLLCFG(port)   (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x084+0x20*port))
#define PLLSTAT(port)  (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x088+0x20*port))
#define PLLFEED(port)  (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x08C+0x20*port))
#define PLL0CON       PLLCON(0)
#define PLL0CFG       PLLCFG(0)
#define PLL0STAT      PLLSTAT(0)
#define PLL0FEED      PLLFEED(0)
#define PLL1CON       PLLCON(1)
#define PLL1CFG       PLLCFG(1)
#define PLL1STAT      PLLSTAT(1)
#define PLL1FEED      PLLFEED(1)

/* Power Control */
#define PCON           (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x0C0))
#define PCONP          (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x0C4))

/* VPB Divider */
#define VPBDIV         (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x100))

/* External Interrupts */
#define EXTINT         (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x140))
#define INTWAKE        (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x144))
#define EXTMODE        (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x148))
#define EXTPOLAR       (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x14C))

/* Reset */
#define RSIR           (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x180))

/* System Controls and Status */
#define SCS            (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x1A0))	

/* Timer */
#define TMR0_BASE_ADDR		0xE0004000
#define TMR1_BASE_ADDR		0xE0008000
#define TMR_BASE_DELTA (TMR1_BASE_ADDR-TMR0_BASE_ADDR)
#define TIR(port)           (*(volatile unsigned int *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x00))
#define TTCR(port)          (*(volatile unsigned int *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x04))
#define TTC(port)           (*(volatile unsigned int *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x08))
#define TPR(port)           (*(volatile unsigned int *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x0C))
#define TPC(port)           (*(volatile unsigned int *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x10))
#define TMCR(port)          (*(volatile unsigned int *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x14))
#define TMR(port,channel)   (*(volatile unsigned int *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x18+(channel)*4))
#define TMR0(port)          TMR(port,0)
#define TMR1(port)          TMR(port,1)
#define TMR2(port)          TMR(port,2)
#define TMR3(port)          TMR(port,3)
#define TCCR(port)          (*(volatile unsigned int *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x28))
#define TCR(port,channel)   (*(volatile unsigned int *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x2C+(channel)*4))
#define TCR0(port)          TCR(port,0)
#define TCR1(port)          TCR(port,1)
#define TCR2(port)          TCR(port,2)
#define TCR3(port)          TCR(port,3)
#define TEMR(port)          (*(volatile unsigned int *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x3C))
#define TCTCR(port)         (*(volatile unsigned int *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x70))

/* Timer 0 */
#define T0IR     TIR(0)
#define T0TCR    TTCR(0)
#define T0TC     TTC(0)
#define T0PR     TPR(0)
#define T0PC     TPC(0)
#define T0MCR    TMCR(0)
#define T0MR0    TMR(0,0)
#define T0MR1    TMR(0,1)
#define T0MR2    TMR(0,2)
#define T0MR3    TMR(0,3)
#define T0CCR    TCCR(0)
#define T0CR0    TCR(0,0)
#define T0CR1    TCR(0,1)
#define T0CR2    TCR(0,2)
#define T0CR3    TCR(0,3)
#define T0EMR    TEMR(0)
#define T0CTCR   TCTCR(0)

/* Timer 1 */
#define T1IR     TIR(1)
#define T1TCR    TTCR(1)
#define T1TC     TTC(1)
#define T1PR     TPR(1)
#define T1PC     TPC(1)
#define T1MCR    TMCR(1)
#define T1MR0    TMR(1,0)
#define T1MR1    TMR(1,1)
#define T1MR2    TMR(1,2)
#define T1MR3    TMR(1,3)
#define T1CCR    TCCR(1)
#define T1CR0    TCR(1,0)
#define T1CR1    TCR(1,1)
#define T1CR2    TCR(1,2)
#define T1CR3    TCR(1,3)
#define T1EMR    TEMR(1)
#define T1CTCR   TCTCR(1)

/* Pulse Width Modulator (PWM) */
#define PWM_BASE_ADDR		0xE0014000
#define PWMIR          (*(volatile unsigned int *)(PWM_BASE_ADDR + 0x00))
#define PWMTCR         (*(volatile unsigned int *)(PWM_BASE_ADDR + 0x04))
#define PWMTC          (*(volatile unsigned int *)(PWM_BASE_ADDR + 0x08))
#define PWMPR          (*(volatile unsigned int *)(PWM_BASE_ADDR + 0x0C))
#define PWMPC          (*(volatile unsigned int *)(PWM_BASE_ADDR + 0x10))
#define PWMMCR         (*(volatile unsigned int *)(PWM_BASE_ADDR + 0x14))
//The following uses a GCC extension ({ })
#define PWMMR(channel) (*(volatile unsigned int *)(PWM_BASE_ADDR + 0x18+({__typeof__ (channel) channel_=(channel);channel_*4+((channel_>3)?0x18:0);})))
#define PWMMR0         PWMMR(0)
#define PWMMR1         PWMMR(1)
#define PWMMR2         PWMMR(2)
#define PWMMR3         PWMMR(3)
#define PWMMR4         PWMMR(4)
#define PWMMR5         PWMMR(5)
#define PWMMR6         PWMMR(6)
#define PWMEMR         (*(volatile unsigned int *)(PWM_BASE_ADDR + 0x3C))
#define PWMPCR         (*(volatile unsigned int *)(PWM_BASE_ADDR + 0x4C))
#define PWMLER         (*(volatile unsigned int *)(PWM_BASE_ADDR + 0x50))

/* Universal Asynchronous Receiver Transmitter */
#define UART0_BASE_ADDR		0xE000C000
#define UART1_BASE_ADDR		0xE0010000
#define UART_BASE_DELTA (UART1_BASE_ADDR-UART0_BASE_ADDR)
#define URBR(port) (*(volatile unsigned int *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x00))
#define UTHR(port) (*(volatile unsigned int *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x00))
#define UDLL(port) (*(volatile unsigned int *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x00))
#define UDLM(port) (*(volatile unsigned int *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x04))
#define UIER(port) (*(volatile unsigned int *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x04))
#define UIIR(port) (*(volatile unsigned int *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x08))
#define UFCR(port) (*(volatile unsigned int *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x08))
#define ULCR(port) (*(volatile unsigned int *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x0C))
#define UMCR(port) (*(volatile unsigned int *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x10))
#define ULSR(port) (*(volatile unsigned int *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x14))
#define UMSR(port) (*(volatile unsigned int *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x18))
#define USCR(port) (*(volatile unsigned int *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x1C))
#define UACR(port) (*(volatile unsigned int *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x20))
#define UFDR(port) (*(volatile unsigned int *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x28))
#define UTER(port) (*(volatile unsigned int *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x30))

/* Universal Asynchronous Receiver Transmitter 0 (UART0) */
#define U0RBR URBR(0)
#define U0THR UTHR(0)
#define U0DLL UDLL(0)
#define U0DLM UDLM(0)
#define U0IER UIER(0)
#define U0IIR UIIR(0)
#define U0FCR UFCR(0)
#define U0LCR ULCR(0)
#define U0MCR UMCR(0)
#define U0LSR ULSR(0)
#define U0MSR UMSR(0)
#define U0SCR USCR(0)
#define U0ACR UACR(0)
#define U0FDR UFDR(0)
#define U0TER UTER(0)

/* Universal Asynchronous Receiver Transmitter 1 (UART1) */
#define U1RBR URBR(1)
#define U1THR UTHR(1)
#define U1DLL UDLL(1)
#define U1DLM UDLM(1)
#define U1IER UIER(1)
#define U1IIR UIIR(1)
#define U1FCR UFCR(1)
#define U1LCR ULCR(1)
#define U1MCR UMCR(1)
#define U1LSR ULSR(1)
#define U1MSR UMSR(1)
#define U1SCR USCR(1)
#define U1ACR UACR(1)
#define U1FDR UFDR(1)
#define U1TER UTER(1)

/* I2C Interface */
#define I2C0_BASE_ADDR		0xE001C000
#define I2C1_BASE_ADDR		0xE005C000
#define I2C_BASE_DELTA (I2C1_BASE_ADDR-I2C0_BASE_ADDR)
#define I2CCONSET(port) (*(volatile unsigned int *)(I2C0_BASE_ADDR+(port)*I2C_BASE_DELTA + 0x00))
#define I2CSTAT(port)   (*(volatile unsigned int *)(I2C0_BASE_ADDR+(port)*I2C_BASE_DELTA + 0x04))
#define I2CDAT(port)    (*(volatile unsigned int *)(I2C0_BASE_ADDR+(port)*I2C_BASE_DELTA + 0x08))
#define I2CADR(port)    (*(volatile unsigned int *)(I2C0_BASE_ADDR+(port)*I2C_BASE_DELTA + 0x0C))
#define I2CSCLH(port)   (*(volatile unsigned int *)(I2C0_BASE_ADDR+(port)*I2C_BASE_DELTA + 0x10))
#define I2CSCLL(port)   (*(volatile unsigned int *)(I2C0_BASE_ADDR+(port)*I2C_BASE_DELTA + 0x14))
#define I2CCONCLR(port) (*(volatile unsigned int *)(I2C0_BASE_ADDR+(port)*I2C_BASE_DELTA + 0x18))

/* I2C Interface 0 */
#define I2C0CONSET      I2CCONSET(0)
#define I2C0STAT        I2CSTAT(0)
#define I2C0DAT         I2CDAT(0)
#define I2C0ADR         I2CADR(0)
#define I2C0SCLH        I2CSCLH(0)
#define I2C0SCLL        I2CSCLL(0)
#define I2C0CONCLR      I2CCONCLR(0)

/* I2C Interface 1 */
#define I2C1CONSET      I2CCONSET(1)
#define I2C1STAT        I2CSTAT(1)
#define I2C1DAT         I2CDAT(1)
#define I2C1ADR         I2CADR(1)
#define I2C1SCLH        I2CSCLH(1)
#define I2C1SCLL        I2CSCLL(1)
#define I2C1CONCLR      I2CCONCLR(1)

/* SPI0 (Serial Peripheral Interface 0) */
#define SPI0_BASE_ADDR		0xE0020000
#define S0SPCR         (*(volatile unsigned int *)(SPI0_BASE_ADDR + 0x00))
#define S0SPSR         (*(volatile unsigned int *)(SPI0_BASE_ADDR + 0x04))
#define S0SPDR         (*(volatile unsigned int *)(SPI0_BASE_ADDR + 0x08))
#define S0SPCCR        (*(volatile unsigned int *)(SPI0_BASE_ADDR + 0x0C))
#define S0SPINT        (*(volatile unsigned int *)(SPI0_BASE_ADDR + 0x1C))

/* SSP Controller (usable as SPI1 but different map) */
#define SSP_BASE_ADDR		0xE0068000
#define SSPCR0         (*(volatile unsigned int * )(SSP_BASE_ADDR + 0x00))
#define SSPCR1         (*(volatile unsigned int * )(SSP_BASE_ADDR + 0x04))
#define SSPDR          (*(volatile unsigned int * )(SSP_BASE_ADDR + 0x08))
#define SSPSR          (*(volatile unsigned int * )(SSP_BASE_ADDR + 0x0C))
#define SSPCPSR        (*(volatile unsigned int * )(SSP_BASE_ADDR + 0x10))
#define SSPIMSC        (*(volatile unsigned int * )(SSP_BASE_ADDR + 0x14))
#define SSPRIS         (*(volatile unsigned int * )(SSP_BASE_ADDR + 0x18))
#define SSPMIS         (*(volatile unsigned int * )(SSP_BASE_ADDR + 0x1C))
#define SSPICR         (*(volatile unsigned int * )(SSP_BASE_ADDR + 0x20))

/* Real Time Clock */
#define RTC_BASE_ADDR		0xE0024000
#define ILR            (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x00))
#define CTC            (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x04))
#define CCR            (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x08))
#define CIIR           (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x0C))
#define AMR            (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x10))
#define CTIME0         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x14))
#define CTIME1         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x18))
#define CTIME2         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x1C))
#define RTCSEC         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x20))
#define RTCMIN         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x24))
#define RTCHOUR        (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x28))
#define RTCDOM         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x2C))
#define RTCDOW         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x30))
#define RTCDOY         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x34))
#define RTCMONTH       (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x38))
#define RTCYEAR        (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x3C))
#define ALSEC          (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x60))
#define ALMIN          (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x64))
#define ALHOUR         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x68))
#define ALDOM          (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x6C))
#define ALDOW          (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x70))
#define ALDOY          (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x74))
#define ALMON          (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x78))
#define ALYEAR         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x7C))
#define PREINT         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x80))
#define PREFRAC        (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x84))

/* A/D Converter 0 (AD0) */
#define AD0_BASE_ADDR		0xE0034000
#define AD0CR          (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x00))
#define AD0GDR         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x04))
#define AD0STAT        (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x30))
#define AD0INTEN       (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x0C))
#define AD0DR0         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x10))
#define AD0DR1         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x14))
#define AD0DR2         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x18))
#define AD0DR3         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x1C))
#define AD0DR4         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x20))
#define AD0DR5         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x24))
#define AD0DR6         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x28))
#define AD0DR7         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x2C))

#define ADGSR          (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x08))
/* A/D Converter 1 (AD1) */
#define AD1_BASE_ADDR		0xE0060000
#define AD1CR          (*(volatile unsigned int *)(AD1_BASE_ADDR + 0x00))
#define AD1GDR         (*(volatile unsigned int *)(AD1_BASE_ADDR + 0x04))
#define AD1STAT        (*(volatile unsigned int *)(AD1_BASE_ADDR + 0x30))
#define AD1INTEN       (*(volatile unsigned int *)(AD1_BASE_ADDR + 0x0C))
#define AD1DR0         (*(volatile unsigned int *)(AD1_BASE_ADDR + 0x10))
#define AD1DR1         (*(volatile unsigned int *)(AD1_BASE_ADDR + 0x14))
#define AD1DR2         (*(volatile unsigned int *)(AD1_BASE_ADDR + 0x18))
#define AD1DR3         (*(volatile unsigned int *)(AD1_BASE_ADDR + 0x1C))
#define AD1DR4         (*(volatile unsigned int *)(AD1_BASE_ADDR + 0x20))
#define AD1DR5         (*(volatile unsigned int *)(AD1_BASE_ADDR + 0x24))
#define AD1DR6         (*(volatile unsigned int *)(AD1_BASE_ADDR + 0x28))
#define AD1DR7         (*(volatile unsigned int *)(AD1_BASE_ADDR + 0x2C))

/* D/A Converter */
#define DAC_BASE_ADDR		0xE006C000
#define DACR           (*(volatile unsigned int *)(DAC_BASE_ADDR + 0x00))

/* Hardware ID */
//We write hardware identifiers here, in its own memory page. 
#define HW_ID_BASE_ADDR		0x0007C000
//The first describes the hardware type:
// 0 for Logomatic
// 1 for Loginator
// 2 for Rocketometer
// remainder to 0xFFFFFFFE are reserved
// 0xFFFFFFFF is unknown (memory block never written to)
//The second word is a serial number, unique to hardware type, so there
//can be a Loginator 0 and Rocketometer 0, but no two rocketometers are
//both labeled 0.
#define HW_TYPE        (*(volatile unsigned int *)(HW_ID_BASE_ADDR + 0x00))
#define HW_SERIAL      (*(volatile unsigned int *)(HW_ID_BASE_ADDR + 0x04))

/* Watchdog */
#define WDG_BASE_ADDR		0xE0000000
#define WDMOD          (*(volatile unsigned int *)(WDG_BASE_ADDR + 0x00))
#define WDTC           (*(volatile unsigned int *)(WDG_BASE_ADDR + 0x04))
#define WDFEED         (*(volatile unsigned int *)(WDG_BASE_ADDR + 0x08))
#define WDTV           (*(volatile unsigned int *)(WDG_BASE_ADDR + 0x0C))

/* USB Controller */
#define USB_BASE_ADDR		0xE0090000			/* USB Base Address */
/* Device Interrupt Registers */
#define USBDevIntSt     (*(volatile unsigned int *)(USB_BASE_ADDR + 0x00))
#define USBDevIntEn     (*(volatile unsigned int *)(USB_BASE_ADDR + 0x04))
#define USBDevIntClr    (*(volatile unsigned int *)(USB_BASE_ADDR + 0x08))
#define DEV_INT_SET     (*(volatile unsigned int *)(USB_BASE_ADDR + 0x0C))
#define USBDevIntPri    (*(volatile unsigned int *)(USB_BASE_ADDR + 0x2C))

/* Endpoint Interrupt Registers */
#define USBEpIntSt      (*(volatile unsigned int *)(USB_BASE_ADDR + 0x30))
#define USBEpIntEn      (*(volatile unsigned int *)(USB_BASE_ADDR + 0x34))
#define USBEpIntClr     (*(volatile unsigned int *)(USB_BASE_ADDR + 0x38))
#define EP_INT_SET      (*(volatile unsigned int *)(USB_BASE_ADDR + 0x3C))
#define USBEpIntPri     (*(volatile unsigned int *)(USB_BASE_ADDR + 0x40))

/* Endpoint Realization Registers */
#define USBReEp         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x44))
#define USBEpInd        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x48))
#define USBMaxPSize     (*(volatile unsigned int *)(USB_BASE_ADDR + 0x4C))

/* Command Reagisters */
#define USBCmdCode      (*(volatile unsigned int *)(USB_BASE_ADDR + 0x10))
#define USBCmdData      (*(volatile unsigned int *)(USB_BASE_ADDR + 0x14))

/* Data Transfer Registers */
#define USBRxData       (*(volatile unsigned int *)(USB_BASE_ADDR + 0x18))
#define USBTxData       (*(volatile unsigned int *)(USB_BASE_ADDR + 0x1C))
#define USBRxPLen       (*(volatile unsigned int *)(USB_BASE_ADDR + 0x20))
#define USBTxPLen       (*(volatile unsigned int *)(USB_BASE_ADDR + 0x24))
#define USBCtrl         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x28))

/* DMA Registers */
#define DMA_REQ_STAT        (*((volatile unsigned int *)USB_BASE_ADDR + 0x50))
#define DMA_REQ_CLR         (*((volatile unsigned int *)USB_BASE_ADDR + 0x54))
#define DMA_REQ_SET         (*((volatile unsigned int *)USB_BASE_ADDR + 0x58))
#define UDCA_HEAD           (*((volatile unsigned int *)USB_BASE_ADDR + 0x80))
#define EP_DMA_STAT         (*((volatile unsigned int *)USB_BASE_ADDR + 0x84))
#define EP_DMA_EN           (*((volatile unsigned int *)USB_BASE_ADDR + 0x88))
#define EP_DMA_DIS          (*((volatile unsigned int *)USB_BASE_ADDR + 0x8C))
#define DMA_INT_STAT        (*((volatile unsigned int *)USB_BASE_ADDR + 0x90))
#define DMA_INT_EN          (*((volatile unsigned int *)USB_BASE_ADDR + 0x94))
#define EOT_INT_STAT        (*((volatile unsigned int *)USB_BASE_ADDR + 0xA0))
#define EOT_INT_CLR         (*((volatile unsigned int *)USB_BASE_ADDR + 0xA4))
#define EOT_INT_SET         (*((volatile unsigned int *)USB_BASE_ADDR + 0xA8))
#define NDD_REQ_INT_STAT    (*((volatile unsigned int *)USB_BASE_ADDR + 0xAC))
#define NDD_REQ_INT_CLR     (*((volatile unsigned int *)USB_BASE_ADDR + 0xB0))
#define NDD_REQ_INT_SET     (*((volatile unsigned int *)USB_BASE_ADDR + 0xB4))
#define SYS_ERR_INT_STAT    (*((volatile unsigned int *)USB_BASE_ADDR + 0xB8))
#define SYS_ERR_INT_CLR     (*((volatile unsigned int *)USB_BASE_ADDR + 0xBC))
#define SYS_ERR_INT_SET     (*((volatile unsigned int *)USB_BASE_ADDR + 0xC0))    
#define MODULE_ID           (*((volatile unsigned int *)USB_BASE_ADDR + 0xFC))

#endif  // __LPC214x_H


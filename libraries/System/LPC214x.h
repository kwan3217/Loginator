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
 *                           All registers use "volatile unsigned long". 
******************************************************************************/

#ifndef __LPC214x_H
#define __LPC214x_H

/* Vectored Interrupt Controller (VIC) */
#define VIC_BASE_ADDR	0xFFFFF000

#define VICIRQStatus   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x000))
#define VICFIQStatus   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x004))
#define VICRawIntr     (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x008))
#define VICIntSelect   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x00C))
#define VICIntEnable   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x010))
#define VICIntEnClr    (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x014))
#define VICSoftInt     (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x018))
#define VICSoftIntClr  (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x01C))
#define VICProtection  (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x020))
#define VICVectAddr    (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x030))
#define VICDefVectAddr (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x034))
#define VICVectAddr0   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x100))
#define VICVectAddr1   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x104))
#define VICVectAddr2   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x108))
#define VICVectAddr3   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x10C))
#define VICVectAddr4   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x110))
#define VICVectAddr5   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x114))
#define VICVectAddr6   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x118))
#define VICVectAddr7   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x11C))
#define VICVectAddr8   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x120))
#define VICVectAddr9   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x124))
#define VICVectAddr10  (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x128))
#define VICVectAddr11  (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x12C))
#define VICVectAddr12  (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x130))
#define VICVectAddr13  (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x134))
#define VICVectAddr14  (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x138))
#define VICVectAddr15  (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x13C))
#define VICVectAddrSlot(slot) (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x100+((slot)*4)))
#define VICVectCntl0   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x200))
#define VICVectCntl1   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x204))
#define VICVectCntl2   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x208))
#define VICVectCntl3   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x20C))
#define VICVectCntl4   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x210))
#define VICVectCntl5   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x214))
#define VICVectCntl6   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x218))
#define VICVectCntl7   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x21C))
#define VICVectCntl8   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x220))
#define VICVectCntl9   (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x224))
#define VICVectCntl10  (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x228))
#define VICVectCntl11  (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x22C))
#define VICVectCntl12  (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x230))
#define VICVectCntl13  (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x234))
#define VICVectCntl14  (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x238))
#define VICVectCntl15  (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x23C))
#define VICVectCntlSlot(slot) (*(volatile unsigned long *)(VIC_BASE_ADDR + 0x200+((slot)*4)))

/* Pin Connect Block */
#define PINSEL_BASE_ADDR	0xE002C000
#define PINSEL0        (*(volatile unsigned long *)(PINSEL_BASE_ADDR + 0x00))
#define PINSEL1        (*(volatile unsigned long *)(PINSEL_BASE_ADDR + 0x04))
#define PINSEL2        (*(volatile unsigned long *)(PINSEL_BASE_ADDR + 0x14))

/* General Purpose Input/Output (GPIO) */
#define GPIO_BASE_ADDR		0xE0028000
#define IOPIN0         (*(volatile unsigned long *)(GPIO_BASE_ADDR + 0x00))
#define IOSET0         (*(volatile unsigned long *)(GPIO_BASE_ADDR + 0x04))
#define IODIR0         (*(volatile unsigned long *)(GPIO_BASE_ADDR + 0x08))
#define IOCLR0         (*(volatile unsigned long *)(GPIO_BASE_ADDR + 0x0C))
#define IOPIN1         (*(volatile unsigned long *)(GPIO_BASE_ADDR + 0x10))
#define IOSET1         (*(volatile unsigned long *)(GPIO_BASE_ADDR + 0x14))
#define IODIR1         (*(volatile unsigned long *)(GPIO_BASE_ADDR + 0x18))
#define IOCLR1         (*(volatile unsigned long *)(GPIO_BASE_ADDR + 0x1C))

/* Fast I/O setup */
#define FIO_BASE_ADDR		0x3FFFC000
#define FIO0DIR        (*(volatile unsigned long *)(FIO_BASE_ADDR + 0x00)) 
#define FIO0MASK       (*(volatile unsigned long *)(FIO_BASE_ADDR + 0x10))
#define FIO0PIN        (*(volatile unsigned long *)(FIO_BASE_ADDR + 0x14))
#define FIO0SET        (*(volatile unsigned long *)(FIO_BASE_ADDR + 0x18))
#define FIO0CLR        (*(volatile unsigned long *)(FIO_BASE_ADDR + 0x1C))
#define FIO1DIR        (*(volatile unsigned long *)(FIO_BASE_ADDR + 0x20)) 
#define FIO1MASK       (*(volatile unsigned long *)(FIO_BASE_ADDR + 0x30))
#define FIO1PIN        (*(volatile unsigned long *)(FIO_BASE_ADDR + 0x34))
#define FIO1SET        (*(volatile unsigned long *)(FIO_BASE_ADDR + 0x38))
#define FIO1CLR        (*(volatile unsigned long *)(FIO_BASE_ADDR + 0x3C))

/* System Control Block(SCB) modules include Memory Accelerator Module,
Phase Locked Loop, VPB divider, Power Control, External Interrupt, 
Reset, and Code Security/Debugging */

#define SCB_BASE_ADDR	0xE01FC000

/* Memory Accelerator Module (MAM) */
#define MAMCR          (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x000))
#define MAMTIM         (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x004))
#define MEMMAP         (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x040))

/* Phase Locked Loop (PLL) */
#define PLLCON         (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x080))
#define PLLCFG         (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x084))
#define PLLSTAT        (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x088))
#define PLLFEED        (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x08C))

/* PLL48 Registers */
#define PLL48CON       (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x0A0))
#define PLL48CFG       (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x0A4))
#define PLL48STAT      (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x0A8))
#define PLL48FEED      (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x0AC))

/* Power Control */
#define PCON           (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x0C0))
#define PCONP          (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x0C4))

/* VPB Divider */
#define VPBDIV         (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x100))

/* External Interrupts */
#define EXTINT         (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x140))
#define INTWAKE        (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x144))
#define EXTMODE        (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x148))
#define EXTPOLAR       (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x14C))

/* Reset */
#define RSIR           (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x180))

/* System Controls and Status */
#define SCS            (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x1A0))	

/* Timer */
#define TMR0_BASE_ADDR		0xE0004000
#define TMR1_BASE_ADDR		0xE0008000
#define TMR_BASE_DELTA (TMR1_BASE_ADDR-TMR0_BASE_ADDR)
#define TIR(port)           (*(volatile unsigned long *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x00))
#define TTCR(port)          (*(volatile unsigned long *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x04))
#define TTC(port)           (*(volatile unsigned long *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x08))
#define TPR(port)           (*(volatile unsigned long *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x0C))
#define TPC(port)           (*(volatile unsigned long *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x10))
#define TMCR(port)          (*(volatile unsigned long *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x14))
#define TMR(port,channel)   (*(volatile unsigned long *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x18+(channel)*4))
#define TMR0(port)          TMR(port,0)
#define TMR1(port)          TMR(port,1)
#define TMR2(port)          TMR(port,2)
#define TMR3(port)          TMR(port,3)
#define TCCR(port)          (*(volatile unsigned long *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x28))
#define TCR(port,channel)   (*(volatile unsigned long *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x2C+(channel)*4))
#define TCR0(port)          TCR(port,0)
#define TCR1(port)          TCR(port,1)
#define TCR2(port)          TCR(port,2)
#define TCR3(port)          TCR(port,3)
#define TEMR(port)          (*(volatile unsigned long *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x3C))
#define TCTCR(port)         (*(volatile unsigned long *)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x70))

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
#define PWMIR          (*(volatile unsigned long *)(PWM_BASE_ADDR + 0x00))
#define PWMTCR         (*(volatile unsigned long *)(PWM_BASE_ADDR + 0x04))
#define PWMTC          (*(volatile unsigned long *)(PWM_BASE_ADDR + 0x08))
#define PWMPR          (*(volatile unsigned long *)(PWM_BASE_ADDR + 0x0C))
#define PWMPC          (*(volatile unsigned long *)(PWM_BASE_ADDR + 0x10))
#define PWMMCR         (*(volatile unsigned long *)(PWM_BASE_ADDR + 0x14))
//The following uses a GCC extension ({ })
#define PWMMR(channel) (*(volatile unsigned long *)(PWM_BASE_ADDR + 0x18+({__typeof__ (channel) channel_=(channel);channel_*4+((channel_>3)?0x18:0);})))
#define PWMMR0         PWMMR(0)
#define PWMMR1         PWMMR(1)
#define PWMMR2         PWMMR(2)
#define PWMMR3         PWMMR(3)
#define PWMMR4         PWMMR(4)
#define PWMMR5         PWMMR(5)
#define PWMMR6         PWMMR(6)
#define PWMEMR         (*(volatile unsigned long *)(PWM_BASE_ADDR + 0x3C))
#define PWMPCR         (*(volatile unsigned long *)(PWM_BASE_ADDR + 0x4C))
#define PWMLER         (*(volatile unsigned long *)(PWM_BASE_ADDR + 0x50))

/* Universal Asynchronous Receiver Transmitter */
#define UART0_BASE_ADDR		0xE000C000
#define UART1_BASE_ADDR		0xE0010000
#define UART_BASE_DELTA (UART1_BASE_ADDR-UART0_BASE_ADDR)
#define URBR(port) (*(volatile unsigned long *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x00))
#define UTHR(port) (*(volatile unsigned long *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x00))
#define UDLL(port) (*(volatile unsigned long *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x00))
#define UDLM(port) (*(volatile unsigned long *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x04))
#define UIER(port) (*(volatile unsigned long *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x04))
#define UIIR(port) (*(volatile unsigned long *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x08))
#define UFCR(port) (*(volatile unsigned long *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x08))
#define ULCR(port) (*(volatile unsigned long *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x0C))
#define UMCR(port) (*(volatile unsigned long *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x10))
#define ULSR(port) (*(volatile unsigned long *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x14))
#define UMSR(port) (*(volatile unsigned long *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x18))
#define USCR(port) (*(volatile unsigned long *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x1C))
#define UACR(port) (*(volatile unsigned long *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x20))
#define UFDR(port) (*(volatile unsigned long *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x28))
#define UTER(port) (*(volatile unsigned long *)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x30))

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
#define I2CCONSET(port) (*(volatile unsigned long *)(I2C0_BASE_ADDR+(port)*I2C_BASE_DELTA + 0x00))
#define I2CSTAT(port)   (*(volatile unsigned long *)(I2C0_BASE_ADDR+(port)*I2C_BASE_DELTA + 0x04))
#define I2CDAT(port)    (*(volatile unsigned long *)(I2C0_BASE_ADDR+(port)*I2C_BASE_DELTA + 0x08))
#define I2CADR(port)    (*(volatile unsigned long *)(I2C0_BASE_ADDR+(port)*I2C_BASE_DELTA + 0x0C))
#define I2CSCLH(port)   (*(volatile unsigned long *)(I2C0_BASE_ADDR+(port)*I2C_BASE_DELTA + 0x10))
#define I2CSCLL(port)   (*(volatile unsigned long *)(I2C0_BASE_ADDR+(port)*I2C_BASE_DELTA + 0x14))
#define I2CCONCLR(port) (*(volatile unsigned long *)(I2C0_BASE_ADDR+(port)*I2C_BASE_DELTA + 0x18))

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
#define S0SPCR         (*(volatile unsigned long *)(SPI0_BASE_ADDR + 0x00))
#define S0SPSR         (*(volatile unsigned long *)(SPI0_BASE_ADDR + 0x04))
#define S0SPDR         (*(volatile unsigned long *)(SPI0_BASE_ADDR + 0x08))
#define S0SPCCR        (*(volatile unsigned long *)(SPI0_BASE_ADDR + 0x0C))
#define S0SPINT        (*(volatile unsigned long *)(SPI0_BASE_ADDR + 0x1C))

/* SSP Controller (usable as SPI1 but different map) */
#define SSP_BASE_ADDR		0xE0068000
#define SSPCR0         (*(volatile unsigned long * )(SSP_BASE_ADDR + 0x00))
#define SSPCR1         (*(volatile unsigned long * )(SSP_BASE_ADDR + 0x04))
#define SSPDR          (*(volatile unsigned long * )(SSP_BASE_ADDR + 0x08))
#define SSPSR          (*(volatile unsigned long * )(SSP_BASE_ADDR + 0x0C))
#define SSPCPSR        (*(volatile unsigned long * )(SSP_BASE_ADDR + 0x10))
#define SSPIMSC        (*(volatile unsigned long * )(SSP_BASE_ADDR + 0x14))
#define SSPRIS         (*(volatile unsigned long * )(SSP_BASE_ADDR + 0x18))
#define SSPMIS         (*(volatile unsigned long * )(SSP_BASE_ADDR + 0x1C))
#define SSPICR         (*(volatile unsigned long * )(SSP_BASE_ADDR + 0x20))

/* Real Time Clock */
#define RTC_BASE_ADDR		0xE0024000
#define ILR            (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x00))
#define CTC            (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x04))
#define CCR            (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x08))
#define CIIR           (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x0C))
#define AMR            (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x10))
#define CTIME0         (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x14))
#define CTIME1         (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x18))
#define CTIME2         (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x1C))
#define SEC            (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x20))
#define MIN            (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x24))
#define HOUR           (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x28))
#define DOM            (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x2C))
#define DOW            (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x30))
#define DOY            (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x34))
#define MONTH          (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x38))
#define YEAR           (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x3C))
#define ALSEC          (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x60))
#define ALMIN          (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x64))
#define ALHOUR         (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x68))
#define ALDOM          (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x6C))
#define ALDOW          (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x70))
#define ALDOY          (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x74))
#define ALMON          (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x78))
#define ALYEAR         (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x7C))
#define PREINT         (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x80))
#define PREFRAC        (*(volatile unsigned long *)(RTC_BASE_ADDR + 0x84))

/* A/D Converter 0 (AD0) */
#define AD0_BASE_ADDR		0xE0034000
#define AD0CR          (*(volatile unsigned long *)(AD0_BASE_ADDR + 0x00))
#define AD0GDR         (*(volatile unsigned long *)(AD0_BASE_ADDR + 0x04))
#define AD0STAT        (*(volatile unsigned long *)(AD0_BASE_ADDR + 0x30))
#define AD0INTEN       (*(volatile unsigned long *)(AD0_BASE_ADDR + 0x0C))
#define AD0DR0         (*(volatile unsigned long *)(AD0_BASE_ADDR + 0x10))
#define AD0DR1         (*(volatile unsigned long *)(AD0_BASE_ADDR + 0x14))
#define AD0DR2         (*(volatile unsigned long *)(AD0_BASE_ADDR + 0x18))
#define AD0DR3         (*(volatile unsigned long *)(AD0_BASE_ADDR + 0x1C))
#define AD0DR4         (*(volatile unsigned long *)(AD0_BASE_ADDR + 0x20))
#define AD0DR5         (*(volatile unsigned long *)(AD0_BASE_ADDR + 0x24))
#define AD0DR6         (*(volatile unsigned long *)(AD0_BASE_ADDR + 0x28))
#define AD0DR7         (*(volatile unsigned long *)(AD0_BASE_ADDR + 0x2C))

#define ADGSR          (*(volatile unsigned long *)(AD0_BASE_ADDR + 0x08))
/* A/D Converter 1 (AD1) */
#define AD1_BASE_ADDR		0xE0060000
#define AD1CR          (*(volatile unsigned long *)(AD1_BASE_ADDR + 0x00))
#define AD1GDR         (*(volatile unsigned long *)(AD1_BASE_ADDR + 0x04))
#define AD1STAT        (*(volatile unsigned long *)(AD1_BASE_ADDR + 0x30))
#define AD1INTEN       (*(volatile unsigned long *)(AD1_BASE_ADDR + 0x0C))
#define AD1DR0         (*(volatile unsigned long *)(AD1_BASE_ADDR + 0x10))
#define AD1DR1         (*(volatile unsigned long *)(AD1_BASE_ADDR + 0x14))
#define AD1DR2         (*(volatile unsigned long *)(AD1_BASE_ADDR + 0x18))
#define AD1DR3         (*(volatile unsigned long *)(AD1_BASE_ADDR + 0x1C))
#define AD1DR4         (*(volatile unsigned long *)(AD1_BASE_ADDR + 0x20))
#define AD1DR5         (*(volatile unsigned long *)(AD1_BASE_ADDR + 0x24))
#define AD1DR6         (*(volatile unsigned long *)(AD1_BASE_ADDR + 0x28))
#define AD1DR7         (*(volatile unsigned long *)(AD1_BASE_ADDR + 0x2C))

/* D/A Converter */
#define DAC_BASE_ADDR		0xE006C000
#define DACR           (*(volatile unsigned long *)(DAC_BASE_ADDR + 0x00))

/* Watchdog */
#define WDG_BASE_ADDR		0xE0000000
#define WDMOD          (*(volatile unsigned long *)(WDG_BASE_ADDR + 0x00))
#define WDTC           (*(volatile unsigned long *)(WDG_BASE_ADDR + 0x04))
#define WDFEED         (*(volatile unsigned long *)(WDG_BASE_ADDR + 0x08))
#define WDTV           (*(volatile unsigned long *)(WDG_BASE_ADDR + 0x0C))

/* USB Controller */
#define USB_BASE_ADDR		0xE0090000			/* USB Base Address */
/* Device Interrupt Registers */
#define DEV_INT_STAT    (*(volatile unsigned long *)(USB_BASE_ADDR + 0x00))
#define DEV_INT_EN      (*(volatile unsigned long *)(USB_BASE_ADDR + 0x04))
#define DEV_INT_CLR     (*(volatile unsigned long *)(USB_BASE_ADDR + 0x08))
#define DEV_INT_SET     (*(volatile unsigned long *)(USB_BASE_ADDR + 0x0C))
#define DEV_INT_PRIO    (*(volatile unsigned long *)(USB_BASE_ADDR + 0x2C))

/* Endpoint Interrupt Registers */
#define EP_INT_STAT     (*(volatile unsigned long *)(USB_BASE_ADDR + 0x30))
#define EP_INT_EN       (*(volatile unsigned long *)(USB_BASE_ADDR + 0x34))
#define EP_INT_CLR      (*(volatile unsigned long *)(USB_BASE_ADDR + 0x38))
#define EP_INT_SET      (*(volatile unsigned long *)(USB_BASE_ADDR + 0x3C))
#define EP_INT_PRIO     (*(volatile unsigned long *)(USB_BASE_ADDR + 0x40))

/* Endpoint Realization Registers */
#define REALIZE_EP      (*(volatile unsigned long *)(USB_BASE_ADDR + 0x44))
#define EP_INDEX        (*(volatile unsigned long *)(USB_BASE_ADDR + 0x48))
#define MAXPACKET_SIZE  (*(volatile unsigned long *)(USB_BASE_ADDR + 0x4C))

/* Command Reagisters */
#define CMD_CODE        (*(volatile unsigned long *)(USB_BASE_ADDR + 0x10))
#define CMD_DATA        (*(volatile unsigned long *)(USB_BASE_ADDR + 0x14))

/* Data Transfer Registers */
#define RX_DATA         (*(volatile unsigned long *)(USB_BASE_ADDR + 0x18))
#define TX_DATA         (*(volatile unsigned long *)(USB_BASE_ADDR + 0x1C))
#define RX_PLENGTH      (*(volatile unsigned long *)(USB_BASE_ADDR + 0x20))
#define TX_PLENGTH      (*(volatile unsigned long *)(USB_BASE_ADDR + 0x24))
#define USB_CTRL        (*(volatile unsigned long *)(USB_BASE_ADDR + 0x28))

/* DMA Registers */
#define DMA_REQ_STAT        (*((volatile unsigned long *)USB_BASE_ADDR + 0x50))
#define DMA_REQ_CLR         (*((volatile unsigned long *)USB_BASE_ADDR + 0x54))
#define DMA_REQ_SET         (*((volatile unsigned long *)USB_BASE_ADDR + 0x58))
#define UDCA_HEAD           (*((volatile unsigned long *)USB_BASE_ADDR + 0x80))
#define EP_DMA_STAT         (*((volatile unsigned long *)USB_BASE_ADDR + 0x84))
#define EP_DMA_EN           (*((volatile unsigned long *)USB_BASE_ADDR + 0x88))
#define EP_DMA_DIS          (*((volatile unsigned long *)USB_BASE_ADDR + 0x8C))
#define DMA_INT_STAT        (*((volatile unsigned long *)USB_BASE_ADDR + 0x90))
#define DMA_INT_EN          (*((volatile unsigned long *)USB_BASE_ADDR + 0x94))
#define EOT_INT_STAT        (*((volatile unsigned long *)USB_BASE_ADDR + 0xA0))
#define EOT_INT_CLR         (*((volatile unsigned long *)USB_BASE_ADDR + 0xA4))
#define EOT_INT_SET         (*((volatile unsigned long *)USB_BASE_ADDR + 0xA8))
#define NDD_REQ_INT_STAT    (*((volatile unsigned long *)USB_BASE_ADDR + 0xAC))
#define NDD_REQ_INT_CLR     (*((volatile unsigned long *)USB_BASE_ADDR + 0xB0))
#define NDD_REQ_INT_SET     (*((volatile unsigned long *)USB_BASE_ADDR + 0xB4))
#define SYS_ERR_INT_STAT    (*((volatile unsigned long *)USB_BASE_ADDR + 0xB8))
#define SYS_ERR_INT_CLR     (*((volatile unsigned long *)USB_BASE_ADDR + 0xBC))
#define SYS_ERR_INT_SET     (*((volatile unsigned long *)USB_BASE_ADDR + 0xC0))    
#define MODULE_ID           (*((volatile unsigned long *)USB_BASE_ADDR + 0xFC))

#endif  // __LPC214x_H


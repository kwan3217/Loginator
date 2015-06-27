
/*
 * Copyright (c) 2006-2011 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef SD_RAW_CONFIG_H
#define SD_RAW_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \addtogroup sd_raw
 *
 * @{
 */
/**
 * \file
 * MMC/SD support configuration (license: GPLv2 or LGPLv2.1)
 */

/**
 * \ingroup sd_raw_config
 * Controls MMC/SD write support.
 *
 * Set to 1 to enable MMC/SD write support, set to 0 to disable it.
 */
#define SD_RAW_WRITE_SUPPORT 1

/**
 * \ingroup sd_raw_config
 * Controls MMC/SD write buffering.
 *
 * Set to 1 to buffer write accesses, set to 0 to disable it.
 *
 * \note This option has no effect when SD_RAW_WRITE_SUPPORT is 0.
 */
#define SD_RAW_WRITE_BUFFERING 1

/**
 * \ingroup sd_raw_config
 * Controls MMC/SD access buffering.
 * 
 * Set to 1 to save static RAM, but be aware that you will
 * lose performance.
 *
 * \note When SD_RAW_WRITE_SUPPORT is 1, SD_RAW_SAVE_RAM will
 *       be reset to 0.
 */
#define SD_RAW_SAVE_RAM 0

/**
 * \ingroup sd_raw_config
 * Controls support for SDHC cards.
 *
 * Set to 1 to support so-called SDHC memory cards, i.e. SD
 * cards with more than 2 gigabytes of memory.
 */
#define SD_RAW_SDHC 1

/**
 * @}
 */

#ifdef ROCKETOMETER
#define SS_PORT_0
#define SPI_SS_PIN	15

//SPI Chip Select Defines for SD Access
#ifdef SS_PORT_1
	#define	SPI_SS_IODIR	IODIR1
	#define	SPI_SS_IOCLR	IOCLR1
	#define	SPI_SS_IOSET	IOSET1  
	#define SPI_SS_IOPIN	IOPIN1
#endif
#ifdef	SS_PORT_0
	#define	SPI_SS_IODIR	IODIR0
	#define	SPI_SS_IOCLR	IOCLR0
	#define	SPI_SS_IOSET	IOSET0
	#define	SPI_SS_IOPIN	IOPIN0
#endif  

#define SPI_SCK_PIN    	4       
#define SPI_MISO_PIN   	5         
#define SPI_MOSI_PIN   	6        

#define SPI_PINSEL     		PINSEL0
#define SPI_SCK_FUNCBIT   	(SPI_SCK_PIN*2)
#define SPI_MISO_FUNCBIT  	(SPI_MISO_PIN*2)
#define SPI_MOSI_FUNCBIT  	(SPI_MOSI_PIN*2)

#define SPI_PRESCALE_REG  	S0SPCCR


/* defines for customisation of sd/mmc port access */
#define configure_pin_mosi() 	SPI_PINSEL &= ~(3 << SPI_MOSI_FUNCBIT); SPI_PINSEL |= (1 << SPI_MOSI_FUNCBIT)
#define configure_pin_sck() 	SPI_PINSEL &= ~(3 << SPI_SCK_FUNCBIT);  SPI_PINSEL |= (1 << SPI_SCK_FUNCBIT)
#define configure_pin_miso() 	SPI_PINSEL &= ~(3 << SPI_MISO_FUNCBIT); SPI_PINSEL |= (1 << SPI_MISO_FUNCBIT)
#define configure_pin_ss() 	SPI_SS_IODIR |= (1<<SPI_SS_PIN)

#define select_card() 				SPI_SS_IOCLR |= (1<<SPI_SS_PIN)
#define unselect_card() 			SPI_SS_IOSET |= (1<<SPI_SS_PIN)
#define configure_pin_available() 	SPI_SS_IODIR  &= ~(1<<SPI_SS_PIN)
#define configure_pin_locked() 		if(1)
#define get_pin_available() 		(!((SPI_SS_IOPIN&(1<<SPI_SS_PIN))>>SPI_SS_PIN))
#define get_pin_locked() (0)

#define SPI_INIT() SPI_PRESCALE_REG = 150;  /* Set frequency to 400kHz */ \
                   S0SPCR = 0x38

#define SPI_UPSHIFT()      /* switch to highest SPI frequency possible */ \
    S0SPCCR = 10; /* ~6MHz-- potentially can be faster */

#define SD_RAW_SEND_BYTE_GUTS     S0SPDR = b; \
     /* wait for byte to be shifted out */ \
    while(!(S0SPSR & 0x80));

#define SD_RAW_REC_BYTE_GUTS      /* send dummy data for receiving some */ \
    S0SPDR = 0xff; \
    while(!(S0SPSR & 0x80)); \
    return S0SPDR;


#else

/* defines for customisation of sd/mmc port access */
#if defined(__AVR_ATmega8__) || \
    defined(__AVR_ATmega48__) || \
    defined(__AVR_ATmega48P__) || \
    defined(__AVR_ATmega88__) || \
    defined(__AVR_ATmega88P__) || \
    defined(__AVR_ATmega168__) || \
    defined(__AVR_ATmega168P__) || \
    defined(__AVR_ATmega328P__)
    #define configure_pin_mosi() DDRB |= (1 << DDB3)
    #define configure_pin_sck() DDRB |= (1 << DDB5)
    #define configure_pin_ss() DDRB |= (1 << DDB2)
    #define configure_pin_miso() DDRB &= ~(1 << DDB4)

    #define select_card() PORTB &= ~(1 << PORTB2)
    #define unselect_card() PORTB |= (1 << PORTB2)
#elif defined(__AVR_ATmega16__) || \
      defined(__AVR_ATmega32__)
    #define configure_pin_mosi() DDRB |= (1 << DDB5)
    #define configure_pin_sck() DDRB |= (1 << DDB7)
    #define configure_pin_ss() DDRB |= (1 << DDB4)
    #define configure_pin_miso() DDRB &= ~(1 << DDB6)

    #define select_card() PORTB &= ~(1 << PORTB4)
    #define unselect_card() PORTB |= (1 << PORTB4)
#elif defined(__AVR_ATmega64__) || \
      defined(__AVR_ATmega128__) || \
      defined(__AVR_ATmega169__)
    #define configure_pin_mosi() DDRB |= (1 << DDB2)
    #define configure_pin_sck() DDRB |= (1 << DDB1)
    #define configure_pin_ss() DDRB |= (1 << DDB0)
    #define configure_pin_miso() DDRB &= ~(1 << DDB3)

    #define select_card() PORTB &= ~(1 << PORTB0)
    #define unselect_card() PORTB |= (1 << PORTB0)
#else
    #error "no sd/mmc pin mapping available!"
#endif

#define configure_pin_available() DDRC &= ~(1 << DDC4)
#define configure_pin_locked() DDRC &= ~(1 << DDC5)

#define get_pin_available() (PINC & (1 << PINC4))
#define get_pin_locked() (PINC & (1 << PINC5))

#define SPI_INIT()  \
    /* initialize SPI with lowest frequency; max. 400kHz during identification mode of card */ \
    SPCR = (0 << SPIE) | /* SPI Interrupt Enable */ \
           (1 << SPE)  | /* SPI Enable */ \
           (0 << DORD) | /* Data Order: MSB first */ \
           (1 << MSTR) | /* Master mode */ \
           (0 << CPOL) | /* Clock Polarity: SCK low when idle */ \
           (0 << CPHA) | /* Clock Phase: sample on rising SCK edge */ \
           (1 << SPR1) | /* Clock Frequency: f_OSC / 128 */ \
           (1 << SPR0); \
    SPSR &= ~(1 << SPI2X) /* No doubled clock frequency */ 

#define SPI_UPSHIFT()     /* switch to highest SPI frequency possible */ \
    SPCR &= ~((1 << SPR1) | (1 << SPR0)); /* Clock Frequency: f_OSC / 4 */ \
    SPSR |= (1 << SPI2X) /* Doubled Clock Frequency: f_OSC / 2 */ \

#define SD_RAW_SEND_BYTE_GUTS     SPDR = b; \
    /* wait for byte to be shifted out */   \
    while(!(SPSR & (1 << SPIF)));           \
    SPSR &= ~(1 << SPIF);

#define SD_RAW_REC_BYTE_GUTS     /* send dummy data for receiving some */ \
    SPDR = 0xff;    \
    while(!(SPSR & (1 << SPIF))); \
    SPSR &= ~(1 << SPIF); \
    return SPDR; \


#endif

#if SD_RAW_SDHC
    typedef uint64_t offset_t;
#else
    typedef uint32_t offset_t;
#endif

/* configuration checks */
#if SD_RAW_WRITE_SUPPORT
#undef SD_RAW_SAVE_RAM
#define SD_RAW_SAVE_RAM 0
#else
#undef SD_RAW_WRITE_BUFFERING
#define SD_RAW_WRITE_BUFFERING 0
#endif

#ifdef __cplusplus
}
#endif

#endif


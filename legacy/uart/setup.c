#include "LPC214x.h"
#include "setup.h"
#include "serial.h"
#include "main.h"
#include "loop.h"
#include "irq.h"
#define PLOCK 0x400

unsigned int CCLK;
unsigned int PCLK;

static void setupPins(void) {
  SCS=0;
  //  C    F    3    5    1    5    0    5
  // 1100 1111 0011 0101 0001 0101 0000 0101
  //  F E  D C  B A  9 8  7 6  5 4  3 2  1 0 
  // Pin 0.00 (TXO0)     - 01 - TxD0
  // Pin 0.01 (RXI0)     - 01 - RxD0
  // Pin 0.02 (STAT0)    - 00 - GPIO 0.02
  // Pin 0.03 (STOP)     - 00 - GPIO 0.03
  // Pin 0.04 (Card SCK) - 01 - SCK0
  // Pin 0.05 (Card DO)  - 01 - MISO0
  // Pin 0.06 (Card DI)  - 01 - MOSI0
  // Pin 0.07 (Card CS)  - 00 - GPIO 0.07
  // Pin 0.08 (TXO1)     - 01 - TxD1
  // Pin 0.09 (RXI1)     - 01 - RxD1
  // Pin 0.10 (7)        - 11 - AD1.2
  // Pin 0.11 (STAT1)    - 00 - GPIO 0.11
  // Pin 0.12 (8)        - 11 - AD1.3
  // Pin 0.13 (BATLV)    - 11 - AD1.4
  // Pin 0.14 (BSL)      - 00 - GPIO 0.14
  // Pin 0.15 (NC)       - 11 - AD1.5
  PINSEL0 = 0xCF351505;

  //  1    5    4    4    1    B    0    4
  // 0001 0101 0100 0100 0001 1011 0000 0100
  //  F E  D C  B A  9 8  7 6  5 4  3 2  1 0 
  // Pin 0.16 0 (NC)   - 00 - GPIO 0.16
  // Pin 0.17 1 (SCK)  - 01 - CAP1.2    (GPS PPS)
  // Pin 0.18 2 (MISO) - 00 - GPIO 0.18 (Acc GS1)
  // Pin 0.19 3 (MOSI) - 00 - GPIO 0.19 (Acc GS2)
  // Pin 0.20 4 (CS)   - 00 - GPIO 0.20 (Sw6)
  // Pin 0.21 5 (6)    - 10 - AD1.6
  // Pin 0.22 6 (5)    - 01 - AD1.7
  // Pin 0.23 7 (Vbus) - 00 - GPIO 0.23
  // Pin 0.24 8 (none) - 00 - Reserved 
  // Pin 0.25 9 (4)    - 01 - AD0.4
  // Pin 0.26 A (D+)   - 00 - Reserved (USB)
  // Pin 0.27 B (D-)   - 01 - Reserved (USB)
  // Pin 0.28 C (3)    - 01 - AD0.1
  // Pin 0.29 D (2)    - 01 - AD0.2
  // Pin 0.30 E (1)    - 01 - AD0.3
  // Pin 0.31 F (LED3) - 00 - GPO only
  PINSEL1 = 0x15441804;

  // Pin 0.2,0.7,0.11,0.18,0.19,0.31 set to out
  IODIR0 =(1<<2) | (1<<7) | (1<<11) | (1<<18) | (1<<19) | (1<<31);
  // Pin 0.7 set to high
  IOSET0 = (1<<7);
  // Pin 0.18,0.19 set to low
  IOCLR0 =(1<<18) | (1<<19);

  S0SPCR = 0x08;  // SPI clk to be pclk/8
  S0SPCR = 0x30;  // master, msb, first clk edge, active high, no ints
}

static void measurePCLK(void) {
  CCLK=FOSC*((PLLSTAT & 0x1F)+1);
  switch (VPBDIV & 0x03) {
    case 0:
      PCLK=CCLK/4;
      break;
    case 1:
      PCLK=CCLK;
      break;
    case 2:
      PCLK=CCLK/2;
      break;
    case 3:
      break;
  }
}

char versionString[]="Rocketometer v0.1 "__DATE__" "__TIME__;

static void feed(void) {
  PLLFEED=0xAA;
  PLLFEED=0x55;
}

static void setupSystem(void) {
  // Setting Multiplier and Divider values
  PLLCFG=0x24;
  feed();

  // Enabling the PLL */
  PLLCON=0x1;
  feed();

  // Wait for the PLL to lock to set frequency
  while(!(PLLSTAT & PLOCK)) ;

  // Connect the PLL as the clock source
  PLLCON=0x3;
  feed();

  // Enabling MAM and setting number of clocks used for Flash memory fetch (4 cclks in this case)
  //MAMTIM=0x3; //VCOM?
  MAMCR=0x2;
  MAMTIM=0x4; //Original

  // Setting peripheral Clock (pclk) to System Clock (cclk)
  VPBDIV=0x1;

  measurePCLK();
}


void setup() {

  setupSystem();
  
  setupPins();
  init_VIC();
  setupUart(0,115200,NULL);
  
  set_light(2,OFF);
  
}


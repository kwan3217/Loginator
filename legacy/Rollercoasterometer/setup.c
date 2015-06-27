#include "LPC214x.h"
#include "sd_raw.h"
#include "rootdir.h"
#include "setup.h"
#include "conparse.h"
#ifdef UART_SYS
#include "uart.h"
#endif
#include "sdbuf.h"
#ifdef ADC_SYS
#include "adc.h"
#endif
#include "main.h"
#include "command.h"
#include "pktwrite.h"
#include "debug.h"
#include "loop.h"
#include "sdbuf.h"
#include "display.h"
#include "flash.h"

#define PLOCK 0x400

static char fn[]="LOK1_XXX.bin";
unsigned int CCLK;
unsigned int PCLK;

static void setClock(int y, int m, int d, int h, int n, int s) {
  YEAR=y;
  MONTH=m;
  DOM=d;
  DOW=0;
  DOY=0;
  HOUR=h;
  MIN=n;
  SEC=s;
}

static void setupClock(void) {
  //Turn off the clock
  CCR=0;
  //Set the PCLK prescaler and set the clock to use it, so as to run in sync with everything else.
  PREINT=PCLK/32768-1;
  PREFRAC=PCLK-((PREINT+1)*32768);
  
  //If the clock year is reasonable, it must have been set by 
  //some process before, so we'll leave it running.
  //If it is year 0, then it is runtime from last reset, 
  //so we should reset it.
  //If it is unreasonable, this is the first time around,
  //and we set it to zero to count runtime from reset
  if(YEAR<2000 || YEAR>2100) {
    CCR|=(1<<1);
    setClock(0,0,0,0,0,0);
    //Pull the subsecond counter out of reset
    CCR&=~(1<<1);
  }
  //Turn the clock on
  CCR|=(1<<0);
}

static void setupTimer1(void) {
  T1TCR=0x02;    //Turn off timer and reset counter   
  T1CCR=(1<<6);  //Turn on capture 1.2 on rising (6) edge
  T1MCR=0x02;    //Reset timer on match
  T1MR0=PCLK;    //Set the reset time to 1 second
  T1TCR=0x01;    //Out of reset and turn timer on
}

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

static int fat_initialize(void) {
  int result=sd_raw_init();
  if(result<0) return result;
  return openroot();
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

static void writeMAMsetup(void) {
  fillPktStart(&sdBuf,PT_MAM);
  fillPktByte(&sdBuf,MAMCR);
  fillPktByte(&sdBuf,MAMTIM);
  if(PKT_SIRF==writeMode) {
    fillShort(&sdBuf,PLLSTAT);
  } else {
    fillPktByte(&sdBuf,((PLLSTAT >>  0) & 0x1F)+1); //MSEL, PLL Multiplier
    fillPktByte(&sdBuf, (PLLSTAT >>  5) & 0x03);     //PSEL, PLL Divisor
    fillPktByte(&sdBuf, (PLLSTAT >>  8) & 0x01);     //PLLE, PLL Enabled
    fillPktByte(&sdBuf, (PLLSTAT >>  9) & 0x01);     //PLLC, PLL Connected
    fillPktByte(&sdBuf, (PLLSTAT >> 10) & 0x01);     //PLOCK, Phase Lock
  }
  fillPktByte(&sdBuf,VPBDIV);
  fillPktInt(&sdBuf,CCLK);                   //Core Clock rate, Hz
  fillPktInt(&sdBuf,PCLK);                   //Peripheral Clock rate, Hz
  fillPktInt(&sdBuf,PREINT);  
  fillPktInt(&sdBuf,PREFRAC);                    
  sdBuf.dataDec=0;
  sdBuf.dataDigits=2;
  fillPktByte(&sdBuf,CCR);
  fillPktFinish(&sdBuf);
}

char versionString[]="Rollercoasterometer v1.3 "__DATE__" "__TIME__;

static void writeVersion(void) {
  fillPktStart(&sdBuf,PT_VERSION);
  fillPktString(&sdBuf,versionString);
  fillPktFinish(&sdBuf);
}

static void logFirmware(void) {
  if(dumpFirmware) {
    int firmSize=512*1024;
    int pktSize=32;
    int numPkts=firmSize/pktSize;
    for(int i=0;i<numPkts;i++) {
      fillPktStart(&sdBuf,PT_FLASH);
      fillPktByte(&sdBuf,0);
      sdBuf.dataDec=0;
      sdBuf.dataDigits=5;
      fillPktInt(&sdBuf,i*pktSize);
      sdBuf.dataDigits=2;
      fillPktByte(&sdBuf,*((char*)(i*pktSize)));
      sdBuf.dataComma=0;
      for(int j=1;j<32;j++) fillPktByte(&sdBuf,*((char*)(i*pktSize+j)));
      fillPktFinish(&sdBuf);
      if(isFlushSDNeeded())flushSD();
	    set_light(0,((i>>5) & 1) >> 0,LP_DUMPFIRMWARE);
	    set_light(1,((i>>5) & 2) >> 1,LP_DUMPFIRMWARE);
	  }
  	set_light(0,0,LP_DUMPFIRMWARE);
  	set_light(1,0,LP_DUMPFIRMWARE);
  }
}

#define READBUFSIZE 512

static void writeFirmware(void) {
  if(writeFirm==2) {
    fillPktStart(&sdBuf,PT_FLASH);
    fillPktByte(&sdBuf,1);
    fillPktInt(&sdBuf,firmAddress);
    sdBuf.dataDec=0;
    sdBuf.dataDigits=5;
    fillPktInt(&sdBuf,firmAddress);
    sdBuf.dataAsciiz=1;
    fillPktString(&sdBuf,firmFile);
    sdBuf.dataAsciiz=0;
    fillPktString(&sdBuf,"writeFirmware is ARMED!");
    fillPktFinish(&sdBuf);
    if(isFlushSDNeeded())flushSD();

    struct fat16_file_struct fd;
    int read;
    int i;
    char* addy=(char*)firmAddress;
    char readbuf[READBUFSIZE];

    // Open the file 
    int result = root_open(&fd,firmFile);
    
    int file_size=fd.dir_entry.file_size;

    fillPktStart(&sdBuf,PT_FLASH);
    fillPktByte(&sdBuf,3);
    fillPktInt(&sdBuf,file_size);
    fillPktFinish(&sdBuf);
    if(isFlushSDNeeded())flushSD();

    if(result<0) { 
      fillPktStart(&sdBuf,PT_ERROR);
      sdBuf.dataAsciiz=1;
      fillPktString(&sdBuf,"root_open() failed");
      sdBuf.dataDec=0;
      sdBuf.dataDigits=8;
      fillPktInt(&sdBuf,(int)&fd);
      fillPktString(&sdBuf,firmFile);
      fillPktInt(&sdBuf,result);
      fillPktFinish(&sdBuf);
      if(isFlushSDNeeded())flushSD();
      return;
    }
    
    //Erase the pertinent bits of EEPROM
    int sector0=flash_sector((char*)firmAddress);
    int sector1=flash_sector((char*)(firmAddress+file_size-1));
    
    result=erase_flash(sector0,sector1);

    fillPktStart(&sdBuf,PT_FLASH);
    fillPktByte(&sdBuf,4);
    fillPktByte(&sdBuf,sector0);
    sdBuf.dataDec=0;
    sdBuf.dataDigits=5;
    fillPktInt(&sdBuf,firmAddress);
    sdBuf.dataDec=1;
    sdBuf.dataDigits=0;
    fillPktByte(&sdBuf,sector1);
    sdBuf.dataDec=0;
    sdBuf.dataDigits=5;
    fillPktInt(&sdBuf,firmAddress+file_size-1);
    sdBuf.dataDigits=2;
    fillPktInt(&sdBuf,result);
    fillPktFinish(&sdBuf);
    if(isFlushSDNeeded())flushSD();

    // Clear the buffer 
    for(i=0;i<READBUFSIZE;i++) {
      readbuf[i]=0;
    }

    // Read the file contents, and print them out
    while( (read=fat16_read_file(&fd,(unsigned char*)readbuf,READBUFSIZE)) > 0 )  {

      result=write_flash(readbuf,READBUFSIZE,addy);
      
      if(result!=IAP_CMD_SUCCESS) {
        fillPktStart(&sdBuf,PT_ERROR);
        sdBuf.dataAsciiz=1;
        fillPktString(&sdBuf,"write_flash() failed");
        sdBuf.dataDec=0;
        sdBuf.dataDigits=8;
        fillPktInt(&sdBuf,(int)readbuf);
        fillPktInt(&sdBuf,READBUFSIZE);
        fillPktInt(&sdBuf,(int)addy);
        sdBuf.dataDigits=2;
        fillPktByte(&sdBuf,result);
        fillPktFinish(&sdBuf);
        if(isFlushSDNeeded())flushSD();
        fat16_close_file(&fd);
        sd_raw_sync();
        return;
      } else {
        fillPktStart(&sdBuf,PT_FLASH);
        fillPktByte(&sdBuf,2);
        sdBuf.dataDec=0;
        sdBuf.dataDigits=8;
        fillPktInt(&sdBuf,(int)readbuf);
        sdBuf.dataDigits=4;
        fillPktInt(&sdBuf,READBUFSIZE);
        sdBuf.dataDigits=8;
        fillPktInt(&sdBuf,(int)addy);
        sdBuf.dataDigits=2;
        fillPktInt(&sdBuf,result);
        fillPktFinish(&sdBuf);
        if(isFlushSDNeeded())flushSD();
      }
      
      
      // Done with current data, so clear buffer
      //  this is because we ALWAYS write out the
      //  entire buffer, and we don't want to write
      //  garbage data on reads smaller than READBUFSIZE
      for(i=0;i<READBUFSIZE;i++)  {
        readbuf[i]=0;
      }

      // Now update the address
      addy = addy + READBUFSIZE;

      // And we should probably bounds-check...
      if((unsigned int)addy > (unsigned int) 0x0007CFFF)  {
        break;
      }

    }

    // Close the file! 
    fat16_close_file(&fd);
    sd_raw_sync();

  }
}


static void feed(void) {
  PLLFEED=0xAA;
  PLLFEED=0x55;
}

static void system_init(void) {
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
}

static void writeSDinfo(void) {
  char buf[36];
  char result=sd_raw_get_cid_csd(buf);
  
  fillPktStart(&sdBuf,PT_SDINFO);
  fillPktByte(&sdBuf,result);
  sdBuf.dataDec=0;
  sdBuf.dataDigits=2;
  fillPktByte(&sdBuf,buf[0]);
  sdBuf.dataComma=0;
  for(int i=1;i<36;i++) fillPktByte(&sdBuf,buf[i]);
  fillPktFinish(&sdBuf);
//      fillString(&sdBuf,"SD Card Setup: result=");
//      fillDec(&sdBuf,result);
//      fillShort(&sdBuf,0x0D0A);
//      int pos=0;
//      fillString(&sdBuf,"  MID=");fillDec(&sdBuf,buf[pos]);pos++;fillShort(&sdBuf,0x0D0A);
//      fillString(&sdBuf,"  OID='");
//      for(int i=0;i<2;i++) {
//        fill(&sdBuf,buf[pos]);pos++;
//      }
//      fillString(&sdBuf,"'\r\n");
//      fillString(&sdBuf,"  PNM='");
//      for(int i=0;i<5;i++) {
//        fill(&sdBuf,buf[pos]);pos++;
//      }
//      fillString(&sdBuf,"'\r\n");
//      fillString(&sdBuf,"  PRV=");fillDec(&sdBuf,buf[pos]);pos++;fillShort(&sdBuf,0x0D0A);
//      unsigned int PSN=0;
//      for(int i=0;i<4;i++) {
//       PSN=(PSN<<8)+buf[pos];pos++;
//      }
//      fillString(&sdBuf,"  PSN=");fillDec(&sdBuf,PSN);fillShort(&sdBuf,0x0D0A);
//      mark(&sdBuf);
}

void setup() {

  system_init();
  
  int count = 0;
  displayMode=DISP_CLOCK;


  
  measurePCLK();
  
  setupTimer1();

  setupClock();
  
  setupPins();
  set_light(2,OFF,LP_ALWAYS);
  
  if(fat_initialize()<0) blinklock(0,1);

  if(readLogCon()<0) blinklock(0,3);
  
  if(writeMode==PKT_NMEA || writeMode==PKT_TEXT) {
    fn[ 9]='t';
	  fn[10]='x';
    fn[11]='t';
  }
  
  count=0;
  do {
    if(count >=99) blinklock(0,0);
    fn[5]=count/100+'0';
    fn[6]=(count%100)/10+'0';
    fn[7]=(count%10)+'0';
    count++;
  } while(root_file_exists(fn));
  
  if(open_debug()<0) blinklock(0,4);
  if(openSD(fn)<0) blinklock(0,2);
  
  if(loadCommands()<0) blinklock(0,4);

  writeVersion();
  
  writeLogCon();
  
  writeFirmware();
  
  logFirmware();
  
  writeADCsetup();
  writeMAMsetup();
  writeSDinfo();
  
  writeAdjGainPkt(&sdBuf,0xFF,GSense);
  setGSense();
  if(isFlushSDNeeded())flushSD();
  
  startRecordUART();
  if(isFlushSDNeeded())flushSD();

 
  //Only set up the ADC if some channels are to be recorded  
  if(nChannels>0 && adcFreq>0) startRecordADC(); 
  debug_print("Finished setup()\n");debug_flush();
}


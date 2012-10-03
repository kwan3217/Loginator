#include "LPC214x.h"
#include "sd_raw.h"
#include "rootdir.h"
#include "setup.h"
#include "conparse.h"
#include "uart.h"
#include "sdbuf.h"
#include "main.h"
#include "command.h"
#include "pktwrite.h"
#include "loop.h"
#include "sdbuf.h"
#include "flash.h"
#include "i2c.h"
#include "acc.h"
#include "gyro.h"
#include "compass.h"
#include "serial.h"
#include "armVIC.h"
#include "irq.h"
#include "IMU.h"
#include "pinger.h"
#include "sensor.h"
#include "motor.h"
#include "control.h"
#define PLOCK 0x400

static char fn[]="RKTV_XXX.bin";
unsigned int CCLK;
unsigned int PCLK;

void set_pin(int pin, int mode) {
  int mask=~(0x3 << ((pin & 0x0F)<<1));
  int val=mode << ((pin & 0x0F)<<1);
  if(pin>=16) {
    PINSEL1=(PINSEL1 & mask) | val;
  } else {
    PINSEL0=(PINSEL0 & mask) | val;
  }
}

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

static void setupTimer0(void) {
  T0TCR=0x02;    //Turn off timer and reset counter
  T0CCR=01101;   //Yes, octal. Each octal digit represents one capture channel.
                 //01 is rising edge
                 //02 is falling edgen
                 //04 is generate an interrupt
                 //Capture rising edge of CAP0.0 (AD1, PPS)
                 //                       CAP0.3 (AD2, Accelerometer)
                 //                       CAP0.2 (AD3, Gyro)
  T0CCR=0;  //Disable timer capture for now
  T0MCR=0x02;    //Reset timer on match
  T0MR0=PCLK;    //Set the reset time to 1 second
  T0TCR=0x01;    //Out of reset and turn timer on
}

static void setupPins(void) {
  SCS=0;

  set_pin( 0,1); //(TXO0)     - 01 - TxD0
  set_pin( 1,1); //(RXI0)     - 01 - RxD0
  set_pin( 2,0); //(STAT0)    - 00 - GPIO
  set_pin( 3,0); //(STOP)     - 00 - GPIO
  set_pin( 4,1); //(Card SCK) - 01 - SCK0
  set_pin( 5,1); //(Card DO)  - 01 - MISO0
  set_pin( 6,1); //(Card DI)  - 01 - MOSI0
  set_pin( 7,0); //(Card CS)  - 00 - GPIO
  set_pin( 8,1); //(TXO1)     - 01 - TxD1
  set_pin( 9,1); //(RXI1)     - 01 - RxD1
  set_pin(10,3); //(7)        - 11 - AD1.2
  set_pin(11,0); //(STAT1)    - 00 - GPIO
  set_pin(12,3); //(8)        - 11 - AD1.3
  set_pin(13,3); //(BATLV)    - 11 - AD1.4
  set_pin(14,0); //(BSL)      - 00 - GPIO
  set_pin(15,0); //(NC)       - 00 - GPIO

  set_pin(16,0); //(NC)   - 00 - GPIO 
  set_pin(17,2); //(SCK)  - 01 - SSP SCK
  set_pin(18,2); //(MISO) - 01 - SSP MISO
  set_pin(19,2); //(MOSI) - 01 - SSP MOSI
  set_pin(20,0); //(CS)   - 00 - GPIO
  set_pin(21,2); //(6)    - 10 - AD1.6
  set_pin(22,1); //(5)    - 01 - AD1.7
  set_pin(23,0); //(Vbus) - 00 - GPIO 
  set_pin(24,0); //(none) - 00 - Reserved 
  set_pin(25,1); //(4)    - 01 - AD0.4
  set_pin(26,0); //(D+)   - 00 - Reserved (USB)
  set_pin(27,1); //(D-)   - 01 - Reserved (USB)
  set_pin(28,2); //(3)    - 10 - CAP0.2
  set_pin(29,2); //(2)    - 10 - CAP0.3
  set_pin(30,3); //(1)    - 11 - CAP0.0
  set_pin(31,0); //(LED3) - 00 - GPO only

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

char versionString[]="Arisa v0.01 "__DATE__" "__TIME__"MDT - I was built to fly outside!";

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
      drainToSD(&sdBuf);
      set_light(0,((i>>5) & 1) >> 0);
      set_light(1,((i>>5) & 2) >> 1);
    }
    set_light(0,0);
    set_light(1,0);
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
    drainToSD(&sdBuf);

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
    drainToSD(&sdBuf);

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
      drainToSD(&sdBuf);
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
    drainToSD(&sdBuf);

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
        drainToSD(&sdBuf);
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
        drainToSD(&sdBuf);
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
    root_delete(firmFile);

  }
}

static void feed(void) {
  PLLFEED=0xAA;
  PLLFEED=0x55;
}

static void system_init(void) {
  // SetMultiplier and Divider values
  PLLCFG=0x24;
  feed();

  // Enable the PLL */
  PLLCON=0x1;
  feed();

  // Wait for the PLL to lock to set frequency
  while(!(PLLSTAT & PLOCK)) ;

  // Connect the PLL as the clock source
  PLLCON=0x3;
  feed();

  // Enable MAM and setting number of clocks used for Flash memory fetch (4 cclks in this case)
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

static void startRecordTime1(void) {
  enableIRQ();

  T1TCR = 0x00000002;  // Reset counter and prescaler and disable timer
  T1CTCR= 0x00000000;  // Timer 1 is to be used as a timer, not a counter
  T1MCR = 0x00000003;  // On match 1 reset the counter and generate interrupt. All other match channels ignored
  T1MR0 = PCLK / adcFreq;  //Match channel 1 value: When T1TC reaches this value, things happen as described above

  T1PR = 0x00000000;   //No prescale - 1 PCLK tick equals 1 Timer1 tick
  T1CCR= 0x00000000;   //No capture on external input for Timer1
  T1EMR= 0x00000000;   //No external output on matches for Timer1

  T1TCR = 0x00000001; // enable timer

  install_irq(TIMER1_INT,time1ISR);
}

void setup() {
  set_light(2,OFF);

  system_init();
  
  int count = 0;
  
  measurePCLK();
  
  setupClock();
  
  setupPins();
//  init_VIC();
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
  
  if(openSD(fn)<0) blinklock(0,2);
  
  if(loadCommands()<0) blinklock(0,4);

  writeVersion();
  
  writeLogCon();
  
  writeFirmware();
  
  logFirmware();
  
  writeMAMsetup();
  writeSDinfo();
  drainToSD(&sdBuf);
  
  startRecordUART();
  drainToSD(&sdBuf);

  //Make sure that the I2C stuff is set up before we turn on the ADC timer
  //For the combined SPI and I2C board
  writeI2Csetup(&sdBuf,21,10,200000);
  drainToSD(&sdBuf);
  setupSensors(&sdBuf);
/*
  fillPktStart(&sdBuf,PT_I2C);
  fillPktString(&sdBuf,"FLOAT");
  volatile float k=47.2;
  k*=29.6;
  fillPktFP(&sdBuf,k);
  fillPktFinish(&sdBuf);
  drain(&spiBuf,&sdBuf);
  if(isFlushSDNeeded())flushSD();
*/
  int imu_result=initIMU();
  fillPktStart(&sdBuf,PT_I2C);
  fillPktString(&sdBuf,"IMU init");
  fillPktInt(&sdBuf,imu_result);
  fillPktFinish(&sdBuf);
  drainToSD(&sdBuf);

  imu_result=initControls();
  fillPktStart(&sdBuf,PT_I2C);
  fillPktString(&sdBuf,"Control Init");
  fillPktInt(&sdBuf,imu_result);
  fillPktFinish(&sdBuf);
  drainToSD(&sdBuf);
  
  startRecordTime1();
  setupTimer0();

}


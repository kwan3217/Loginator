#include "adc.h"
#include "LPC214x.h"

//Set sample clock to near 4.5MHz (Divisor 14). Much faster and
//marginally better noise than Divisor 255
#define ADCDIV 14

void convertBothADC(int AD0, int AD1) {
  AD0CR=(1<<21) | ((ADCDIV-1)<<8) | (1 << AD0); //Turn on AD0, program the divisor, set the input channel
  AD1CR=(1<<21) | ((ADCDIV-1)<<8) | (1 << AD1); //Turn on AD1, program the divisor, set the input channel
  ADGSR|=0x01000000; //Start conversion on both
  int temp;
  do {
    temp=AD1GDR; //Check if channel 1 is done
  } while((temp & 0x80000000) == 0);
  AD0CR=0x00000000; //Stop AD0
  AD1CR=0x00000000; //Stop AD1
}

int readoutADC0() {
  return (AD0GDR & 0xFFC0) >> 6;
}

int readoutADC1() {
  return (AD1GDR & 0xFFC0) >> 6;
}

#ifdef HILEVEL
#include <stdlib.h>
#include "conparse.h"
#include "armVIC.h"
#include "load.h"
#include "circular.h"
#include "setup.h"
#include "main.h"
#include "sdbuf.h"
#include "pktwrite.h"
#include "serial.h"
#include "stringex.h"
#include "irq.h"
#include "tictoc.h"

circular adcBuf;
static int binNum;
static unsigned int adcReading[9];
static unsigned int adcReadingCopy[9];
static unsigned int tscCopy;
static char firstTimeAround;
char hasNewADC;

void writeADC() {
  hasNewADC=0;
  fillPktStart(&adcBuf,PT_ANALOG);
  if(writeMode==PKT_NMEA) fillPktInt(&adcBuf,tscCopy/60000);
  for(int i=0;i<nChannels;i++) {
    if(channelActive[i]) {
      fillPktShort(&adcBuf,adcReadingCopy[i] & 0xFFFF);
    }
  }
  if(writeMode==PKT_SIRF) fillPktInt(&adcBuf,tscCopy);
  fillPktFinish(&adcBuf);
}

//Internal names for each ADC pin (AD0 is internal battery monitor)
//             0 1 2 3 4 5 6 7 8
int adcSide[]={1,0,0,0,0,1,1,1,1};
int adcChan[]={4,3,2,1,4,7,6,2,3};

int adcSequence[5][2]={{3,4},{2,7},{1,6},{4,2},{0,3}};
int adcMask[5+1]={3,3,3,3,2,0};
int adcBackref[5][2]={{1,0},{2,5},{3,6},{4,7},{-1,8}};

volatile static int inAdcIsr=0;

static void adcISR(void) {
    enableIRQ();
  if(inAdcIsr) return;
  inAdcIsr=1;
  T1IR = 1; // Clear T1 interrupt on match channel 0

  int j;
  hasLoad(LOAD_ADC);
  
  for(j=0;adcMask[j]>0;j++) {
    convertBothADC(adcSequence[j][0],adcSequence[j][1]);
    if(adcMask[j] & 0x01) {
      adcReading[adcBackref[j][0]]=readoutADC0();
    }
    if(adcMask[j] & 0x02) {
      adcReading[adcBackref[j][0]]=readoutADC1();
    }
  }

  unsigned int tsc = T0TC;

  binNum++; 
  if(binNum==adcBin) {
    tscCopy=tsc;
    for(j=0;j<9;j++) {
      adcReadingCopy[j]=adcReading[j];
      adcReading[j]=0;
    }
    binNum=0;
    hasNewADC=1;
  }
  inAdcIsr=0;
  VICVectAddr= 0;
}

void writeADCsetup() {
  firstTimeAround=1;
  
  fillPktStart(&adcBuf,PT_COLUMNS);
  fillPktShort(&adcBuf,adcFreq);
  fillPktShort(&adcBuf,adcBin);
  fillPktByte(&adcBuf,ADCDIV);
  
  for(int i=0;i<nChannels;i++) {
    if(channelActive[i]) {
      fillPktByte(&adcBuf,i);
	  adcBuf.dataDec=0;
      fillPktByte(&adcBuf,adcSide[i]<<4 | adcChan[i]);
	  adcBuf.dataDec=1;
    }
  }
  fillPktFinish(&adcBuf);
}

void startRecordADC(void) {
  enableIRQ();
  install_irq(TIMER1_INT,adcISR);

  T1TCR = 0x00000002;  // Reset counter and prescaler and disable timer
  T1CTCR= 0x00000000;  // Timer 0 is to be used as a timer, not a counter
  T1MCR = 0x00000003;  // On match 0 reset the counter and generate interrupt. All other match channels ignored
  T1MR0 = PCLK / adcFreq;  //Match channel 0 value: When T1TC reaches this value, things happen as described above

  T1PR = 0x00000000;   //No prescale - 1 PCLK tick equals 1 Timer1 tick
  T1CCR= 0x00000000;   //No capture on external input for Timer1
  T1EMR= 0x00000000;   //No external output on matches for Timer1

  T1TCR = 0x00000001; // enable timer
}
#endif

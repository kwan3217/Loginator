#include <stdlib.h>
#include "LPC214x.h"
#include "adc.h"
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

#ifdef ADC_SYS

//Set sample clock to near 4.5MHz (Divisor 14). Much faster and
//marginally better noise than Divisor 255
#define ADCDIV 14

circular adcBuf;
static int binNum;
static unsigned int adcReading[9];
static unsigned int adcReadingCopy[9];
static unsigned int tsc1Copy;
static unsigned int adcAccHistory[16][3];
static unsigned int mean[3];
static unsigned int variance[3];
static int slope[3];
static unsigned int slopesq[3];
static int historyPointer;
static char firstTimeAround;
static char hasNewADC;

static void calcStats(void) {
  if(firstTimeAround) return;
  for(int j=0;j<3;j++) {
    mean[j]=0;
    for(int i=0;i<16;i++) mean[j]+=adcAccHistory[i][j];
    mean[j]>>=4;
    for(int i=0;i<16;i++) variance[j]+=(((signed)(adcAccHistory[i][j]-mean[j]))*((signed)(adcAccHistory[i][j]-mean[j])));
    variance[j]>>=4;
    slope[j]=((signed)adcAccHistory[(historyPointer-1) & 0x0F][j])-((signed)adcAccHistory[(historyPointer-2) & 0x0F][j]);
    slopesq[j]=slope[j]*slope[j];
  }
}

static void recordStats(unsigned int x, unsigned int y, unsigned int z) {
  adcAccHistory[historyPointer][0]=x;
  adcAccHistory[historyPointer][1]=y;
  adcAccHistory[historyPointer][2]=z;
  historyPointer++;
  if(historyPointer==16) firstTimeAround=0;
  historyPointer&=0x0F;
}

static int decideAdjustGain(void) {
  if(!autoGSense) return 0;
  if(firstTimeAround) return 0;
  calcStats();
  char voteHigher=0;
  char voteLower=1;
  fillPktStart(&sdBuf,PT_DECIDE);
  for(int i=0;i<3;i++) {
    fillPktInt(&sdBuf,variance[i]);
    fillPktInt(&sdBuf,slopesq[i]);
//    if(variance[i]<slopesq[i]) {
      fillPktInt(&sdBuf,mean[i]/adcBin-512);
      fillPktInt(&sdBuf,abs(mean[i]/adcBin-512));
      char thisVoteLower=((abs(mean[i]/adcBin-512))<128);
      voteLower&=thisVoteLower;
      fillPktByte(&sdBuf,thisVoteLower);
      fillPktByte(&sdBuf,voteLower);
      char thisVoteHigher=((abs(mean[i]/adcBin-512))>384);
      voteHigher|=thisVoteHigher;
      fillPktByte(&sdBuf,thisVoteHigher);
      fillPktByte(&sdBuf,voteHigher);
//    }
  }
  fillPktFinish(&sdBuf);
  if(voteLower) return -1;
  if(voteHigher) return 1;
  return 0;  
}

void writeAdjGainPkt(circular* buf, char from, char to) {
  fillPktStart(buf,PT_GAINADJ);
  fillPktByte(buf,from);
  fillPktByte(buf,to);
  fillPktFinish(buf);
}

void adjustGain() {
  if(!hasNewADC) return;
  hasNewADC=0;
  int adjust=decideAdjustGain();
  int newGSense=GSense;
  if(GSense>0 && adjust<0) newGSense--;
  if(GSense<3 && adjust>0) newGSense++;
  if(newGSense!=GSense) {
    GSense=newGSense;
    writeAdjGainPkt(&sdBuf,GSense,newGSense);
    firstTimeAround=1;
    historyPointer=0;
    setGSense();
  }
}

void displayAcc() {
  char buf[12];
  putstring_serial0("XA");
  to0Dec(buf,adcReadingCopy[1],5);
  putstring_serial0(buf);
  putstring_serial0(" YA");
  to0Dec(buf,adcReadingCopy[2],5);
  putstring_serial0(buf);
  putstring_serial0(" ZA");
  to0Dec(buf,adcReadingCopy[3],5);
  putstring_serial0(buf);
  putstring_serial0(" GS");
  to0Dec(buf,GSense,1);
  putstring_serial0(buf);
}

void displayRot() {
  char buf[12];
  putstring_serial0("XR");
  to0Dec(buf,adcReadingCopy[5],5);
  putstring_serial0(buf);
  putstring_serial0(" YR");
  to0Dec(buf,adcReadingCopy[6],5);
  putstring_serial0(buf);
  putstring_serial0(" ZR");
  to0Dec(buf,adcReadingCopy[7],5);
  putstring_serial0(buf);
  putstring_serial0(" BT");
  to0Dec(buf,adcReadingCopy[0],5);
  putstring_serial0(buf);
}


static void writeADC(void) {
  fillPktStart(&adcBuf,PT_ANALOG);
  if(writeMode==PKT_NMEA) fillPktInt(&adcBuf,tsc1Copy/60000);
  for(int i=0;i<nChannels;i++) {
    if(channelActive[i]) {
      fillPktShort(&adcBuf,adcReadingCopy[i] & 0xFFFF);
    }
  }
  if(writeMode==PKT_SIRF) fillPktInt(&adcBuf,tsc1Copy);
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
  if(inAdcIsr) return;
  inAdcIsr=1;
  T0IR = 1; // Clear T0 interrupt on match channel 0

  int temp=0;
  int j;
  hasLoad(LOAD_ADC);
  
  for(j=0;adcMask[j]>0;j++) {
    AD0CR=(1<<21) | ((ADCDIV-1)<<8) | (1 << adcSequence[j][0]); //Turn on AD0, program the divisor, set the input channel
    AD1CR=(1<<21) | ((ADCDIV-1)<<8) | (1 << adcSequence[j][1]); //Turn on AD1, program the divisor, set the input channel
    switch(adcMask[j]) {
      case 1:
        AD0CR|=0x01000000; //Start conversion just on AD0
        break;
      case 2:
        AD1CR|=0x01000000; //Start conversion just on AD1
        break;
      case 3:
        ADGSR|=0x01000000; //Start conversion on both
    }
    do {
      switch(adcMask[j]) {
        case 1:
          temp=AD0GDR; //Check if channel 0 is done
          break;
        case 2:
        case 3:
          temp=AD1GDR; //Check if channel 1 is done
          break;
      }
    } while((temp & 0x80000000) == 0);
    if(adcMask[j] & 0x01) {
      AD0CR=0x00000000; //Stop AD0
      adcReading[adcBackref[j][0]]+=(AD0GDR & 0xFFC0) >> 6;
    }
    if(adcMask[j] & 0x02) {
      AD1CR=0x00000000; //Stop AD1
      adcReading[adcBackref[j][1]]+=(AD1GDR & 0xFFC0) >> 6;
    }
  }

  unsigned int tsc1 = T1TC;

  binNum++; 
  if(binNum==adcBin) {
    tsc1Copy=tsc1;
    for(j=0;j<9;j++) {
      adcReadingCopy[j]=adcReading[j];
      adcReading[j]=0;
    }
    recordStats(adcReadingCopy[1],adcReadingCopy[2],adcReadingCopy[3]);
    binNum=0;
    hasNewADC=1;
    writeADC();
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

void setGSense() {
  int set=0,clr=0;
  if(GSense & (1<<0)) {
    set|=(1<<18);
  } else {
    clr|=(1<<18);
  }
  if(GSense & (1<<1)) {
    set|=(1<<19);
  } else {
    clr|=(1<<19);
  }
  IOSET0=set;
  IOCLR0=clr;
}

void startRecordADC(void) {
  enableIRQ();
  // Timer0  interrupt is an IRQ interrupt
  VICIntSelect &= ~0x00000010;
  // Enable Timer0 interrupt
  VICIntEnable |= 0x00000010;
  // Use slot 3 for Timer0 interrupt
  VICVectCntl3 = 0x24;
  // Set the address of ISR for slot 3
  VICVectAddr3 = (unsigned int)adcISR;

  T0TCR = 0x00000002;  // Reset counter and prescaler and disable timer
  T0CTCR= 0x00000000;  // Timer 0 is to be used as a timer, not a counter
  T0MCR = 0x00000003;  // On match 0 reset the counter and generate interrupt. All other match channels ignored
  T0MR0 = PCLK / adcFreq;  //Match channel 0 value: When T0TC reaches this value, things happen as described above

  T0PR = 0x00000000;   //No prescale - 1 PCLK tick equals 1 Timer0 tick
  T0CCR= 0x00000000;   //No capture on external input for Timer0
  T0EMR= 0x00000000;   //No external output on matches for Timer0

  T0TCR = 0x00000001; // enable timer
}

#endif

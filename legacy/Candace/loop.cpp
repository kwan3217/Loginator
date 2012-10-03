#include "LPC214x.h"
#include "loop.h"
#include "setup.h"
#include "uart.h"
#include "main.h"
#include "sdbuf.h"
#include "command.h"
#include "load.h"
#include "pktwrite.h"
#include "gps.h"
#include "conparse.h"
#include "sd_raw.h"
#include "tictoc.h"
#include "imu.h"
#include "script.h"
#include "acc.h"

static unsigned int lastcap;
int readyForOncePerSecond=0;
static volatile int timeToRead=0;
int met=0;

static void fillTime(circular& buf) {
  buf.dataComma=1;
  buf.dataDigits=4;
  buf.dataDelim='-';
  fillPktShort(buf,YEAR);
  buf.dataDigits=2;
  fillPktByte(buf,MONTH);
  fillPktByte(buf,DOM);
  buf.dataDelim='T';
  fillPktByte(buf,HOUR);
  buf.dataDelim=':';
  fillPktByte(buf,MIN);
  fillPktByte(buf,SEC);
}

static void checkPPS(void) {
  unsigned int thiscap=T0CR0;

  if(lastcap!=thiscap) {
    hasLoad(LOAD_LOAD);
    lastcap=thiscap;
    fillPktStart(sdBuf,PT_PPS);
    int USEC=thiscap/60;
    fillPktChar(sdBuf,',');
    fillTime(sdBuf);
    sdBuf.dataDelim='.';
    sdBuf.dataDigits=6;
    fillPktInt(sdBuf,USEC);
    fillPktFinish(sdBuf);
  }
}
 
static void oncePerSecond(void) {
  displayClock();
  ackAcc();
  writeCommand();
  met++;
}

void time1ISR(void) {
  T1IR = 1; // Clear T1 interrupt on match channel 0
  timeToRead++;
  VICVectAddr= 0;
}

void logFirmware(void) {
  static int line=0;
  static int done=0;
  const int firmSize=512*1024;
  const int pktSize=pktSize;
  const int numPkts=firmSize/pktSize;
  if(!done) {
    fillPktStart(sdBuf,PT_FLASH);
    fillPktByte(sdBuf,0);
    sdBuf.dataDec=0;
    sdBuf.dataDigits=5;
    fillPktInt(sdBuf,line*pktSize);
    sdBuf.dataBase85=1;
    fillPktBlock(sdBuf,((char*)(line*pktSize)),pktSize);
    fillPktFinish(sdBuf);
    drainToSD(sdBuf);
    line++;
    if(line>=numPkts) done=1;
  }
}

void loop(void) {
  checkPPS();
  if(readyForOncePerSecond) {
    readyForOncePerSecond=0;
    oncePerSecond();
  } else if(timeToRead>0) {
    hasLoad(LOAD_ADC);
    scriptTimeToRead();
    timeToRead=0;
  }
  for(int i=0;i<2;i++) if(uartbuf[i].readylen()>0) {
    hasLoad(LOAD_UART);
    drainToSD(uartbuf[i]);
  }

  countLoad();
  clearLoad();
}


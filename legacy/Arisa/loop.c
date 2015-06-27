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
#include "IMU.h"
#include "script.h"
#include "acc.h"

static unsigned int lastcap;
int readyForOncePerSecond=0;
static volatile int timeToRead=0;

static void fillTime(circular* buf) {
  fill0Dec(buf,YEAR,4);
  fill0Dec(buf,MONTH,2);
  fill0Dec(buf,DOM,2);
  fill(buf,',');
  fill0Dec(buf,HOUR,2);
  fill0Dec(buf,MIN,2);
  fill0Dec(buf,SEC,2);
}

static void checkPPS(void) {
  unsigned int thiscap=T0CR0;

  if(lastcap!=thiscap) {
    hasLoad(LOAD_LOAD);
    lastcap=thiscap;
    fillPktStart(&sdBuf,PT_PPS);
    if(PKT_SIRF==writeMode) {
      fillShort(&sdBuf,YEAR);
      fill(&sdBuf,MONTH);
      fill(&sdBuf,DOM);
      fill(&sdBuf,HOUR);
      fill(&sdBuf,MIN);
      fill(&sdBuf,SEC);
      fillInt(&sdBuf,thiscap);
    } else {
      int USEC=thiscap/60;
      fill(&sdBuf,',');
      fillTime(&sdBuf);
      fill(&sdBuf,':');
      fill0Dec(&sdBuf,USEC,6);
    }
    fillPktFinish(&sdBuf);
  }
}
 
static void oncePerSecond(void) {
  displayClock();
  ackAcc();
  writeCommand();
}

void time1ISR(void) {
  T1IR = 1; // Clear T1 interrupt on match channel 0
  timeToRead++;
  VICVectAddr= 0;
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
  for(int i=0;i<2;i++) if(readylen(&uartbuf[i])>0) {
    hasLoad(LOAD_UART);
    drainToSD(&uartbuf[i]);
  }

  countLoad();
  clearLoad();
}


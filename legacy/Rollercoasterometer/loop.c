#include "LPC214x.h"
#include "loop.h"
#include "adc.h"
#include "setup.h"
#include "uart.h"
#include "main.h"
#include "sdBuf.h"
#include "command.h"
#include "load.h"
#include "pktwrite.h"
#include "gps.h"
#include "conparse.h"
#include "debug.h"
#include "sd_raw.h"
#include "display.h"

static unsigned int lastcap;
int readyForOncePerSecond=0;

static void fillTime(circular* buf) {
  fill0Dec(buf,YEAR,4);
  fill0Dec(buf,MONTH,2);
  fill0Dec(buf,DOM,2);
  fill(buf,',');
  fill0Dec(buf,HOUR,2);
  fill0Dec(buf,MIN,2);
  fill0Dec(buf,SEC,2);
}

static int PPSLight=0;
 
static void checkPPS(void) {
  unsigned int thiscap=T1CR2 % 60000000;
  if(lastcap!=thiscap) {
    hasLoad(LOAD_LOAD);
    lastcap=thiscap;
    PPSLight=1-PPSLight;
//	set_light(1,PPSLight,LP_ALWAYS); 
/*
  	if(hasRMC) {
	    SEC+=1;
      if(SEC>=60) {
	      SEC-=60;
  	    MIN+=1;
	      if(MIN>=60) {
	        MIN-=60;
  		    HOUR+=1;
        }
	    }
	    hasRMC=0;
	  }
	  display();
  */
  	fillPktStart(&sdBuf,PT_PPS);
	  if(PKT_SIRF==writeMode) {
      fillShort(&sdBuf,YEAR);
      fill(&sdBuf,MONTH);
      fill(&sdBuf,DOM);
      fill(&sdBuf,HOUR);
      fill(&sdBuf,MIN);
      fill(&sdBuf,SEC);
      fillInt(&sdBuf,lastcap);
    } else {
      int USEC=lastcap/60;
      fill(&sdBuf,',');
      fillTime(&sdBuf);
      fill(&sdBuf,'.');
      fill0Dec(&sdBuf,USEC,6);
    }
	  fillPktFinish(&sdBuf);
  }
}

static void oncePerSecond(void) {
  writeCommand();
  if((IOPIN0 & (1<<3)) == 0) {
    incDisplayMode();
  }
  /*
  if((IOPIN0 & (1<<20)) != 0) {
    displayLightOn();
  } else {
    displayLightOff();
  }
  */
  display();
}

void loop(void) {
  checkPPS();

  if(readylen(&adcBuf)>0) {
    hasLoad(LOAD_ADC);
    if(isFlushSDNeeded()) {
      hasLoad(LOAD_FLUSH);
      flushSD();
    }
    drain(&adcBuf,&sdBuf);
    adjustGain();
  }

  for(int i=0;i<2;i++) if(readylen(&uartbuf[i])>0) {
    hasLoad(LOAD_UART);
    if(isFlushSDNeeded()) {
      hasLoad(LOAD_FLUSH);
      flushSD();
    }
    if(timestamp[i]) {
      fillTime(&sdBuf);
      fill(&sdBuf,' ');
      fillDec(&sdBuf,i);
      fill(&sdBuf,' ');
    }
    drain(&uartbuf[i],&sdBuf);
  }
  if(isFlushSDNeeded()) {
    hasLoad(LOAD_FLUSH); 
    flushSD();
  }

  set_light(0,GPSLight,LP_GPSLOCK);
  if(readyForOncePerSecond) {
    readyForOncePerSecond=0;
    oncePerSecond();
  }

  countLoad();
  clearLoad();
  if(powerSave) sleep();
}


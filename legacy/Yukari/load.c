#include <string.h>
#include "LPC214x.h"
#include "load.h"
#include "loop.h"
#include "pktwrite.h"
#include "main.h"
#include "conparse.h"
#include "sdbuf.h"
#include "setup.h"
#include "serial.h"
#include "stringex.h"
#include "loop.h"

static int lasttoc=-1;
static int load[16];
static int lastIdle;
volatile static int loadMask;

char clockBuf[32];

void displayClock() {
  to0Dec(clockBuf,YEAR,4);
  clockBuf[4]='/';
  to0Dec(clockBuf+5,MONTH,2);
  clockBuf[7]='/';
  to0Dec(clockBuf+8,DOM,2);
  clockBuf[10]=' ';
  to0Dec(clockBuf+11,HOUR,2);
  clockBuf[13]=':';
  to0Dec(clockBuf+14,MIN,2);
  clockBuf[16]=':';
  to0Dec(clockBuf+17,SEC,2);
  clockBuf[19]=' ';
  
  unsigned int used=(PCLK-lastIdle)/6000;
  to0Dec(clockBuf+20,used/100,2);
  clockBuf[22]='.';
  to0Dec(clockBuf+23,used%100,2);
  clockBuf[25]='%';clockBuf[26]='\r';clockBuf[27]='\n';
  tx_serialz(0,clockBuf);
  
}

static void writeLoad(void) {
  fillPktStart(&sdBuf,PT_LOAD);
  fillPktShort(&sdBuf,YEAR);
  fillPktByte(&sdBuf,MONTH);
  fillPktByte(&sdBuf,DOM);
  fillPktByte(&sdBuf,HOUR);
  fillPktByte(&sdBuf,MIN);
  fillPktByte(&sdBuf,SEC);
  if(PKT_SIRF==writeMode) fillPktShort(&sdBuf,CTC);
  for(int i=0;i<16;i++) {
    fillPktInt(&sdBuf,load[i]);
  }
  fillPktFinish(&sdBuf);
}

int countLoad() {
  int result=0;

  int thistoc=T0TC;
  //Handle the last little bit of each timer cycle
  if(lasttoc>0) {
    if(thistoc<lasttoc) {
      //Get the last little bit of time around the corner
      hasLoad(LOAD_LOAD);
      load[loadMask]+=PCLK-lasttoc;
      writeLoad();
      readyForOncePerSecond=1;
	    result=1;
      //Reset the counters
      lastIdle=load[0];
      for(int i=0;i<16;i++) load[i]=0;
      //Count the rest of the time in this tick
      load[loadMask]=thistoc;
    } else {
      load[loadMask]+=(thistoc-lasttoc);
    }
  }
  lasttoc=thistoc;
  return result;
}

void hasLoad(char task) {
  loadMask|=(1<<task);
  set_light(1,ON);
}

void clearLoad() {
  loadMask=0;
  set_light(1,OFF);
}

void sleep() {
  PCON|=1; //idle mode
}

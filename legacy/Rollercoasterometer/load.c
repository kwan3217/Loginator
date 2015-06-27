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
#include "display.h"
#include "loop.h"

static int lasttoc=-1;
static int load[16];
static int lastIdle;
volatile static int loadMask;

void displayClock() {
  char buf[32];
  clearDisplay();
  to0Dec(buf,YEAR,4);
  putstring_serial0(buf);
  putc_serial0('/');
  to0Dec(buf,MONTH,2);
  putstring_serial0(buf);
  putc_serial0('/');
  to0Dec(buf,DOM,2);
  putstring_serial0(buf);
  strcpy(buf,"      ");
  putstring_serial0(buf);
  to0Dec(buf,HOUR,2);
  putstring_serial0(buf);
  putc_serial0(':');
  to0Dec(buf,MIN,2);
  putstring_serial0(buf);
  putc_serial0(':');
  to0Dec(buf,SEC,2);
  putstring_serial0(buf);
  putc_serial0(' ');
  
  unsigned int used=(PCLK-lastIdle)/6000;
  to0Dec(buf,used/100,2);
  putstring_serial0(buf);
  putc_serial0('.');
  to0Dec(buf,used%100,2);
  putstring_serial0(buf);
  putc_serial0('%');
  
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

  int thistoc=T1TC;
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
  set_light(1,ON,LP_LOAD);
}

void clearLoad() {
  loadMask=0;
  set_light(1,OFF,LP_LOAD);
}

void sleep() {
  PCON|=1; //idle mode
}

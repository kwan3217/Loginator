#include "LPC214x.h"

void delay(unsigned int ms) {
  unsigned int TC0=TTC(0);
  //count off whole minutes
  while(ms>=1000*timerSec) {
    //wait for the top of the minute
    while(TTC(0)>TC0) ;
    //wait for the bottom of the minute
    while(TTC(0)<TC0) ;
    ms-=1000*timerSec;
  }
  if(ms==0) return;
  unsigned int TC1=TC0+ms*(PCLK/1000);
  if(TC1>timerInterval) {
    //Do this while we are waiting
    TC1-=timerInterval;
    //wait for the top of the minute
    while(TTC(0)>TC0) ;
  }
  //wait for the rest of the minute
  while(TTC(0)<TC1) ;
}



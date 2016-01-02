#include "sim.h"

void SimTimer::advance(int port, uint32_t ticks) {
  uint32_t timerWrap=0xFFFF'FFFF;
  for(int i=0;i<4;i++) {
    if(TMCR[port] & (1<<(i*3+1))) { //If this match channel will reset the timer...
      if(TMR[port][i]<timerWrap) timerWrap=TMR[port][i]; //If this match channel will be hit first, remember it
    }
  }
  uint32_t timerLeft=timerWrap-TTC[port];
  if(ticks>timerLeft) {
    //We are going to wrap
    //This is the point where we would fire an interrupt or something
    TTC[port]=ticks-timerLeft;
  } else {
    TTC[port]+=ticks;
  }
}

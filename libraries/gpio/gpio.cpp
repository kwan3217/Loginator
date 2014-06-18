#include "LPC214x.h"
#include "gpio.h"

const int light_pin[3]={
#ifdef ROCKETOMETER
  //stat0 is red,   P0.8
  //stat1 is green  P0.7 
  //stat2 is blue   P0.9 
  8,7,9
#else
  #ifdef LOGINATOR
    //stat0 is red,   P0.8 (Shared with TX1)
    //stat1 is green  P0.1 (shared with RX0)
    //stat2 is blue   P0.0 (shared with TX0)
    8,1,0
  #else
    //stat0 is red    P0.2
    //stat1 is green  P0.11
    //stat2 is USB (also red)  P0.31
    2,11,31
  #endif
#endif
};

void blinklock(int blinkcode) {
  VICIntEnClr=0xFFFFFFFF;
  set_light(0,0);
  set_light(1,0);
  set_light(2,0);
  for(;;) {
    for(int i=0;(blinkcode >> i)>0 && i<32;i++) {
      set_light(0,1);
      delay(((blinkcode >> i) & 1)*1000+1000);
      set_light(0,0);
      delay(1000);
    }
    set_light(2,1);
    delay(1000);
    set_light(2,0);
    delay(1000);
  }
}

static int flickerLight=0;            

void flicker(int on) {
  set_light(flickerLight,on);       
  if(on==0) {             
;    flickerLight++;        
;    if(flickerLight>=3) flickerLight=1;
  }
}

void flicker() {
  static int on;
  flicker(on);
  on++;             
  if(on>=2) on=0;
}

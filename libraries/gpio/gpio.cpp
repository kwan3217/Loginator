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
    //stat0 is red,   P0.8
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




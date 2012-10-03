#include "LPC214x.h"
#include "gpio.h"

#define LOGINATOR

#ifdef LOGINATOR
  //stat0 is red,   P0.8
  //stat1 is green  P0.1 (shared with RX0)
  //stat2 is blue   P0.0 (shared with TX0)
const int light_pin[3]={8,1,0};
#else
  //stat0 is red    P0.2
  //stat1 is green  P0.11
  //stat2 is USB (also red)  P0.31
const int light_pin[3]={2,11,31};
#endif

void set_light(int statnum, int on) {
  set_pin(light_pin[statnum],0,1); //Set pin to GPIO write
  gpio_write(light_pin[statnum],on==0); //low if on, high if off
}




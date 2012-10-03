#ifndef gpio_h
#define gpio_h

#include "LPC214x.h"

void set_light(int num, int onoff);

inline void set_pin(int pin, int mode) {
  int mask=~(0x3 << ((pin & 0x0F)<<1));
  int val=mode << ((pin & 0x0F)<<1);
  if(pin>=16) {
    PINSEL1=(PINSEL1 & mask) | val;
  } else {
    PINSEL0=(PINSEL0 & mask) | val;
  }
}

inline void gpio_set_write(int pin) {
  IODIR0|= (1 << pin); //Set the selected bit, leave the others alone
}

inline void gpio_set_read(int pin) {
  IODIR0&=~ (1 << pin); //Clear the selected bit, leave the others alone
}

inline void set_pin(int pin, int mode, int write) {
  set_pin(pin,mode);
  if(write) {
    gpio_set_write(pin);
  } else {
    gpio_set_read(pin);
  }
}

inline int get_pin(int pin) {
  if(pin>=16) {
    return PINSEL1>>(pin & 0x0F) & 0x03;
  } else {
    return PINSEL0>>(pin & 0x0F) & 0x03;
  }
}

inline void gpio_write(int pin, int level) {
  if(level==0) {
    IOCLR0=(1<<pin);
  } else {
    IOSET0=(1<<pin);
  }
}

inline int gpio_read(int pin) {
  return (IOPIN0 >> pin) & 1; 
}

extern const int pinMap[];
#endif


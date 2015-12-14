#ifndef gpio_h
#define gpio_h

#include "LPC214x.h"

#define ON   1
#define OFF  0

#define LOW 0
#define HIGH 1

static inline void set_pin(int pin, int mode) {
  int shift=((pin & 0x0F)<<1);
  int mask=~(0x3 << shift);
  int val=   mode << shift;
  PINSEL(pin>=16?1:0)=(PINSEL(pin>=16?1:0) & mask) | val;
}

static inline void gpio_set_write(int pin) {
  IODIR(0)|= (1 << pin); //Set the selected bit, leave the others alone
}

static inline void gpio_set_read(int pin) {
  IODIR(0)&=~ (1 << pin); //Clear the selected bit, leave the others alone
}

static inline void set_pin(int pin, int mode, int write) {
  set_pin(pin,mode);
  if(write) {
    gpio_set_write(pin);
  } else {
    gpio_set_read(pin);
  }
}

static inline int get_pin(int pin) {
  return PINSEL(pin>=16?1:0)>>(pin & 0x0F) & 0x03;
}

static inline void gpio_write(int pin, int level) {
  if(level==0) {
    IOCLR(0)=(1<<pin);
  } else {
    IOSET(0)=(1<<pin);
  }
}

static inline int gpio_read(int pin) {
  return (IOPIN(0) >> pin) & 1; 
}

extern const int light_pin[];

static inline void set_light(int statnum, int on) {
  set_pin(light_pin[statnum],0,1); //Set pin to GPIO write
  gpio_write(light_pin[statnum],on==0); //low if on, high if off
}

void blinklock(int blinkcode);
void flicker(int on);
void flicker(void);

#endif


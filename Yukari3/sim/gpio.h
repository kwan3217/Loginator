#ifndef gpio_h
#define gpio_h

#include "LPC214x.h"
#include "Time.h"

#define ON   1
#define OFF  0

#define LOW 0
#define HIGH 1

void set_pin(int pin, int mode);
void gpio_set_write(int pin);
void gpio_set_read(int pin);
void set_pin(int pin, int mode, int write);
int get_pin(int pin);
void gpio_write(int pin, int level);
int gpio_read(int pin);
extern const int light_pin[];
void set_light(int statnum, int on);
void blinklock(int blinkcode);
void flicker(int on);
void flicker(void);

#endif


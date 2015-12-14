#ifndef TIME_H
#define TIME_H

#include <inttypes.h>
#include "LPC214x.h"

/** Crystal Oscillator rate in Hz. 12MHz in original Logomatic, all Loginator and Rocketometer circuits, and any LPC214x which is intended to use USB */
static const int32_t FOSC=12000000;
/** Peripheral clock rate in Hz */
extern unsigned int PCLK;
extern unsigned int CCLK;
/** Number of seconds between resets of the main timer count. Since the timer count is a 32-bit number, this must be less than 71.5 seconds (71, since it's an integer).
Values used in the past have included 1 and 60, the current preferred value. This makes the timer roll over only once per minute. */
static const unsigned int timerSec=60;
/** Number of PCLK ticks per minute (number of timerSec seconds). Note that this is not minus 1, so the timer reset value should be set to one less than this */
extern unsigned timerInterval;
/** Number of ticks difference between timer0 and timer1. This value is positive if timer0 was started before timer1 (the current case by setup_clock) */
extern int32_t dtc01;
/** Number of ticks between a read of one timer and an immediately subsequent read of the other */
extern int32_t timer_read_ticks;

void setup_clock(void);
void delay(unsigned int ms);
void setup_pll(unsigned int channel, unsigned int M);
void set_rtc(int y, int m, int d, int h, int n, int s);
void time_mark(void);
static inline int msPassed(uint32_t TC0) {
  uint32_t TC1=TTC(0);
  uint32_t TC1c=TC1+((TC1<TC0)?timerInterval:0);
  uint32_t dtms=(TC1c-TC0)/(PCLK/1000);
  return dtms;
}

/**Given a timer value from timer1, return the timer value for timer0 at the same instant. Depends on measurements
   conducted in setup_clock(), will not be valid if either timer is stopped, reset, or jammed to a different value.
   Currently only valid if timer1 is started AFTER timer0, which is how setup_clock works -- IE It handles the case
   where the value walks off the end of the second, but not backwards off the beginning, which can only happen if
   timer1 is started first.
 @param ttc1 - timer value on timer1
 @return equivalent value of timer0 at the same instant
*/
static inline uint32_t timer1_to_timer0(uint32_t ttc1) {
  uint32_t ttc0=ttc1+dtc01;
  if(ttc0>=(PCLK*timerSec)) ttc0-=(PCLK*timerSec);
  return ttc0;
}
#endif


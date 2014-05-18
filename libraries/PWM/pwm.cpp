/* 
Loginator library PWM - handle Pulse Width Modulation

(C) 2014 Chris Jeppesen, Licensed under GPL 3.0 or later

Partially derived from GPL code as follows, 
from http://www.ocfreaks.com/lpc2148-pwm-programming-tutorial/

(C) Umang Gajera | Power_user_EX - www.ocfreaks.com 2011-13.
More Embedded tutorials @ www.ocfreaks.com/cat/embedded

LPC2148 PWM Tutorial Example 1 - RC Servo Control using PWM.
License : GPL.

*/
#include "pwm.h"
#include "gpio.h"
#include "Serial.h"
                           //0  1  2  3  4  5  6
static const int pwmP0[]  ={-1, 0, 7, 1, 8,21, 9};
static const int pwmMode[]={-1, 2, 2, 2, 2, 1, 2};

void initPWM(const unsigned char channelMask, const unsigned int prescale, const unsigned int period, const unsigned int defaultPulse) {
  /*Assuming that PLL0 has been setup with CCLK = 60Mhz and PCLK also = 60Mhz.*/
  /*This is a per the Setup & Init Sequence given in the tutorial*/

  //Set PWM outputs on channels selected by mask
  for(int i=1;i<=6;i++) if(((channelMask >> i) & 1) == 1) {
    set_pin(pwmP0[i],pwmMode[i]);
    PWMMR(i) = defaultPulse;
  }
  PWMPR = prescale-1; // 1 micro-second resolution
  PWMMR0 = period; // 20ms = 20k us - period duration
  PWMMCR = (1<<1); // Reset PWMTC on PWMMR0 match
  PWMLER = channelMask | (1<<0); // update MR0 and selected channels
  PWMPCR = channelMask<<8; // enable PWM output on selected channels, single-edge on all channels
  PWMTCR = (1<<1) ; //Reset PWM TC & PR

  //Now , the final moment - enable everything
  PWMTCR = (1<<0) | (1<<3); // enable counters and PWM Mode
}

void setPWM(const unsigned char channelMask, const unsigned int pulse[]) {
  for(int i=1;i<=6;i++) if(((channelMask >> i) & 1) == 1) {
    PWMMR(i) = pulse[i];
  }
  PWMLER |= channelMask; 
}

void setPWM(const unsigned char channel, const unsigned int pulse) {
  static unsigned int pulses[6];
  pulses[channel]=pulse;
  setPWM(1 << channel,pulses);
}

void setServo(const unsigned char channel, const signed char val) {
  //Server counts in microseconds, calculate how many counts. val=-128 is 2ms (2000us), val=127 is 1ms (1000us)
  const int center=1500; //zero value in us
  const int scale=500;   //maximum deviation in us
  const int us=center-((int)val)*scale/128; //number of us to set to
  setPWM(channel,us);
}

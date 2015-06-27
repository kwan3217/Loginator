#ifndef pwm_h
#define pwm_h

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


void initPWM(const unsigned char channelMask, const unsigned int prescale=60, const unsigned int period=20000, const unsigned int defaultPulse=1500);
void setPWM(const unsigned char channel, const unsigned int pulse[]);
void setPWM(const unsigned char channel, const unsigned int pulse);
void setServo(const unsigned char channel, const signed char val);

#endif

#ifndef LPCDUINO_H
#define LPCDUINO_H

#include "gpio.h"

/* LPCduino.h - functions intended to emulate system peripheral functions in 
   Arduino library -- mostly GPIO and analog read.
*/

#define OUTPUT 0
#define INPUT  1


void pinMode(int pin, int mode);
void digitalWrite(int pin, int level);
int digitalRead(int pin);
void analogWrite(int pin, int val);
int analogRead(int pin);

#endif

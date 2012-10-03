#ifndef TIME_H
#define TIME_H

#define FOSC 12000000
extern unsigned int PCLK,CCLK;

void setup_clock(void);
void delay(unsigned int);

#endif


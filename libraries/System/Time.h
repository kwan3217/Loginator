#ifndef TIME_H
#define TIME_H

#define FOSC 12000000
extern unsigned int PCLK,CCLK;

void setup_clock(void);
void delay(unsigned int ms);
void setup_pll(unsigned int channel, unsigned int M);
void set_rtc(int y, int m, int d, int h, int n, int s);
  
#endif


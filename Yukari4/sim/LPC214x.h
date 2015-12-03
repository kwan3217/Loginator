#ifndef LPC214x_h
#define LPC214x_h

unsigned int TTC(int channel);
unsigned int& TCR(int channel, int port);
unsigned int& TCCR(int channel);
unsigned int HW_TYPE();
unsigned int HW_SERIAL();
unsigned int MAMCR();
unsigned int MAMTIM();
unsigned int PLLSTAT(int channel);
unsigned int VPBDIV();
unsigned int PREINT();
unsigned int PREFRAC();
unsigned int CCR();
unsigned int I2CCONSET(int port);
unsigned int& RTCHOUR();
unsigned int& RTCMIN();
unsigned int& RTCSEC();
unsigned int& RTCYEAR();
unsigned int& RTCMONTH();
unsigned int& RTCDOM();
unsigned int& RTCDOW();
unsigned int& RTCDOY();

#endif

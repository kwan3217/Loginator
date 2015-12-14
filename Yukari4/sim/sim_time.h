#ifndef sim_time_h
#define sim_time_h

unsigned int TTC(int channel);
unsigned int& TCR(int channel, int port);
unsigned int& TCCR(int channel);
unsigned int& RTCHOUR();
unsigned int& RTCMIN();
unsigned int& RTCSEC();
unsigned int& RTCYEAR();
unsigned int& RTCMONTH();
unsigned int& RTCDOM();
unsigned int& RTCDOW();
unsigned int& RTCDOY();

#endif

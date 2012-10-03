#ifndef pinger_h
#define pinger_h

#include "sensor.h"

void setupPinger(circular* buf);
void readPinger(sensor* this, unsigned int TC);
void calPinger(sensor* this);
int checkPinger(sensor* this);
void writePinger(sensor* this, circular* buf);

#endif

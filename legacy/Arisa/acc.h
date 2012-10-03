#ifndef acc_h
#define acc_h

#include "sensor.h"

void readAcc(sensor* this, unsigned int TC);
void calAcc(sensor* this);
int checkAcc(sensor* this);
void writeAcc(sensor* this, circular* buf);
void setupAcc(circular* buf);
void ackAcc(void);

extern fp extraAccScale;

#endif

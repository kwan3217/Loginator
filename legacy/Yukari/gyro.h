#ifndef gyro_h
#define gyro_h

#include "circular.h"
#include "sensor.h"

void readGyro(sensor* this, unsigned int TC);
void parseGyro(sensor* this);
void calGyro(sensor* this);
int checkGyro(sensor* this);
void writeGyro(sensor* this, circular* buf);
void setupGyro(circular* buf);

extern fp xg_extra,yg_extra,zg_extra;

#endif

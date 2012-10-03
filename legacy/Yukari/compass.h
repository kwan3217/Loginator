#ifndef compass_h
#define compass_h

#include "circular.h"
#include "float.h"
#include "sensor.h"

void setupCompass(circular* buf);
void readCompass(sensor* this, unsigned int TC);
void parseCompass(sensor* this);
void calCompass(sensor* this);
int checkCompass(sensor* this);
void writeCompass(sensor* this, circular* buf);

#endif

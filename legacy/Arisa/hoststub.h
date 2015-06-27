#ifndef HOSTSTUB_H
#define HOSTSTUB_H

#include "float.h"

extern const int CCLK;

int fill(circular* buf, char c);
int fillString(circular* buf, char* s);
void fillPktStart(circular* buf,int type);
void fillPktFinish(circular* buf);
int fillPktByte(circular* buf, char in);
int fillPktChar(circular* buf, char in);
int fillPktShort(circular* buf, short in);
int fillPktFP(circular* buf, fp f);
int fillPktFloatExact(circular* buf, float f);

#endif

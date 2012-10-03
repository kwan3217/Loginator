#ifndef sdbuf_h
#define sdbuf_h

#include "circular.h"
#include "fat16.h"

extern struct fat16_file_struct ouf;
extern circular sdBuf;

int openSD(char*);
int flushSDLast(void);
void drainToSD(circular* buf);

#endif

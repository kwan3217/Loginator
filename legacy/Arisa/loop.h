#ifndef loop_h
#define loop_h

#include "circular.h"

void loop(void);
void time0ISR(void);
void time1ISR(void);
extern int readyForOncePerSecond;
extern circular spiBuf;
#endif

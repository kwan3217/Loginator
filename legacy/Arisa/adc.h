#ifndef adc_h
#define adc_h

#undef HILEVEL

//Low level direct read
void convertBothADC(int AD0, int AD1);
int readoutADC0(void);
int readoutADC1(void);

#ifdef HILEVEL
#include "circular.h"
#include "setup.h"

extern circular adcBuf;
extern char hasNewADC;

//High level timed read
void writeADCsetup(void);
void startRecordADC(void);
void displayAcc(void);
void displayRot(void);
void writeADC(void);
#endif
#endif

#ifndef adc_h
#define adc_h

#include "circular.h"
#include "setup.h"

#ifdef ADC_SYS

extern circular adcBuf;

void writeADCsetup(void);
void startRecordADC(void);
void displayAcc(void);
void setGSense(void);
void writeAdjGainPkt(circular* buf, char from, char to);
void adjustGain(void);
void displayRot(void);

#endif

#endif

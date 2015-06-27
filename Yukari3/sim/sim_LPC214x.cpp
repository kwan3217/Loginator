#include "LPC214x.h"
#include "robot.h"
#include <inttypes.h>

unsigned int TTC(int channel) {return state->ttc%(uint32_t(60)*1000000*60);}
unsigned int& TCR(int channel, int port) {return state->TCR[channel][port];}
unsigned int& TCCR(int channel) {return state->TCCR[channel];}
unsigned int HW_TYPE() {return 3;}
unsigned int HW_SERIAL() {return 1;}
unsigned int MAMCR() {return 0;}
unsigned int MAMTIM() {return 0;}
unsigned int PLLSTAT(int channel) {return 0;}
unsigned int VPBDIV() {return 0;}
unsigned int PREINT() {return 0;}
unsigned int PREFRAC() {return 0;}
unsigned int CCR() {return 0;}
unsigned int I2CCONSET(int port) {return (1 << 3);} //SI in StateTwoWire.h

unsigned int& RTCHOUR() {return state->rtcHour ;}
unsigned int& RTCMIN()  {return state->rtcMin  ;}
unsigned int& RTCSEC()  {return state->rtcSec  ;}
unsigned int& RTCYEAR() {return state->rtcYear ;}
unsigned int& RTCMONTH(){return state->rtcMonth;}
unsigned int& RTCDOM()  {return state->rtcDom  ;}
unsigned int& RTCDOW()  {return state->rtcDow  ;}
unsigned int& RTCDOY()  {return state->rtcDoy  ;}



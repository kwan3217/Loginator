#include "LPC214x.h"
#include "robot.h"
#include <inttypes.h>

unsigned int TTC(int channel) {return state->ttc%(uint32_t(60)*1000000*60);}
unsigned int& TCR(int channel, int port) {return state->TCR[channel][port];}
unsigned int& TCCR(int channel) {return state->TCCR[channel];}
unsigned int& RTCHOUR() {return state->rtcHour ;}
unsigned int& RTCMIN()  {return state->rtcMin  ;}
unsigned int& RTCSEC()  {return state->rtcSec  ;}
unsigned int& RTCYEAR() {return state->rtcYear ;}
unsigned int& RTCMONTH(){return state->rtcMonth;}
unsigned int& RTCDOM()  {return state->rtcDom  ;}
unsigned int& RTCDOW()  {return state->rtcDow  ;}
unsigned int& RTCDOY()  {return state->rtcDoy  ;}


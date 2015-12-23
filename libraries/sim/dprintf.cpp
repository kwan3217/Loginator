#include "sim.h"
#include <stdio.h>
#include <stdarg.h>

const char* enumNames[]=   {"SPI","SD", "Register","SD transfer","SCB","UART","SPI","I2C","I2C","HMC5883","SSP","SSP","Gyro","Playback"};
const bool streamEnabled[]={false,false,false,      false,        true, true,  false,false,false,true,     true, true, true,  true};
static DebugStream lastStream;

void dprintf(DebugStream stream, const char* pattern, ...) {
  lastStream=stream;
  if(!streamEnabled[stream]) return;
  fprintf(stderr,"%20s: ",enumNames[int(stream)]);

  va_list args;
  va_start(args,pattern);
  vfprintf(stderr,pattern,args);
  va_end(args);
}

void dnprintf(const char* pattern, ...) {
  if(!streamEnabled[lastStream]) return;
  va_list args;
  va_start(args,pattern);
  vfprintf(stderr,pattern,args);
  va_end(args);
}

#include "sim.h"
#include <stdio.h>
#include <stdarg.h>

const char* enumNames[]={"SPI","SD","Register","SD transfer","SCB","UART","SPI"};
const bool streamEnabled[]={true,true,true,      true,         true, true,  false};
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

//#include "Serial.h"
#include "registers.h"
//#include "dump.h"

//IntelHex d(Serial);

void setup() {
//  Serial.begin(38400);
//  d.region(0x1FFFF'0000,65536);
}

void loop() {
  for(int i=0x4000'C000;i<=0x4000'C054;i+=4) {
    for(int j=3;j>=0;j--) {
      unsigned char c=(i>>(j*8) & 0xFF);
      unsigned char c1=c & 0x0F;
      unsigned char c2=(c>>4) & 0x0F;
      while (!(ULSR(0) & (1<<5))); UTHR(0)=c2+(c2>9?('A'-10):'0');
      while (!(ULSR(0) & (1<<5))); UTHR(0)=c1+(c1>9?('A'-10):'0'); 
    }
    while (!(ULSR(0) & (1<<5))); UTHR(0)=':';

    unsigned int val=*((unsigned int*)i);
    for(int j=3;j>=0;j--) {
      unsigned char c=(val>>(j*8) & 0xFF);
      unsigned char c1=c & 0x0F;
      unsigned char c2=(c>>4) & 0x0F;
      while (!(ULSR(0) & (1<<5))); UTHR(0)=c2+(c2>9?('A'-10):'0');
      while (!(ULSR(0) & (1<<5))); UTHR(0)=c1+(c1>9?('A'-10):'0'); 
    }
    while (!(ULSR(0) & (1<<5))); UTHR(0)=13;
    while (!(ULSR(0) & (1<<5))); UTHR(0)=10;
  }

  for(int i=0x4002'C000;i<=0x4002'C080;i+=4) {
    for(int j=3;j>=0;j--) {
      unsigned char c=(i>>(j*8) & 0xFF);
      unsigned char c1=c & 0x0F;
      unsigned char c2=(c>>4) & 0x0F;
      while (!(ULSR(0) & (1<<5))); UTHR(0)=c2+(c2>9?('A'-10):'0');
      while (!(ULSR(0) & (1<<5))); UTHR(0)=c1+(c1>9?('A'-10):'0'); 
    }
    while (!(ULSR(0) & (1<<5))); UTHR(0)=':';

    unsigned int val=*((unsigned int*)i);
    for(int j=3;j>=0;j--) {
      unsigned char c=(val>>(j*8) & 0xFF);
      unsigned char c1=c & 0x0F;
      unsigned char c2=(c>>4) & 0x0F;
      while (!(ULSR(0) & (1<<5))); UTHR(0)=c2+(c2>9?('A'-10):'0');
      while (!(ULSR(0) & (1<<5))); UTHR(0)=c1+(c1>9?('A'-10):'0'); 
    }
    while (!(ULSR(0) & (1<<5))); UTHR(0)=13;
    while (!(ULSR(0) & (1<<5))); UTHR(0)=10;
  }
  for(int i=0x2009'8000;i<=0x2009'8080;i+=4) {
    for(int j=3;j>=0;j--) {
      unsigned char c=(i>>(j*8) & 0xFF);
      unsigned char c1=c & 0x0F;
      unsigned char c2=(c>>4) & 0x0F;
      while (!(ULSR(0) & (1<<5))); UTHR(0)=c2+(c2>9?('A'-10):'0');
      while (!(ULSR(0) & (1<<5))); UTHR(0)=c1+(c1>9?('A'-10):'0'); 
    }
    while (!(ULSR(0) & (1<<5))); UTHR(0)=':';

    unsigned int val=*((unsigned int*)i);
    for(int j=3;j>=0;j--) {
      unsigned char c=(val>>(j*8) & 0xFF);
      unsigned char c1=c & 0x0F;
      unsigned char c2=(c>>4) & 0x0F;
      while (!(ULSR(0) & (1<<5))); UTHR(0)=c2+(c2>9?('A'-10):'0');
      while (!(ULSR(0) & (1<<5))); UTHR(0)=c1+(c1>9?('A'-10):'0'); 
    }
    while (!(ULSR(0) & (1<<5))); UTHR(0)=13;
    while (!(ULSR(0) & (1<<5))); UTHR(0)=10;
  }
  for(;;); //Don't run any farther
}

#include "Serial.h"
#include "registers.h"
//#include "dump.h"
#include <math.h>
//IntelHex d(Serial);

#define sqrtf(f) __builtin_sqrtf((f))

void NMI_Handler() {
  Serial.println("NMI Handler");
}

void HardFault_Handler() {
  Serial.println("HardFault Handler");
}

void MMFault_Handler() {
  Serial.println("MMFault Handler");
}

void BusFault_Handler() {
  Serial.println("BusFault Handler");
}

void UsageFault_Handler() {
  Serial.println("UsageFault Handler");
}

void SVCall_Handler() {
  Serial.println("SVCall Handler");
}

void PendSV_Handler() {
  Serial.println("PendSV Handler");
}

void SysTick_Handler() {
  Serial.println("SysTick Handler");
}

void setup() {
  //Turn on FPU
  volatile unsigned int& CPACR=*((unsigned int*)0xE000'ED88);
  CPACR|=(0x0F<<20);
  Serial.begin(38400);
//  d.region(0x1FFFF'0000,65536);
}

void printreg(unsigned int i0, unsigned int i1) {
  for(unsigned int i=i0;i<=i1;i+=4) {
    Serial.print(i,HEX,8);
    Serial.print(": ");

    Serial.println(*((unsigned int*)i),HEX,8);
  }
  Serial.println();
}

void printreg(unsigned int i) {
  printreg(i,i);
}

float
mysqrt (float f,float g)
{
  return sqrtf(f*g);
}

typedef void (*fvoid)(void);

void loop() {
  printreg(0,0x1fc);
  //remap the memory at address 0
  volatile unsigned int& MEMMAP=*((unsigned int*)0x400F'C040);
  MEMMAP=1;
  printreg(0,0x1fc);
  volatile float f=2.0;
  f=sqrtf(f*2.7f);
  Serial.println(f,6); 
  for(;;); //Don't run any farther
}

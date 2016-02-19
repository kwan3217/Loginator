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

void setup() {
  //Turn on FPU
  volatile unsigned int& CPACR=*((unsigned int*)0xE000'ED88);
  CPACR|=(0x0F<<20);
  //Map our vector table into place
  volatile unsigned int& MEMMAP=*((unsigned int*)0x400F'C040);
  MEMMAP=1;
  
  Serial.begin(38400);
  printreg(0x0010043c);

  //There appears to be a block of registers at around 0x0010'0000 and a lock bit 
  //for it at 0x0020'0000, perhaps in turn with a lock bit for it at 0x0x0020'0010. Experiment with these bits.
  volatile unsigned int& lockHi=*((unsigned int*)0x0020'0010);
  volatile unsigned int& lockLo=*((unsigned int*)0x0020'0000);
  lockHi=7;
  lockLo=0x145;
  for(int i=0;i<15;i++) CPACR=CPACR;
  lockLo|=(1<<6);
  for(int i=0;i<15;i++) CPACR=CPACR;
  printreg(0x0010043c);
  lockLo&=~(1<<6);
  for(int i=0;i<15;i++) CPACR=CPACR;
  printreg(0x0010043c);
}

typedef void (*fvoid)(void);

void loop() {
  volatile float f=2.0;
  f=sqrtf(f*2.7f);
  Serial.println(f,6); 
  for(;;); //Don't run any farther
}

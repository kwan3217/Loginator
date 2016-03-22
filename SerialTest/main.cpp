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

template<int N> void IRQ_Handler(void);
template<>
void IRQ_Handler<0>() {
  Serial.println("IRQ0 Handler");
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

asm("delay_cycles: \n\
subs    r0, #10 \n\
movs    r1, #5  \n\
udiv    r0, r0, r1 \n\
adds    r0, r0, #1 \n\                               
b.n     delay_cycles+0xe\n\
subs    r0, r0, #1\n\
cmp     r0, #0\n\
bne.n   delay_cycles+0xc\n\
bx      lr\n");

asm("lock_bootblock: \n\
mov.w   r0, #2097152 \n\
ldr     r1, [r0, #0] \n\
bic.w   r1, r1, #64 \n\
str     r1, [r0, #0] \n\
movs    r0, #16 \n\
b.n     delay_cycles \n");

asm("unlock_bootblock: \n\
mov.w   r0, #2097152 \n\
ldr     r1, [r0, #0] \n\
orr.w   r1, r1, #64 \n\
str     r1, [r0, #0] \n\
movs    r0, #16 \n\
b.n     delay_cycles \n");

extern "C" {
void delay_cycles(int cycles);
void lock_bootblock();
void unlock_bootblock();
}

volatile unsigned int& IOCON_P0_0=*((unsigned int*)0x4002'c000);
volatile unsigned int& DIR0      =*((unsigned int*)0x2009'8000);
volatile unsigned int& PIN0      =*((unsigned int*)0x2009'8014);
volatile unsigned int& SET0      =*((unsigned int*)0x2009'8018);
volatile unsigned int& CLR0      =*((unsigned int*)0x2009'8018);
volatile unsigned int& MASK0     =*((unsigned int*)0x2009'8010);
volatile unsigned int& CPACR     =*((unsigned int*)0xE000'ED88);
typedef void (*fvoid)(void);

void setup() {
  //Turn on FPU
  CPACR|=(0x0F<<20);
  //Map our vector table into place
  MEMMAP()=1;

  //Turn on GPIO (directly with registers to start, then adapt libraries)
  PCONP()|=(1<<15); //Turn on GPIO subsystem (should already be on)
  IOCON_P0_0=(0<< 0) |  //Function 0, GPIO
             (0<< 3) |  //No pullup/pulldown
             (0<< 5) |  //No hysteresis
             (0<< 6) |  //Not inverted
             (0<< 9) |  //Output slew enabled
             (0<<10) ;  //Normal push-pull mode (not simulated open-drain)
  DIR0=(1<<0); //Set pin 0 to output
  SET0=(1<<0); //Set pin to high
  MASK0=0; //Enable all bits 
  Serial.begin(38400);
  printreg(0x0020'0000,0x0020'0010);  
  volatile fp f=sqrtf(2.0f*2.7f);
  Serial.println(f,6); 
  unlock_bootblock();
  lock_bootblock();
  Serial.println(f,6); 
//  lock_bootblock();
}


void loop() {
  static int i=0;
  if (i==1) {
    SET0=1;
  } else {
    CLR0=1;
  }
  i=1-i;
}

#include "Time.h"
#include "LPC214x.h"

unsigned int PCLK,CCLK;

static void measurePCLK(void) {
  CCLK=FOSC*((PLLSTAT & 0x1F)+1);
  switch (VPBDIV & 0x03) {
    case 0:
      PCLK=CCLK/4;
      break;
    case 1:
      PCLK=CCLK;
      break;
    case 2:
      PCLK=CCLK/2;
      break;
    case 3:
      break;
  }
}

void setup_clock(void) {
  // Setting peripheral Clock (pclk) to System Clock (cclk)
  VPBDIV=0x1;

  measurePCLK();

  //Set up Timer0 to count up to 1 second at full speed, then auto reset with no interrupt.
  //This is needed for the accurate delay function and the task manager
  TTCR(0) = (1 << 1);       // Reset counter and prescaler and halt timer
  TCTCR(0) = 0; //Drive timer from PCLK, not external pin
  TMCR(0) = (1 <<1);     // On MR0, reset but no int.
  TMR0(0) = PCLK;  //Reset when timer equals PCLK rate, effectively once per second
  TPR(0) = 0;  //No prescale, 1 timer tick equals 1 PCLK tick
  TTCR(0) = (1 << 0); // start timer


  //Set up PWM timer to support analogWrite(). Ticks at 1MHz, resets every 1024 ticks. 
  PWMPR    = PCLK/1000000-1;      /* Load prescaler  */
  PWMPCR = (1 << 13);             /* Enable PWM5 and no others, single sided */
  PWMMCR = (1 <<  1);             /* On match with timer reset the counter   */
  PWMMR0 = 0x400;                 /* set cycle rate to 1024 ticks            */
  PWMMR5 = 0x200;                 /* set edge of PWM5 to 512 ticks           */
  PWMLER = (1 <<  0) | (1 << 5);  /* enable shadow latch for match 0 and 5   */
  PWMTCR = 0x00000002;            /* Reset counter and prescaler             */
  PWMTCR = 0x00000009;            /* enable counter and PWM, release counter from reset */
}

/**accurate delay. Relies on Timer0 running without pause at PCLK and resetting
 at 1 second, as by setup_clock(). Code only reads, never writes, Timer0 registers */
void delay(unsigned int count) {
  int TC0=TTC(0);
  //count off whole seconds
  while(count>=1000) {
    //wait for the top of the second
    while(TTC(0)>TC0) ;
    //wait for the bottom of the second
    while(TTC(0)<TC0) ;
    count-=1000;
  }
  if(count==0) return;
  int TC1=TC0+count*(PCLK/1000);
  if(TC1>PCLK) {
    //Do this while we are waiting
    TC1-=PCLK;
    //wait for the top of the second
    while(TTC(0)>TC0) ;
  }
  //wait for the rest of the second
  while(TTC(0)<TC1) ;
}



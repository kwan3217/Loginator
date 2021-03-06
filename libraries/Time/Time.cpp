#include "Time.h"
unsigned int PCLK,CCLK,timerInterval;
int32_t timer_read_ticks, dtc01;

/** Measure the PCLK and CCLK rate. This is done by knowing the oscillator
    frequency and looking at the PLL registers for CCLK and the peripheral bus
    clock divider register for PCLK. This is idempotent so it can (should) be
    called in all the setup or begin routines which need PCLK, like Serial. */
void measurePCLK(void) {
  #if MCU == MCU_ARM7TDMI
  CCLK=FOSC*((PLLSTAT(0) & 0x1F)+1);
  switch (VPBDIV() & 0x03) {
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
  #endif

  #if MCU == MCU_CORTEXM4
// for LPC407x/408x
//  PCLK=CCLK/(PCLKSEL() & 0x03);
    CCLK=FOSC;
    PCLK=CCLK;
  #endif
}

//Number of days in each month, with 0 as a placeholder. the previous month.
                    //       J  F  M  A  M  J  J  A  S  O  N  D
const char monthTable[]={ 0,31,28,31,30,31,30,31,31,30,31,30,31};

void set_rtc(int y, int m, int d, int h, int n, int s) {
  RTCYEAR()=y;
  RTCMONTH()=m;
  RTCDOM()=d;
  RTCDOY()=0;
  for(int i=0;i<m;i++)RTCDOY()+=monthTable[m];
  RTCDOY()+=d;
  if((m>2) & ((y-2000)%4==0)) RTCDOY()++;
  if(y>2000) {
    int DOC=RTCDOY()-1+y/4+(y-2001)*365+1; //Day in centry beginning 1 Jan 2001, which was Monday
    RTCDOW()=(DOC % 7); //0-Sunday -- 6-Saturday
  } else RTCDOW()=0;
  RTCHOUR()=h;
  RTCMIN()=n;
  RTCSEC()=s;
}

//This is called by something like a PPS detector. If the PPS is in the first half
//of the second, just set the CTC count back to zero. If it's not, set the CTC to 
//zero but increment the RTC as well.
void time_mark() {
  bool botOfSecond=(CTC()& 0xFFFF)>=(1<<15);
  //Reset the subsecond counter
  CCR()|=(1<<1);
  CCR()&=~(1<<1);
  if(botOfSecond) {
    int s=RTCSEC();
    int n=RTCMIN();
    int h=RTCHOUR();
    int d=RTCDOM();
    int m=RTCMONTH();
    int y=RTCYEAR();
    s++;
    if(s>=60) {
      s-=60;
      n++;
      if(n>=60) {
        n-=60;
        h++;
        if(h>=24) {
          h-=24;
          d++;
          if(d>(monthTable[m]+(((m==2)&&(y%4==0))?1:0))) { //Take into account leap year, but not Gregorian calendar. Valid from 1901 to 2099
            d=1;
            m++;
            if(m>12) {
              m=1;
              y++;
            }
          }
        }
      }
    }
    set_rtc(y,m,d,h,n,s);
  }
}

/** Sets up clock-related systems. Does the following:
#Set PCLK to run at the same rate as CCLK
#Measure PCLK
#Reset both timers
#Reset the RTC if it is not within a reasonable date range
#Start the timers and RTC in as close to sync as possible
#Measure the difference between the timers
*/
void setup_clock(void) {
  static bool idempotent;
  if(idempotent) return;
  idempotent=true;
  timerInterval=PCLK*timerSec;

  //Set up Timer0 and Timer1 to count up to timerSec seconds at full speed, then auto reset with no interrupt.
  //This is needed for the accurate delay function and the task manager. Timer1 may be re-setup as needed, but
  //we do it here so that both are in as near as sync as possible.
  for(int i=0;i<2;i++) {
    TTCR(i) = (1 << 1);       // Reset counter and prescaler and halt timer
    TCTCR(i) = 0; //Drive timer from PCLK, not external pin
    TMCR(i) = (1 <<1);     // On MR0, reset but no int.
    TMR(i,0) = timerInterval-1;  //Reset when timer equals PCLK rate, effectively once per timerSec seconds
    TPR(i) = 0;  //No prescale, 1 timer tick equals 1 PCLK tick
  }
  //Turn off the real-time clock
  CCR()=0;
  //Set the PCLK prescaler and set the real-time clock to use it, so as to run in sync with everything else.
  PREINT()=PCLK/32768-1;
  PREFRAC()=PCLK-((PREINT()+1)*32768);

  //If the clock year is reasonable, it must have been set by
  //some process before, so we'll leave it running.
  //If it is year 0, then it is runtime from last reset,
  //so we should reset it.
  //If it is unreasonable, this is the first time around,
  //and we set it to zero to count runtime from reset
  if(RTCYEAR()<2000 || RTCYEAR()>2100) {
    CCR()|=(1<<1);
    set_rtc(0,0,0,0,0,0);
    //Pull the subsecond counter out of reset
    CCR()&=~(1<<1);
  }

  //These are close together so that Timer0, Timer1, and RTC are as near in-phase as possible
  TTCR(0) = (1 << 0); // start timer
  TTCR(1) = (1 << 0); // start timer
  CCR()|=(1<<0); //Turn the real-time clock on

  //Now that the timers are running, measure the difference between them
  //We expect that timer1 will be a few ticks less than timer0, since it
  //was started later.
  uint32_t ttc0,ttc1;
  int32_t dtc01_m,dtc10_m;

  //Because of the order we start the timers above, we expect dtc01 to be positive, but all the code below
  //works just as well if the clocks are started in the other order and dtc01 is negative.

  //measure the clock difference when reading ttc0 first. This should be *greater* than
  //the difference between the timers by the amount of time between reading the timers,
  //interpreted as the time it takes to read one timer. Since timer0 started first,
  //it should always read more than timer1 and this value should be positive.
  //Further, since it takes a nonzero amount of time to read a timer, this value should be more positive,
  //IE greater than the true difference between the timers. Since we run this code so soon after
  //zeroing and starting the timers, we won't worry about timer wraparound.
  ttc0=TTC(0);
  ttc1=TTC(1);
  dtc01_m=ttc0-ttc1; //should be positive

  //measure the clock difference when reading ttc1 first. This will be *less* than
  //the difference between the timers by the amount of time between reading the timers,
  //interpreted as the time it takes to read one timer. This value will be the sum of {the difference
  //between timer 1 and 0 (should be negative)} and the time it takes to read a timer.
  ttc1=TTC(1);
  ttc0=TTC(0);
  dtc10_m=ttc1-ttc0; //may be positive, negative, or zero

  //So now we have two equations, two knowns, and two unknowns, so it should be solvable:
  //known:
  //  dtc01_m, dtc10_m: measured values above.
  //unknown:
  //  dtc01 - the actual difference between timer 0 and 1, positive
  //          when timer 0 is ahead of (greater than) timer 1 at the
  //          same instant, which is the expected case
  //dtc01_m=dtc01+timer_read_ticks
  //dtc10_m=-dtc01+timer_read_ticks
  //Solve eq1 for timer_read_ticks
  //dtc01_m-dtc01=timer_read_ticks
  //Substitute into eq2
  //dtc10_m=-dtc01+dtc01_m-dtc01
  //dtc10_m=dtc01_m-2*dtc01
  //Solve for dtc01
  //dtc10_m-dtc01_m=-2*dtc01
  //dtc01_m-dtc10_m=2*dtc01
  dtc01=(dtc01_m-dtc10_m)/2;
  //Use that value to plug in and solve for timer_read_ticks
  timer_read_ticks=dtc01_m-dtc01;
}



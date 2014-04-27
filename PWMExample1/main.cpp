#include "LPC214x.h"
#include "gpio.h"

#define PLOCK 0x00000400
#define PWMPRESCALE 60   //60 PCLK cycles to increment TC by 1 i.e 1 Micro-second

const int channel=4;

void initPWM(void);

void setup() {
//  initClocks(); //Initialize CPU and Peripheral Clocks @ 60Mhz
  initPWM(); //Initialize PWM
}

void loop() {
  if( !((IOPIN0) & (1<<16)) ) { // Check P0.16
    PWMMR(channel) = 1000;
    PWMLER = (1<<channel); //Update Latch Enable bit for PWMMR1
  } else {
    PWMMR(channel) = 1750;
    PWMLER = (1<<channel);
  }
}
                           //0  1  2  3  4  5  6
static const int pwmP0[]  ={-1, 0, 7, 1, 8,21, 9};
static const int pwmMode[]={-1, 2, 2, 2, 2, 1, 2};

/*
(C) Umang Gajera | Power_user_EX - www.ocfreaks.com 2011-13.
More Embedded tutorials @ www.ocfreaks.com/cat/embedded

LPC2148 PWM Tutorial Example 1 - RC Servo Control using PWM.
License : GPL.
*/

void initPWM(void)
{
    /*Assuming that PLL0 has been setup with CCLK = 60Mhz and PCLK also = 60Mhz.*/
    /*This is a per the Setup & Init Sequence given in the tutorial*/

    //PINSEL0 = (1<<1); // Select PWM1 output for Pin0.0
    set_pin(pwmP0[channel],pwmMode[channel]);
    PWMPCR = 0x0; //Select Single Edge PWM - by default its single Edged so this line can be removed
    PWMPR = PWMPRESCALE-1; // 1 micro-second resolution
    PWMMR0 = 20000; // 20ms = 20k us - period duration
    PWMMR(channel) = 1000; // 1ms - pulse duration i.e width
    PWMMCR = (1<<1); // Reset PWMTC on PWMMR0 match
    PWMLER = (1<<channel) | (1<<0); // update MR0 and MR1
    PWMPCR = (1<<(channel+8)); // enable PWM output
    PWMTCR = (1<<1) ; //Reset PWM TC & PR

    //Now , the final moment - enable everything
    PWMTCR = (1<<0) | (1<<3); // enable counters and PWM Mode

    //PWM Generation goes active now!!
    //Now you can get the PWM output at Pin P0.0!
}

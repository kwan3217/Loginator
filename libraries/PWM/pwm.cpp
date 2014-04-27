#include "pwm.h"
#include "gpio.h"
                    //0  1  2  3  4  5  6
static const int pwmP0[]  ={-1, 0, 7, 1, 8,21, 9};
static const int pwmMode[]={-1, 2, 2, 2, 2, 1, 2};

void setup_pwm(const unsigned int prescale, const unsigned int period) {
  PWMPR    = prescale-1;   // PWM will tick once every prescale PCLK ticks 
  PWMMCR = (1 <<  1);      // On match with timer reset the counter   
  PWMMR(0) = period;       // set cycle rate          
  PWMLER |= (1 <<  0);     // enable shadow latch for match 0   
  PWMTCR = 0x00000002;     // Reset counter and prescaler       
  PWMPCR |= (0x3F << 9);   // Turn on all the PWMs
  PWMTCR = 0x00000009;     // enable counter and PWM, release counter from reset 
}

void setup_pwm(const unsigned int us) {
  setup_pwm(PCLK/1000000,us);
}

void pwm_write(const unsigned int channel, const unsigned int val) {
  gpio_set_write(pwmP0[channel]);           //Make sure the pin is in output mode
  set_pin(pwmP0[channel],pwmMode[channel]); //Put pin into PWM mode
  PWMMR(channel) = val;
  PWMPCR |= (1 << (channel+8));
  PWMLER |= (1 << channel);                 // Enable Shadow latch 
}

void servoWrite(const int channel, const signed char val) {
  //Server counts in microseconds, calculate how many counts. val=-128 is 2ms (2000us), val=127 is 1ms (1000us)
  const int center=1500; //zero value in us
  const int scale=500;   //maximum deviation in us
  const int us=center-((int)val)*scale/128; //number of us to set to
  pwm_write(channel,us);
}



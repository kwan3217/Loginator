#include "LPC214x.h"
#include "gpio.h"
#include "pwm.h"
#include "Time.h"

const unsigned char channelSteer=4;
const unsigned char channelThrottle=6;
const unsigned char channelMask=(1<<channelSteer)|(1<<channelThrottle);
#define PWMPRESCALE 60   //60 PCLK cycles to increment TC by 1 i.e 1 Micro-second

const int right=-1;
const int left=1;

void setup() {
//  initPWM(); //Initialize PWM
  initPWM(channelMask); 
  setServo(channelThrottle,0);
/*
  set_pin(0,0);
  gpio_set_write(0);
  set_pin(1,0);
  gpio_set_write(1);
  for(int i=0;i<5;i++) {
    gpio_write(0,0);
    delay(250);
    gpio_write(0,1);
    delay(250);
    gpio_write(1,0);
    delay(250);
    gpio_write(1,1);
    delay(250);
  }
  delay(1000);
  const int throttleMax=24;
  gpio_write(1,0);
  setServo(channelThrottle,throttleMax);
  delay(2000);
  setServo(channelSteer,127*right);
  delay(2000);
  setServo(channelSteer,0);
  delay(2000);
  setServo(channelThrottle,0);
  gpio_write(1,1);
  */
//  for(;;);
}

void loop() {
  for(signed char i=0;i<127;i++) {
    setServo(channelSteer,i);
    delay(10);
  }
/*
  for(signed char i=0;i<127;i++) {
    setServo(channelThrottle,i);
    delay(10);
  }
*/
  for(signed char i=127;i>0;i--) {
    setServo(channelSteer,i);
    delay(10);
  }
/*
  for(signed char i=127;i>0;i--) {
    setServo(channelThrottle,i);
    delay(10);
  }
*/
  delay(2000);
  for(signed char i=0;i>-128;i--) {
    setServo(channelSteer,i);
    delay(10);
  }
/*
  for(signed char i=0;i>-128;i--) {
    setServo(channelThrottle,i);
    delay(10);
  }
*/
  for(signed char i=-128;i<0;i++) {
    setServo(channelSteer,i);
    delay(10);
  }
/*
  for(signed char i=-128;i<0;i++) {
    setServo(channelThrottle,i);
    delay(10);
  }
*/
}



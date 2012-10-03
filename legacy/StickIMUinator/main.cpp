#include "LPC214x.h"
#include "system.h"
#include "main.h"
#include "Serial.h"
#include "i2c_bitbang.h"

static const int pwm_pin[]={8,9,21};
static const int pwm_channel[]={4,6,5};
static const int pwm_mode[]={2,2,1};
static const int pwm_mr[]={0x18,0x1C,0x20,0x24,0x40,0x44,0x48};

#define RED 0
#define GREEN 1
#define BLUE 2

#define PWMMR(channel)     (*(volatile unsigned long *)(PWM_BASE_ADDR + pwm_mr[channel]))

void analogWriteIMU(int channel, int val) {
  if(0==val) {
    set_pin(pwm_pin[channel],0); //Set pin to GPIB output
    gpio_set_write(pwm_pin[channel]);
    gpio_write(pwm_pin[channel],LOW);
  } else {
    set_pin(pwm_pin[channel],pwm_mode[channel]); //Set pin 0.21 to mode 1 (PWM5)
    PWMMR(pwm_channel[channel]) = val;
    PWMLER |= (1 << pwm_channel[channel]);  /* Enable Shadow latch */
  }
}

static void init_pwm_IMUinator(void) {
  PWMPR    = 9;                    /* Load prescaler. One tick is 1/6 microsecond, 10 PCLK ticks  */

  PWMPCR = (1 << (8+5)) | (1 << (8+4)) | (1 << (8+6));                       /* Enable PWM5 and no others, single sided */
  PWMMCR = (1 <<  1);                       /* On match with timer reset the counter   */
  PWMMR0 = 60000;                           /* set cycle rate to 6000 ticks, 1kHz            */
  PWMMR5 = 10;                           /* set edge of PWM5 to 512 ticks           */
  PWMMR6 = 10;                           /* set edge of PWM6 to 512 ticks           */
  PWMMR4 = 10;                           /* set edge of PWM6 to 512 ticks           */
  PWMLER = (1 <<  0) | (1 << 4)| (1 << 5)| (1 << 6);            /* enable shadow latch for match 0, 4, 5, 6   */
  PWMTCR = 0x00000002;                      /* Reset counter and prescaler             */
  PWMTCR = 0x00000009;                      /* enable counter and PWM, release counter from reset */
  analogWriteIMU(RED,0);
  analogWriteIMU(GREEN,0);
  analogWriteIMU(BLUE,0);
}

void blinklock(int blinkcode) {
  init_pwm_IMUinator();
  for(;;) {
    for(int i=0;i<32;i++) {
      analogWriteIMU(((blinkcode>>i) & 1)?GREEN:RED,255/2);
      delay(500);
      analogWriteIMU(((blinkcode>>i) & 1)?GREEN:RED,0);
      delay(500+((0==(i%4))?500:0));
    }
    analogWriteIMU(BLUE,255/2);
    delay(500);
    analogWriteIMU(BLUE,0);
    delay(500);
  }
}

static inline int putc_serial(int port,int ch) {
  while (!(ULSR(port) & 0x20));
  return (UTHR(port) = ch);
}

#define COMPASS_ADDR 0x17
char compassID[4]="IJK";

void setup() {
  init_pwm_IMUinator();
//  Serial.begin(0,9600);
//  int result=i2c_bitbang_setup(2, 3, 100000);
//  Serial.print("I2C setup result: ");
//  Serial.println(result,DEC);
}

void loop() {
/*  int result=i2c_bitbang_tx_string(COMPASS_ADDR,"\x02\x01",2);
  Serial.print("Result 1: ");
  Serial.print(result,DEC);
  result=i2c_bitbang_txrx_string(COMPASS_ADDR,"\x0A",1,compassID,3);
  Serial.print("Result 2: ");
  Serial.print(result,DEC);
  Serial.print("ID Code: ");
  Serial.print(compassID[0],HEX);
  Serial.print(compassID[1],HEX);
  Serial.print(compassID[2],HEX);
  Serial.print(" ");
  Serial.println(compassID);
  delay(500);
  */
//black->red
/*
  for(int i=0;i<255;i++) {
    analogWriteIMU(RED,i);
    delay(20);
  }
  delay(5000);
  analogWriteIMU(RED,0);
  delay(500);
//red->yellow  
  for(int i=0;i<255;i++) {
    analogWriteIMU(GREEN,i);
    delay(20);
  }

  delay(5000);
  analogWriteIMU(GREEN,0);
  delay(500);

//green->cyan  
  for(int i=0;i<255;i++) {
    analogWriteIMU(BLUE,i);
    delay(20);
  }

  delay(5000);
  analogWriteIMU(BLUE,0);
  delay(500);

  analogWriteIMU(RED,255);
  analogWriteIMU(BLUE,255);

//white->black  
  for(int i=0;i<500;i++) {
    analogWriteIMU(GREEN,i);
    delay(50);
    if(0 == (i % 50)) {
      analogWriteIMU(RED,0);
      analogWriteIMU(GREEN,0);
      analogWriteIMU(BLUE,0);
      delay(500);
      analogWriteIMU(RED,255);
      analogWriteIMU(GREEN,i);
      analogWriteIMU(BLUE,255);
    }
  }

  delay(5000);
  analogWriteIMU(RED,0);
  analogWriteIMU(GREEN,0);
  analogWriteIMU(BLUE,0);
*/
  analogWriteIMU(RED,2550);
  analogWriteIMU(GREEN,2550);
  analogWriteIMU(BLUE,2550);
  delay(65535);
}

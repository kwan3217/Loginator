#include "LPCduino.h"
#include "pwm.h"
//Pin 13 is STAT0 (red light)
//Pin 12 is STAT1 (green light)

//LPCduino numbers:
// 0 - P0.13, BATLVL
// 1 - P0.30, Loginator AD1/IC (Compass Interrupt)
// 2 - P0.29, Loginator AD2/IG (Gyro    Interrupt)
// 3 - P0.28, Loginator AD3/IA (Acc     Interrupt)
// 4 - P0.25, Loginator AD4/CSG (Chip Select Gyro)
// 5 - P0.22, Loginator AD5
// 6 - P0.21, Loginator AD6, PWM5
// 7 - P0.10, Loginator AD7
// 8 - P0.12, Loginator AD8/ADREF (2.5V reference)
// 9 - P0.20, Loginator CS1/CSA (Chip Select Acc)
//10 - P0.19, Loginator MOSI1
//11 - P0.18, Loginator MISO1
//12 - P0.17, Loginator SCK1
//13 - P0.02, Loginator SCL0, Logomatic STAT0
//14 - P0.11, Loginator SCL1, Logomatic STAT1
//15 - P0.08, Loginator TX1, PWMR, PWM4
//16 - P0.09, Loginator RX1,       PWM6

      //LPCduino number: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16
const int pinMap[]    ={13,30,29,28,25,22,21,10,12,20,19,18,17, 2,11, 8, 9};  //P0.XX number for LPCduino number
const int pinADCside[]={ 1, 0, 0, 0, 0, 1, 1, 1, 1};
const int pinADCchan[]={ 4, 3, 2, 1, 4, 7, 6, 2, 3};
const int pinADCmode[]={ 3, 1, 1, 1, 1, 1, 2, 3, 3};
const int pinPWMmode[]={ 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2};
const int pinPWMavail[]={0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1};
const int pinPWMchan[]={ 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6};

void pinMode(int pin, int mode) {
//  if(pin<=0 || pin>=sizeof(pinMap)/sizeof(int)) return; //DON'T do this to pin 0, BATLVL
  set_pin(pinMap[pin],0); //Make sure the selected pin is GPIO
  //In IODIR0, a set bit is output, cleared is input
  if(mode==INPUT) {
    gpio_set_read(pinMap[pin]); //Clear the selected bit, leave the others alone
  } else {
    gpio_set_write(pinMap[pin]); //Set the selected bit, leave the others alone
  }
}

void digitalWrite(int pin, int level) {
//  if(pin<=0 || pin>14) return; //DON'T do this to pin 0, BATLVL

  //On Arduino, Pin 13 is connected to the anode of an LED, cathode to ground
  //so high turns the light on. On Logomatic, the pin is the cathode, VCC is
  //anode, so reverse sense of level for the light "pins" only.
  //No exposed pins 14 and 13, so this doesn't affect anything on the outside
  //world.
  if(pin>=13) level=(level==HIGH?LOW:HIGH);
  gpio_write(pinMap[pin],level);
}

int digitalRead(int pin) {
  return gpio_read(pinMap[pin]) & 1; 
}

//Set sample clock to near 4.5MHz (Divisor 14). Much faster and
//marginally better noise than Divisor 255
#define ADCDIV 14

int analogRead(int pin) {
  if(pin<0 || pin>8) return -1;
  int oldMode=get_pin(pinMap[pin]);
  set_pin(pinMap[pin],pinADCmode[pin]);
  if(pinADCside[pin]==0) {
    AD0CR=(1<<21) | ((ADCDIV-1)<<8) | (1 << pinADCchan[pin]); //Turn on AD0, program the divisor, set the input channel
    AD0CR|=0x01000000; //Start conversion just on AD0
  } else {
    AD1CR=(1<<21) | ((ADCDIV-1)<<8) | (1 << pinADCchan[pin]); //Turn on AD1, program the divisor, set the input channel
    AD1CR|=0x01000000; //Start conversion just on AD1
  }
  int temp;
  do {
    if(pinADCside[pin]==0) {
      temp=AD0GDR; //Check if channel 0 is done
    } else {
      temp=AD1GDR; //Check if channel 1 is done
    }
  } while((temp & 0x80000000) == 0);
  int reading;
  if(pinADCside[pin]==0) {
    AD0CR=0x00000000; //Stop AD0
    reading=(AD0GDR & 0xFFC0) >> 6;
  } else {
    AD1CR=0x00000000; //Stop AD1
    reading=(AD1GDR & 0xFFC0) >> 6;
  }
  set_pin(pinMap[pin],oldMode);
  return reading;
}

void analogWrite(int pin, int val) {
  if(!pinPWMavail[pin]) return;
  set_pin(pinMap[pin],pinPWMmode[pin]); //Put pin into PWM mode
  PWMMR(pinPWMchan[pin]) = val;
  PWMLER |= (1 << pinPWMchan[pin]);                          /* Enable Shadow latch */
}

void setServoPeriod(const unsigned int usecPeriod) {
  PWMMR(0) = usecPeriod;
  PWMLER |= (1 <<  0);  /* enable shadow latch for match 0 and 5   */
}

void switchServo(const int pin, const bool on) {
  if(!pinPWMavail[pin]) return;
  if(on) {
    gpio_set_write(pinMap[pin]); 
  } else {
    gpio_set_read(pinMap[pin]); 
  }
  PWMPCR |= ((on?1:0) << (8+pinPWMchan[pin]));
}

void servoWrite(const int pin, const signed char val) {
  //Server counts in microseconds, calculate how many counts. val=-128 is 2ms (2000us), val=127 is 1ms (1000us)
  unsigned int us=1500-val*1000/128;
  analogWrite(pin,us);
}




#include "LPCduino.h"

//Pin 13 is STAT0 (red light)
//Pin 12 is STAT1 (green light)

                //P0.XX: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14
const int pinMap[]    ={13,30,29,28,25,22,21,10,12,20,19,18,17, 2,11};
const int pinADCside[]={ 1, 0, 0, 0, 0, 1, 1, 1, 1};
const int pinADCchan[]={ 4, 3, 2, 1, 4, 7, 6, 2, 3};
const int pinADCmode[]={ 3, 1, 1, 1, 1, 1, 2, 3, 3};
const int pinPWMavail[]={0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0};
const int pinPWMchan[]={ 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0};

void pinMode(int pin, int mode) {
  if(pin<=0 || pin>14) return; //DON'T do this to pin 0, BATLVL
  set_pin(pinMap[pin],0); //Make sure the selected pin is GPIO
  //In IODIR0, a set bit is output, cleared is input
  if(mode==INPUT) {
    gpio_set_read(pinMap[pin]); //Clear the selected bit, leave the others alone
  } else {
    gpio_set_write(pinMap[pin]); //Set the selected bit, leave the others alone
  }
}

void digitalWrite(int pin, int level) {
  if(pin<=0 || pin>14) return; //DON'T do this to pin 0, BATLVL

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
  set_pin(pinMap[pin],1); //Put pin into PWM mode
  PWMMR(pinPWMchan[pin]) = val;
  PWMLER = (1 << pinPWMchan[pin]);                          /* Enable Shadow latch */
}



/*********************************************************************************
 * Logomatic Version Kwan Firmware
 * Modified beyond recognition from original 2008 Sparkfun firmware
 * Kwan Systems 2009
 * ******************************************************************************/

/*******************************************************
 *          Header Files
 ******************************************************/
#include <stdio.h>
#include <string.h>
#include "LPC214x.h"

//Main application subfiles
#include "setup.h"
#include "loop.h"
#include "main.h"
#include "conparse.h"

/*******************************************************
 *            MAIN
 * Arduino-style structure here
 ******************************************************/

int main (void) {
  setup();
  for(;;) loop();
}

/*******************************************************
 *          Utility functions
 ******************************************************/
 
static int light_mask[3]={0x00000004,0x00000800,0x80000000};
//int padding[1024];

void set_light(int statnum, int onoff) {
  onoff=(onoff!=0);
  if(onoff){ 
    //on 
    IOCLR0 = light_mask[statnum]; 
  } else { 
    // Off
    IOSET0 = light_mask[statnum]; 
  } 
}

int blinkEnabled=0;

void blinklock(int checkBlinkEnabled, int blinkcode) {
  if(checkBlinkEnabled & !blinkEnabled) return;
  set_light(0,OFF);
  set_light(1,OFF);
  for(;;) {
    for(int i=0;i<32;i++) {
      set_light((blinkcode >> i) & 1,ON);
      delay(1000);
      set_light((blinkcode >> i) & 1,OFF);
      delay(1000);
    }
    set_light(0,ON);
    set_light(1,ON);
    delay(1000);
    set_light(0,OFF);
    set_light(1,OFF);
    delay(1000);
  }
}

void delay(int count) {
  int i;
  count *= 10000;
  for(i = 0; i < count; i++) asm volatile ("nop");
}

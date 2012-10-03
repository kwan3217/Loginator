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

int purposeMask=LP_PPS          | 
				LP_ALWAYS       ;

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

void set_light(int statnum, int onoff, int purpose) {
//  if(0==(purpose & purposeMask)) return;
  onoff=(onoff!=0);
//  if(statnum==2) onoff=1-onoff;
  if(onoff && !powerSave){ 
    //on 
    IOCLR0 = light_mask[statnum]; 
  } else { 
    // Off
    IOSET0 = light_mask[statnum]; 
  } 
}

void blinklock(int maintainWatchdog, int blinkcode) {
  if(blinkcode==0) {
    for(;;) {
      set_light(0,ON,LP_BLINKLOCK);
      delay(50);
      set_light(0,OFF,LP_BLINKLOCK);
      set_light(1,ON,LP_BLINKLOCK);
      delay(50);
      set_light(1,OFF,LP_BLINKLOCK);
    }
  } else {
    for(;;) {
      for(int i=0;i<blinkcode;i++) {
        set_light(0,ON,LP_BLINKLOCK);
        delay(250);
        set_light(0,OFF,LP_BLINKLOCK);
        delay(250);
      }
      set_light(1,ON,LP_BLINKLOCK);
      delay(250);
      set_light(1,OFF,LP_BLINKLOCK);
      delay(250);
    }
  }
}

void delay(int count) {
  int i;
  count *= 10000;
  for(i = 0; i < count; i++) asm volatile ("nop");
}

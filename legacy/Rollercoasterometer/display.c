#include <string.h>
#include "stringex.h"
#include "display.h"
#include "serial.h"
#include "LPC214x.h"
#include "gps.h"
#include "load.h"

int displayMode;  

char debugDisplay0[17];
char debugDisplay1[17];

void clearDisplay() {
  putstring_serial0("\xFE\x01");
}

void displayLightOff() {
  putstring_serial0("\x7C\x80");
  set_light(2,0);
  set_light(0,1);
}

void displayLightOn() {
  putstring_serial0("\x7C\x9D");
  set_light(2,1);
  set_light(0,1);
}

void incDisplayMode() {
  displayMode++;
  if(displayMode>4) displayMode=0;
}

static void displayDebug() {
  clearDisplay();
  for(int i=0;i<16;i++) {
    if(debugDisplay0[i]==0) {
      putc_serial0(' ');
    } else {
      putc_serial0(debugDisplay0[i]);
    }
  }
  for(int i=0;i<16;i++) {
    if(debugDisplay1[i]==0) {
      putc_serial0(' ');
    } else {
      putc_serial0(debugDisplay1[i]);
    }
  }
}

void display() {
  clearDisplay();
  switch(displayMode) {
    case DISP_CLOCK:
      displayClock();
      break;
    case DISP_LAT:
      displayCoords();
      break;
    case DISP_ACC:
      displayAcc();
      break;
    case DISP_ROT:
      displayRot();
      break;
    case DISP_DEB:
      displayDebug();
      break;
  }
}

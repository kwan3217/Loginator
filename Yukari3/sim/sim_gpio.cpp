#include <stdio.h>
#include <stdlib.h>

void blinklock(int code) {
  printf("Blinklock! Code %d (%08x)\n",code,code);
  exit(code);
}

void set_pin(int pin, int mode, int write) {

}



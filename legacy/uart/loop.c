#include "LPC214x.h"
#include "loop.h"
#include "setup.h"
#include "main.h"
#include "serial.h"
#include <stdio.h>

void loop(void) {
  char buf[255];
  sprintf(buf,"The quick brown fox jumps over the lazy dog!\r\n");
  if(!tx_serialz(0,buf)) sleep();
}


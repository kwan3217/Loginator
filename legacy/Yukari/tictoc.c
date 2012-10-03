#include "tictoc.h"
#include "pktwrite.h"
#include "i2c.h"

unsigned int last_tic;
unsigned int last_toc;

void writeTicToc(circular* buf) {
  //Write the results in a packet
  fillPktStart(buf,PT_I2C);
  buf->dataDigits=8;
  fillPktInt(buf,last_tic);
  fillPktInt(buf,last_toc);
  fillPktInt(buf,last_toc-last_tic);
  fillPktInt(buf,I2CSPEED);
  fillPktFinish(buf);
}


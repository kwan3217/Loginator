#include "motor.h"
#include "i2c_int.h"
#include "main.h"

void spinMotors(int drive, int steer) {
  if(drive>255) drive=255;
  if(drive<-255) drive=-255;
  if(steer>255) steer=255;
  if(steer<-255) steer=-255;

  char MTmsg[5];
  MTmsg[0]=1;
  if(drive>0) {
    MTmsg[1]=drive & 0xFF;
    MTmsg[2]=0;
  } else {
    MTmsg[1]=0;
    MTmsg[2]=(-drive) & 0xFF;
  }
  if(steer>0) {
    MTmsg[3]=steer & 0xFF;
    MTmsg[4]=0;
  } else {
    MTmsg[3]=0;
    MTmsg[4]=(-steer) & 0xFF;
  }
  i2c0.tx_string(0x55, MTmsg, 5);
}


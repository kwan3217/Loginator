#include "motor.h"
#include "i2c.h"
#include "main.h"

void spinMotors(int A, int B, int Tail) {
  if(A>255) A=255;
  if(A<0) A=0;
  if(B>255) B=255;
  if(B<0) B=0;
  if(Tail>255) Tail=255;
  if(Tail<-255) Tail=-255;

  char MTmsg[5];
  MTmsg[0]=1;
  MTmsg[1]=A & 0xFF;
  MTmsg[2]=B & 0xFF;
  if(Tail>0) {
    MTmsg[3]=Tail & 0xFF;
    MTmsg[4]=0;
  } else {
    MTmsg[3]=0;
    MTmsg[4]=(-Tail) & 0xFF;
  }
  i2c_tx_string(0x55, MTmsg, 5);
}


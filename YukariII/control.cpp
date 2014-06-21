#include "control.h"

//From the vehicle state and desired heading, steer toward the target
void Control::control() {
  hdgError=nav.hdg-guide.desiredHdg;
  coercedHeading(hdgError);
  ufsteerCmd=P*hdgError;
  fsteerCmd=ufsteerCmd;
  if(fsteerCmd> 100) fsteerCmd= 100;
  if(fsteerCmd<-100) fsteerCmd=-100;
  if(fsteerCmd>-5 && fsteerCmd<5) fsteerCmd=0;
  steerCmd=fsteerCmd;
}

void Control::begin() {
  P=config.P;for(int i=0;i<config.Ps;i++) P/=10;
  I=config.I;for(int i=0;i<config.Is;i++) I/=10;
  D=config.D;for(int i=0;i<config.Ds;i++) D/=10;
}

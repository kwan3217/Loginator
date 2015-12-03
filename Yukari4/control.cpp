#include "control.h"

//From the vehicle state and desired heading, steer toward the target
void Control::control() {
//Heading error is positive if vehicle is pointing to the right of the target
  hdgError=nav.hdg-guide.targetBearing; 
  coercedHeading(hdgError);
//P is supposed to be negative (negative feedback) so ufsteercmd will be positive
//iff we want to steer to the right
  ufsteerCmd=config.P*hdgError; //Unconstrained float steering command
  fsteerCmd=ufsteerCmd;  //Constrained float steering command
  if(fsteerCmd> 100) fsteerCmd= 100;
  if(fsteerCmd<-100) fsteerCmd=-100;
  if(fsteerCmd>-5 && fsteerCmd<5) fsteerCmd=0;
  steerCmd=fsteerCmd*right; //Invert the steering command if necessary for the servo
  throttleCmd=config.throttle*((hasButton&!guide.runFinished&((nav.gyroT-nav.tButton)<config.tCutoff))?1:0);
}



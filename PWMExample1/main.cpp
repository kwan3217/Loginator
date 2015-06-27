#include "LPC214x.h"
#include "gpio.h"
#include "pwm.h"
#include "Time.h"
#include "YukariConfig.h"
#include "float.h"

const unsigned char channelMask=(1<<channelSteer)|(1<<channelThrottle);

void setup() {
  initPWM(channelMask); 
}

static fp time() {
  static unsigned int oldTTC=0;
  static fp min=0;
  unsigned int ttc=TTC(0);
  if(oldTTC>ttc) {
    min++;
  }
  oldTTC=ttc;
  return min*60+fp(ttc)/fp(PCLK);
}

void loop() {
  if(time()<5) {
    static bool done=false;
    if(!done) {
      setServo(channelThrottle,0);
      setServo(channelSteer,0);
      done=true;
    }
  } else if(time()<7) {
    static bool done=false;
    if(!done) {
      setServo(channelThrottle,25);
      done=true;
    }
  } else if(time()<8) {
    static bool done=false;
    if(!done) {
      setServo(channelSteer,127*right);
      done=true;
    }
  } else if(time()<9) {
    static bool done=false;
    if(!done) {
      setServo(channelSteer,0);
      done=true;
    }
  } else if(time()<10) {
    static bool done=false;
    if(!done) {
      setServo(channelThrottle,0);
      done=true;
    }
  } else if(time()<61) {
    static bool done=false;
    if(!done) {
      setServo(channelSteer,127*left);
      done=true;
    }
  } else if(time()<62) {
    static bool done=false;
    if(!done) {
      setServo(channelSteer,0);
      done=true;
    }
  }
}



#include "pwm.h"
#include "robot.h"
#include "YukariConfig.h"

void initPWM(const unsigned char channelMask, const unsigned int prescale, const unsigned int period, const unsigned int defaultPulse) {

}

/*
void setPWM(const unsigned char channelMask, const unsigned int pulse[]) {
}

void setPWM(const unsigned char channel, const unsigned int pulse) {
}
*/

/** This is the interface that the simulator cares about. Set to a value between -127 and 127 */
void setServo(const unsigned char channel, const signed char val) {
  if(channel==channelThrottle) {
    state->setCmdSpd(val);
  } else {
    state->setCmdSteer(val);
  }
}

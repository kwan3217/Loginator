#include "robot.h"
#include <string.h>
RobotState::RobotState():ttc(0),rtcHour(0),rtcMin(0),rtcSec(0),rtcYear(0),rtcMonth(0),rtcDom(0),rtcDoy(0),rtcDow(0) {}

bool RobotState::hasGPS() {
  return gpsTransPointer<strlen(gpsBuf);
};


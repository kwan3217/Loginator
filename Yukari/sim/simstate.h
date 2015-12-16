#ifndef sim_h
#define sim_h
#include "float.h" 
#include <stdio.h>

#include "robot.h"

class SimState:public RobotState {
public:
  int lat0; ///< Latitude of initial point, cm'
  int lon0; ///< Longitude of initial point, cm'
  unsigned long shadowTTC; ///< At last GPS fix, ticks from sim start
  unsigned long gyroLastTTC; ///< At last gyro measurement, ticks from sim start
  double x; ///< Easting, cm' from initial point
  double y; ///< Northing, cm' from initial point
  double hdg; ///< Heading, radians east of north
  double spd; ///< speed, cm'/s
  double shadowX; ///< At last GPS fix, Easting, cm' from initial point
  double shadowY; ///< At last GPS fix, Northing, cm' from initial point
  double shadowHdg; ///< At last GPS fix, Heading, radians east of north
  double shadowSpd; ///< At last GPS fix, speed, cm'/s
  double steer; ///< steering angle, radians right of straight ahead
  double cmdSpd; ///<commanded speed, cm'/s
  double cmdSteer; ///<commanded steering angle, radians right of straight ahead
  /** Initialize the sim state
   @param Llat0 latitude of initial point in deg
   @param Llon0 longitude of initial point in deg east
   @param Lhdg initial heading in deg east of north
  */
  static constexpr double walkSpd=134.112; //3mi/hr in cm/s
  static const int walkThrottle=25;
  static constexpr double maxSpd=walkSpd*127.0/walkThrottle; ///<Maximum speed in cm'/s
  static constexpr double spdTime=2.0; ///<Seconds to accelerate from 0 to full speed
  static constexpr double spdRate=maxSpd/spdTime; ///<Speed change rate in cm'/s^2
  static constexpr double maxSteer=20.0*PI/180.0; ///<Maximum steering angle in rad
  static constexpr double steerTime=0.2; ///<Seconds to steer from neutral to hard-over
  static constexpr double steerRate=maxSteer/steerTime; ///<Speed change rate in rad/s
  static constexpr double ggaTime=0.2; ///<Time from start of second to GGA sentence
  static constexpr double rmcTime=0.3; ///<Time from start of second to RMC sentence
  FILE* stateLog,*nmeaLog;
  virtual void propagate(int ms);
  virtual void setCmdSpd(int Lservo) {cmdSpd=maxSpd/127*Lservo*forward;};
  virtual void setCmdSteer(int Lservo) {cmdSteer=maxSteer/127*Lservo*right;};
  double rtc0; ///< Time of init in unix seconds, including fractional part
  double t() {return double(ttc)/60e6;};
  bool hasGGA,hasRMC;
  SimState(double Llat0,double Llon0,double Lhdg);
  void incRTC();
};

#endif


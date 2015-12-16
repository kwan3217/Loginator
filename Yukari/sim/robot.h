#ifndef robot_h
#define robot_h
void setup();  //Robot setup code, defined in ../main.cpp
void loop();   //Robot loop code, likewise

#include "YukariConfig.h" //For directions of steering and throttle

class RobotState {
public:
  char gpsBuf[256];
  int gpsTransPointer;
  bool hasGPS();
  unsigned long ttc; ///< ticks from sim start
  unsigned int TCCR[2]; ///<Timer capture control register, one for each timer
  unsigned int TCR[2][4]; ///<Timer capture register, four channels for each timer
  unsigned int rtcHour,rtcMin,rtcSec,rtcYear,rtcMonth,rtcDom,rtcDoy,rtcDow; ///<RTC registers, automatically incremented on second rollover 
  virtual void propagate(int ms)=0;
  virtual void setCmdSpd(int Lservo)=0;
  virtual void setCmdSteer(int Lservo)=0; 
  RobotState();
  double xRate; ///< Pitch rate, rad/s
  double yRate; ///< Vehicle heading change rate, rad/s
  double zRate; ///< Roll rate, rad/s
};

extern RobotState* state;

#endif

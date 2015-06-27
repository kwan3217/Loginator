#ifndef compass_h
#define compass_h

#include "float.h"
#include "Quaternion.h"

class Compass {
public:
  Quaternion e;
  Quaternion nose,nose_r;
  fp sens; //Gyro sensitivity in rad/s/DN
  fp hdg,rmcHdg,gyroHdg; //Heading in degrees
  fp rmcSpd; //Speed in knots
  fp gyroT,lastT,deltaT;  //Time of last gyro reading in seconds, derived from TC so wraps every minute
  int32_t sod,minuteofs;
  uint32_t gyroPktCount;
  fp avgGx, avgGy, avgGz;
  fp calGx, calGy, calGz;
  fp lat,lon;
  static const fp clat;
  static const int yscl=37564;
  static const int avgGMin=200;
  static const int avgGMax=100;
  Compass():nose(0,0,-1,0) {};
  void handleRMC(uint32_t TC, uint32_t hms, int32_t lat, int32_t lon, int32_t spd, int32_t spdScale, int32_t hdg, int32_t hdgScale, int32_t dmy);
  void handleGyroCfg(uint8_t ctrl4);
  void handleGyro(uint32_t TC, int16_t gx, int16_t gy, int16_t gz);
};

#endif

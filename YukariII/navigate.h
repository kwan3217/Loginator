#ifndef navigate_h
#define navigate_h

#include <stdint.h>
#include "config.h"
#include "Quaternion.h"
#define PI 3.1415926535897932
#define clat 0.765243525639  //cosine of latitude of AVC2014 start line

inline void coerceHeading(fp& hdg) {
  if(hdg>360) hdg-=360;
  if(hdg<0)   hdg+=360;
}

inline void coercedHeading(fp& dhdg) {
  if(dhdg> 180) dhdg-=360;
  if(dhdg<-180) dhdg+=360;
}

//Handles sensor input and manages vehicle state. If we had a sophisticated 
//Kalman filter, it would live here. We have an ad-hoc heading 
//estimator instead.

class Navigate {
public:
  Config& config;
  Quaternion e;
  Quaternion nose,nose_r;
  fp sens; //Gyro sensitivity in rad/s/DN
  fp hdg,gyroHdg,rmcHdg,dHdg; //Heading in degrees, filtered and straight from the gyro
  fp P[2][2];         //Heading estimate covariance
  fp rmcSpd; //Speed in knots
  fp gyroT; //Time of current compass calc, in seconds, does not wrap
  fp lastT; //Time of last compass calc, in seconds, wraps every 60s
  fp deltaT;  //Time difference between compass calcs, in seconds
  int32_t sod,minuteofs;
  uint32_t gyroPktCount;
  fp avgGx, avgGy, avgGz;
  fp calGx, calGy, calGz;
  int compassCountdown=0; //If compassCountdown<0, use RMC as compass reference
                          //Set compassCountdown to 400 if |calGy|>0.5, otherwise decrement
  fp lat,lon,firstLat,firstLon,dLat,dLon;
  fp hdgOfs; //Difference between gyro heading and RMC heading, hdgOfs=gyroHdg-Hdg, so Hdg=gyroHdg+hdgOfs
  fp startHdg;
  bool hasInit,hasRMC;
//  static const int yscl=37564;
  //static const int yscl=32768;
  static const int navgG=50;
  int16_t avgGsample[3][navgG*2];
  int head;
  Navigate(Config& Lconfig):config(Lconfig),nose(0,1,0,0) {};
  void handleRMC(uint32_t TC, fp lat, fp lon, fp spd,  fp hdg);
  void handleGyroCfg(uint8_t ctrl4);
  void setSens(uint8_t fs);
  void handleGyro(uint32_t TC, int16_t gx, int16_t gy, int16_t gz);
  void initFilter();
};

#endif

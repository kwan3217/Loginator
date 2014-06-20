#ifndef compass_h
#define compass_h

#include "Quaternion.h"

class Compass {
public:
  Quaternion e;
  Quaternion nose,nose_r;
  fp sens; //Gyro sensitivity in rad/s/DN
  fp rmcHdg,gyroHdg; //Heading in degrees from the GPS and gyro
  fp hdg,hdgRate;    //Heading in degrees from filter, rate in deg/s, to be used in control loop
  fp P[2][2];         //Heading estimate covariance
  fp rmcSpd; //Speed in knots
  fp gyroT; //Time of current compass calc, in seconds, does not wrap
  fp lastT; //Time of last compass calc, in seconds, wraps every 60s
  fp deltaT;  //Time difference between compass calcs, in seconds
  fp sigGyro,sigRmc;  //square root of uncertainty variances of gyro and rmc measurments
  int32_t sod,minuteofs;
  uint32_t gyroPktCount;
  fp avgGx, avgGy, avgGz;
  fp calGx, calGy, calGz;
  fp lat,lon,firstLat,firstLon,dLat,dLon;
  fp Rgyro;
  bool hasHdg,hasInit;
  static const fp clat;
  static const int yscl=37564;
  static const int navgG=100;
  int16_t avgGsample[3][navgG*2];
  int head,tail;
  Compass():nose(0,0,-1,0) {};
  void handleRMC(uint32_t TC, uint32_t hms, int32_t lat, int32_t lon, int32_t spd, int32_t spdScale, int32_t hdg, int32_t hdgScale, int32_t dmy);
  void handleGyroCfg(uint8_t ctrl4);
  void setSens(uint8_t fs);
  void handleGyro(uint32_t TC, int16_t gx, int16_t gy, int16_t gz);
  void initFilter();
  void filterRMC(fp rmcHdg);
  void filterGyro(fp dgyroHdg, fp deltaT);
};

#endif

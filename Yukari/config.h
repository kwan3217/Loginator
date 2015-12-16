#ifndef config_h
#define config_h

#include "float.h"
#include "Quaternion.h"
class Config {
public:
  Config():
  gyroSens(3),
  gyroODR(3),
  gyroBW(3),
  P(-1),
  I( 0),
  D( 0),
  throttle(20),
  compassCountdownMax(400),
  maneuverRate(0.8),
  compassSpeed(3.0),
  tCutoff(300),
  tickSize(31)
  {gyroScl[0]=1.0;gyroScl[1]=1.0;gyroScl[2]=1.0;};
  int gyroSens;
  int gyroODR;
  int gyroBW;
  fp P;
  fp I;
  fp D;
  static const int maxWaypoints=10;
  int nWaypoints;
  Vector<2> waypoint[maxWaypoints];
  int throttle;
  Vector<3> gyroScl;
  int compassCountdownMax;
  fp maneuverRate,compassSpeed;
  fp tCutoff;
  fp tickSize;
};

extern Config config;
#endif

#ifndef config_h
#define config_h

#include "float.h"

class Config {
public:
  Config():gyroSens(3),gyroODR(3),gyroBW(3),
  P(-1),Ps(0),
  I( 0),Is(0),
  D( 0),Ds(0),
  throttle(20),
  yscl(32768)
  {};
  int gyroSens;
  int gyroODR;
  int gyroBW;
  int P,Ps;
  int I,Is;
  int D,Ds;
  static const int maxWaypoints=10;
  int nWaypoints;
  fp dlatWaypoint[maxWaypoints];
  fp dlonWaypoint[maxWaypoints];
  int throttle;
  int yscl;
};

#endif

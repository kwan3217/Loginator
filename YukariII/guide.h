#ifndef GNC_h
#define GNC_h

#include <stdint.h>
#include "float.h"
#include "navigate.h"
#include "config.h"

// lat...,   lon...   - absolute latitude and longitude
// dlat...,  dlon...  - position relative to the origin, in degrees of latitude (lon is multiplied by cos(40deg))
// ddlat..., ddlon... - position relative to some other position, in degrees of latitude

class Guide {
public:
  Navigate& nav;
  Config& config;
  int currentBaseWaypoint,target;
  fp ddlatBasepath,ddlonBasepath;
  fp dlatSteerTo,dlonSteerTo; //Point to steer to, in dlat and dlon from starting point
  fp ddlatSteerTo,ddlonSteerTo;
  fp ddlatToGo,ddlonToGo;
  fp desiredHdg;
  fp distBasepath,nlatBasepath,nlonBasepath;
  fp dotp;
  bool runFinished,hasNewWaypoint;
  void begin();
  void guide();
  void control();
  void setupNextWaypoint();
  Guide(Navigate& Lnav, Config& Lconfig):nav(Lnav),config(Lconfig) {};
};

#endif

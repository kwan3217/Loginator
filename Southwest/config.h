#ifndef config_h
#define config_h

#include "float.h"

#ifdef LPC2148
#include "packet.h"
#include "file.h"
#include "cluster.h"
#endif

class Config {
public:
  int errno;
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
  static const char configFilename[];
#ifdef LPC2148
  Cluster& fs;
  File f;
  CCSDS& ccsds;
  Config(Cluster& Lfs, CCSDS& Lccsds):gyroSens(3),gyroODR(3),gyroBW(3),
                                      P(-1),Ps(0),
                                      I( 0),Is(0),
                                      D( 0),Ds(0),
                                      throttle(20),yscl(32768),
                                      fs(Lfs),f(fs),ccsds(Lccsds) {};
  bool begin();
#else
  Config():gyroSens(3),gyroODR(3),gyroBW(3),
           P(-1),Ps(0),
           I( 0),Is(0),
           D( 0),Ds(0),
           throttle(20),yscl(32768) {};
  bool begin();
  bool begin(char* buf, int size, int apid=0x22);
#endif
  bool handleData(int tagid, char* buf);
  bool handleInt(char* buf, int& val);
  bool handleWaypoint(char* buf, fp* out, fp scale);
};

#endif

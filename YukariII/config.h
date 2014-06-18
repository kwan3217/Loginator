#ifndef config_h
#define config_h

#include "packet.h"
#include "file.h"
#include "cluster.h"

class Config {
public:
  int errno;
  int gyroSens;
  int gyroODR;
  int gyroBW;
  int P,Ps;
  int I,Is;
  int D,Ds;
  
  Cluster& fs;
  File f;
  CCSDS& ccsds;
  Config(Cluster& Lfs, CCSDS& Lccsds):gyroSens(3),gyroODR(3),gyroBW(3),
                                      P(-1),Ps(0),
                                      I( 0),Is(0),
                                      D( 0),Ds(0),
                                      fs(Lfs),f(fs),ccsds(Lccsds) {};
  static const char configFilename[];
  bool begin();
  bool handleData(int tagid, char* buf);
  bool handleInt(char* buf, int& val);
};

#endif

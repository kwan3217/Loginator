#ifndef playback_h
#define playback_h

#include "robot.h"
#include <stdio.h>

class PlaybackState {
public:
  char gpsBuf[256];
  int gpsTransPointer;
  bool hasGPS();

  void begin(char* infn);
  bool done();
  FILE* inf;
  virtual void propagate(int ms);
};

#endif

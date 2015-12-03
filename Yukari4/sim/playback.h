#ifndef playback_h
#define playback_h

#include "robot.h"
#include <stdio.h>

class PlaybackState:public RobotState {
public:
  PlaybackState(char* infn, int fs);
  bool done();
  FILE* inf;
  double sensX, sensY, sensZ;  
  virtual void propagate(int ms);
  virtual void setCmdSpd(int Lservo) {/*printf("tc: %lu Spd: %d\n",ttc,Lservo);*/};
  virtual void setCmdSteer(int Lservo) {/*printf("tc: %lu Steer: %d\n",ttc,Lservo);*/}; 
};

#endif

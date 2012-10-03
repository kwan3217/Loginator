#ifndef pinger_h
#define pinger_h

#include "sensor.h"

class ping:public sensor {
public:
  ping();
  virtual void read(unsigned int TC);
  virtual void calibrate();
  virtual void write(circular& buf);
};

extern ping Ping;
#endif

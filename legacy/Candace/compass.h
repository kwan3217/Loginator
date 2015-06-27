#ifndef compass_h
#define compass_h

#include "sensor.h"

class bfld:public sensor {
public:
  bfld();
  virtual void setup(circular& buf);
  virtual void read(unsigned int TC);
  virtual void calibrate();
  virtual void write(circular& buf);
};

extern bfld Bfld;

#endif

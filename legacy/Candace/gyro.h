#ifndef gyro_h
#define gyro_h

#include "sensor.h"

class gyro:public sensor {
public:
  gyro();
  virtual void setup(circular& buf);
  virtual void read(unsigned int TC);
  virtual void calibrate();
  virtual int check();
  virtual void write(circular& buf);
  fp calLast[4];
};

extern gyro Gyro;
extern fp xg_extra,yg_extra,zg_extra;

#endif

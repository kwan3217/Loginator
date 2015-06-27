#ifndef acc_h
#define acc_h

#include "sensor.h"

class acc:public sensor {
public:
  acc();
  virtual void setup(circular& buf);
  virtual void read(unsigned int TC);
  virtual void calibrate();
  virtual void write(circular& buf);
};

extern acc Acc;
void ackAcc(void);
extern fp extraAccScale;

#endif

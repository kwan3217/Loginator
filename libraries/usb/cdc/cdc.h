#ifndef CDC_H
#define CDC_H

#include "usb.h"

class CDC {
private:
  USB& usb;
  static const char descConfiguration[];
public:
  CDC(USB& Lusb):usb(Lusb) {};
  virtual bool begin();
};

#endif



#ifndef MSC_H
#define MSC_H

#include "usb.h"
#include "sdhc.h"

class MSC:public USB {
private:
  SDHC& sd;
  static const char descDevice[];
  static const char descConfiguration[];
public:
  MSC(SDHC& Lsd):sd(Lsd) {};
  virtual bool begin();
};

#endif

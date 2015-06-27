#ifndef control_h
#define control_h

#include "navigate.h"
#include "guide.h"

class Control {
public:
  Navigate& nav;
  Guide& guide;
  Config& config;
  fp P,hdgError;
  fp I,hdgIntError;
  fp D,hdgRate;
  fp ufsteerCmd,fsteerCmd;
  int8_t steerCmd;

  Control(Navigate& Lnav, Guide& Lguide, Config& Lconfig):nav(Lnav),guide(Lguide),config(Lconfig) {};
  void control();
  void begin();
};

#endif

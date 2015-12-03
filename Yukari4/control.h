#ifndef control_h
#define control_h

#include "navigate.h"
#include "guide.h"
#include "YukariConfig.h"

class Control {
public:
  Navigate& nav;
  Guide& guide;
  Config& config;
  fp hdgError;
  fp hdgIntError;
  fp hdgRate;
  fp ufsteerCmd,fsteerCmd;
  int8_t steerCmd,throttleCmd;
  bool hasButton;

  Control(Navigate& Lnav, Guide& Lguide, Config& Lconfig):nav(Lnav),guide(Lguide),config(Lconfig) {};
  void control();
  void begin() {};
  void buttonPress() {hasButton=true;};
};

#endif

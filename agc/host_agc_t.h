#ifndef HOST_AGC_T_H
#define HOST_AGC_T_H
#include "agc_io.h"

class host_agc_t: public agc_io {
public:
  //Was in EmbeddedDemo.cpp. Do the usual virtual thing if you really want to extend these
  void print_state();
  char* Lines;
  char* LineAddr[044*02000]; //one for each word of fixed memory
  virtual void ChannelSetup();
  virtual void ChannelOutput(int Channel, int Value);
  virtual int  ChannelInput();
  void printDSKY();
  host_agc_t(fixedbanks_r Lfixed, char* LLines);
};

#endif

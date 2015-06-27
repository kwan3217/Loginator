#ifndef AGC_IO_H
#define AGC_IO_H
#include "agc_engine.h"

/**AGC I/O hardware interface. This represents all the hardware external to the computer
   itself, including those things attached by I/O channels. In this implementation, the
   I/O channels are represented as 512 additional memory cells.*/
class agc_io: public agc_t {
public:
  //Was in EmbeddedDemo.cpp. Do the usual virtual thing if you really want to extend these
  /** There are also "input/output channels".  Output channels are acted upon
   immediately, but input channels are buffered from asynchronous data. */
  int16_t InputChannel[n_channels];
  int16_t OutputChannel7;
  /** Output channel 010 is used to control the lights and displays on the DSKY.
   Each entry controls up to two digits and a +/- sign, or one of six lights
   In the real hardware, this "memory" is in the form of latching relays in 
   the DSKY itself. The original author chose to keep this memory internal
   to the AGC state. */
  int16_t OutputChannel10[16];
  bool ChannelIsSetUp;
  virtual int  ReadIO(int Channel);
/** Any kind of setup needed by your i/o-channel model. Guaranteed to be called
  at or before first ChannelInput/ChannelOutput, before channels are read/written*/
  virtual void ChannelSetup() {ChannelIsSetUp=true;};
  virtual void ChannelOutput(int Channel, int Value);
  virtual int  ChannelInput();
/** Routine I/O handling. A function for handling anything routinely needed (i.e., executed on
  a regular schedule) by the i/o channel model of ChannelInput and
  ChannelOutput.  There are no good reasons that I know of why this
  would be needed, other than by my reference model (see SocketAPI.c),
  so you might just want to let this empty. */
  virtual void ChannelRoutine() {};
  agc_io(fixedbanks_r Lfixed):agc_t(Lfixed) {  
    InputChannel[030] = 037777;
    InputChannel[031] = 077777;
    InputChannel[032] = 077777;
    InputChannel[033] = 077777;
    //All other channels are zero by default
  };
};

#endif

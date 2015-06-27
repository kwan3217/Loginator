#include "agc_io.h"
#include <cstdio>

//-----------------------------------------------------------------------------
// The simulated CPU in yaAGC calls this function whenever it wants to write
// output data to an "i/o channel", other than i/o channels 1 and 2, which are
// overlapped with the L and Q central registers.  For example, in an embedded
// design, this would physically control the individual electrical signals
// comprising the i/o port.  In my recommended reference design (see
// SocketAPI.c) data would be streamed out a socket connection from a port.
// In a customized version, FOR EXAMPLE, data might be written to a shared
// memory array, and other execution threads might be woken up to process the
// changed data.
int agc_io::ReadIO(int Address) {
  return (InputChannel[Address]);
}

void agc_io::ChannelOutput(int Channel, int Value) {

  // 2005-07-04 RSB.  The necessity for this was pointed out by Mark
  // Grant via Markus Joachim.  Although channel 033 is an input channel,
  // the CPU writes to it from time to time, to "reset" bits 11-15 to 1.
  // Apparently, these are latched inputs, and this resets the latches.
  if (Channel == 033)
    Value = (InputChannel[Channel] | 076000);

  InputChannel[Channel] = Value;
  if (Channel == 010)
    {
      // Channel 10 is converted externally to the CPU into up to 16 ports,
      // by means of latching relays.  We need to capture this data.
      OutputChannel10[(Value >> 11) & 017] = Value;
    }

  if (!ChannelIsSetUp) ChannelSetup();

}

//----------------------------------------------------------------------
// The simulated CPU in yaAGC calls this function to check for input data
// once for each call to agc_engin.  This input data may be of two kinds:
// 	1. Data available on an "i/o channel"; in this case, a value
//	   of 0 is returned; you can handle as much or as little data
//	   of this kind in any given invocation; or
//	2. A request for an "unprogrammed sequence" to automatically
//	   increment or decrement a counter.  In this case a value of
//	   1 is returned.  The function must return immediately upon
//	   one of these requests, in order ot preserve system timing.
// The former type of data is supposed to be directly written to the
// array State->InputChannel[], while the latter is supposed to call the
// function UnprogrammedIncrement() to handle the actual incrementing.
// ChannelInput() has the responsibility of raising an interrupt-request
// flag (in the array State->InterruptRequests[]) if the i/o channel
// data is supposed to cause an interrupt.  (An example would
// be if the input data represented a DSKY keystroke.)  Interrupt-raising
// due to overflow of counters is handled automatically by the function
// UnprogrammedChannel() and doesn't need to be addressed directly.
//
// For example, in an embedded design, this input data would reflect the
// physical states of individual electrical signals.
// In my recommended reference design (see SocketAPI.c) the data would be
// taken from an incoming stream of a socket connection to a port.
// In a customized version, FOR EXAMPLE, data might indicate changes in a
// shared memory array partially controlled by other execution threads.
//
// Note:  You are guaranteed that yaAGC processes at least one instruction
// between any two calls to ChannelInput.

int agc_io::ChannelInput() {
  int RetVal = 0;

  if (!ChannelIsSetUp) ChannelSetup();

  // If there are changes to the input channels, write the data
  // directly to the array State->InputChannel[].  Don't forget to
  // raise a flag in State->InterruptRequests if the incoming data
  // is supposed to do that.  (Mainly, DSKY keystrokes.)

  // If the inputs request unprogrammed counter-increment sequences,
  // then call the function UnprogrammedChannel(State,Counter,IncType)
  // to process them.  The different unprogrammed sequences are
  // related to the IncTypes as follows:
  //	PINC	000
  //	PCDU	001
  //	MINC	002
  //	MCDU	003
  //	DINC	004
  //	SHINC	005
  //	SHANC	006
  // (Refer to the developer page on www.ibiblio.org/apollo/index.html.)
  // Only registers 32 (octal) through 60 (octal) may actually used as
  // counters, and not all of them.  (Refer to the AGC assembly-language
  // manual at www.ibiblio.org/apollo/index.html.)

  return (RetVal);
}





/*
  Copyright 2003-2005,2009 Ronald S. Burkey <info@sandroid.org>
  
  This file is part of yaAGC.

  yaAGC is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  yaAGC is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with yaAGC; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  
  In addition, as a special exception, Ronald S. Burkey gives permission to
  link the code of this program with the Orbiter SDK library (or with 
  modified versions of the Orbiter SDK library that use the same license as 
  the Orbiter SDK library), and distribute linked combinations including 
  the two. You must obey the GNU General Public License in all respects for 
  all of the code used other than the Orbiter SDK library. If you modify 
  this file, you may extend this exception to your version of the file, 
  but you are not obligated to do so. If you do not wish to do so, delete 
  this exception statement from your version. 
 
  Filename:	agc_engine.c
  Purpose:	This is the main engine for binary simulation of the Apollo AGC
  		computer.  It is separate from the Display/Keyboard (DSKY) 
		simulation and Apollo hardware simulation, though compatible
		with them.  The executable binary may be created using the
		yayul (Yet Another YUL) assembler.
  Compiler:	GNU gcc.
  Contact:	Ron Burkey <info@sandroid.org>
  Reference:	http://www.ibiblio.org/apollo/index.html

  The technical documentation for the Apollo Guidance & Navigation (G&N) system,
  or more particularly for the Apollo Guidance Computer (AGC) may be found at 
  http://hrst.mit.edu/hrs/apollo/public.  That is, to the extent that the 
  documentation exists online it may be found there.  I'm sure -- or rather 
  HOPE -- that there's more documentation at NASA and MIT than has been made 
  available yet.  I personally had no knowledge of the AGC, other than what 
  I had seen in the movie "Apollo 13" and the HBO series "From the Earth to 
  the Moon", before I conceived this project last night at midnight and 
  started doing web searches.  So, bear with me; it's a learning experience!

  Also at hrst.mit.edu are the actual programs for the Command Module (CM) and
  Lunar Module (LM) AGCs.  Or rather, what's there are scans of 1700-page
  printouts of assembly-language listings of SOME versions of those programs.
  (Respectively, called "Colossus" and "Luminary".)  I'll worry about how to
  get those into a usable version only after I get the CPU simulator working!

  What THIS file contains is basicly a pure simulation of the CPU, without any
  input and output as such.  (I/O, to the DSKY or to CM or LM hardware
  simulations occurs through the mechanism of sockets, and hence the DSKY
  front-end and hardware back-end simulations may be implemented as complete
  stand-alone programs and replaced at will.)  There is a single globally
  interesting function, called agc_engine, which is intended to be called once
  per AGC instruction cycle -- i.e., every 11.7 microseconds.  (Yes, that's
  right, the CPU clock speed was a little over 85 KILOhertz.  That's a factor
  that obviously makes the simulation much easier!)  The function may be called
  more or less often than this, to speed up or slow down the apparent passage
  of time.

  This function is intended to be completely portable, so that it may be run in
  a PC environment (Microsoft Windows) or in any *NIX environment, or indeed in
  an embedded target if anybody should wish to create an actual physical
  replacement for an AGC.  Also, multiple copies of the simulation may be run
  on the same PC -- for example to simulation a CM and LM simultaneously.
*/

#define AGC_ENGINE_C
#include "agc_engine.h"

// If COARSE_SMOOTH is 1, then the timing of coarse-alignment (in terms of
// bursting and separation of bursts) is according to the Delco manual.
// However, since the simulated IMU has no physical inertia, it adjusts
// instantly (and therefore jerkily).  The COARSE_SMOOTH constant creates
// smaller bursts, and therefore smoother FDAI motion.  Normally, there are
// 192 pulses in a burst.  In the simulation, there are 192/COARSE_SMOOTH
// pulses in a burst.  COARSE_SMOOTH should be in integral divisor of both
// 192 and of 50*1024.  This constrains it to be any power of 2, up to 64.
const auto COARSE_SMOOTH=8;

// Some helpful constants in parsing the "address" field from an instruction
// or from the Z register.
const auto SIGNAL_00  =000000;
const auto SIGNAL_01  =002000;
const auto SIGNAL_10  =004000;
const auto SIGNAL_11  =006000;
const auto SIGNAL_0011=001400;
const auto MASK9      =000777;
const auto MASK10     =001777;
const auto MASK12     =007777;

// Here are arrays which tell (for each instruction, as determined by the
// uppermost 5 bits of the instruction) how many extra machine cycles are
// needed to execute the instruction.  (In other words, the total number of
// machine cycles for the instruction, minus 1.) The opcode and quartercode
// are taken into account.  There are two arrays -- one for normal
// instructions and one for "extracode" instructions.
static const int InstructionTiming[32] = {
  0, 0, 0, 0,			// Opcode = 00.
  1, 0, 0, 0,			// Opcode = 01.
  2, 1, 1, 1,			// Opcode = 02.
  1, 1, 1, 1,			// Opcode = 03.
  1, 1, 1, 1,			// Opcode = 04.
  1, 2, 1, 1,			// Opcode = 05.
  1, 1, 1, 1,			// Opcode = 06.
  1, 1, 1, 1			// Opcode = 07.
};

// Note that the following table does not properly handle the EDRUPT or
// BZF/BZMF instructions, and extra delay may need to be added specially for
// those cases.  The table figures 2 MCT for EDRUPT and 1 MCT for BZF/BZMF.
static const int ExtracodeTiming[32] = {
  1, 1, 1, 1,			// Opcode = 010.
  5, 0, 0, 0,			// Opcode = 011.
  1, 1, 1, 1,			// Opcode = 012.
  2, 2, 2, 2,			// Opcode = 013.
  2, 2, 2, 2,			// Opcode = 014.
  1, 1, 1, 1,			// Opcode = 015.
  1, 0, 0, 0,			// Opcode = 016.
  2, 2, 2, 2			// Opcode = 017.
};

// A way, for debugging, to disable interrupts. The 0th entry disables 
// everything if 0.  Entries 1-10 disable individual interrupts.
int DebuggerInterruptMasks[11] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

//-----------------------------------------------------------------------------
// Stuff for doing structural coverage analysis.  Yes, I know it could be done
// much more cleverly.
/*
int CoverageCounts = 0;			// Increment coverage counts is != 0.
unsigned ErasableReadCounts[8][0400];
unsigned ErasableWriteCounts[8][0400];
unsigned ErasableInstructionCounts[8][0400];
unsigned FixedAccessCounts[40][02000];
unsigned IoReadCounts[01000];
unsigned IoWriteCounts[01000];
// For debugging the CDUX,Y,Z inputs.
FILE *CduLog = nullptr;
*/

/** Construct an AGC instance
  @param fixed Reference to rope to be used. Rope can be in embedded ROM. Rope
         must be in bank order and not have the parity bit (run yaYUL output
         through libraries/agc/agc_embedrope.pl to create a suitable binary rope
         image, then use objcopy and ld to link it
*/
agc_t::agc_t(fixedbanks_r fixed):
  CycleCounter(0),
  Fixed(fixed),
  ExtraCode(0),
  AllowInterrupt(0),
  PendFlag(0),
  PendDelay(0),
  ExtraDelay(0)
{
  //Erasable memory is also clear by default
  Z() = 04000;	// Initial program counter.
}

int agc_t::CpuReadIO (int Address) {
  if (Address < 0 || Address > 0777) return (0);
  if (Address == RegL || Address == RegQ) return (Erasable[0][Address]);
  return ReadIO(Address);
}

void agc_t::CpuWriteIO (int Address, int Value) {
  //static int Downlink = 0;
  // The value should be in AGC format.
  Value &= 077777;
  if (Address < 0 || Address > 0777)
    return;
  CoverageIoWriteCounts(Address);
  if (Address == RegL || Address == RegQ) {
    Erasable[0][Address] = Value;
    return;
  }
  if(Address==7) {
    superbank=Value;
  }
  // 2005-06-25 RSB.  DOWNRUPT stuff.  I assume that the 20 ms. between
  // downlink transmissions is due to the time needed for transmitting,
  // so I don't interrupt at a regular rate,  Instead, I make sure that
  // there are 20 ms. between transmissions
  if (Address == 034)
    Downlink |= 1;
  else if (Address == 035)
    Downlink |= 2;
  if (Downlink == 3) {
    //InterruptRequests[8] = 1;	// DOWNRUPT.
    DownruptTimeValid = 1;
    DownruptTime = CycleCounter + (AGC_PER_SECOND / 50);
    Downlink = 0;
  }
  ChannelOutput(Address, Value);
}

//-----------------------------------------------------------------------------
// This function does all of the processing associated with converting a
// 12-bit "address" as used within instructions or in the Z register, to a
// pointer to the actual word in the simulated memory.  In other words, here
// we take memory bank-selection into account.
// CDJ for embedded - Note that this does an evil typecast from const to non-const.
// If you use this address to write to a fixed bank, you violate const-correctness,
// and on certain embedded systems such as the LPC2148 where the rope really is in
// read-only memory, it will trigger a data exception
int16_t* agc_t::FindMemoryWord (int LAddress12) {
  //int PseudoAddress;
  int AdjustmentEB, AdjustmentFB;

  // Get rid of the parity bit.
  //Address12 = Address12;

  // Make sure the darn thing really is 12 bits.
  LAddress12 &= 07777;

  // It should be noted as far as unswitched-erasable and common-fixed memory
  // is concerned, that the following rules actually do result in continuous
  // block of memory that don't have problems in crossing bank boundaries.
  if (LAddress12 < 00400)	// Unswitched-erasable.
    return (&Erasable[0][LAddress12 & 00377]);
  else if (LAddress12 < 01000)	// Unswitched-erasable (continued).
    return (&Erasable[1][LAddress12 & 00377]);
  else if (LAddress12 < 01400)	// Unswitched-erasable (continued).
    return (&Erasable[2][LAddress12 & 00377]);
  else if (LAddress12 < 02000)	// Switched-erasable.
    {
      // Recall that the parity bit is accounted for in the shift below.
      AdjustmentEB = (7 & (EB() >> 8));
      return (&Erasable[AdjustmentEB][LAddress12 & 00377]);
    }
  else if (LAddress12 < 04000)	// Fixed-switchable.
    {
      AdjustmentFB = (037 & (FB() >> 10));
      // Account for the superbank bit.
      if (030 == (AdjustmentFB & 030) && (superbank & 0100) != 0)
	AdjustmentFB += 010;
      return (int16_t*)(&Fixed[AdjustmentFB][LAddress12 & 01777]);
    }
  else if (LAddress12 < 06000)	// Fixed-fixed.
    return (int16_t*)(&Fixed[2][LAddress12 & 01777]);
  else				// Fixed-fixed (continued).
    return (int16_t*)(&Fixed[3][LAddress12 & 01777]);
}

//-----------------------------------------------------------------------------
// Assign a new value to "erasable" memory, performing editing as necessary
// if the destination address is one of the 4 editing registers.  The value to
// be written is a properly formatted AGC value in D1-15.  The difference between
// Assign and AssignFromPointer is simply that Assign needs a memory bank number
// and an offset into that bank, while AssignFromPointer simply uses a pointer
// directly to the simulated memory location.

/*
void agc_t::CollectErasableWriteCounts(int Bank, int Offset) {
  if (CoverageCounts) ErasableWriteCounts[Bank][Offset]++;
}
*/

void agc_t::Assign(int Bank, int Offset, int Value) {
  if (Bank < 0 || Bank >= 8)        return;// Non-erasable memory.
  if (Offset < 0 || Offset >= 0400) return;
  CoverageErasableWriteCounts(Bank, Offset);
  if (Bank == 0)
    {
      switch (Offset)
	{
	case RegZ:
	  NextZ = Value & 07777;
	  break;
	case RegCYR:
	  Value &= 077777;
	  if (0 != (Value & 1))
	    Value = (Value >> 1) | 040000;
	  else
	    Value = (Value >> 1);
	  break;
	case RegSR:
	  Value &= 077777;
	  if (0 != (Value & 040000))
	    Value = (Value >> 1) | 040000;
	  else
	    Value = (Value >> 1);
	  break;
	case RegCYL:
	  Value &= 077777;
	  if (0 != (Value & 040000))
	    Value = (Value << 1) + 1;
	  else
	    Value = (Value << 1);
	  break;
	case RegEDOP:
	  Value &= 077777;
	  Value = ((Value >> 7) & 0177);
	  break;
	case RegZERO:
	  Value = AGC_P0;
	  break;
	default:
	  // No editing of the Value is needed in this case.
	  break;
	}
      if (Offset >= REG16 || (Offset >= 020 && Offset <= 023))
	Erasable[0][Offset] = Value & 077777;
      else
	Erasable[0][Offset] = Value & 0177777;
    }
  else
    Erasable[Bank][Offset] = Value & 077777;
}

void agc_t::AssignFromPointer(int16_t * Pointer, int Value) {
  int Address;
  Address = Pointer - Erasable[0];
  if (Address >= 0 && Address < 04000)
    {
      Assign (Address / 0400, Address & 0377, Value);
      return;
    }
}

/** Convert an AGC-formatted word to CPU-native format.
@param Input AGC-formatted word (ones complement, may be 16-bit)
@return host-formatted word (probably twos complement)
*/
int agc2cpu (int Input) {
  if (0 != (040000 & Input))
    return (-(037777 & ~Input));
  else
    return (037777 & Input);
}

//-----------------------------------------------------------------------------
// Convert a native CPU-formatted word to AGC format. If the input value is
// out of range, it is truncated by discarding high-order bits.

int cpu2agc (int Input)
{
  if (Input < 0)
    return (077777 & ~(-Input));
  else
    return (077777 & Input);
}

//-----------------------------------------------------------------------------
// Double-length versions of the same. 

int agc2cpu2 (int Input)
{
  if (0 != (02000000000 & Input))
    return (-(01777777777 & ~Input));
  else
    return (01777777777 & Input);
}

int cpu2agc2 (int Input)
{
  if (Input < 0)
    return (03777777777 & ~(01777777777 & (-Input)));
  else
    return (01777777777 & Input);
}

// Returns +1, -1, or +0 (in SP) format, on the basis of whether an
// accumulator-style "16-bit" value (really 17 bits including parity)
// contains overflow or not.  To do this for the accumulator itself,
// use ValueOverflowed(GetAccumulator(State)).

int16_t ValueOverflowed (int Value)
{
  switch (Value & 0140000)
    {
    case 0040000:
      return (agc_t::AGC_P1);
    case 0100000:
      return (agc_t::AGC_M1);
    default:
      return (agc_t::AGC_P0);
    }
}

// Return an overflow-corrected value from a 16-bit (plus parity ) SP word.
// This involves just moving bit 16 down to bit 15.

int16_t OverflowCorrected (int Value)
{
  return ((Value & 037777) | ((Value >> 1) & 040000));
}

// Sign-extend a 15-bit SP value so that it can go into the 16-bit (plus parity)
// accumulator.

int
SignExtend (int16_t Word)
{
  return ((Word & 077777) | ((Word << 1) & 0100000));
}

//-----------------------------------------------------------------------------
// Here are functions to convert a DP into a more-decent 1's-
// complement format in which there's not an extra sign-bit to contend with.
// (In other words, a 29-bit format in which there's a single sign bit, rather
// than a 30-bit format in which there are two sign bits.)  And vice-versa.
// The DP value consists of two adjacent SP values, MSW first and LSW second,
// and we're given a pointer to the second word.  The main difficulty here
// is dealing with the case when the two SP words don't have the same sign,
// and making sure all of the signs are okay when one or more words are zero.
// A sign-extension is added a la the normal accumulator.

int SpToDecent (int16_t * LsbSP)
{
  int16_t Msb, Lsb;
  int Value, Complement;
  Msb = LsbSP[-1];
  Lsb = *LsbSP;
  if (Msb == agc_t::AGC_P0 || Msb == agc_t::AGC_M0)	// Msb is zero.
    {
      // As far as the case of the sign of +0-0 or -0+0 is concerned,
      // we follow the convention of the DV instruction, in which the
      // overall sign is the sign of the less-significant word.
      Value = SignExtend (Lsb);
      if (Value & 0100000)
	Value |= ~0177777;
      return (07777777777 & Value);	// Eliminate extra sign-ext. bits.
    }
  // If signs of Msb and Lsb words don't match, then make them match.
  if ((040000 & Lsb) != (040000 & Msb))
    {
      if (Lsb == agc_t::AGC_P0 || Lsb == agc_t::AGC_M0)	// Lsb is zero.
	{
	  // Adjust sign of Lsb to match Msb.
	  if (0 == (040000 & Msb))
	    Lsb = agc_t::AGC_P0;
	  else
	    Lsb = agc_t::AGC_M0;	// 2005-08-17 RSB.  Was "Msb".  Oops!
	}
      else			// Lsb is not zero.
	{
	  // The logic will be easier if the Msb is positive.
	  Complement = (040000 & Msb);
	  if (Complement)
	    {
	      Msb = (077777 & ~Msb);
	      Lsb = (077777 & ~Lsb);
	    }
	  // We now have Msb positive non-zero and Lsb negative non-zero.
	  // Subtracting 1 from Msb is equivalent to adding 2**14 (i.e.,
	  // 0100000, accounting for the parity) to Lsb.  An additional 1 
	  // must be added to account for the negative overflow.
	  Msb--;
	  Lsb = ((Lsb + 040000 + agc_t::AGC_P1) & 077777);
	  // Restore the signs, if necessary.
	  if (Complement)
	    {
	      Msb = (077777 & ~Msb);
	      Lsb = (077777 & ~Lsb);
	    }
	}
    }
  // We now have an Msb and Lsb of the same sign; therefore,
  // we can simply juxtapose them, discarding the sign bit from the 
  // Lsb.  (And recall that the 0-position is still the parity.)
  Value = (03777740000 & (Msb << 14)) | (037777 & Lsb);
  // Also, sign-extend for further arithmetic.
  if (02000000000 & Value)
    Value |= 04000000000;
  return (Value);
}

void DecentToSp (int Decent, int16_t * LsbSP)
{
  int Sign;
  Sign = (Decent & 04000000000);
  *LsbSP = (037777 & Decent);
  if (Sign)
    *LsbSP |= 040000;
  LsbSP[-1] = OverflowCorrected (0177777 & (Decent >> 14));	// Was 13.
}

// Adds two sign-extended SP values.  The result may contain overflow.
int
AddSP16 (int Addend1, int Addend2)
{
  int Sum;
  Sum = Addend1 + Addend2;
  if (Sum & 0200000)
    {
      Sum += agc_t::AGC_P1;
      Sum &= 0177777;
    }
  return (Sum);
}

// Absolute value of an SP value.

int16_t AbsSP (int16_t Value)
{
  if (040000 & Value)
    return (077777 & ~Value);
  return (Value);
}

// Check if an SP value is negative.

//static int
//IsNegativeSP (int16_t Value)
//{
//  return (0 != (0100000 & Value));
//}

// Negate an SP value.

int16_t NegateSP (int16_t Value)
{
  return (077777 & ~Value);
}

//-----------------------------------------------------------------------------
// The following are various operations performed on counters, as defined
// in Savage & Drake (E-2052) 1.4.8.  The functions all return 0 normally,
// and return 1 on overflow.


//#include <stdio.h>
//static int TrapPIPA = 0;
static inline void TrapPIPAPINC(int16_t i0, int16_t i1, int16_t i2) {
//  if (TrapPIPA) printf ("PINC: %o %o %o\n", i0, i1, i2);
}
static inline void TrapPIPAMINC(int16_t i0, int16_t i1, int16_t i2) {
//  if (TrapPIPA) printf ("MINC: %o %o %o\n", i0, i1, i2);
}

// 1's-complement increment
int agc_t::CounterPINC(int16_t * Counter) {
  int16_t i;
  int lOverflow = 0;
  i = *Counter;
  if (i == 037777)
    {
      lOverflow = 1;
      i = agc_t::AGC_P0;
    }
  else
    {
      lOverflow = 0;
      int16_t i0=i;
      int16_t i1 = ((i0 + 1) & 077777);
      int16_t i2=i1;
      if (i2 == agc_t::AGC_P0)	// Account for -0 to +1 transition.
        i2++;
      TrapPIPAPINC(i0,i1,i2);
      i=i2;
    }
  *Counter = i;
  return (lOverflow);
}

// 1's-complement increment
int agc_t::CounterPINCChannel(int16_t Counter) {
  int16_t i;
  int lOverflow = 0;
  i = ReadIO(Counter);
  if (i == 037777)
    {
      lOverflow = 1;
      i = agc_t::AGC_P0;
    }
  else
    {
      lOverflow = 0;
      int16_t i0=i;
      int16_t i1 = ((i0 + 1) & 077777);
      int16_t i2=i1;
      if (i2 == agc_t::AGC_P0)	// Account for -0 to +1 transition.
        i2++;
      TrapPIPAPINC(i0,i1,i2);
      i=i2;
    }
  ChannelOutput(Counter,i);
  return (lOverflow);
}

// 1's-complement decrement, but only of negative integers.
int CounterMINC (int16_t * Counter) {
  int16_t i;
  int Overflow = 0;
  i = *Counter;
  if (i == (int16_t) 040000)
    {
      Overflow = 1;
      i = agc_t::AGC_M0;
    }
  else
    {
      Overflow = 0;
      int16_t i0=i;
      int16_t i1 = ((i0 - 1) & 077777);
      int16_t i2=i1;
      if (i2== agc_t::AGC_M0)	// Account for +0 to -1 transition.
        i2--;
      TrapPIPAMINC(i0,i1,i2);
      i2=i;
    }
  *Counter = i;
  return (Overflow);
}

// 2's-complement increment.
int CounterPCDU (int16_t * Counter) {
  int16_t i;
  int Overflow = 0;
  i = *Counter;
  if (i == (int16_t) 077777)
    Overflow = 1;
  i++;
  i &= 077777;
  *Counter = i;
  return (Overflow);
}

// 2's-complement decrement.
int CounterMCDU (int16_t * Counter) {
  int16_t i;
  int Overflow = 0;
  i = *Counter;
  if (i == 0)
    Overflow = 1;
  i--;
  i &= 077777;
  *Counter = i;
  return (Overflow);
}

// Diminish increment.
int agc_t::CounterDINC(int CounterNum, int16_t * Counter) {
  //static int CountTIME6 = 0;
  int RetVal = 0;
  //int IsTIME6, FlushTIME6;
  int16_t i;
  //IsTIME6 = (Counter == &c (RegTIME6));
  //FlushTIME6 = 0;
  i = *Counter;
  if (i == AGC_P0 || i == AGC_M0)	// Zero?
    {
      // Emit a ZOUT.
      if (CounterNum != 0)
        ChannelOutput (0x80 | CounterNum, 017);
    }
  else if (040000 & i)			// Negative?
    {
      i++;
      if (i == AGC_M0)
        {
          RetVal = -1;
	  //if (IsTIME6)
	  //  FlushTIME6 = 1;
	}
      //else if (IsTIME6)
      //  {
      //    CountTIME6--;
      //    if (CountTIME6 <= -160)
      //      FlushTIME6 = 1;
      //  }
      // Emit a MOUT.
      if (CounterNum != 0)
        ChannelOutput (0x80 | CounterNum, 016);
    }
  else					// Positive?
    {
      i--;
      if (i == AGC_P0)
        {
          RetVal = 1;
	  //if (IsTIME6)
	  //  FlushTIME6 = 1;
	}
      //else if (IsTIME6)
      //  {
      //    CountTIME6++;
      //    if (CountTIME6 >= 160)
      //      FlushTIME6 = 1;
      //  }
      // Emit a POUT.
      if (CounterNum != 0)
        ChannelOutput (0x80 | CounterNum, 015);
    }
  *Counter = i;
  //if (FlushTIME6 && CountTIME6)
  //  {
  //    ChannelOutput (State, 0165, CountTIME6);
  //    CountTIME6 = 0;
  //  }
  return (RetVal);
}

// Left-shift increment.
int CounterSHINC (int16_t * Counter) {
  int16_t i;
  int Overflow = 0;
  i = *Counter;
  if (020000 & i)
    Overflow = 1;
  i = (i << 1) & 037777;
  *Counter = i;
  return (Overflow);
}

// Left-shift and add increment.
int CounterSHANC (int16_t * Counter) {
  int16_t i;
  int Overflow = 0;
  i = *Counter;
  if (020000 & i)
    Overflow = 1;
  i = ((i << 1) + 1) & 037777;
  *Counter = i;
  return (Overflow);
}

// Pinch hits for the above in setting interrupt requests with INCR,
// AUG, and DIM instructins.  The docs aren't very forthcoming as to
// which counter registers are affected by this ... but still.

void agc_t::InterruptRequest(int16_t LAddress10, int Sum) {
  if (ValueOverflowed (Sum) == AGC_P0)
    return;
  if (IsReg (LAddress10, RegTIME1))
    CounterPINC (&c (RegTIME2));
  else if (IsReg (LAddress10, RegTIME6))
    InterruptRequests[1] = 1;
  else if (IsReg (LAddress10, RegTIME5))
    InterruptRequests[2] = 1;
  else if (IsReg (LAddress10, RegTIME3))
    InterruptRequests[3] = 1;
  else if (IsReg (LAddress10, RegTIME4))
    InterruptRequests[4] = 1;
}

//-------------------------------------------------------------------------------
// The case of PCDU or MCDU triggers being applied to the CDUX,Y,Z counters
// presents a special problem.  The AGC expects these triggers to be
// applied at a certain fixed rate.  The DAP portion of Luminary or Colossus
// applies a digital filter to the counts, in order to eliminate electrical
// noise, as well as noise caused by vibration of the spacecraft.  Therefore,
// if the simulated IMU applies PCDU/MCDU triggers too fast, the digital
// filter in the DAP will simply reject the count, and therefore the spacecraft's
// orientation cannot be measured by the DAP.  Consequently, we have to
// fake up a kind of FIFO on the triggers to the CDUX,Y,Z counters so that
// we can increment or decrement the counters at no more than the fixed rate.
// (Conversely, of course, the simulated IMU has to be able to supply the
// triggers *at least* as fast as the fixed rate.)
//
// Actually, there are two different fixed rates for PCDU/MCDU:  400 counts
// per second in "slow mode", and 6400 counts per second in "fast mode".
//
// *** FIXME! All of the following junk will need to move to agc_t, and will
//     somehow have to be made compatible with backtraces. ***
// The way the FIFO works is that it can hold an ordered set of + counts and
// - counts.  For example, if it held 7,-5,10, it would mean to apply 7 PCDUs,
// followed by 5 MCDUs, followed by 10 PCDUs.  If there are too many sign-changes
// buffered, triggers will be transparently dropped.
#define MAX_CDU_FIFO_ENTRIES 128
#define NUM_CDU_FIFOS 3			// Increase to 5 to include OPTX, OPTY.
#define FIRST_CDU 032
typedef struct {
  int Ptr;				// Index of next entry being pulled.
  int Size;				// Number of entries.
  int IntervalType;			// 0,1,2,0,1,2,...
  uint64_t NextUpdate;			// Cycle count at which next counter update occurs.
  int32_t Counts[MAX_CDU_FIFO_ENTRIES];
} CduFifo_t;
static CduFifo_t CduFifos[NUM_CDU_FIFOS];// For registers 032, 033, and 034.
static int CduChecker = 0;		// 0, 1, ..., NUM_CDU_FIFOS-1, 0, 1, ...

/*
void agc_t::LogCdu(char* format, int a,int b,int c=0) {
  if (CduLog != nullptr)
    fprintf (CduLog,format, a, b, c);
}
*/

// Here's an auxiliary function to add a count to a CDU FIFO.  The only allowed
// increment types are:
//	001	PCDU "slow mode"
//	003	MCDU "slow mode"
//	021	PCDU "fast mode"
//	023	MCDU "fast mode"
// Within the FIFO, we distinguish these cases as follows:
//	001	Upper bits = 00
//	003	Upper bits = 01
//	021	Upper bits = 10
//	023	Upper bits = 11
// The least-significant 30 bits are simply the absolute value of the count.
void agc_t::PushCduFifo (int Counter, int IncType) {
  CduFifo_t *CduFifo;
  int Next, Interval;
  int32_t Base;
  if (Counter < FIRST_CDU || Counter >= FIRST_CDU + NUM_CDU_FIFOS)
    return;
  switch (IncType)
    {
    case 1:
      Interval = 213;
      Base = 0x00000000;
      break;
    case 3:
      Interval = 213;
      Base = 0x40000000;
      break;
    case 021:
      Interval = 13;
      Base = 0x80000000;
      break;
    case 023:
      Interval = 13;
      Base = 0xC0000000;
      break;
    default:
      return;
    }
  LogCdu("< " FORMAT_64U " %o %02o\n",CycleCounter,Counter,IncType);
  CduFifo = &CduFifos[Counter - FIRST_CDU];
  // It's a little easier if the FIFO is completely empty.
  if (CduFifo->Size == 0)
    {
      CduFifo->Ptr = 0;
      CduFifo->Size = 1;
      CduFifo->Counts[0] = Base + 1;
      CduFifo->NextUpdate = CycleCounter + Interval;
      CduFifo->IntervalType = 1;
      return;
    }
  // Not empty, so find the last entry in the FIFO.
  Next = CduFifo->Ptr + CduFifo->Size - 1;
  if (Next >= MAX_CDU_FIFO_ENTRIES)
    Next -= MAX_CDU_FIFO_ENTRIES;
  // Last entry has different sign from the new data?
  if ((CduFifo->Counts[Next] & 0xC0000000) != (unsigned) Base)
    {
      // The sign is different, so we have to add a new entry to the
      // FIFO.
      if (CduFifo->Size >= MAX_CDU_FIFO_ENTRIES)
        {  
	  // No place to put it, so drop the data.
	  return;
	}
      CduFifo->Size++;
      Next++;
      if (Next >= MAX_CDU_FIFO_ENTRIES)
        Next -= MAX_CDU_FIFO_ENTRIES;
      CduFifo->Counts[Next] = Base + 1;
      return;
    }
  // Okay, add in the new data to the last FIFO entry.  The sign is assured
  // to be compatible.  The size of the FIFO doesn't increase. We also don't
  // bother to check for arithmetic overflow, since only the wildest IMU
  // failure could cause it.
  CduFifo->Counts[Next]++;
}

// Here's an auxiliary function to perform the next available PCDU or MCDU
// from a CDU FIFO, if it is time to do so.  We only check one of the CDUs
// each time around (in order to preserve proper cycle counts), so this function 
// must be called at at least an 6400*NUM_CDU_FIFO cps rate.  Returns 0 if no
// counter was updated, non-zero if a counter was updated.
int agc_t::ServiceCduFifo() {
  int Count, RetVal = 0, HighRate, DownCount;
  CduFifo_t *CduFifo;
  int16_t *Ch;
  // See if there are any pending PCDU or MCDU counts we need to apply.  We only
  // check one of the CDUs, and the CDU to check is indicated by CduChecker.
  CduFifo = &CduFifos[CduChecker];

  if (CduFifo->Size > 0 && CycleCounter >= CduFifo->NextUpdate)
    {  
      // Update the counter.
      Ch = &Erasable[0][CduChecker + FIRST_CDU];
      Count = CduFifo->Counts[CduFifo->Ptr];
      HighRate = (Count & 0x80000000);
      DownCount = (Count & 0x40000000);
      if (DownCount)
        {
          CounterMCDU (Ch);
	  LogCdu(">\t\t" FORMAT_64U " %o 03\n", CycleCounter, CduChecker + FIRST_CDU);
	}
      else
        {
          CounterPCDU (Ch);
	  LogCdu(">\t\t" FORMAT_64U " %o 01\n", CycleCounter, CduChecker + FIRST_CDU);
	}
      Count--;
      // Update the FIFO.
      if (0 != (Count & ~0xC0000000))
        CduFifo->Counts[CduFifo->Ptr] = Count;
      else
        {
	  // That FIFO entry is exhausted.  Remove it from the FIFO.
	  CduFifo->Size--;
	  CduFifo->Ptr++;
	  if (CduFifo->Ptr >= MAX_CDU_FIFO_ENTRIES)
	    CduFifo->Ptr = 0;
	}
      // And set next update time.
      // Set up for next update time.  The intervals is are of the form
      // x, x, y, depending on whether CduIntervalType is 0, 1, or 2.
      // This is done because with a cycle type of 1024000/12 cycles per
      // second, the exact CDU update times don't fit on exact cycle
      // boundaries, but every 3rd CDU update does hit a cycle boundary.
      if (CduFifo->NextUpdate == 0)
        CduFifo->NextUpdate = CycleCounter;
      if (CduFifo->IntervalType < 2)
	{
	  if (HighRate)
	    CduFifo->NextUpdate += 13;
	  else
	    CduFifo->NextUpdate += 213;
	  CduFifo->IntervalType++;
	}  
      else 
	{
	  if (HighRate)
	    CduFifo->NextUpdate += 14;
	  else
	    CduFifo->NextUpdate += 214;
	  CduFifo->IntervalType = 0;
	}
      // Return an indication that a counter was updated.
      RetVal = 1;
    }

  CduChecker++;
  if (CduChecker >= NUM_CDU_FIFOS)
    CduChecker = 0;

  return (RetVal);
}

//----------------------------------------------------------------------------
// This function is used to update the counter registers on the basis of
// commands received from the outside world.

void agc_t::UnprogrammedIncrement(int Counter, int IncType) {
  int16_t *Ch;
  int lOverflow = 0;
  Counter &= 0x7f;
  Ch = &Erasable[0][Counter];
  CoverageErasableWriteCounts(0,Counter);
  switch (IncType)
    {
    case 0:  
      //TrapPIPA = (Counter >= 037 && Counter <= 041);
      lOverflow = CounterPINC (Ch);
      break;
    case 1: 
    case 021: 
      // For the CDUX,Y,Z counters, push the command into a FIFO.
      if (Counter >= FIRST_CDU && Counter < FIRST_CDU + NUM_CDU_FIFOS)
        PushCduFifo(Counter, IncType);
      else
        lOverflow = CounterPCDU (Ch);
      break;
    case 2:  
      //TrapPIPA = (Counter >= 037 && Counter <= 041);
      lOverflow = CounterMINC (Ch);
      break;
    case 3:  
    case 023:
      // For the CDUX,Y,Z counters, push the command into a FIFO.
      if (Counter >= FIRST_CDU && Counter < FIRST_CDU + NUM_CDU_FIFOS)
        PushCduFifo (Counter, IncType);
      else
        lOverflow = CounterMCDU (Ch);
      break;
    case 4:  
      lOverflow = CounterDINC (Counter, Ch);
      break;
    case 5:  
      lOverflow = CounterSHINC (Ch);
      break;
    case 6:  
      lOverflow = CounterSHANC (Ch);
      break;
    default:
      break;
    }
  if (lOverflow)
    {
      // On some counters, overflow is supposed to cause
      // an interrupt.  Take care of setting the interrupt request here.
     
    }
//  TrapPIPA = 0;
}

//----------------------------------------------------------------------------
// Function handles the coarse-alignment output pulses for one IMU CDU drive axis.  
// It returns non-0 if a non-zero count remains on the axis, 0 otherwise.
            
int agc_t::BurstOutput (int DriveBitMask, int CounterRegister, int Channel) {
  static int CountCDUX = 0, CountCDUY = 0, CountCDUZ = 0;  // In target CPU format.
  int DriveCount = 0, DriveBit, Direction = 0, Delta, DriveCountSaved;
  if (CounterRegister == RegCDUXCMD)
    DriveCountSaved = CountCDUX;
  else if (CounterRegister == RegCDUYCMD)
    DriveCountSaved = CountCDUY;
  else if (CounterRegister == RegCDUZCMD)
    DriveCountSaved = CountCDUZ;
  else
    return (0);
  // Driving this axis?
  DriveBit = (ReadIO(014) & DriveBitMask);
  // If so, we must retrieve the count from the counter register.
  if (DriveBit)
    {
      DriveCount = Erasable[0][CounterRegister];
      Erasable[0][CounterRegister] = 0;
    }
  // The count may be negative.  If so, normalize to be positive and set the
  // direction flag.
  Direction = (040000 & DriveCount);
  if (Direction)
    {
      DriveCount ^= 077777;
      DriveCountSaved -= DriveCount;
    }
  else
    DriveCountSaved += DriveCount;
  if (DriveCountSaved < 0)
    {
      DriveCountSaved = -DriveCountSaved;
      Direction = 040000;
    }
  else
    Direction = 0;
  // Determine how many pulses to output.  The max is 192 per burst.
  Delta = DriveCountSaved;
  if (Delta >= 192 / COARSE_SMOOTH)
    Delta = 192 / COARSE_SMOOTH;
  // If the count is non-zero, pulse it.
  if (Delta > 0)
    {
      ChannelOutput (Channel, Direction | Delta);
      DriveCountSaved -= Delta;
    }
  if (Direction)
    DriveCountSaved = -DriveCountSaved;
  if (CounterRegister == RegCDUXCMD)
    CountCDUX = DriveCountSaved;
  else if (CounterRegister == RegCDUYCMD)
    CountCDUY = DriveCountSaved;
  else if (CounterRegister == RegCDUZCMD)
    CountCDUZ = DriveCountSaved;
  return (DriveCountSaved);
}

//-----------------------------------------------------------------------------
// Execute one machine-cycle of the simulation.  Use agc_engine_init prior to 
// the first call of agc_engine, to initialize State, and then call agc_engine 
// thereafter every (simulated) 11.7 microseconds.
//
// Returns:
//      0 -- success
// I'm not sure if there are any circumstances under which this can fail ...

// Note on addressing of bits within words:  The MIT docs refer to bits
// 1 through 15, with 1 being the least-significant, and 15 the most 
// significant.  A 16th bit, the (odd) parity bit, would be bit 0 in this
// scheme.  Now, we're probably not going to use the parity bit in our
// simulation -- I haven't fully decided this at the time I'm writing
// this note -- so we have a choice of whether to map the 15 bits that ARE
// used to D0-14 or to D1-15.  I'm going to choose the latter, even though
// it requires slightly more processing, in order to conform as obviously
// as possible to the MIT docs.

#define SCALER_OVERFLOW 160
#define SCALER_DIVIDER 3
static int ScalerCounter = 0;

// Fine-alignment.
// The gyro needs 3200 pulses per second, and therefore counts twice as
// fast as the regular 1600 pps counters.
#define GYRO_OVERFLOW 160
#define GYRO_DIVIDER (2 * 3)
static unsigned GyroCount = 0;
static unsigned OldChannel14 = 0, GyroTimer = 0;

// Coarse-alignment.
// The IMU CDU drive emits bursts every 600 ms.  Each cycle is 
// 12/1024000 seconds long.  This happens to mean that a burst is
// emitted every 51200 CPU cycles, but we multiply it out below
// to make it look pretty
#define IMUCDU_BURST_CYCLES ((600 * 1024000) / (1000 * 12 * COARSE_SMOOTH))
static uint64_t ImuCduCount = 0;
static unsigned ImuChannel14 = 0;

int agc_t::fetch() {
      // Store the current value of several registers.
  CurrentEB = EB();
  CurrentFB = FB();
  CurrentBB = BB();
  // Reform 16-bit accumulator and test for overflow in accumulator.
  Accumulator = A() & 0177777;
  Overflow = (ValueOverflowed (Accumulator) != AGC_P0);
  //Qumulator = GetQ (State);
  //OverflowQ = (ValueOverflowed (Qumulator) != AGC_P0);

  // After each instruction is executed, the AGC's Z register is updated to
  // indicate the next instruction to be executed.
  ProgramCounter = Z();
  // However, since the Z register contains only 12 bits, the address has to
  // be massaged to get a 16-bit address.
  WhereWord = FindMemoryWord(ProgramCounter);

  // Fetch the instruction itself.
  //Instruction = *WhereWord;
  if (SubstituteInstruction) {
      Instruction = BRUPT();
      if (0100000 & Instruction)
        sExtraCode = 1;
      Instruction &= 077777;
    }
  else
    {
      // The index is sometimes positive and sometimes negative.  What to
      // do if the result has overflow, I can't say.  I arbitrarily
      // overflow-correct it.
      sExtraCode = ExtraCode;
      Instruction =
	OverflowCorrected (AddSP16
			   (SignExtend (IndexValue),
			    SignExtend (*WhereWord)));
      Instruction &= 077777;
      // Handle interrupts.
      if (DebuggerInterruptMasks[0] &&
	  !InIsr && AllowInterrupt && !ExtraCode &&
	  IndexValue == 0 && !PendFlag && !Overflow &&
	  ValueOverflowed (L()) == AGC_P0 &&
	  ValueOverflowed (Q()) == AGC_P0 &&
	  //ProgramCounter > 060 &&
	  Instruction != 3 && Instruction != 4 && Instruction != 6)
	{
	  int i, j;
	  // We use the InterruptRequests array slightly oddly.  Since the
	  // interrupts are numbered 1 to 10 (NUM_INTERRUPT_TYPES), we begin
	  // indexing the array at 1, so that entry 0 does not hold an
	  // interrupt request.  Instead, we use entry 0 to tell the last
	  // interrupt type that occurred.  In searches, we begin one up from
	  // the last interrupt, and then wrap around.  This keeps the same
	  // interrupt from happening over and over to the exclusion of all
	  // other interrupts.  (I have no clue as to whether the AGC actually
	  // did this or not.)  Moreover, I assume interrupt vectoring takes
	  // one additional machine cycle.  Don't really know, however.
	  // Search for the next interrupt request.
	  i = InterruptRequests[0];	// Last interrupt serviced.
	  if (i == 0)
	    i = InterruptRequests[0] = NUM_INTERRUPT_TYPES;	// Initialization.
	  j = i;		// Ending point.
	  do
	    {
	      i++;		// Index at which to start searching.
	      if (i > NUM_INTERRUPT_TYPES)
		i = 1;
	      if (InterruptRequests[i] && DebuggerInterruptMasks[i])
		{
		  BacktraceAdd (i);
		  // Clear the interrupt request.
		  InterruptRequests[i] = 0;
		  InterruptRequests[0] = i;
		  // Set up the return stuff.
		  ZRUPT() = ProgramCounter;
		  BRUPT() = Instruction;
		  // Vector to the interrupt.
		  InIsr = 1;
		  NextZ = 04000 + 4 * i;
		  return 1;
		}
	    }
	  while (i != j);
	}
    }

  return 0;
}

void agc_t::decode() {
  //Decode the instruction opcode and address(es)
  OpCode = Instruction & ~MASK12;
  QuarterCode = Instruction & ~MASK10;
  Address12 = Instruction & MASK12;
  Address10 = Instruction & MASK10;
  Address9 = Instruction & MASK9;
  ExtendedOpcode = Instruction >> 9;	//2;
  if (sExtraCode) ExtendedOpcode |= 0100;

}

int agc_t::delay() {
      // Add delay for multi-MCT instructions.  Works for all instructions
  // except EDRUPT, BZF, and BZMF.  For those, an extra cycle is added
  // AFTER executing the instruction -- not because it's more logically
  // correct, just because it's easier.
  if (!PendFlag)
    {
      int i;
      i = QuarterCode >> 10;
      if (ExtraCode)
	i = ExtracodeTiming[i];
      else
	i = InstructionTiming[i];
      if (i)
	{
	  PendFlag = 1;
	  PendDelay = i;
	  return (1);
	}
    }
  else
    PendFlag = 0;

  // Now that the index value has been used, get rid of it.
  IndexValue = AGC_P0;
  // And similarly for the substitute instruction from a RESUME.
  SubstituteInstruction = 0;
  return 0;
}

void agc_t::updateZ() {

  // Compute the next value of the instruction pointer.  I haven't found
  // any explanation so far as to what happens if the pointer is already at
  // the end of a memory block, so I don't know if it's supposed to roll to
  // the next pseudo-address, or wrap to the beginning of the bank, or what.
  // My assumption is that the programmer (or assembler, perhaps) simply
  // wasn't supposed to allow this to happen.  (In fixed memory, this is
  // literally true, since the bank terminates with a bugger word rather than
  // with an instruction, so the issue is only what happens in erasable
  // memory.)  As a first cut, therefore, I simply increment the thing without
  // checking for a problem.  (The increment is by 2, since bit 0 is the
  // parity and the address only starts at bit 1.)
  NextZ = 1 + Z();
  // I THINK that the Z register is updated before the instruction executes,
  // which is important if you have an instruction that directly accesses
  // the value in Z.  (I deduce this from descriptions of the TC register,
  // which imply that the contents of Z is directly transferred into Q.)
  Z() = NextZ;
}

void agc_t::execute() {

  // Execute the instruction.  Refer to p.34 of 1689.pdf for an easy
  // picture of what follows.
  opcode[ExtendedOpcode](*this);

}

int agc_t::do_timers() {
      //----------------------------------------------------------------------
  // Here we take care of counter-timers.  There is a basic 1/1600 second
  // clock that is used to drive the timers.  1/1600 second happens to
  // be SCALER_OVERFLOW/SCALER_DIVIDER machine cycles, and the variable
  // ScalerCounter has already been updated the correct number of
  // multiples of SCALER_DIVIDER.  Note that incrementing a timer register
  // takes 1 machine cycle.

  // This can only iterate once, but I use 'while' just in case.
  while (ScalerCounter >= SCALER_OVERFLOW)
    {
      // First, update SCALER1 and SCALER2.
      ScalerCounter -= SCALER_OVERFLOW;
      if (CounterPINCChannel(ChanSCALER1))
	{
	  ExtraDelay++;
	  CounterPINCChannel(ChanSCALER2);
	}
      // Check whether there was a pulse into bit 5 of SCALER1.
      // If so, the 10 ms. timers TIME1 and TIME3 are updated.
      // Recall that the registers are in AGC integer format,
      // and therefore are actually shifted left one space.
      if (0 == (017 & ReadIO(ChanSCALER1)))
	{
	  ExtraDelay++;
	  if (CounterPINC (&c (RegTIME1)))
	    {
	      ExtraDelay++;
	      CounterPINC (&c (RegTIME2));
	    }
	  ExtraDelay++;
	  if (CounterPINC (&c (RegTIME3)))
	    InterruptRequests[3] = 1;
	  // I have very little data about what TIME5 is supposed to do.
	  // From the table on p. 1-64 of Savage & Drake, I assume
	  // it works just like TIME3.
	  ExtraDelay++;
	  if (CounterPINC (&c (RegTIME5)))
	    InterruptRequests[2] = 1;
	}
      // TIME4 is the same as TIME3, but 5 ms. out of phase.
      if (010 == (017 & ReadIO(ChanSCALER1)))
	{
	  ExtraDelay++;
	  if (CounterPINC (&c (RegTIME4)))
	    InterruptRequests[4] = 1;
	}
      // I'm not sure if TIME6 is supposed to count when the T6 RUPT
      // is disabled or not.  For the sake of argument, I'll assume
      // that it is.  Nor am I sure how many bits this counter has.
      // I'll assume 14.  Nor if it's out of phase with SCALER1.
      // Nor ... well, you get the idea.
      ExtraDelay++;
      if (CounterDINC (0, &c (RegTIME6)))
	if (040000 & ReadIO(013))
	  InterruptRequests[1] = 1;
      // Return, so as to account for the time occupied by updating the
      // counters.
      return (1);
    }
  return 0;
}

void agc_t::do_Gyro() {
  //----------------------------------------------------------------------
  // Same principle as for the counter-timers (above), but for handling
  // the 3200 pulse-per-second fictitious register 0177 I use to support
  // driving the gyro.
    int j;
#ifdef GYRO_TIMING_SIMULATED
  // Update the 3200 pps gyro pulse counter.
  GyroTimer += GYRO_DIVIDER;
  while (GyroTimer >= GYRO_OVERFLOW)
    {
      GyroTimer -= GYRO_OVERFLOW;
      // We get to this point 3200 times per second.  We increment the
      // pulse count only if the GYRO ACTIVITY bit in channel 014 is set.
      if (0 != (InputChannel[014] & 01000) &&
          GYROCTR() > 0)
	{
          GyroCount++;
	  GYROCTR()--;
	  if (GYROCTR() == 0)
	    InputChannel[014] &= ~01000;
	}
    }

  // If 1/4 second (nominal gyro pulse count of 800 decimal) or the gyro
  // bits in channel 014 have changed, output to channel 0177.
  i = (ReadIO(014) & 01740);  // Pick off the gyro bits.
  if (i != OldChannel14 || GyroCount >= 800)
    {
      j = ((OldChannel14 & 0740) << 6) | GyroCount;
      OldChannel14 = i;
      GyroCount = 0;
      ChannelOutput (State, 0177, j);
    }
#else // GYRO_TIMING_SIMULATED
#define GYRO_BURST 800
#define GYRO_BURST2 1024
  if (0 != (ReadIO(014) & 01000))
    if (0 != GYROCTR())
      {
        // If any torquing is still pending, do it all at once before
	// setting up a new torque counter.
        while (GyroCount)
	  {
	    j = GyroCount;
	    if (j > 03777)
	      j = 03777;
	    ChannelOutput(0177, OldChannel14 | j);
	    GyroCount -= j;
	  }
	// Set up new torque counter.
	GyroCount = GYROCTR();
	GYROCTR() = 0;
	OldChannel14 = ((ReadIO(014) & 0740) << 6);
	GyroTimer = GYRO_OVERFLOW * GYRO_BURST - GYRO_DIVIDER;
      }
  // Update the 3200 pps gyro pulse counter.
  GyroTimer += GYRO_DIVIDER;
  while (GyroTimer >= GYRO_BURST * GYRO_OVERFLOW)
    {
      GyroTimer -= GYRO_BURST * GYRO_OVERFLOW;
      if (GyroCount)
        {
	  j = GyroCount;
	  if (j > GYRO_BURST2)
	    j = GYRO_BURST2;
	  ChannelOutput(0177, OldChannel14 | j);
	  GyroCount -= j;
	}
    }
#endif // GYRO_TIMING_SIMULATED

}

void agc_t::do_CDU() {
  int i;
  //----------------------------------------------------------------------
  // ... and somewhat similar principles for the IMU CDU drive for
  // coarse alignment.

  i = (ReadIO(014) & 070000);	// Check IMU CDU drive bits.
  if (ImuChannel14 == 0 && i != 0)		// If suddenly active, start drive.
    ImuCduCount = CycleCounter - IMUCDU_BURST_CYCLES;
  if (i != 0 && (CycleCounter - ImuCduCount) >= IMUCDU_BURST_CYCLES) // Time for next burst.
    {
      // Adjust the cycle counter.
      ImuCduCount += IMUCDU_BURST_CYCLES;
      // Determine how many pulses are wanted on each axis this burst.
      ImuChannel14 = BurstOutput (040000, RegCDUXCMD, 0174);
      ImuChannel14 |= BurstOutput (020000, RegCDUYCMD, 0175);
      ImuChannel14 |= BurstOutput (010000, RegCDUZCMD, 0176);
    }
}

void agc_t::do_Shaft() {
  //----------------------------------------------------------------------
  // Finally, stuff for driving the optics shaft & trunnion CDUs.  Nothing
  // fancy like the fine-alignment and coarse-alignment stuff above.
  // Just grab the data from the counter and dump it out the appropriate
  // fictitious port as a giant lump.

  if (OPTX() && 0 != (ReadIO(014) & 02000))
    {
      ChannelOutput (0172, OPTX());
      OPTX() = 0;
    }
  if (OPTY() && 0 != (ReadIO(014) & 04000))
    {
      ChannelOutput(0171, OPTY());
      OPTY() = 0;
    }
}

void agc_t::cleanup() {
      // All done!
  if (!PendFlag)
    {
      ZERO() = AGC_P0;
      superbank &= 0160;
      Z() = NextZ;
      if (!KeepExtraCode)
	ExtraCode = 0;
      // Values written to EB and FB are automatically mirrored to BB,
      // and vice versa.
      if (CurrentBB != BB())
	{
	  FB() = (BB() & 076000);
	  EB() = (BB() & 07) << 8;
	}
      else if (CurrentEB != EB() || CurrentFB != FB())
	BB() = (FB() & 076000) | ((EB() & 03400) >> 8);
      EB() &= 03400;
      FB() &= 076000;
      BB() &= 076007;
    }
}

int agc_t::earlyExit() {
  if (ChannelInput()) return 1;

  // If in --debug-dsky mode, don't want to take the chance of executing
  // any AGC code, since there isn't any loaded anyway.

  //----------------------------------------------------------------------
  // This stuff takes care of extra CPU cycles used by some instructions.

  // A little extra delay, needed sometimes after branch instructions that
  // don't always take the same amount of time.
  if (ExtraDelay) {
    ExtraDelay--;
    return 1;
  }

  // If an instruction that takes more than one clock-cycle is in progress,
  // we simply return.  We don't do any of the actual computations for such
  // an instruction until the last clock cycle for it is reached.
  // (Except for a few weird cases dealt with by ExtraDelay as above.)
  if (PendFlag && PendDelay > 0) {
    PendDelay--;
    return 1;
  }

  //----------------------------------------------------------------------
  // Take care of any PCDU or MCDU operations that are lingering in CDU
  // FIFOs. If a CDU counter was serviced, a cycle was used up, and we must
  // return.
  if (ServiceCduFifo ()) return 1;

  if(do_timers()) return 1;
  return 0;
}

//----------------------------------------------------------------------
// The following little thing is useful only for debugging yaDEDA with
// the --debug-deda command-line switch.  It just outputs the contents
// of the address that was specified by the DEDA at 1/2 second intervals.
void agc_t::debugDeda() {
  int16_t Data;
  Data = Erasable[0][DedaAddress];
  DedaWhen = CycleCounter + 1024000 / 24;	// 1/2 second.
  ShiftToDeda ((DedaAddress >> 6) & 7);
  ShiftToDeda ((DedaAddress >> 3) & 7);
  ShiftToDeda (DedaAddress & 7);
  ShiftToDeda (0);
  ShiftToDeda ((Data >> 12) & 7);
  ShiftToDeda ((Data >> 9) & 7);
  ShiftToDeda ((Data >> 6) & 7);
  ShiftToDeda ((Data >> 3) & 7);
  ShiftToDeda (Data & 7);
}

int agc_t::step() {
  static int Count = 0;
  KeepExtraCode=0;

  sExtraCode = 0;

  // For DOWNRUPT
  if (DownruptTimeValid && CycleCounter >= DownruptTime) {
    InterruptRequests[8] = 1;	// Request DOWNRUPT
    DownruptTimeValid = 0;
  }

  CycleCounter++;

  if (DedaMonitor && CycleCounter >= DedaWhen) debugDeda();

  //----------------------------------------------------------------------
  // Update the thingy that determines when 1/1600 second has passed.
  // 1/1600 is the basic timing used to drive timer registers.  1/1600
  // second happens to be 160/3 machine cycles.

  ScalerCounter += SCALER_DIVIDER;

  //-------------------------------------------------------------------------

  // Handle server stuff for socket connections used for i/o channel
  // communications.  Stuff like listening for clients we only do
  // every once and a while---nominally, every 100 ms.  Actually 
  // processing input data is done every cycle.
  if (Count == 0) ChannelRoutine();
  Count = ((Count + 1) & 017777);

  // Get data from input channels.  Return immediately if a unprogrammed 
  // counter-increment was performed.
  if(earlyExit()) return(0);

  do_Gyro();
  do_CDU();
  do_Shaft();

  //----------------------------------------------------------------------  
  // Okay, here's the stuff that actually has to do with decoding instructions.

  if(!fetch()) {
    decode();
    if(delay()) return(0);

    updateZ();
    execute();
  }

  cleanup();

  return (0);
}

// This stub-function is here to keep agc_engine from slowing itself down by
// saving backtrace information, which is useful only for a debugger we're not
// building into the code anyway.
void agc_t::BacktraceAdd(int Cause) {
  // Keep this empty.
}

//----------------------------------------------------------------------
// This function is useful only for debugging the socket interface, and
// so can be left as-is.

void agc_t::ShiftToDeda(int Data){
}




/*
  Copyright 2003-2006,2009 Ronald S. Burkey <info@sandroid.org>

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

  Filename:	agc_engine.h
  Purpose:	Header file for AGC emulator engine.
  Contact:	Ron Burkey <info@sandroid.org>
  Reference:	http://www.ibiblio.org/apollo

  For more insight, I'd highly recommend looking at the documents
  http://hrst.mit.edu/hrs/apollo/public/archive/1689.pdf and
  http://hrst.mit.edu/hrs/apollo/public/archive/1704.pdf.
*/

#ifndef AGC_ENGINE_H
#define AGC_ENGINE_H

#include <cstdint>
#define USE_STD_STRING
#ifdef USE_STD_STRING
#include <string>
#endif
//----------------------------------------------------------------------------
// Constants.

/** Max number of symbols in a yaAGC sym-dump. */
const auto MAX_SYM_DUMP=25;

/** Max number of files in a file dump */
const auto MAX_FILE_DUMP=25;

/** Physical AGC timing was generated from a master 1024 KHz clock, divided by 12.
 This resulted in a machine cycle of just over 11.7 microseconds.  Note that the
 constant is unsigned long long. */
const auto AGC_PER_SECOND= ((1024000 + 6) / 12);


#define NUM_INTERRUPT_TYPES 10

class agc_t;
//typedef char* StringType;
typedef void (*opcode_f)(agc_t&);
/** Class defining an instance of the AGC emulator.

 Each instance of the AGC CPU simulation has a data structure of type agc_t
 that contains the CPU's internal states, the complete memory space, and any
 other little handy items needed to track execution by the CPU.
 AGC is a word-based machine rather than a byte-based machine. All memory --
 registers, RAM, and ROM -- is 16-bit, consisting of 15 bits
 of data and one of (odd) parity.  The MIT documents consistently
 use octal, so we do as well. The virtual AGC currently ignores the parity bit,
 but since modern processors (anything targetable by C++) like to work in bytes,
 memory is represented as 16-bit words with one bit ignored, neglected, and wasted.

 Physical memory in the AGC consists of 36 banks of fixed (read-only) memory,
 each of 1024 words, and 8 banks of erasable (read-write) memory, each of
 256 words. This gives 36ki words of fixed memory and 2ki words of erasable. This would
 require 16 bits of address. Since instructions have 12 bits or less available
 for addresses, a strategy involving banking registers is used to supply the rest of
 the address bits.

 This memory is addressed in hardware as one hardware linear address space, as
 described in http://www.ibiblio.org/apollo/MirkoMattioliMemoryMap.pdf . The hardware
 address hardly matters, as the virtual AGC implements both kinds of memory as arrays
 of banks, each of which is an array of words.

 The logical addressing is as follows: All memory below 01777 is erasable. This space
 is big enough to cover 4 erasable banks. Banks 0, 1, and 2 are always available
 (unswitched-erasable) and cover 00000-01377. Any bank (including the unswitched banks)
 can appear in the remaining space 01400-01777. Which bank is visible is controlled
 by the EBANK register, which itself is stored in erasable bank 0 (like all the registers)
 and therefore is always visible. Erasable memory addresses are written as E%01o,%04o , 
 always in octal, with a one-digit bank number followed by a 4-digit address in the bank.
 Since the switched banks always appear starting at 01400, their addresses are shown 
 relative to that point as well. As a special case, banks E0, E1, and E2 are shown without
 bank numbers, and with their addresses starting at 0000 (for E0), 0400 (for E1), or
 1000 (for E2).

 The next block of memory, between 02000 and 07777 is fixed. The block from 02000-03777 is
 switched fixed memory, and can be any one of the 36 banks, controlled by the FBANK register
 and superbank bit. The block from 04000-07777 is always fixed banks 2 and 3 (fixed-fixed
 memory). Addresses in fixed memory are written as %02o,%04o , always in octal, with a
 two-digit bank number followed by a 4-digit address in the bank. Since switched fixed banks
 always appear starting at address 02000, the addresses are written as starting at 2000
 also. As a special case, memory in banks 2 and 3 is not written with a bank number, but
 instead written as an address starting at 4000 (for bank 2) or 6000 (for bank 3) since
 this is where they will always appear.

 There is a separate address space for I/O channels, which function as memory in some ways
 but under the control of other hardware than the CPU -- IE, they can change spontaneously
 depending on outside input. These are not banked, and are numbered from 0000-0777, the vast
 majority of which is unused. The limit is based on the 9-bit addresses for instructions
 which use the I/O address space. The input channels appear to share the same
 9-bit space with the output channels, so using WRITE on a channel and then READ will normally
 return the same value, subject to the hardware outside. Note that there are lots of
 special cases dealing with the channels - for instance, some of the registers are
 duplicated in channel space, some of the registers are latching, superbank bits are in a
 channel, etc. The virtual AGC represents the channels as another array in RAM, so all of
 them will act like memory if not assigned to another purpose. The real AGC most likely
 consigns writes to unimplemented channels to the bit bucket.
*/
class agc_t {
public:
/**Count the total number of clock cycles since
   CPU-startup.  A 64-bit integer is used, because with a 32-bit integer
   you'd get only about 14 hours before the counter wraps around.*/
  uint64_t CycleCounter;
  static const auto n_fixedwords=02000; ///<Size of a single bank of fixed memory in words
  static const auto n_erasewords=00400; ///<Size of a single bank of erasable memory in words
  typedef const int16_t fixedbank[n_fixedwords]; ///< Typedef for a single fixed memory bank
  typedef       int16_t erasebank[n_erasewords]; ///< Typedef for a single erasable memory bank
  static const auto n_fixedbanks=36;
  typedef fixedbank     fixedbanks[n_fixedbanks];     ///< Typedef for entire fixed memory
  typedef fixedbanks&   fixedbanks_r;     ///< Typedef for reference to fixed memory (Man, C++ typedefs can get complicated)
  static const auto n_erasebanks=8;
  typedef erasebank     erasebanks[n_erasebanks];     ///< Typedef for entire erasable memory
  typedef erasebanks&   erasebanks_r;     ///< Typedef for reference to erasable memory

/** Erasable memory. Banks 0,1,2 are "unswitched erasable". */
  erasebanks Erasable;
/** Fixed memory. Banks 2,3 are "fixed-fixed", IE available no matter what the
    bank register is set to.
  Originally this was an array of arrays, kept within the structure. We change
  it to a pointer to a fixedbank, and take advantage of the pointer<->array
  duality so that no other code has to change. This way fixedbank doesn't have
  to be contiguous with the rest of the structure, advantageous if we
  want to store the rope in real-life ROM (such as flash in an embedded processor)
  There are actually only 36 (0-043) fixed banks, but the calculation of bank
  numbers by the AGC can theoretically go 0-39 (0-047). This form doesn't restrict
  the number of banks (no bounds check) but woe be unto the one who tries to read
  off the end of the rope. */
  fixedbanks_r Fixed;
// Constants related to "input/output channels".
  static const auto n_channels=512;
  static const auto ChanSCALER2=03;
  static const auto ChanSCALER1=04;
  static const auto ChanS=07;
  int16_t superbank;   ///< Holds contents of I/O channel 7, which contains the superbank bits.
  int16_t IndexValue;  ///< The indexing value.
  int8_t InterruptRequests[1 + NUM_INTERRUPT_TYPES];	// 0-index not used.
  // CPU internal flags.
  unsigned ExtraCode:1;		///< Set by the "Extend" instruction.
  unsigned AllowInterrupt:1;
  unsigned InIsr:1;		///< Set when in an ISR, reset when in normal code.
  unsigned SubstituteInstruction:1;	///< Use BBRUPT register.
  unsigned PendFlag:1;		///< Multi-MCT instruction pending.
  unsigned PendDelay:3;		///< Countdown to pending instruction.
  unsigned ExtraDelay:3;	///< Countdown extra delay, for special cases.
  unsigned DownruptTimeValid:1;	///< Set if the DownruptTime field is valid.
  uint64_t DownruptTime;	///< Time when next DOWNRUPT occurs.
  int Downlink;
  int NextZ;

  //Methods: All the functions which previously started with agc_t* State get moved in here
  int step();
  agc_t(const char *RomImage, const char *CoreDump, int AllOrErasable);
  agc_t(fixedbanks_r fixed);
  int agc_load_binfile(const char *RomImage);
  virtual int ReadIO (int Address)=0;
  int CpuReadIO (int Address);
  void CpuWriteIO (int Address, int Value);
  void MakeCoreDump (const char *CoreDump);
//void UnblockSocket (int SocketNum);
//FILE *rfopen (const char *Filename, const char *mode);
  void BacktraceAdd (int Cause);
  int BacktraceRestore (int n);
  void BacktraceDisplay (int Num);

  // API for yaAGC-to-peripheral communications.
  void ShiftToDeda(int Data);
  int ServiceCduFifo();
  int BurstOutput(int, int, int);
  int CounterDINC(int CounterNum, int16_t * Counter);
  int CounterPINC(int16_t * Counter);
  int CounterPINCChannel(int16_t Channel);
  //Previously defined only in agc_engine.c
  int earlyExit();
  void cleanup();
  int  fetch();
  int  delay();
  void updateZ();
  void execute();
  void do_Shaft();
  void do_CDU();
  void do_Gyro();
  int  do_timers();
  void debugDeda();
  void UnprogrammedIncrement(int Counter, int IncType);
  void InterruptRequest(int16_t Address10, int Sum);
  void Assign(int Bank, int Offset, int Value);
  void PushCduFifo (int Counter, int IncType);

//Functions defined in agc_engine.c but only used in agc_instr.c
  void AssignFromPointer(int16_t * Pointer, int Value);

//Functions defined in agc_engine.c and used in agc_instr.c also
  int16_t* FindMemoryWord(int Address12);

//Previously instruction_state_t
  uint16_t ProgramCounter, Instruction, OpCode, QuarterCode, sExtraCode;
  int16_t *WhereWord;
  uint16_t Address12, Address10, Address9;
  int ValueK, KeepExtraCode;
  int16_t CurrentEB, CurrentFB, CurrentBB;
  uint16_t ExtendedOpcode;
  int Overflow, Accumulator;
  void decode();

  virtual void ChannelSetup()=0;
  virtual void ChannelOutput(int Channel, int Value)=0;
  virtual int  ChannelInput()=0;
  virtual void ChannelRoutine() {};

  static const opcode_f opcode[0200];
#ifdef USE_STD_STRING
  static const std::string opcode_name[0200];
#else
  static const char* opcode_name[0200];
#endif
  //What I really want is for the compiler/linker to figure out if a
  //virtual method is never overridden, and treat it as non-virtual
  //in that case. Can't do that, so only add virtual if you think
  //you really are going to override. So, we have a bunch of non-virtual
  //inline methods with empty bodies, for debugging purposes.

//coverage stuff
  void CoverageIoReadCounts(int Address) {};
  void CoverageIoWriteCounts(int Address) {};
  void CoverageErasableWriteCounts(int Bank, int Offset) {};

  //CDU logging stuff
  void LogCdu(char* format, int a1,int a2,int a3=0) {};

// Some helpful [inline functions] for manipulating registers.
  int16_t& c(int Reg) {return Erasable[0][Reg];}
//#define c(Reg) Erasable[0][Reg]
  static bool IsA (int Address) {return Address == RegA ;}
  static bool IsL (int Address) {return Address == RegL ;}
  static bool IsQ (int Address) {return Address == RegQ ;}
  static bool IsEB(int Address) {return Address == RegEB;}
  static bool IsZ (int Address) {return Address == RegZ ;}
  static bool IsReg(int Address,int16_t Reg) {return Address == Reg;}

// Number of registers to treat as 16 bits rather than 15 bits.  I started here
// with 020, but I found that rupt 4 will load BB into the accumulator and check
// for overflow, with bad results.
  static const auto REG16=3;

// Handy names for the memory locations associated with special-purpose 
// registers, in octal.
  static const auto RegA      =000;  
  static const auto RegL      =001;
  static const auto RegQ      =002;
  static const auto RegEB     =003;
  static const auto RegFB     =004;
  static const auto RegZ      =005;
  static const auto RegBB     =006;
  static const auto RegZERO   =007;
  static const auto RegARUPT  =010;
  static const auto RegLRUPT  =011;
  static const auto RegQRUPT  =012;
// Addresses 013 and 014 are spares.
  static const auto RegZRUPT  =015;
  static const auto RegBBRUPT =016;
  static const auto RegBRUPT  =017;
  static const auto RegCYR    =020;
  static const auto RegSR     =021;
  static const auto RegCYL    =022;
  static const auto RegEDOP   =023;
// Addresses 024-057 are counters.
  static const auto RegCOUNTER=024;
  static const auto RegTIME2  =024;
  static const auto RegTIME1  =025;
  static const auto RegTIME3  =026;
  static const auto RegTIME4  =027;
  static const auto RegTIME5  =030;
  static const auto RegTIME6  =031;
  static const auto RegCDUX   =032;
  static const auto RegCDUY   =033;
  static const auto RegCDUZ   =034;
  static const auto RegOPTY   =035;
  static const auto RegOPTX   =036;
  static const auto RegPIPAX  =037;
  static const auto RegPIPAY  =040;
  static const auto RegPIPAZ  =041;
// 042-044 are spares in the CM, rotational hand controller in LM.
  static const auto RegRHCP   =042;
  static const auto RegRHCY   =043;
  static const auto RegRHCR   =044;
  static const auto RegINLINK =045;
  static const auto RegRNRAD  =046;
  static const auto RegGYROCTR=047;
  static const auto RegCDUXCMD=050;
  static const auto RegCDUYCMD=051;
  static const auto RegCDUZCMD=052;
  static const auto RegOPTYCMD=053;
  static const auto RegOPTXCMD=054;
// 055-056 are spares.
  static const auto RegOUTLINK=057;
  static const auto RegALTM   =060;
// Addresses 061-03777 are general-purpose RAM.
  static const auto RegRAM    =060;
// Addresses 04000-0117777 are ROM (core memory).
  static const auto RegCORE =04000;
  static const auto RegEND=0120000;

//Register access functions
// Handy names for the memory locations associated with special-purpose 
// registers, in octal.
  int16_t& A      () {return c(RegA      );}
  int16_t& L      () {return c(RegL      );}
  int16_t& Q      () {return c(RegQ      );}
  int16_t& Z      () {return c(RegZ      );}
  int16_t& EB     () {return c(RegEB     );}
  int16_t& FB     () {return c(RegFB     );}
  int16_t& BB     () {return c(RegBB     );}
  int16_t& ZRUPT  () {return c(RegZRUPT  );}
  int16_t& BRUPT  () {return c(RegBRUPT  );}
  int16_t& ZERO   () {return c(RegZERO   );}
  int16_t& GYROCTR() {return c(RegGYROCTR);}
  int16_t& OPTY   () {return c(RegOPTY   );}
  int16_t& OPTX   () {return c(RegOPTX   );} 
// Some numerical constant, in AGC format.
  static const int16_t AGC_P0=0;
  static const int16_t AGC_M0=077777;
  static const int16_t AGC_P1= 1;
  static const int16_t AGC_M1= 077776;

/** Compute the "diminished absolute value".  The input data and output data
 are both in AGC 1's-complement format. */
  static int16_t dabs (int16_t Input) {
    if (0 != (040000 & Input))
      Input = 037777 & ~Input;    // Input was negative, but now is positive.
    if (Input > 1)                // "diminish" it if >1.
      Input--;
    else
      Input = AGC_P0;
    return (Input);
  }

  // Same, but for 16-bit registers.
  static int odabs (int Input) {
    if (0 != (0100000 & Input))
      Input = (0177777 & ~Input); // Input was negative, but now is positive.
    if (Input > 1)                // "diminish" it if >1.
      Input--;
    else
      Input = AGC_P0;
    return (Input);
  }

};

#define FORMAT_64U "%llu"
#define FORMAT_64O "%llo"

typedef struct
{
  int Socket;
  unsigned char Packet[4];
  int Size;
  int ChannelMasks[256];
  //int DedaBufferCount;
  //int DedaBufferWanted;
  //int DedaBufferReadout;
  //int DedaBufferDefault;
  //int DedaBuffer[9];
} Client_t;

void ChannelRoutineGeneric(void *State, void (*UpdatePeripherals) (void *, Client_t *));

#define DEFAULT_MAX_CLIENTS 0

#ifdef AGC_ENGINE_C
// MAX_CLIENTS is the maximum number of hardware simulations which can be
// attached.  The DSKY is always one, presumably.  The array is a list of 
// the sockets used for the clients.  Thus stuff shown below is the 
// DEFAULT setup.  The max number of clients can be change during runtime
// initialization by setting MAX_CLIENTS to a different number, allocating
// new arrays of clients and sockets corresponding to the new size, and 
// then pointing the Clients and ServerSockets pointers at those arrays.
int MAX_CLIENTS = DEFAULT_MAX_CLIENTS;
static Client_t DefaultClients[DEFAULT_MAX_CLIENTS];
static int DefaultSockets[DEFAULT_MAX_CLIENTS];
Client_t *Clients = DefaultClients;
int *ServerSockets = DefaultSockets;
int NumServers = 0;
int SocketInterlaceReload = 50;
int DebugDeda = 0, DedaQuiet = 0;
int DedaMonitor = 0;
int DedaAddress;
uint64_t /* unsigned long long */ DedaWhen;
int CmOrLm = 0;	// Default is 0 (LM); other choice is 1 (CM)
int LastRhcPitch = 0, LastRhcYaw = 0, LastRhcRoll = 0;
#else //AGC_ENGINE_C
extern int MAX_CLIENTS;
extern Client_t *Clients;
extern int *ServerSockets;
extern int NumServers;
extern int SocketInterlaceReload;
extern int DebugDeda, DedaQuiet;
extern int DedaMonitor;
extern int DedaAddress;
extern uint64_t /* unsigned long long */ DedaWhen;
extern int CmOrLm;
extern int LastRhcPitch, LastRhcYaw, LastRhcRoll;
#endif //AGC_ENGINE_C

//---------------------------------------------------------------------------
// Function prototypes.

char *nbfgets (char *Buffer, int Length);
void nbfgets_ready (const char *);
int16_t OverflowCorrected (int Value);
int SignExtend (int16_t Word);
int AddSP16 (int Addend1, int Addend2);
void UnprogrammedIncrement (agc_t& State, int Counter, int IncType);

//Functions defined in agc_engine.c but only used in agc_instr.c
//-----------------------------------------------------------------------------
int agc2cpu (int Input);
int cpu2agc (int Input);
int agc2cpu2 (int Input);
int cpu2agc2 (int Input);
int SpToDecent (int16_t * LsbSP);
void DecentToSp (int Decent, int16_t * LsbSP);
int16_t AbsSP (int16_t Value);
int16_t NegateSP (int16_t Value);

//Functions defined in agc_engine.c and used in agc_instr.c also
int16_t ValueOverflowed (int Value);


#endif // AGC_ENGINE_H



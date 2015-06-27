#include "host_agc_t.h"
#include <cstdio>
#include <ncurses.h>

//-----------------------------------------------------------------------------
// Any kind of setup needed by your i/o-channel model.

host_agc_t::host_agc_t(fixedbanks_r Lfixed, char* LLines):agc_io(Lfixed),Lines(LLines) {
  char* ptr=Lines;
  LineAddr[0]=ptr;
  for(int i=1;i<012000;i++) {
    while(*ptr!=0x0A) ptr++;
    *ptr=0;
    ptr++;
    LineAddr[i]=ptr;
  }
};

void host_agc_t::print_state() {
  int i_line;
  char* line;
  static char erase[]="Erasable instruction executed";
  int FB,bankaddr,dispbankaddr;
  if(ProgramCounter<02000) {
    FB=-1;
    bankaddr=ProgramCounter;
    line=erase;
  } else {
    if(ProgramCounter>=06000) {
      FB=3;
      bankaddr=ProgramCounter-06000;
      dispbankaddr=ProgramCounter;
    } else if(ProgramCounter>=04000) {
      FB=2;
      bankaddr=ProgramCounter-04000;
      dispbankaddr=ProgramCounter;
    } else if(ProgramCounter>=02000) {
      FB = (037 & (CurrentFB >> 10));
      // Account for the superbank bit.
      if (030 == (FB & 030) && (superbank & 0100) != 0)  FB += 010;
      bankaddr=ProgramCounter-02000;
      dispbankaddr=ProgramCounter;
    }
    i_line=FB*02000+bankaddr;
    line=LineAddr[i_line];
  }
  mvprintw(0,0,"Z: %02o,%04o Ins: %05o PD:%d ED:%d %-120s",FB,ProgramCounter,Instruction,PendDelay,ExtraDelay,((PendDelay==0)&&(ExtraDelay==0))?line:"delay");
  mvprintw(1,0,"A: %06o L: %06o",A()& 0100000,L() & 010000);
}

void host_agc_t::ChannelSetup() {
  // ... anything you like ...
  ChannelIsSetUp=true;
}

void host_agc_t::ChannelOutput(int Channel, int Value) {

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

  const auto channelTop=12;
  const auto channelWidth=15;
  mvprintw(channelTop+(Channel%16),(Channel/16)*channelWidth,"%03o <- %05o",Channel,Value);
  // ... anything you like ...
  // You don't need to worry about channels 1 and 2 here.
  if(010==Channel||011==Channel) {
    //These control the DSKY display, so update that
    printDSKY();
  }
}

void host_agc_t::printDSKY() {
  //Two digits each, prog, verb, noun. One leading space to make things one-based, and one trailing null so printf works 
  static char rm[]="   ";
  static char rn[]="   ";
  static char rv[]="   ";
//Sign then 5 digits for r1, r2, r3, plus trailing null
  static char r1[]="      ";
  static char r2[]="      ";
  static char r3[]="      ";
//Some of the indicator lamps
//v - VEL
//n - NO ATT
//a - ALT
//g - GIMBAL LOCK
//t - TRACKER
//p - PROG
  static char rL[]="012vnagtp";
  static const char map[32]={
 //  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    ' ','?','?','1','?','?','?','?','?','?','?','?','?','?','?','4','?','?','?','7','?','0','?','?','?','2','?','3','6','8','5','9'
  };
                     //  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
  static char* ptrc[]={  0,r3,r3,r2,r2,r2,r1,r1, 0,rn,rv,rm, 0, 0, 0, 0};
  static char  digc[]={  0, 4, 2, 5, 3, 1, 4, 2, 0, 1, 1, 1, 0, 0, 0, 0};
  static char* ptrd[]={  0,r3,r3,r3,r2,r2,r1,r1,r1,rn,rv,rm, 0, 0, 0, 0};
  static char  digd[]={  0, 5, 3, 1, 4, 2, 5, 3, 1, 2, 2, 2, 0, 0, 0, 0};
  static char  sigb[]={  0,-3,+3, 0,-2,+2,-1,+1, 1, 0, 0, 0, 0, 0, 0, 0};
  for(int i=1;i<=11;i++) {
    int d=((OutputChannel10[i]>> 0) & ((1<<5)-1));
    int c=((OutputChannel10[i]>> 5) & ((1<<5)-1));
    int b=((OutputChannel10[i]>>10) & ((1<<1)-1));
    int a=((OutputChannel10[i]>>11) & ((1<<4)-1));
    if(a!=i) {
//      printf("Huh? a=%2d, i=%2d\n",a,i);
    } else {
//      printf("a: %d, b: %d, c: %d, d: %d\n",a,b,c,d);
      if(   ptrc[i])           ptrc[a][digc[a]]=map[c];
      if(   ptrd[i])           ptrd[a][digd[a]]=map[d];
      if((0!=sigb[i])&&ptrc[a]) ptrc[a][0]=(b>0)?((sigb[a]>0)?'+':'-'):' ';
    }
  }
  int i=12;
  for(int j=3;j<=9;j++) {
    rL[j]=(rL[j] & (0xFF^0x20)) | ((InputChannel[011]>>j & 0x01)?0x00:0x20);
  }
  const auto dskyTop=6;
  mvprintw(dskyTop+0,2,"%6s",rL+3);
  mvprintw(dskyTop+1,7,"%2s",rm+1);
  mvprintw(dskyTop+2,4,"%2s",rv+1);
  mvprintw(dskyTop+2,7,"%2s",rn+1);
  mvprintw(dskyTop+3,3,"%6s",r1);
  mvprintw(dskyTop+4,3,"%6s",r2);
  mvprintw(dskyTop+5,3,"%6s",r3);
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

int host_agc_t::ChannelInput() {
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

extern const agc_t::fixedbanks _binary_rope_bin_start;
extern char _binary_rope_fullmap_start[];
host_agc_t State(_binary_rope_bin_start,_binary_rope_fullmap_start);

void setup() {
  initscr();
//  nodelay();
//  noecho();
  cbreak();
  State.print_state();
}

void loop() {
  State.step();
  State.print_state();
  getch();
//  State.check_input();
  refresh();
}

int main() {
  setup();
  for(;;) loop();
}


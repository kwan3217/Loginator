#ifndef dump_H
#define dump_H

#include "Print.h"

extern char _btext[],_etext[];
extern char __xz_start__[];
extern char __xz_end__[];

class Dump {
protected:
  Print& out;
  int preferredLen;
  virtual void begin() {};
  virtual void end() {};
  virtual void line(const char* start, int base, int len)=0;
public:
  Dump(Print& Lout, int LpreferredLen):out(Lout),preferredLen(LpreferredLen) {};
  void region(const char* start, int base, int len, int rec_len);
  void region(const char* start, int len, int rec_len) {region(start,(int)start,len,rec_len);};
  void dumpText() {region(_btext,_etext-_btext,preferredLen);};
  void dumpXz()   {region(__xz_start__,0,__xz_end__-__xz_start__,preferredLen);}
};

class IntelHex: public Dump {
  unsigned char checksum;
  unsigned int addr;
  void print_byte(unsigned char len);
  void begin_line(unsigned char len, unsigned short a, unsigned char type);
  void end_line();
  void address(int ia);
  virtual void line(const char* start, int base, int len);
  virtual void begin();
  virtual void end();
public:
  IntelHex(Print& Lout):Dump(Lout,32) {};
};

//Converts 4 bytes of binary into 5 printable ASCII characters from 33 (!) to 117 (u)
//This implementation does not use the "z" shortcut so that any binary string of 4n bytes
//will be encoded into 5n characters. 
class Base85: public Dump {
  void print_group(const char* p, int len);
  virtual void line(const char* start, int base, int len);
public:
  Base85(Print& Lout):Dump(Lout,64) {};
};

class Hd: public Dump {
  virtual void line(const char* start, int base, int len);
public:
  Hd(Print& Lout):Dump(Lout,16) {};
};
#endif

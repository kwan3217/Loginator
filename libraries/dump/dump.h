#ifndef dump_H
#define dump_H

#include "Print.h"

extern char btext[],etext[];
extern char source_start[];
extern char source_end[];

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
  void dumpText() {region(btext,etext-btext,preferredLen);};
  void dumpSource() {region(source_start,0,source_end-source_start,preferredLen);}
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

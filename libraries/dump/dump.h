#ifndef dump_H
#define dump_H

#include "Print.h"

class dump {
protected:
  Print* out;
public:
  dump(Print* Lout):out(Lout) {};
  virtual void region(const char* start, int base, int len, int rec_len)=0;
  void region(const char* start, int len, int rec_len) {region(start,(int)start,len,rec_len);};
  void dumpText();
};

class IntelHex: public dump {
  unsigned char checksum;
  unsigned int addr;
  void print_byte(unsigned char len);
  void begin_line(unsigned char len, unsigned short a, unsigned char type);
  void end_line();
  void address(int ia);
  void line(const char* start, int base, int len);
  void begin();
  void end();
public:
  IntelHex(Print* Lout):dump(Lout) {};
  virtual void region(const char* p, int base, int len, int rec_len);
};

//Converts 4 bytes of binary into 5 printable ASCII characters from 33 (!) to 117 (u)
//This implementation does not use the "z" shortcut so that any binary string of 4n bytes
//will be encoded into 5n characters. 
class Base85: public dump {
  void print_group(const char* p, int len);
  void line(const char* start, int base, int len);
  void begin();
  void end();
public:
  Base85(Print* Lout):dump(Lout) {};
  virtual void region(const char* p, int base, int len, int rec_len);
};

#endif

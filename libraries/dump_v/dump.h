#ifndef dump_H
#define dump_H

#include "Print.h"
#include "tarball.h"
#include <stddef.h> //for size_t
extern char btext[], etext[];

class Dump {
protected:
  Print& out;
  int preferredLen;
public:
  Dump(Print& Lout, int LpreferredLen):out(Lout),preferredLen(LpreferredLen) {};
  virtual void begin() {};
  virtual void end() {};
  virtual void line(const char* start, size_t base, size_t len)=0;
  void region(const char* start, size_t base, size_t len, unsigned int rec_len);
  void region(const char* start, size_t len, unsigned int rec_len) {region(start,(size_t)start,len,rec_len);};
  void region(const char* start, size_t len) {region(start,0,len,preferredLen);};
  void dumpText() {region(btext,etext-btext,preferredLen);};
  void dumpSource() {region(source_start,0,source_end-source_start,preferredLen);}
};

class IntelHex: public Dump {
private:
  unsigned char checksum;
  unsigned int addr;
  void print_byte(unsigned char len);
  void begin_line(unsigned char len, unsigned short a, unsigned char type);
  void end_line();
  void address(size_t ia);
public:
  IntelHex(Print& Lout):Dump(Lout,32) {};
  virtual void begin();
  virtual void end();
  virtual void line(const char* start, size_t base, size_t len);
};

//Converts 4 bytes of binary into 5 printable ASCII characters from 33 (!) to 117 (u)
//This implementation does not use the "z" shortcut so that any binary string of 4n bytes
//will be encoded into 5n characters. 
class Base85: public Dump {
private:
  void print_group(const char* p, size_t len);
public:
  Base85(Print& Lout):Dump(Lout,64) {};
  Base85(Print& Lout, int LpreferredLen):Dump(Lout,LpreferredLen) {};
  virtual void line(const char* start, size_t base, size_t len);
};

class Hd: public Dump {
public:
  Hd(Print& Lout):Dump(Lout,16) {};
  Hd(Print& Lout, int LpreferredLen):Dump(Lout,LpreferredLen) {};
  virtual void line(const char* start, size_t base, size_t len);
};
#endif

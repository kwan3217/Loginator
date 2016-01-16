#ifndef dump_H
#define dump_H

#include "Print.h"
#include "tarball.h"
#include <stddef.h> //for size_t
extern char btext[], etext[];

/** Memory dump class. Given a block of memory, prints an ASCII encoding of
    that block to the designated Print class. This is designed
    to support such things as Intel Hex, Base85, etc.
@tparam T CRTP derived class. This class uses CRTP to implement static polymorphism.
*/
template<class T>
class Dump {
protected:
  Print& out;
  int preferredLen;
public:
  Dump(Print& Lout, int LpreferredLen):out(Lout),preferredLen(LpreferredLen) {};
  //virtual void line(const char* start, size_t base, size_t len)=0;
  void region(const char* start, size_t base, size_t len, unsigned int rec_len);
  void region(const char* start, size_t len, unsigned int rec_len) {region(start,(size_t)start,len,rec_len);};
  void region(const char* start, size_t len) {region(start,0,len,preferredLen);};
  void dumpText() {region(btext,etext-btext,preferredLen);};
  void dumpSource() {region(source_start,0,source_end-source_start,preferredLen);}
};

template<class T> inline
void Dump<T>::region(const char* p, size_t base, size_t len, unsigned int rec_len) {
	static_cast<T*>(this)->begin();
  while(len>0) {
    if(rec_len>len) rec_len=len;
    static_cast<T*>(this)->line(p,base,rec_len);
    base+=rec_len;
    p+=rec_len;
    len-=rec_len;
  }
  static_cast<T*>(this)->end();
}

class IntelHex: public Dump<IntelHex> {
private:
  unsigned char checksum;
  unsigned int addr;
public:
  //Lower level routines
  void print_byte(unsigned char len);
  void begin_line(unsigned char len, unsigned short a, unsigned char type);
  void end_line();
  void address(size_t ia);
  //high-level routines
  IntelHex(Print& Lout):Dump<IntelHex>(Lout,32) {};
  /*virtual*/ void begin();
  /*virtual*/ void end();
  /*virtual*/ void line(const char* start, size_t base, size_t len);
};

//Converts 4 bytes of binary into 5 printable ASCII characters from 33 (!) to 117 (u)
//This implementation does not use the "z" shortcut so that any binary string of 4n bytes
//will be encoded into 5n characters. 
class Base85: public Dump<Base85> {
private:
  void print_group(const char* p, size_t len);
public:
  Base85(Print& Lout):Dump(Lout,64) {};
  Base85(Print& Lout, int LpreferredLen):Dump(Lout,LpreferredLen) {};
  /*virtual*/ void line(const char* start, size_t base, size_t len);
};

class Hd: public Dump<Hd> {
public:
  Hd(Print& Lout):Dump(Lout,16) {};
  Hd(Print& Lout, int LpreferredLen):Dump(Lout,LpreferredLen) {};
  virtual void line(const char* start, size_t base, size_t len);
};
#endif

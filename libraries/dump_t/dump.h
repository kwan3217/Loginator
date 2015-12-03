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
@tparam P Print class to print the dump to.
*/
template<class T, class P>
class Dump {
protected:
  P& out;
  int preferredLen;
public:
  Dump(P& Lout, int LpreferredLen):out(Lout),preferredLen(LpreferredLen) {};
  /*virtual*/ void begin() {};
  /*virtual*/ void end() {};
  //virtual void line(const char* start, size_t base, size_t len)=0;
  void region(const char* start, size_t base, size_t len, unsigned int rec_len);
  void region(const char* start, size_t len, unsigned int rec_len) {region(start,(size_t)start,len,rec_len);};
  void region(const char* start, size_t len) {region(start,0,len,preferredLen);};
  void dumpText() {region(btext,etext-btext,preferredLen);};
  void dumpSource() {region(source_start,0,source_end-source_start,preferredLen);}
};

/** Intel Hex dump class. Given a block of memory, dumps the memory in Intel
    HEX format as documented on https://en.wikipedia.org/wiki/Intel_HEX .
    Specifically this uses type 4 records to specify the high bytes of a 32-bit
    address, then a series of type 0 records to represent the data, followed
    by a type 1 record to mark the end of file.
@tparam P Print class to print the dump to.
*/
template<class P>
class IntelHex: public Dump<IntelHex<P>,P> {
private:
  unsigned char checksum;
  unsigned int addr;
  void print_byte(unsigned char len);
  void begin_line(unsigned char len, unsigned short a, unsigned char type);
  void end_line();
  void address(size_t ia);
public:
  IntelHex(P& Lout):Dump<IntelHex<P>,P>(Lout,32) {};
  /*virtual*/ void begin();
  /*virtual*/ void end();
  /*virtual*/ void line(const char* start, size_t base, size_t len);
};

/** Base85 dump class. Converts 4 bytes of binary into 5 printable ASCII
    characters from 33 (!) to 117 (u). This implementation does not use
    the "z" shortcut so that any binary string of 4n bytes will be encoded
    into 5n characters.
@tparam P Print class to print the dump to.
*/
template<class P>
class Base85: public Dump<Base85<P>,P> {
private:
  void print_group(const char* p, size_t len);
public:
  Base85(P& Lout):Dump<Base85<P>,P>(Lout,64) {};
  Base85(P& Lout, int LpreferredLen):Dump<Base85<P>,P>(Lout,LpreferredLen) {};
  /*virtual*/ void line(const char* start, size_t base, size_t len);
};

/** Hex dump class. Prints an address, followed by a number of hex bytes, followed
    by the ascii equivalent. The hex bytes are broken up into blocks of four bytes
    (eight hex digits) and the ascii equivalent is printed only for printable
    characters from 32 to 126 inclusive.
@tparam P Print class to print the dump to.
*/
template<class P>
class Hd: public Dump<Hd<P>,P> {
public:
  Hd(P& Lout):Dump<Hd<P>,P>(Lout,16) {};
  Hd(P& Lout, int LpreferredLen):Dump<Hd<P>,P>(Lout,LpreferredLen) {};
  /*virtual*/ void line(const char* start, size_t base, size_t len);
};

#include "dump.inc"
#endif

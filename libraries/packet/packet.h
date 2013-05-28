#ifndef packet_h
#define packet_h
#include <inttypes.h>
#include "Circular.h"
#include "float.h"

class Packet {
protected: 
  Circular& buf;
public:
  Packet(Circular &Lbuf):buf(Lbuf) {};
  virtual bool start(unsigned short apid, unsigned short* seq=0, unsigned int TC=0xFFFFFFFF)=0;
  virtual bool finish()=0;
  bool fill(char in) {return buf.fill(in);};
  virtual bool fill16(unsigned short in)=0;
  virtual bool fill32(unsigned int in)=0;
  virtual bool fill64(uint64_t in)=0;
  virtual bool fillfp(fp f)=0;
  virtual bool fill(const char* in);
  virtual bool fill(const char* in, int length);
};

class CCSDS: public Packet{
public:
  CCSDS(Circular &Lbuf):Packet(Lbuf) {};
  virtual bool start(unsigned short apid, unsigned short* seq=0, unsigned int TC=0xFFFFFFFF);
  virtual bool finish();
  virtual bool fill16(unsigned short in);
  virtual bool fill32(unsigned int in);
  virtual bool fill64(uint64_t in);
  virtual bool fillfp(fp f);
};

#endif


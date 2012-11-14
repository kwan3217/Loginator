#ifndef packet_h
#define packet_h
#include "Circular.h"
#include "float.h"

class Packet {
protected:
  Circular& buf;
public:
  Packet(Circular &Lbuf):buf(Lbuf) {};
  virtual bool start(unsigned short apid, unsigned short* seq=0, unsigned int TC=0xFFFFFFFF, unsigned int TS=0)=0;
  virtual bool finish()=0;
  bool fill(char in) {return buf.fill(in);};
  virtual bool fill16(unsigned short in)=0;
  virtual bool fill32(unsigned int in)=0;
  virtual bool fillfp(fp f)=0;
  virtual bool fill(const char* in);
  virtual bool fill(const char* in, int length);
};

class CCSDS: public Packet{
public:
  CCSDS(Circular &Lbuf):Packet(Lbuf) {};
  virtual bool start(unsigned short apid, unsigned short* seq=0, unsigned int TC=0xFFFFFFFF, unsigned int TS=0);
  virtual bool finish();
  virtual bool fill16(unsigned short in);
  virtual bool fill32(unsigned int in);
  virtual bool fillfp(fp f);
};

#endif


#ifndef packet_h
#define packet_h
#include <inttypes.h>
#include "Circular.h"
#include "float.h"

class Packet {
protected:    
  Circular* buf;
public:
  virtual bool start(Circular &Lbuf, uint16_t apid, uint16_t* seq=0, uint32_t TC=0xFFFFFFFF)=0;
  virtual bool finish(uint16_t tag)=0;
  bool fill(char in) {return buf->fill(in);};
  virtual bool fill16(uint16_t in)=0;
  virtual bool fill32(uint32_t in)=0;
  virtual bool fill64(uint64_t in)=0;
  virtual bool fillfp(fp f)=0;
  virtual bool fill(const char* in);
  virtual bool fill(const char* in, uint32_t length);
};

class CCSDS: public Packet{
private:
  uint16_t lock_apid;
public:
  CCSDS():lock_apid(0) {};
  virtual bool start(Circular &Lbuf, uint16_t apid, uint16_t* seq=0, uint32_t TC=0xFFFFFFFF);
  virtual bool finish(uint16_t tag);
  virtual bool fill16(uint16_t in);
  virtual bool fill32(uint32_t in);
  virtual bool fill64(uint64_t in);
  virtual bool fillfp(fp f);
};

#endif


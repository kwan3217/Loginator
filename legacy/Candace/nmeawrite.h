#ifndef nmeawrite_h
#define nmeawrite_h
#include "circular.h"
#include "pktwrite.h"

class nmeawrite:public pktwrite {
public:
  nmeawrite(const char* Lext):pktwrite(Lext) {}
  virtual void fillPktStartCore(circular& buf,int type);
  virtual void fillPktFinishCore(circular& buf);
  virtual int fillPktByte(circular& buf, char in);
  virtual int fillPktShort(circular& buf, short in);
  virtual int fillPktInt(circular& buf, int in);
  virtual int fillPktString(circular& buf, const char* in);
  virtual int fillPktStringn(circular& buf, const char* in, int length);
  virtual int fillPktBlock(circular& buf, const char* in, int length);
  virtual int fillPktChar(circular& buf, char in);
  virtual int fillPktFP(circular& buf, fp f);
};

extern nmeawrite NMEAWrite;

#endif


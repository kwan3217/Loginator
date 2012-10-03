#ifndef sirfwrite_h
#define sirfwrite_h
#include "circular.h"
#include "pktwrite.h"

class sirfwrite:public pktwrite {
public:
  sirfwrite(const char* Lext):pktwrite(Lext) {}
  virtual void fillPktStartCore(circular& buf,int type);
  virtual void fillPktFinishCore(circular& buf);
  virtual int fillPktByte(circular& buf, char in);
  virtual int fillPktShort(circular& buf, short in);
  virtual int fillPktInt(circular& buf, int in);
  virtual int fillPktString(circular& buf, const char* in);
  virtual int fillPktStringn(circular& buf, const char* in, int length) {return fillPktBlock(buf,in,length);}
  virtual int fillPktBlock(circular& buf, const char* in, int length);
  virtual int fillPktChar(circular& buf, char in);
  virtual int fillPktFP(circular& buf, fp f);
};

extern sirfwrite SiRFWrite;

#endif


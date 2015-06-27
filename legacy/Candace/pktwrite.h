#ifndef pktwrite_h
#define pktwrite_h
class pktwrite;
#include "circular.h"
#include "setup.h"
#include "float.h"

//These must be in the same order as headerNMEASIRF in pktwrite.c
#define PT_COLUMNS   0 
#define PT_GAINADJ   1
#define PT_DECIDE    2
#define PT_ANALOG    3
#define PT_MAM       4
#define PT_FILE      5
#define PT_LOAD      6
#define PT_PPS       7
#define PT_VERSION   8
#define PT_UART      9
#define PT_SDINFO   10
#define PT_BAUD     11
#define PT_FLASH    12
#define PT_ERROR    13
#define PT_I2C      14
#define PT_I2CSETUP 15

class pktwrite {
public:
  const char* const ext;
  pktwrite(const char* const Lext):ext(Lext) {}
  virtual void fillPktStartCore(circular& buf,int type=-1)=0;
  virtual void fillPktFinishCore(circular& buf)=0;
  virtual int fillPktByte(circular& buf, char in)=0;
  virtual int fillPktShort(circular& buf, short in)=0;
  virtual int fillPktInt(circular& buf, int in)=0;
  virtual int fillPktString(circular& buf, const char* in)=0;
  virtual int fillPktStringn(circular& buf, const char* in, int length)=0;
  virtual int fillPktBlock(circular& buf, const char* in, int length)=0;
  virtual int fillPktChar(circular& buf, char in)=0;
  virtual int fillPktFP(circular& buf, fp f)=0;
  void fillPktStart(circular& buf,int type);
  void fillPktFinish(circular& buf, int port=-1);
  void fillPktPayload(circular& buf, char* payload, int len, int port=-1);
};

extern pktwrite *PktWriter;

//"ambient" packet writer - uses PktWrite
void fillPktStartCore(circular& buf,int type=-1);
void fillPktFinishCore(circular& buf);
int fillPktByte(circular& buf, char in);
int fillPktShort(circular& buf, short in);
int fillPktInt(circular& buf, int in);
int fillPktString(circular& buf, const char* in);
int fillPktStringn(circular& buf, const char* in, int length);
int fillPktBlock(circular& buf, const char* in, int length);
int fillPktChar(circular& buf, char in);
int fillPktFP(circular& buf, fp f);
void fillPktStart(circular& buf,int type);
void fillPktFinish(circular& buf, int port=-1);
void fillPktPayload(circular& buf, char* payload, int len, int port=-1);

#endif


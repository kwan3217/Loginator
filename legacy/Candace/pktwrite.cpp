#include "circular.h"
#include "pktwrite.h"
#include "setup.h"
#include "uart.h"
#include "conparse.h"
#include "sirfwrite.h"
#include "nmeawrite.h"
#include "main.h"

pktwrite *PktWriter;

void pktwrite::fillPktStart(circular& buf,int type) {
  //Reset all formatting parameters so that 
  //it doesn't matter what the last packet was
  buf.dataDigits=0;
  buf.dataDec=1;
  buf.dataAsciiz=0;
  buf.dataComma=1;
  buf.dataDelim=',';
  fillPktStartCore(buf,type);
}

void pktwrite::fillPktFinish(circular& buf, int port) {
  fillPktFinishCore(buf);
  if(port>=0) {
    buf.send(port);
  } else {
    buf.mark();
  }
}

void pktwrite::fillPktPayload(circular& buf, char* payload, int len, int port) {
  fillPktStartCore(buf);
  fillPktStringn(buf,payload,len);
  fillPktFinish(buf,port);
}

//"ambient" packet writer - uses PktWriter
void fillPktStartCore(circular& buf,int type) {
  PktWriter->fillPktStartCore(buf,type);
}

void fillPktFinishCore(circular& buf) {
  PktWriter->fillPktFinishCore(buf);
}

int fillPktByte(circular& buf, char in) {
  return PktWriter->fillPktByte(buf,in);
}

int fillPktShort(circular& buf, short in) {
  return PktWriter->fillPktShort(buf,in);
}

int fillPktInt(circular& buf, int in) {
    return PktWriter->fillPktInt(buf,in);
}

int fillPktString(circular& buf, const char* in) {
    return PktWriter->fillPktString(buf,in);
}

int fillPktStringn(circular& buf, const char* in, int length) {
    return PktWriter->fillPktStringn(buf,in,length);
}

int fillPktBlock(circular& buf, const char* in, int length) {
    return PktWriter->fillPktBlock(buf,in,length);
}

int fillPktChar(circular& buf, char in) {
    return PktWriter->fillPktChar(buf,in);
}

int fillPktFP(circular& buf, fp f) {
    return PktWriter->fillPktFP(buf,f);
}

void fillPktStart(circular& buf,int type) {
  PktWriter->fillPktStart(buf,type);
}

void fillPktFinish(circular& buf, int port) {
  PktWriter->fillPktFinish(buf,port);
}

void fillPktPayload(circular& buf, char* payload, int len, int port) {
  PktWriter->fillPktPayload(buf,payload,len,port);
}



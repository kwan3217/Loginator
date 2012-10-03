#include "sirfwrite.h"

sirfwrite SiRFWrite("bin");

static char headerSIRF[]={0x2A,0x33,0x2F,0x2C,0x17,0x20,0x15,0x19,0x1A,0x2B,0x18,0x16,0x40,0x41,0x42,0x43};

void sirfwrite::fillPktStartCore(circular& buf, int type) {
  buf.fillShort(0xA0A2);
  buf.fillShort(0);
  if(type>=0)buf.fill(headerSIRF[type]);
}

void sirfwrite::fillPktFinishCore(circular& buf) {
  int len=buf.unreadylen()-4;
  buf.pokeMid(2,(len >> 8) & 0xFF);
  buf.pokeMid(3,(len >> 0) & 0xFF);
  short checksum=0;
  for(int i=0;i<len;i++) {
    checksum=(checksum+buf.peekMid(i+4)) & 0x7FFF;
  }
  buf.fillShort(checksum);
  buf.fillShort(0xB0B3);
}

int sirfwrite::fillPktByte(circular& buf, char in) {
  return buf.fill(in);
}

int sirfwrite::fillPktChar(circular& buf, char in) {
  return buf.fill(in);
}

int sirfwrite::fillPktShort(circular& buf, short in) {
  return buf.fillShort(in);
}

int sirfwrite::fillPktInt(circular& buf, int in) {
  return buf.fillInt(in);
}

int sirfwrite::fillPktString(circular& buf, const char* in) {
  int i=0;
  while(in[i]!=0) {
    int result=buf.fill(in[i]);
    if (result<0) return result;
    i++;
  }
  if(buf.dataAsciiz) {
    return buf.fill(0);
  } else {
    return 0;
  }
}

int sirfwrite::fillPktBlock(circular& buf, const char* in, int len) {
  for(int i=0;i<len;i++) {
    int result=buf.fill(in[i]);
    if (result<0) return result;
  }
  return 0;
}

int sirfwrite::fillPktFP(circular& buf, fp f) {
  return buf.fillStringn((char*)&f, sizeof(fp));
}


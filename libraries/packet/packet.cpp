#include "packet.h"
#include "Time.h"

bool Packet::fill(const char* in) {
  while(*in) {
    if(!fill(*in)) return false;
    in++;
  } 
  return true;
}

bool Packet::fill(const char* in, int length) {
  for(int i=0;i<length;i++) {
    if(!fill(in[i])) return false;
  }
  return true;
}

bool CCSDS::start(unsigned short apid, unsigned short* seq, unsigned int TC) {
  const int Ver=0;  //0 for standard CCSDS telecommand according to CCSDS 102.0-B-5 11/2000
  const int Type=0; //0 for telemetry, 1 for command
  int Sec=TC<3600000000U?1:0;  //Presence of secondary header
  const int Grp=0x03;  //Grouping flags - 3->not in a group of packets
  if(!fill16(((apid & 0x7FF) | (Sec & 0x01) << 11 | (Type & 0x01) << 12 | (Ver & 0x07) << 13))) return false;
  unsigned short seq_=0;
  if(seq) seq_=*seq;
  if(!fill16(((seq_ & 0x3FFF) | (Grp & 0x03) << 14))) return false;
  if(!fill16(0xDEAD)) return false;
  if(TC<3600000000U) {
    if(!fill32(TC)) return false;
  }
  if(seq) *seq=(*seq+1)& 0x3FFF;
  return true;
}

bool CCSDS::finish() {
  int len=buf.unreadylen()-7;
  buf.pokeMid(4,(len >> 8) & 0xFF);
  buf.pokeMid(5,(len >> 0) & 0xFF);
  buf.mark();
  return true;
}

//Fill in Big-endian order as specified by CCSDS 102.0-B-5, 1.6a
bool CCSDS::fill16(unsigned short in) {
  if(!buf.fill((char)((in >> 8) & 0xFF))) return false;
  if(!buf.fill((char)((in >> 0) & 0xFF))) return false;
  return true;
}

bool CCSDS::fill32(unsigned int in) {
  if(!buf.fill((char)((in >> 24) & 0xFF))) return false;
  if(!buf.fill((char)((in >> 16) & 0xFF))) return false;
  if(!buf.fill((char)((in >>  8) & 0xFF))) return false;
  if(!buf.fill((char)((in >>  0) & 0xFF))) return false;
  return true;
}

bool CCSDS::fillfp(fp f) {
  char* fc=(char*)(&f);
  return buf.fill(fc,sizeof(f));
}


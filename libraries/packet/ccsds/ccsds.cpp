#include "ccsds.h"
#include "Time.h"
#include "gpio.h" //for blinklock
#include "Serial.h" //for printing error messages

void CCSDS::forget() {
  Packet::forget();
  lock_apid=0;
}

bool CCSDS::start(Circular &Lbuf, uint16_t apid, uint32_t TC) {
  if(lock_apid>0) {
    Serial.print("Tried to start a packet when one already in process: old: 0x");
    Serial.print(lock_apid,HEX);Serial.print(" new: 0x");Serial.print(apid,HEX);
    blinklock(apid);
  }
  buf=&Lbuf;
  lock_apid=apid;

  const int Ver=0;  //0 for standard CCSDS telecommand according to CCSDS 102.0-B-5 11/2000
  const int Type=0; //0 for telemetry, 1 for command
  int Sec=(TC!=0xFFFFFFFFU)?1:0;  //Presence of secondary header
  const int Grp=0x03;  //Grouping flags - 3->not in a group of packets
  if(!fill16(((apid & 0x7FF) | (Sec & 0x01) << 11 | (Type & 0x01) << 12 | (Ver & 0x07) << 13))) return false;
  if(!fill16(((seq[apid] & 0x3FFF) | (Grp & 0x03) << 14))) return false;
  if(!fill16(0xDEAD)) return false; //Reserve space in the packet for length
  if(Sec) {
    //Secondary header: count of microseconds since beginning of minute
    if(!fill32(TC)) return false;
  }
  seq[apid]=(seq[apid]+1)& 0x3FFF;
  return true;
}

bool CCSDS::finish(uint16_t tag) {
  //If the buffer is already full, we know not to do this
  if(buf->isFull()) {
    lock_apid=0; //otherwise the lock will never be released
    return false;
  }
  int len=buf->unreadylen()-7;
  if(len<0) {
    Serial.print("Bad packet length for tag 0x");Serial.print(tag,HEX,2);Serial.print(": ");Serial.print(len);
    blinklock(tag);
  }
  buf->pokeMid(4,(len >> 8) & 0xFF);
  buf->pokeMid(5,(len >> 0) & 0xFF);
  buf->mark();
  lock_apid=0;
  buf=0;
  return true;
}

//Fill in Big-endian order as specified by CCSDS 102.0-B-5, 1.6a
bool CCSDS::fill16(uint16_t in) {
  if(!buf->fill((char)((in >> 8) & 0xFF))) return false;
  if(!buf->fill((char)((in >> 0) & 0xFF))) return false;
  return true;
}

bool CCSDS::fill32(uint32_t in) {
  if(!buf->fill((char)((in >> 24) & 0xFF))) return false;
  if(!buf->fill((char)((in >> 16) & 0xFF))) return false;
  if(!buf->fill((char)((in >>  8) & 0xFF))) return false;
  if(!buf->fill((char)((in >>  0) & 0xFF))) return false;
  return true;
}

bool CCSDS::fill64(uint64_t in) {
  if(!buf->fill((char)((in >> 56) & 0xFF))) return false;
  if(!buf->fill((char)((in >> 48) & 0xFF))) return false;
  if(!buf->fill((char)((in >> 40) & 0xFF))) return false;
  if(!buf->fill((char)((in >> 32) & 0xFF))) return false;
  if(!buf->fill((char)((in >> 24) & 0xFF))) return false;
  if(!buf->fill((char)((in >> 16) & 0xFF))) return false;
  if(!buf->fill((char)((in >>  8) & 0xFF))) return false;
  if(!buf->fill((char)((in >>  0) & 0xFF))) return false;
  return true;
}

bool CCSDS::fillfp(fp f) {
  if(sizeof(fp)==4) {
    int* fi=(int*)(&f);
    return fill32(*fi);
  } else {
    uint64_t* fl=(uint64_t*)(&f);
    return fill64(*fl);
  }
}


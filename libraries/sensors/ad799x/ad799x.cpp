#include "ad799x.h"
#include "Task.h"
#include "Time.h"

#undef AD799x_DEBUG 

AD799x::AD799x(TwoWire *Lport, int a0):port(Lport),ADDRESS(addr_msb | (a0 & 0x01)) {}

AD799x::AD799x() {
  int i=1;
  while(i<N_ID && HW_ID_PART_TYPE(i)!=3) i++;
  if(i==N_ID) return;
  int a0=HW_ID_ADDRESS(i) & 0x01; //Just pick off the low bit
  ADDRESS=addr_msb+a0;
  port=WireA[HW_ID_PORT_NUM(i)];
}

bool AD799x::begin(uint8_t Lchannels, bool vref) {
  return writeConfig(Lchannels,vref);
}

// Read n bytes from the device at 'address'
bool AD799x::read(uint8_t *data, int n) {
  if(!port->requestFrom(ADDRESS, n)) return false;
  for(int i=0;i<n;i++) data[i]=(uint8_t)port->read();
  return true;
}

bool AD799x::write(uint8_t data) {
  port->beginTransmission(ADDRESS);
  port->write(data);
  port->endTransmission();
  return true;
}

bool AD799x::writeConfig(uint8_t Lchannels, bool vref) {
  channels=Lchannels;
  nChannels=0;
  for(int i=0;i<4;i++) if(((channels >> i) & 0x01)>0) nChannels++;
  return write((channels<<4) | ((vref?1:0)<<3) | 0x0);
}

bool AD799x::read(uint16_t data[]) {
  return read((uint8_t*)data,nChannels*2);
}

static inline uint16_t swap_endian16(uint16_t data) {
  return ((data & 0xFF)<<8) | ((data & 0xFF00) >> 8);
}

void AD799x::format(uint16_t ch[]) {
  static uint16_t ch0=0,ch1=0,ch2=0,ch3=0;
  if(nChannels>=0) {ch0=swap_endian16(ch[0]); ch[0]=0;}
  if(nChannels>=1) {ch1=swap_endian16(ch[1]); ch[1]=0;}
  if(nChannels>=2) {ch2=swap_endian16(ch[2]); ch[2]=0;}
  if(nChannels>=3) {ch3=swap_endian16(ch[3]); ch[3]=0;}
  if(nChannels>=0) ch[(ch0>>12) & 0x03]=ch0 & 0xFFF;  
  if(nChannels>=1) ch[(ch1>>12) & 0x03]=ch1 & 0xFFF;  
  if(nChannels>=2) ch[(ch2>>12) & 0x03]=ch2 & 0xFFF;  
  if(nChannels>=3) ch[(ch3>>12) & 0x03]=ch3 & 0xFFF;  
} 




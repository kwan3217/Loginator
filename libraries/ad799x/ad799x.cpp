#include "ad799x.h"
#include "Task.h"
#include "Time.h"

#undef AD799x_DEBUG 

AD799x::AD799x(TwoWire &Lport, int a0):port(Lport),ADDRESS(0x28+a0) {}

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
bool AD799x::begin(uint8_t Lchannels, bool vref) {
  return writeConfig(Lchannels,vref);
}

// Read 1 byte from the BMP085 at 'address'
bool AD799x::read(uint8_t *data, int n) {
  if(!port.requestFrom(ADDRESS, n)) return false;
  for(int i=0;i<n;i++) data[i]=(uint8_t)port.read();
  return true;
}

bool AD799x::write(uint8_t data) {
  port.beginTransmission(ADDRESS);
  port.write(data);
  port.endTransmission();
  return true;
}

bool AD799x::writeConfig(uint8_t Lchannels, bool vref) {
  channels=Lchannels;
  nChannels=0;
  for(int i=0;i<4;i++) if(((channels << i) & 0x01)>0) nChannels++;
  return write((channels<<4) | ((vref?1:0)<<3) | 0x0);
}

static inline uint16_t swap_endian16(uint16_t data) {
  return ((data & 0xFF)<<8) | ((data & 0xFF00) >> 8);
}

bool AD799x::read(uint16_t data[]) {
  if(!read((uint8_t*)data,nChannels*2)) return false;
  for(int i=0;i<nChannels;i++) data[i]=swap_endian16(data[i]);
  return true;
}



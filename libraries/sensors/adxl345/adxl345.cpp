#include "adxl345.h"

uint8_t ADXL345::whoami() {
  char buf[2];
  s->rx_block(p0,0x80,buf,2);
  return buf[1];
}

bool ADXL345::begin() {
  s->claim_cs(p0);
  s->tx_block(p0,"\x2D\x00",2);
  s->tx_block(p0,"\x2D\x10",2);
  s->tx_block(p0,"\x2D\x08",2);
//DATA_FORMAT - Register 0x31
//D7 - SELF_TEST  0=off
//D6 - SPI        0=4 wire
//D5 - INT_INVERT 0=direct
//D4 - reserved   0
//D3 - FULL_RES   1=13bit for 16g
//D2 - JUSTIFY    0=right justified (normal)
//D1-D0 - RANGE   11=+-16g
  s->tx_block(p0,"\x31\x0B",2);
  return true;
}

bool ADXL345::fillConfig(Packet& ccsds) {
  char buf[2];
  s->rx_block(p0,0x80 | 0x31,buf,2);//Read (0x80) single byte at address 0x31
  if(!ccsds.fill(buf+1,1))    return false; 
  return true;
}

void ADXL345::read(int16_t& x, int16_t& y, int16_t& z) {
  char buf[7];

  s->rx_block(p0,0xF2,buf,7);
  x=((int16_t)buf[2])<<8 | ((int16_t)buf[1]);
  y=((int16_t)buf[4])<<8 | ((int16_t)buf[3]);
  z=((int16_t)buf[6])<<8 | ((int16_t)buf[5]);
}



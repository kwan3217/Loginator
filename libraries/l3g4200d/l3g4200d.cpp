#include "l3g4200d.h"

uint8_t L3G4200D::read() {
  char buf[2];
  s->rx_block(p0,0x8F,buf,2);
  return buf[1];
}

uint8_t L3G4200D::begin() {
  s->claim_cs(p0);
  char buf[]="\x60\x0F\x00\x08\x30\x00";
  s->tx_block(p0,buf,6);
  return 1;
}

void L3G4200D::read(int16_t& x, int16_t& y, int16_t& z, uint8_t& t, uint8_t& status) {
  char buf[9];

  s->rx_block(p0,0xC0|0x26,buf,9);
  t=buf[1];
  status=buf[2];
  x=((int16_t)buf[4])<<8 | ((int16_t)buf[3]);
  y=((int16_t)buf[6])<<8 | ((int16_t)buf[5]);
  z=((int16_t)buf[8])<<8 | ((int16_t)buf[7]);
}



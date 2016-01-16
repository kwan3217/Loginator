#include "l3g4200d.h"

uint8_t L3G4200D::whoami() {
  char buf[2];
  s->rx_block(p0,0x8F,buf,2);
  return buf[1];
}

uint8_t L3G4200D::begin(char sens, char odr, char bw) {
  s->claim_cs(p0);
  char buf[6];
  buf[0]=0x60; //Write (0x00) multiple bytes (0x40) starting at address 0x20 
  buf[1]=(odr & 0x03)<<6 | (bw & 0x03) << 4 | 0x0F; //Register 0x20 - Control 1 
                                                    //Set output data rate, LPF bandwidth, and 
                                                    //turn on part and activate all 3 gyros
  buf[2]=0x00; //Register 0x21 - Control 2 - high-pass filter settings (not used)
  buf[3]=0x08; //Register 0x22 - Control 3 - Interrupt control - turn on data ready interrupt
  buf[4]=sens<<4; //Register 0x23 - Control 4 - sensitivity among other things
  buf[5]=0x00; //Register 0x24 - Control 5 - filter and interrupt controls (use un-HPF data)
  s->tx_block(p0,buf,6);
  return 1;
}

bool L3G4200D::fillConfig(Packet& packet) {
  char buf[6];
  s->rx_block(p0,0x80 | 0x40 | 0x20,buf,6);//Read (0x80) multiple bytes (0x40) starting at address 0x20
  #include "write_packet_gyrocfg.INC"
  return true;
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



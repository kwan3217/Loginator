/*#include "l3g4200d.h"
#include "sim.h"

uint8_t L3G4200D::whoami() {
  return 0xD3; 
}

static char buf[6];
static fp sens;

uint8_t L3G4200D::begin(char fs, char odr, char bw) {
  fp FS=((fp)(250 << fs));
  double sens=FS/360.0; //rotations per second full scale
  sens*=2*PI;   //radians per second full scale
  sens/=32768;  //radians per second per DN
  buf[0]=0x60; //Write (0x00) multiple bytes (0x40) starting at address 0x20 
  buf[1]=(odr & 0x03)<<6 | (bw & 0x03) << 4 | 0x0F; //Register 0x20 - Control 1 
                                                    //Set output data rate, LPF bandwidth, and 
                                                    //turn on part and activate all 3 gyros
  buf[2]=0x00; //Register 0x21 - Control 2 - high-pass filter settings (not used)
  buf[3]=0x08; //Register 0x22 - Control 3 - Interrupt control - turn on data ready interrupt
  buf[4]=fs<<4; //Register 0x23 - Control 4 - sensitivity among other things
  buf[5]=0x00; //Register 0x24 - Control 5 - filter and interrupt controls (use un-HPF data)
  return 1;
}

bool L3G4200D::fillConfig(Packet& ccsds) {
  #include "write_packet_gyrocfg.inc"
  return true;
}

void L3G4200D::read(int16_t& x, int16_t& y, int16_t& z, uint8_t& t, uint8_t& status) {
  double actualX=-state->xRate/sens;
  x=(int16_t)actualX; //Convert from rad/s to DN
  double actualY=-state->yRate/sens;
  y=(int16_t)actualY; //Convert from rad/s to DN
  double actualZ=-state->zRate/sens;
  z=(int16_t)actualZ; //Convert from rad/s to DN
}

*/

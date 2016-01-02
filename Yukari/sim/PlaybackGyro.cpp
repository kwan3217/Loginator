#include "PlaybackGyro.h"

void PlaybackGyro::setFromPacket(int16_t x, int16_t y, int16_t z) {
  dprintf(SIMGYRO,"Measurement: x=%d, y=%d, z=%d\n",x,y,z);
  reg[0x28]=uint8_t( x       & 0xFF);
  reg[0x29]=uint8_t((x >> 8) & 0xFF);
  reg[0x2A]=uint8_t( y       & 0xFF);
  reg[0x2B]=uint8_t((y >> 8) & 0xFF);
  reg[0x2C]=uint8_t( z       & 0xFF);
  reg[0x2D]=uint8_t((z >> 8) & 0xFF);
}

//Slave->Master (read) transfers. This will always happen last, so do the auto-increment here.
uint8_t PlaybackGyro::transferMISO() {
  uint8_t result=0xFF;
  if(getAddr) {

  } else {
    if(read) {
      dprintf(SIMGYRO,"Reading register 0x%02x, value=%d (0x%02x)\n",addr,value);
      result=reg[addr];
    }
    if(multi) {
      addr++;
      dprintf(SIMGYRO,"Auto-increment register address, now addressing register 0x%02x\n",addr);
    }
  }
  return result;
}

//Master->Slave (address or write) transfers
void PlaybackGyro::transferMOSI(uint8_t value) {
  if(getAddr) {
    getAddr=false;
    addr=value & ((1<<6)-1);
    read=(value & 1<<6)!=0;
    multi=(value & 1<<7)!=0;
    dprintf(SIMGYRO,"Addressing: Received 0x%02x, Read=%d, Multi=%d, Addr=%d, sending 0x%02x\n",value,read,multi,addr,result);
  } else {
    if(!read) {
      dprintf(SIMGYRO,"Writing register 0x%02x, value=%d (0x%02x)\n",addr,value);
      reg[addr]=value;
    }
  }
}



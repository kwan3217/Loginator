#include "PlaybackHmc.h"

uint8_t PlaybackHmc::readByte(bool ack) {
  //Ack will be false on last byte, true on others
  uint8_t result=reg[addr];
  dprintf(SIMHMC,"Reading %d (0x%02x) from register %d (0x%02x) on HMC5883\n",result,result,addr,addr);
  addr++;
  return result;
}

void PlaybackHmc::writeByte(uint8_t write, bool& ack) {
  ack=true;
  if(getAddr) {
	addr=write;
	getAddr=false;
    dprintf(SIMHMC,"Addressing register %d (0x%02x) on HMC5883\n",write,write);
  } else {
    dprintf(SIMHMC,"Writing %d (0x%02x) to register %d (0x%02x) on HMC5883\n",write,write,addr,addr);
    addr++;
  }
}


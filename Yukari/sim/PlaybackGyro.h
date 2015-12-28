#ifndef PlaybackGyro_h
#define PlaybackGyro_h

#include "sim.h"

class PlaybackGyro: public SimSpiSlave {
private:
  uint8_t reg[0x38+1]; //Highest numbered register, plus one for register 0
  bool getAddr=false;
  bool read=false;
  bool multi=false;
  int addr;
public:
  PlaybackGyro() {reg[0x0F]=0b1101'0011;};
  void setFromPacket(int16_t x, int16_t y, int16_t z);
  virtual void csOut (int value) override {getAddr=true;dprintf(SIMGYRO,"csOut=%d (%s)\n",value,value==0?"selected":"deselected");};
  virtual int  csIn  (         ) override {dprintf(SIMGYRO,"csIn=%d\n",1);return 1;};
  virtual void csMode(bool out ) override {dprintf(SIMGYRO,"csMode=%s\n",out?"out":"in");};
  virtual uint8_t transfer(uint8_t value) override;
};

#endif

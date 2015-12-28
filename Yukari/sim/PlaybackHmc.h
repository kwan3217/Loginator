#ifndef PlaybackHmc_h
#define PlaybackHmc_h

#include "sim.h"

class PlaybackHmc: public SimI2cSlave {
private:
  bool getAddr=false;
  int addr;
  uint8_t reg[13];
public:
  virtual void start() override {getAddr=true;dprintf(SIMHMC,"HMC5883 received start\n");};
  virtual void stop() override {dprintf(SIMHMC,"HMC5883 received stop\n");};
  virtual void repeatStart() override {getAddr=true;dprintf(SIMHMC,"HMC5883 received repeated start\n");};
  virtual uint8_t readByte(bool ack) override;
  virtual void writeByte(uint8_t write, bool& ack) override;
};

#endif

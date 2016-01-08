#ifndef HMCSENSOR_H
#define HMCSENSOR_H

#include <inttypes.h>
#include "Wire.h"
#include "packet.h"

template<class P, class W>
class HMC5883 {
  private:
    static const int ADDRESS=0x1E;  // I2C address of HMC5883L

    W& port; //Make this a reference because the part is hard-wired to a port and can't be changed at runtime
    int8_t read(uint8_t address);
    int16_t read16(uint8_t address);
  public:
    HMC5883(W& Lport):port(Lport) {};
    void begin();
    void whoami(char* id);
    void read(int16_t& x, int16_t& y, int16_t& z);
    bool fillConfig(P& packet);
};

#include "hmc5883.inc"

#endif

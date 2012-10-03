#ifndef HMCSENSOR_H
#define HMCSENSOR_H

#include <inttypes.h>
#include "Wire.h"

class HMC5883 {
  private:
    static const int HMC5883_ADDRESS=0x1E;  // I2C address of HMC5883L

    TwoWire *port;
    int8_t read(uint8_t address);
    int16_t read_int16(uint8_t address);
  public:
    HMC5883(TwoWire *Lport):port(Lport) {};
    void begin();
    void read(char* id);
    void read(int16_t& x, int16_t& y, int16_t& z);
};

#endif

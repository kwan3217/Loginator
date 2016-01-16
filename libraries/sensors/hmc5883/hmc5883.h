#ifndef HMCSENSOR_H
#define HMCSENSOR_H

#include <inttypes.h>
#include "Wire.h"
#include "packet.h"

class HMC5883 {
  private:
    static const int ADDRESS=0x1E;  // I2C address of HMC5883L

    TwoWire* port; //Make this a pointer so that the port can be changed at runtime
    int8_t read(uint8_t address);
    int16_t read16(uint8_t address);
  public:
    HMC5883(TwoWire* Lport):port(Lport) {};
    void begin();
    void begin(TwoWire* Lport) {port=Lport;begin();};
    void whoami(char* id);
    void read(int16_t& x, int16_t& y, int16_t& z);
    bool fillConfig(Packet& ccsds);
};

#endif

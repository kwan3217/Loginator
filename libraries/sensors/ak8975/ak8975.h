#ifndef ak8975_h
#define ak8975_h

#include <inttypes.h>
#include "Wire.h"
#include "packet.h"

class AK8975 {
  private:
    int partAddress;  // I2C address of AK8975

    TwoWire* port; //Make this a pointer so that the port can be changed at runtime
    int8_t read(uint8_t address);
    int16_t read16(uint8_t address);
    void startMeasurement();
    void startMeasurementCore();
    void finishMeasurementCore();
    bool checkMeasurement();
    int16_t x,y,z;
    uint8_t st1,st2;
    uint32_t TC0;
    bool start, ready;
    static const int delayMS=9; ///< Wait this many milliseconds between starting and reading the sensor
  public:
    AK8975(TwoWire* Lport,int LpartAddress=0x0C):port(Lport) {partAddress=LpartAddress;};
    void begin();
    void begin(TwoWire* Lport) {port=Lport;begin();};
    uint8_t whoami();
    void read(int16_t& Lx, int16_t& Ly, int16_t& Lz, uint8_t& Lst1, uint8_t& Lst2) {Lx=x;Ly=y;Lz=z;Lst1=st1;Lst2=st2;};
    bool fillConfig(Packet& ccsds);
    void blockMeasurement();
    bool noblockMeasurement();
};

#endif

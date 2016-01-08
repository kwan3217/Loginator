#ifndef BMPSENSOR_H
#define BMPSENSOR_H

#include <inttypes.h>
#include "Wire.h"
#include "Serial.h"
#include "packet.h"

/**
  Class to provide access to a FAT formatted filesystem by cluster and sector.
  Also provides services to read and write the file allocation table.

@tparam Pk Packet class used to write debug information to a packet
@tparam Pr Print class used to write debug information to a Print object (passed through to members c and de)
@tparam W TwoWire class used to access the sensor
*/
template<class Pk, class Pr, class W>
class BMP180 {
  private:
    // Calibration values
    int16_t  ac1;
    int16_t  ac2; 
    int16_t  ac3; 
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
    int16_t  b1; 
    int16_t  b2;
    int16_t  mb;
    int16_t  mc;
    int16_t  md;
    // b5 is calculated in getTemperature(...), this variable is also used in getPressure(...)
    // so ...Temperature(...) must be called before ...Pressure(...).
    int32_t b5; 
    W& port; //Reference since the part is hard-wired to a pin and can't be changed at runtime
    void print(const char* tag, int arg) {
      if(!ouf) return;
      ouf->print(tag);
      ouf->println(arg,DEC);
    };
    volatile int32_t UT,UP;
    volatile int32_t UTshadow, UPshadow;
    volatile bool start,tempReady,presReady;
    volatile uint32_t TC0;
    int8_t read(uint8_t address);
    int16_t read_int16(uint8_t address);
    int16_t getTemperature(uint16_t ut);
    int32_t getPressure(uint32_t up);
    void finishTemp();
    void finishPres();
    void finishTempCore();
    void finishPresCore();
    void startMeasurementCore();
    void startMeasurement();
    bool checkMeasurement();
    static const unsigned char ADDRESS=0x77;  // I2C address of BMP085
    unsigned char OSS;  // Oversampling Setting
    unsigned int timer_ch;
    static const unsigned int tempDelayMS=5; ///<It takes this many milliseconds after measurement kickoff for temperature measurement to become valid
                 unsigned int presDelayMS;   ///<It takes this many more milliseconds after temperature is done for pressure measurement to become valid
  public:
    Pr* ouf;
    bool begin(unsigned int Ltimer_ch);
    BMP180(W& Lport):port(Lport),OSS(3),presDelayMS(2+(3<<OSS)) {};
    bool readCalibration();
    void printCalibration(Pr& Louf);
    void fillCalibration(Pk& pkt);
    unsigned char getOSS() {return OSS;};
    bool setOSS(unsigned char Loss) {if(start) return false;OSS=Loss;presDelayMS=2+(3<<OSS);return true;};
    int getTemperatureRaw() {return UTshadow;};
    int getPressureRaw() {return UPshadow;};
    int16_t getTemperature() {return getTemperature(UTshadow);};
    int32_t getPressure() {return getPressure(UPshadow);};
    void blockMeasurement();
    bool noblockMeasurement();
    unsigned char whoami() {return read(0xD0);};

};

#include "bmp180.inc"

#endif

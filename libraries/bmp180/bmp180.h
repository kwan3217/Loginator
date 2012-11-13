#ifndef BMPSENSOR_H
#define BMPSENSOR_H

#include <inttypes.h>
#include "Wire.h"
#include "Serial.h"
#include "packet.h"

class BMP180 {
  private:
    // Calibration values
    short  ac1;
    short  ac2; 
    short  ac3; 
    unsigned short ac4;
    unsigned short ac5;
    unsigned short ac6;
    short  b1; 
    short  b2;
    short  mb;
    short  mc;
    short  md;
    // b5 is calculated in getTemperature(...), this variable is also used in getPressure(...)
    // so ...Temperature(...) must be called before ...Pressure(...).
    int  b5; 
    TwoWire &port;
    void print(char* tag, int arg) {
      if(!ouf) return;
      ouf->print(tag);
      ouf->println(arg,DEC);
    };
    volatile int UT,UP;
    volatile bool start;
    int8_t read(uint8_t address);
    int16_t read_int16(uint8_t address);
    int16_t getTemperature(uint16_t ut);
    int32_t getPressure(uint32_t up);
    static void finishTempTask(void*);
    static void finishPresTask(void*);
    void finishTemp();
    void finishPres();
    void finishTempCore();
    void finishPresCore();
    void startMeasurementCore();
    static const unsigned char ADDRESS=0x77;  // I2C address of BMP085
    unsigned char OSS;  // Oversampling Setting
  public:
    Stream *ouf;
    BMP180(TwoWire &Lport);
    volatile bool ready;
    void begin();
    void printCalibration(Stream *Louf);
    void fillCalibration(Packet& pkt);
    unsigned char getOSS() {return OSS;};
    bool setOSS(unsigned char Loss) {if(start) return false;OSS=Loss;return true;};
    void startMeasurement();
    int getTemperatureRaw() {return UT;};
    int getPressureRaw() {return UP;};
    int16_t getTemperature() {return getTemperature(UT);};
    int32_t getPressure() {return getPressure(UP);};
    int readMeasurement(int& t, int& p);
};

#endif

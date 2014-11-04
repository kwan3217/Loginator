#ifndef BMPSENSOR_H
#define BMPSENSOR_H

#include <inttypes.h>
#include "Wire.h"
#include "Serial.h"
#include "packet.h"

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
    TwoWire &port;
    void print(const char* tag, int arg) {
      if(!ouf) return;
      ouf->print(tag);
      ouf->println(arg,DEC);
    };
    volatile int32_t UT,UP;
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
    unsigned int timer_ch;
  public:
    Stream *ouf;
    BMP180(TwoWire &Lport);
    volatile bool ready;
    bool begin(unsigned int Ltimer_ch);
    bool readCalibration();
    void printCalibration(Stream *Louf);
    void fillCalibration(Packet& pkt);
    unsigned char getOSS() {return OSS;};
    bool setOSS(unsigned char Loss) {if(start) return false;OSS=Loss;return true;};
    void startMeasurement();
    int getTemperatureRaw() {return UT;};
    int getPressureRaw() {return UP;};
    int16_t getTemperature() {return getTemperature(UT);};
    int32_t getPressure() {return getPressure(UP);};
    void takeMeasurement();
    unsigned char whoami() {return read(0xD0);};

};

#endif

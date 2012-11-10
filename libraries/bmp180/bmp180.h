#ifndef BMPSENSOR_H
#define BMPSENSOR_H

#include <inttypes.h>
#include "Wire.h"
#include "Serial.h"

#define BMP180_ADDRESS 0x77  // I2C address of BMP085

const unsigned char OSS = 3;  // Oversampling Setting

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
    int32_t  b5; 
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
  public:
    Stream *ouf;
    BMP180(TwoWire &Lport);
    volatile bool ready;
    void begin();
    void printCalibration(Stream *Louf);
    void startMeasurement();
    int getTemperatureRaw() {return UT;};
    int getPressureRaw() {return UP;};
    int16_t getTemperature() {return getTemperature(UT);};
    int32_t getPressure() {return getPressure(UP);};
    int readMeasurement(int& t, int& p);
};

#endif

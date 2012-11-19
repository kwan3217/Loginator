//"I'll call you... MPU!"
//"MPU... What's that?"
//"It's like CPU, only neater!"
//--Edward Wong Hau Pepelu Tivruski IV and the CPU of the D-135 Artificial Satellite
//  Cowboy Bebop Session 9, "Jamming with Edward"

#ifndef MPU60X0_H
#define MPU60X0_H

#include <inttypes.h>
#include "Wire.h"

class MPU60x0 {
public:
  virtual unsigned char read(unsigned char addr)=0;
  virtual void write(unsigned char addr, unsigned char data)=0;
  virtual int16_t read16(unsigned char addr) {return ((int16_t)read(addr))<<8 | ((int16_t)read(addr+1));};
  unsigned char whoami() {return read(0x75);};
  virtual bool read(int16_t& ax, int16_t& ay, int16_t& az, int16_t& gx, int16_t& gy, int16_t& gz, int16_t& t);
  virtual bool begin(); //Do anything necessary to init the part. Bus is available at this point.
};

//I2C version of MPU60x0
class MPU6050: public MPU60x0 {
private:
  static const int ADDRESS=0x68;  // 7-bit I2C address of MPU60x0
  TwoWire& port;
  int A0;
public:
  virtual unsigned char read(unsigned char addr);
  virtual void write(unsigned char addr, unsigned char data);
  virtual int16_t read16(unsigned char addr);
  MPU6050(TwoWire& Lport,int LA0):port(Lport),A0(LA0) {};
  virtual bool read(int16_t& ax, int16_t& ay, int16_t& az, int16_t& gx, int16_t& gy, int16_t& gz, int16_t& t);
};

//SPI version of MPU60x0. The 6000 supports both I2C and SPI,
//but use the 6050 class to access it as I2C even if it is
//a 6000.
class MPU6000:public MPU60x0 {

};

#endif

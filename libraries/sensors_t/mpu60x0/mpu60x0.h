#ifndef MPU60X0_H
#define MPU60X0_H

#include <inttypes.h>
#include "Wire.h"
#include "packet.h"

/** Driver for Motion Processor Unit 6000 series. This part contains an integrated 3-axis accelerometer and 3-axis gyroscope. Also handles the 6050 embedded in an 9150 part. 
"I'll call you... MPU!"
"MPU... What's that?"
"It's like CPU, only neater!"
--Edward Wong Hau Pepelu Tivruski IV and the CPU of the D-135 Artificial Satellite
  Cowboy Bebop Session 9, "Jamming with Edward"

@tparam T CRTP derived class
*/

template <class T>
class MPU60x0 {
private:
//  virtual unsigned char read(unsigned char addr)=0;
//  virtual void write(unsigned char addr, unsigned char data)=0;
/*  virtual*/int16_t read16(unsigned char addr) {return ((int16_t)read(addr))<<8 | ((int16_t)read(addr+1));};
public:
  MPU60x0() {};
  unsigned char whoami() {return read(0x75);};
/*  virtual*/ bool read(int16_t& ax, int16_t& ay, int16_t& az, int16_t& gx, int16_t& gy, int16_t& gz, int16_t& t);
  bool begin(uint8_t gyro_scale, uint8_t acc_scale, uint8_t bandwidth=0x03, uint8_t sample_rate=0); ///<Do anything necessary to init the part. Bus is available at this point.
};

/** I2C version of MPU60x0

@tparam P Packet class used to write to a packet
@tparam W TwoWire class used to access the sensor
*/
template<class P, class W>
class MPU6050: public MPU60x0<MPU6050<P,W>> {
private:
  W& port;
  uint8_t ADDRESS;  ///< I2C address of part
  static const char addr_msb=0x68; ///<ORed with low bit of a0 to get actual address
  int A0;
  virtual unsigned char read(unsigned char addr);
  virtual void write(unsigned char addr, unsigned char data);
  virtual int16_t read16(unsigned char addr);
public:
  MPU6050(W& Lport,int LA0):port(Lport),ADDRESS(addr_msb+LA0) {};
  virtual bool read(int16_t& ax, int16_t& ay, int16_t& az, int16_t& gx, int16_t& gy, int16_t& gz, int16_t& t);
  bool fillConfig(P& packet);
};

/**SPI version of MPU60x0. The 6000 supports both I2C and SPI,
but use the 6050 class to access it as I2C even if it is
a 6000. */
class MPU6000:public MPU60x0<MPU6000> {

};

#include "mpu60x0.inc"

#endif

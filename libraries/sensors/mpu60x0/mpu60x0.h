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
*/
class MPU60x0 {
private:
  virtual unsigned char read(unsigned char addr)=0;
  virtual void write(unsigned char addr, unsigned char data)=0;
  virtual int16_t read16(unsigned char addr) {return ((int16_t)read(addr))<<8 | ((int16_t)read(addr+1));};
public:
  MPU60x0() {};
  unsigned char whoami() {return read(0x75);};
  virtual bool read(int16_t& ax, int16_t& ay, int16_t& az, int16_t& gx, int16_t& gy, int16_t& gz, int16_t& t);
  bool begin(uint8_t gyro_scale, uint8_t acc_scale, uint8_t bandwidth=0, uint8_t sample_rate=0); ///<Do anything necessary to init the part. Bus is available at this point.
};

/** I2C version of MPU60x0
*/
class MPU6050: public MPU60x0 {
private:
  TwoWire* port;
  uint8_t ADDRESS;  ///< I2C address of part
  static const char addr_msb=0x68; ///<ORed with low bit of a0 to get actual address
  virtual unsigned char read(unsigned char addr);
  virtual void write(unsigned char addr, unsigned char data);
  virtual int16_t read16(unsigned char addr);
  static int i_hwDesc;
public:
  /** Init part based on hardware description */
  MPU6050();
  /** Init part directly
    @param Lport I2C port to use with this part
    @param LA0 least significant bit of part address */
  MPU6050(TwoWire& Lport, int LA0):port(&Lport),ADDRESS(addr_msb | (LA0& 0x01)) {};
  virtual bool read(int16_t& ax, int16_t& ay, int16_t& az, int16_t& gx, int16_t& gy, int16_t& gz, int16_t& t);
  bool fillConfig(Packet& ccsds);
};

/**SPI version of MPU60x0. The 6000 supports both I2C and SPI,
but use the 6050 class to access it as I2C even if it is
a 6000. */
class MPU6000:public MPU60x0 {

};

#endif

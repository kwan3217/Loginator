#include "mpu60x0.h"

bool MPU60x0::begin() {
  //Do anything necessary to init the part. Bus is available at this point.
  //Set External sync (none, 0x00) and Low Pass Filter (~40Hz bandwidth, 0x03)
  write(0x1A,0x00<<3 | 0x03<<0);
  //Set the gyroscope scale, (250deg/s, 0)
  write(0x1B,0 << 7 | 0 << 6 | 0 << 5 | 0 << 3);
  //Set the accelerometer scale (+-2g, 0)
  write(0x1C,0 << 7 | 0 << 6 | 0 << 5 | 0 << 3);
  //Set up the clock source (x gyro, 1)
  write(0x6B,0 << 7 | 0 << 6 | 0 << 5 | 0x01 << 0);
}

// Read 1 byte from the sensor at 'address'
unsigned char MPU6050::read(uint8_t address) {
  port.beginTransmission(ADDRESS+A0);
  port.send(address);
  port.endTransmission();
  
  port.requestFrom(ADDRESS+A0, 1);
  return port.receive();
}

// Read a 16-bit integer in big-endian format from the sensor
// First byte will be from 'address'
// Second byte will be from 'address'+1
int16_t MPU6050::read16(uint8_t address) {
  int16_t msb, lsb;
  
  port.beginTransmission(ADDRESS+A0);
  port.send(address);
  port.endTransmission();
  
  port.requestFrom(ADDRESS+A0, 2);
  msb = port.receive();
  lsb = port.receive();
  
  return msb<<8 | lsb;
}

bool MPU6050::write(uint8_t address, uint8_t data) {
  port.beginTransmission(ADDRESS);
  port.send(address);
  port.send(data);
  port.endTransmission();
}

bool MPU60x0::read(int16_t& ax, int16_t& ay, int16_t& az, int16_t& gx, int16_t& gy, int16_t& gz, int16_t& t) {
  ax=read16(0x3B);
  ay=read16(0x3D);
  az=read16(0x3F);
  t =read16(0x41);
  gx=read16(0x43);
  gy=read16(0x45);
  gz=read16(0x47);
  return true;
}

bool MPU6050::read(int16_t& ax, int16_t& ay, int16_t& az, int16_t& gx, int16_t& gy, int16_t& gz, int16_t& t) {
  int msb, lsb;
  port.beginTransmission(ADDRESS+A0);
  port.send(0x3B);
  port.endTransmission();
  
  port.requestFrom(ADDRESS+A0, 14);
  msb = port.receive();
  lsb = port.receive();
  ax= msb<<8 | lsb;
  msb = port.receive();
  lsb = port.receive();
  ay= msb<<8 | lsb;
  msb = port.receive();
  lsb = port.receive();
  az= msb<<8 | lsb;
  msb = port.receive();
  lsb = port.receive();
  t= msb<<8 | lsb;
  msb = port.receive();
  lsb = port.receive();
  gx= msb<<8 | lsb;
  msb = port.receive();
  lsb = port.receive();
  gy= msb<<8 | lsb;
  msb = port.receive();
  lsb = port.receive();
  gz= msb<<8 | lsb;
  return true;
}




#include "hmc5883.h"

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void HMC5883::begin() {
  //Set it to single-shot mode
  port.beginTransmission(ADDRESS);
  port.send(0x00);  //Address the mode register
  port.send(3<<5 | 6<<2 | 0<<0); //MA - 0b11  = 8 samples average
                                 //DO - 0b110 = 75Hz measurment rate
                                 //MS - 0b00  = normal measurement mode
  port.send(1<<5 );              //GN - 0b001 = +-1.3Ga (1090DN/Ga)
  port.send(0<<0 );              //MD - 0b00  = continuous measurement mode
  port.endTransmission();
}

// Read 1 byte from the BMP085 at 'address'
int8_t HMC5883::read(uint8_t address) {
  port.beginTransmission(ADDRESS);
  port.send(address);
  port.endTransmission();
  
  port.requestFrom(ADDRESS, 1);
  return port.receive();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int16_t HMC5883::read_int16(uint8_t address) {
  uint8_t msb, lsb;
  
  port.beginTransmission(ADDRESS);
  port.send(address);
  port.endTransmission();
  
  port.requestFrom(ADDRESS, 2);
  msb = port.receive();
  lsb = port.receive();
  
  return (int16_t) msb<<8 | lsb;
}

void HMC5883::read(int16_t& x, int16_t& y, int16_t& z) {
  x=read_int16(3);
  y=read_int16(5);
  z=read_int16(7);
}

void HMC5883::read(char* id) {
  id[0]=read(10);
  id[1]=read(11);
  id[2]=read(12);
  id[3]=0;
}




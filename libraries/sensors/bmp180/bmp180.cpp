#include "bmp180.h"
#include "Time.h"

#undef BMP180_DEBUG

BMP180::BMP180():OSS(3),presDelayMS(2+(3<<OSS)) {
  int i=1;
  while(i<N_ID && HW_ID_PART_TYPE(i)!=6) i++;
  if(i==N_ID) return;
  port=WireA[HW_ID_PORT_NUM(i)];
}

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
bool BMP180::begin(unsigned int Ltimer_ch) {
  timer_ch=Ltimer_ch;
  port->beginTransmission(ADDRESS);
  port->write(0xE0);
  port->write(0x6B);
  port->endTransmission();
  delay(25);
  return readCalibration();
}

bool BMP180::readCalibration() {
  ac1 = read_int16(0xAA);
  ac2 = read_int16(0xAC);
  ac3 = read_int16(0xAE);
  ac4 = read_int16(0xB0);
  ac5 = read_int16(0xB2);
  ac6 = read_int16(0xB4);
  b1 = read_int16(0xB6);
  b2 = read_int16(0xB8);
  mb = read_int16(0xBA);
  mc = read_int16(0xBC);
  md = read_int16(0xBE);
  if (ac1==0 || ac1==0xFFF) return false;
  if (ac2==0 || ac2==0xFFF || ac2==ac1) return false;
  return true;
}

void BMP180::printCalibration(Stream *Louf) {
  ouf=Louf;
  ouf->println("BMP180 calibration constants");
  ouf->print("ac1: ");
  ouf->println(ac1,DEC);
  ouf->print("ac2: ");
  ouf->println(ac2,DEC);
  ouf->print("ac3: ");
  ouf->println(ac3,DEC);
  ouf->print("ac4: ");
  ouf->println((unsigned int)ac4,DEC);
  ouf->print("ac5: ");
  ouf->println((unsigned int)ac5,DEC);
  ouf->print("ac6: ");
  ouf->println((unsigned int)ac6,DEC);
  ouf->print("b1: ");
  ouf->println(b1,DEC);
  ouf->print("b2: ");
  ouf->println(b2,DEC);
  ouf->print("mb: ");
  ouf->println(mb,DEC);
  ouf->print("mc: ");
  ouf->println(mc,DEC);
  ouf->print("md: ");
  ouf->println(md,DEC);
}

void BMP180::fillCalibration(Packet& pkt) {
  pkt.fill16(ac1);
  pkt.fill16(ac2);
  pkt.fill16(ac3);
  pkt.fill16(ac4);
  pkt.fill16(ac5);
  pkt.fill16(ac6);
  pkt.fill16(b1);
  pkt.fill16(b2);
  pkt.fill16(mb);
  pkt.fill16(mc);
  pkt.fill16(md);
}  
 
// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
int16_t BMP180::getTemperature(uint16_t ut) {
  int32_t x1, x2;
  int16_t result;
  
  print("x1t: ",x1 = (((int32_t)ut - (int32_t)ac6)*(int32_t)ac5) >> 15);
  print("x2t: ",x2 = ((int32_t)mc << 11)/(x1 + md));
  print("b5:  ",b5 = x1 + x2);
  print("getT:",result=((b5 + 8)>>4));

  return result;  
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so getTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
int32_t BMP180::getPressure(uint32_t up) {
  int32_t x1, x2, x3, b3, b6, p;
  uint32_t b4, b7;
  
  print("b6: ",b6 = b5 - 4000);
  // Calculate B3
  print("x1a: ",x1 = (b2 * ((b6 * b6)>>12))>>11);
  print("x2a: ",x2 = (ac2 * b6)>>11);
  print("x3a: ",x3 = x1 + x2);
  print("b3:  ",b3 = (((((int32_t)ac1)*4 + x3)<<OSS) + 2)>>2);
  
  // Calculate B4
  print("x1b: ",x1 = (ac3 * b6)>>13);
  print("x2b: ",x2 = (b1 * ((b6 * b6)>>12))>>16);
  print("x3b: ",x3 = ((x1 + x2) + 2)>>2);
  print("b4:  ",b4 = (ac4 * (uint32_t)(x3 + 32768))>>15);
  
  print("b7:  ",b7 = ((uint32_t)(up - b3) * (50000>>OSS)));
  if (b7 < 0x80000000)
    print("pa:   ",p = (b7<<1)/b4);
  else
    print("pb:   ",p = (b7/b4)<<1);
    
  print("x1c: ",x1= (p>>8) * (p>>8));
  print("x1d: ",x1 = (x1 * 3038)>>16);
  print("x2:  ",x2 = (-7357 * p)>>16);
  print("pc:  ",p += (x1 + x2 + 3791)>>4);
  
  return p;
}

// Read 1 byte from the BMP085 at 'address'
int8_t BMP180::read(uint8_t address) {
  port->beginTransmission(ADDRESS);
  port->write(address);
  port->endTransmission();
  
  port->requestFrom(ADDRESS, 1);
  return (int8_t)port->read();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int16_t BMP180::read_int16(uint8_t address) {
  uint8_t msb, lsb;
  
  port->beginTransmission(ADDRESS);
  port->write(address);
  port->endTransmission();
  
  port->requestFrom(ADDRESS, 2);
  msb = port->read();
  lsb = port->read();
  
  return (int16_t) msb<<8 | lsb;
}

void BMP180::startMeasurementCore() {
  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  port->beginTransmission(ADDRESS);
  port->write(0xF4);
  port->write(0x2E);
  port->endTransmission();
}

void BMP180::startMeasurement() {
  if(start) return; //If a measurement is already started, keep doing that one
  startMeasurementCore();
  start=true;
  tempReady=false;
  presReady=false;
  TC0=TTC(0);
}

bool BMP180::checkMeasurement() {
  uint32_t dtms=msPassed(TC0);
  if(!start) {
    return true;
  } else if(!tempReady) {
    if(tempDelayMS<=dtms) {
      tempReady=true;
      TC0=TTC(0);
      finishTempCore();
      return false;
    } else {
      return false;
    }
  } else if(!presReady) {
    if(presDelayMS<dtms) {
      presReady=true;
      finishPresCore();
      presReady=true;
      start=false;
      UTshadow=UT;
      UPshadow=UP;
      return true;
    } else {
      return false;
    }
  }
}

//Manages start and checking of a measurement. Call this in a busy loop, it will return quickly if no measurement is available.
// @return If true, a new consistent temperature and pressure are availalble through getTemperature() and getPressure().
bool BMP180::noblockMeasurement() {
  if(!start) startMeasurement();
  return checkMeasurement();
}

void BMP180::finishTempCore() {
  // Read two bytes from registers 0xF6 and 0xF7
  UT = read_int16(0xF6);

  //Start pressure measurement 
  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  port->beginTransmission(ADDRESS);
  port->write(0xF4);
  port->write(0x34 | (OSS<<6));
  port->endTransmission();
}

void BMP180::finishPresCore() {
  uint8_t msb, lsb, xlsb;
    
  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  port->beginTransmission(ADDRESS);
  port->write(0xF6);
  port->endTransmission();
  port->requestFrom(ADDRESS, 3);
  
  msb = port->read();
  lsb = port->read();
  xlsb = port->read();
  
  UP = (((uint32_t) msb << 16) | ((uint32_t) lsb << 8) | (uint32_t) xlsb) >> (8-OSS);
}

void BMP180::blockMeasurement() {
  startMeasurementCore();
  delay(tempDelayMS);
  finishTempCore();
  delay(presDelayMS);
  finishPresCore();
  UTshadow=UT;
  UPshadow=UP;
}




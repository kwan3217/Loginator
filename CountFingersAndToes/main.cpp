#define ROCKETOMETER

//Check if all attached hardware properly responds. We do this by
//checking the part ID of the parts that support it, and by opening
//the partition on the microSD card. The AD7991 cannot be detected
//by this method.

#include <string.h>
#include "gpio.h"
#include "StateTwoWire.h"
#include "HardSPI.h"
#include "Serial.h"
#include "bmp180.h"
#include "hmc5883.h"
#include "mpu60x0.h"
#include "ad799x.h"
#include "Time.h"
#include "LPC214x.h"
#include "dump.h"
#include "packet.h"
#include "sdhc.h"
#include "Partition.h"
#include "cluster.h"
#include "direntry.h"
#include "file.h"
#include "FileCircular.h"

//int temperature, pressure;
//int temperatureRaw, pressureRaw;
int16_t bx,by,bz;    //compass (bfld)
uint16_t hx[4];      //HighAcc
int16_t max,may,maz; //MPU60x0 acc
int16_t mgx,mgy,mgz; //MPU60x0 gyro
int16_t mt;          //MPU60x0 temp
char buf[SDHC::BLOCK_SIZE]; //Use when you need some space to do a sd write

StateTwoWire Wire1(0);
SDHC sd(&SPI,15);
Partition p(sd);
Cluster fs(p);
File f(fs);
Hd sector(Serial);
BMP180 bmp180(Wire1);
HMC5883 hmc5883(Wire1);
MPU6050 mpu6050(Wire1,0);
AD799x ad799x(Wire1);
Base85 d(Serial);
FileCircular pktStore(f);
CCSDS ccsds(pktStore);
unsigned short pktseq[16];
bool timeToRead;
unsigned int lastTC,nextTC,interval;

void setup() {
  Serial.begin(38400);

  Serial.print("Device type:   0x");Serial.println(HW_TYPE,HEX,8);
  Serial.print("Device serial: 0x");Serial.println(HW_SERIAL,HEX,8);
  //Dump source package to serial
  int len=source_end-source_start;
  char* base=source_start;
  Serial.print("Source package length: ");
  Serial.println(len,DEC);
  Serial.print("Source package start: 0x");
  Serial.println((int)base,DEC,8);
  d.begin();
  while(len>0) {
    d.line(base,0,len>120?120:len);
    base+=120;
    len-=120;
  }
  d.end();

  Wire1.begin();
  SPI1.begin(1000000,1,1);

  bool worked=sd.begin();
  Serial.print("sd");    Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(sd.errnum);

  worked=p.begin(1);
  Serial.print("p");     Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(p.errnum);

  worked=fs.begin();  
  Serial.print("fs");    Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(fs.errnum);
  sector.begin();
  fs.print(Serial,sector);

  mpu6050.begin();
  Serial.print("MPU6050 identifier (should be 0x68): 0x");
  Serial.println(mpu6050.whoami(),HEX);

  hmc5883.begin();
  char HMCid[4];
  hmc5883.whoami(HMCid);
  Serial.print("HMC5883L identifier (should be 'H43'): ");
  Serial.println(HMCid);

  worked=ad799x.begin(0xB); //Turn on channels 0, 1, and 3
  Serial.print("ad799x");Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(0);

  worked=bmp180.begin();
  Serial.print("bmp180");Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(0);
  Serial.print("BMP180 identifier (should be 0x55): 0x");
  Serial.println(bmp180.whoami(),HEX);

  bmp180.printCalibration(&Serial);

}

void loop() {

}



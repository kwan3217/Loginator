#define ROCKETOMETER

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
uint16_t h[4];      //HighAcc
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
  Serial.begin(230400);
  //Dump code to serial port
  int len=source_end-source_start;
  char* base=source_start;
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
  Serial.print("sd");    Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(sd.errno);

  worked=p.begin(1);
  Serial.print("p");     Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(p.errno);

  worked=fs.begin();  
  Serial.print("fs");    Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(fs.errno);
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

  worked=ad799x.begin((1<<0) | (1<<1) | (1<<3)); //Turn on channels 0, 1, and 3
  Serial.print("ad799x");Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(0);

  worked=bmp180.begin();
  Serial.print("bmp180");Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(0);
  Serial.print("BMP180 identifier (should be 0x55): 0x");
  Serial.println(bmp180.whoami(),HEX);

  bmp180.printCalibration(&Serial);

  bmp180.ouf=0;
  ccsds.start(0x02);

  bmp180.fillCalibration(ccsds);

  ccsds.finish();

  Serial.println("t,tc,bx,by,bz,max,may,maz,mgx,mgy,mgz,mt,hx,hy,hz,T,P");
//  Serial.println("t,tc,Traw,Praw");

//  taskManager.schedule(100,0,sensorTimingTask,0); 
  lastTC=TTC(0);
  interval=(PCLK/1000)*10;
  nextTC=(lastTC+interval) % timerInterval;
}

void loop() {
  static int phase=0;
  phase++;
  unsigned int TC=TTC(0);
  mpu6050.read(max,may,maz,mgx,mgy,mgz,mt);
/*
  ccsds.start(0x06,pktseq,TC);
  ccsds.fill16(max);ccsds.fill16(may); ccsds.fill16(maz); ccsds.fill16(mgx); ccsds.fill16(mgy); ccsds.fill16(mgz); ccsds.fill16(mt);
  ccsds.finish();
*/
  if(0==(phase%10)) {
    //Only read the compass and HighAcc once every 10 times we read the 6DoF
    TC=TTC(0);
    hmc5883.read(bx,by,bz);
/*  
  ccsds.start(0x04,pktseq,TC);
    ccsds.fill16(bx);  ccsds.fill16(by);  ccsds.fill16(bz);
    ccsds.finish();
*/
    TC=TTC(0);
    ad799x.read(h);
    ad799x.format(h);
/*
    ccsds.start(0x0B,pktseq,TC);
    ccsds.fill((char*)hx,6);
    ccsds.finish();
*/
    if(200==phase) {
      TC=TTC(0);  
      bmp180.takeMeasurement();
      auto temperatureRaw=bmp180.getTemperatureRaw();
      auto pressureRaw=bmp180.getPressureRaw();
      auto temperature=bmp180.getTemperature();
      auto pressure=bmp180.getPressure();
      auto TC1=TTC(0);
/*
      ccsds.start(0x0A,pktseq,TC);
      ccsds.fill16(temperatureRaw);
      ccsds.fill32(pressureRaw);
      ccsds.fill16(temperature);
      ccsds.fill32(pressure);
      ccsds.fill32(TC1);
      ccsds.finish();
*/
      Serial.print(RTCHOUR,DEC,2);
      Serial.print(":");Serial.print(RTCMIN,DEC,2);
      Serial.print(":");Serial.print(TC/PCLK,DEC,2);
      Serial.print(".");Serial.print((TC%PCLK)/(PCLK/1000),DEC,3);
      Serial.print(",");Serial.print(TC,DEC,10);
      Serial.print(",");Serial.print(bx, DEC); 
      Serial.print(",");Serial.print(by, DEC); 
      Serial.print(",");Serial.print(bz, DEC); 
      Serial.print(",");Serial.print(max, DEC);
      Serial.print(",");Serial.print(may, DEC);
      Serial.print(",");Serial.print(maz, DEC);
      Serial.print(",");Serial.print(mgx, DEC);
      Serial.print(",");Serial.print(mgy, DEC);
      Serial.print(",");Serial.print(mgz, DEC);
      Serial.print(",");Serial.print(mt, DEC);
      Serial.print(",");Serial.print(h[0], DEC); 
      Serial.print(",");Serial.print(h[1], DEC); 
      Serial.print(",");Serial.print(h[3], DEC); 
      Serial.print(",");Serial.print(temperature/10, DEC);    
      Serial.print(".");Serial.print(temperature%10, DEC);
      Serial.print(",");Serial.print((unsigned int)pressure, DEC); 
      Serial.println();
      phase=0;
    }
  }
  TC=TTC(0);  
/*
  if(pktStore.drain()) {
    unsigned int TC1=TTC(0);
    ccsds.start(0x08,pktseq,TC);
    ccsds.fill32(TC1);
    ccsds.finish();
  }
  if(f.size()>=maxLogSize) {
    closeLog();
    openLog();
  }
*/
}



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

//Once the log becomes greater or equal to this length, cycle the log file
//This way we don't violate the FAT file size limit, and don't choke our processing
//program with data.
//Set to 1MiB so that we can test the feature
unsigned int maxLogSize=1024U*1024U*1U;

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
unsigned short pktseq[32];
bool timeToRead;
unsigned int lastTC,nextTC,interval;

void openLog() {
  static char fn[13];
  if(fn[0]!='r') strcpy(fn,"rkto0000.sds");
  static int i=0;
  Serial.println(fn);
  while(i<9999 && f.find(fn)) {
    i++;
    fn[4]='0'+i/1000;
    fn[5]='0'+(i%1000)/100;
    fn[6]='0'+(i%100)/10;
    fn[7]='0'+(i%10);
    Serial.println(fn);
  }
  bool worked=f.openw(fn,pktStore.headPtr());
  Serial.print("f.openw(\"");Serial.print(fn);Serial.print("\"): ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(f.errno);
}

void closeLog() {
//  f.sync(buf); //Sync the directory entry - after this, we may reuse this file
               //object on a new file.
}

static const char version_string[]="Rocketometer v0.02 " __DATE__ " " __TIME__;

void setup() {
  Serial.begin(230400);
  Serial.println(version_string);
  Wire1.begin();

  SPI1.begin(1000000,1,1);

  bool worked=sd.begin();
  Serial.print("sd");    Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(sd.errno);

  worked=p.begin(1);
  Serial.print("p");     Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(p.errno);

  worked=fs.begin();  
  Serial.print("fs");    Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(fs.errno);
  sector.begin();
  fs.print(Serial);//,sector);

  openLog();
  //Dump code to serial port and packet file
  int len=source_end-source_start;
  char* base=source_start;
  d.begin();
  while(len>0) {
    d.line(base,0,len>120?120:len);
    ccsds.start(0x03,pktseq);
    ccsds.fill16(base-source_start);
    ccsds.fill(base,len>120?120:len);
    ccsds.finish();
    pktStore.drain(); 
    base+=120;
    len-=120;
  }
  d.end();
  mpu6050.begin();
  Serial.print("MPU6050 identifier (should be 0x68): 0x");
  Serial.println(mpu6050.whoami(),HEX);

  hmc5883.begin();
  char HMCid[4];
  hmc5883.whoami(HMCid);
  Serial.print("HMC5883L identifier (should be 'H43'): ");
  Serial.println(HMCid);
  ccsds.start(0x0E);
  hmc5883.fillConfig(ccsds);
  ccsds.finish();

  char channels=0x0B;
  worked=ad799x.begin(channels); 
  Serial.print("ad799x");Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(0);
  ccsds.start(0x0D);
  ccsds.fill(ad799x.getAddress());
  ccsds.fill(channels);
  ccsds.fill(ad799x.getnChannels());
  ccsds.fill((uint8_t)worked);
  ccsds.finish();

  worked=bmp180.begin();
  Serial.print("bmp180");Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(0);
  Serial.print("BMP180 identifier (should be 0x55): 0x");
  Serial.println(bmp180.whoami(),HEX);

  bmp180.printCalibration(&Serial);

  bmp180.ouf=0;
  ccsds.start(0x02);

  bmp180.fillCalibration(ccsds);

  ccsds.finish();

  ccsds.start(0x0C);
  ccsds.fill32(HW_TYPE);
  ccsds.fill32(HW_SERIAL);
  ccsds.fill(MAMCR);
  ccsds.fill(MAMTIM);
  ccsds.fill16(PLL0STAT);
  ccsds.fill(VPBDIV);
  ccsds.fill32(FOSC);                   //Crystal frequency, Hz
  ccsds.fill32(CCLK);                   //Core Clock rate, Hz
  ccsds.fill32(PCLK);                   //Peripheral Clock rate, Hz
  ccsds.fill32(PREINT);  
  ccsds.fill32(PREFRAC);                    
  ccsds.fill(CCR);
  ccsds.fill(version_string);
  ccsds.finish();

  Serial.println("t,tc,bx,by,bz,max,may,maz,mgx,mgy,mgz,mt,h0,h1,h2,h3,T,P");
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
  ccsds.start(0x06,pktseq,TC);
  ccsds.fill16(max);ccsds.fill16(may); ccsds.fill16(maz); ccsds.fill16(mgx); ccsds.fill16(mgy); ccsds.fill16(mgz); ccsds.fill16(mt);
  ccsds.finish();
  if(0==(phase%10)) {
    //Only read the compass and HighAcc once every 10 times we read the 6DoF
    TC=TTC(0);
    hmc5883.read(bx,by,bz);
    ccsds.start(0x04,pktseq,TC);
    ccsds.fill16(bx);  ccsds.fill16(by);  ccsds.fill16(bz);
    ccsds.finish();
    TC=TTC(0);
    ad799x.read(hx);
    ccsds.start(0x0B,pktseq,TC);
    ccsds.fill((char*)hx,8);
    ccsds.finish();
    if(200==phase) {
//      ad799x.format(hx);
      TC=TTC(0);  
      bmp180.takeMeasurement();
      auto temperatureRaw=bmp180.getTemperatureRaw();
      auto pressureRaw=bmp180.getPressureRaw();
      auto temperature=bmp180.getTemperature();
      auto pressure=bmp180.getPressure();
      auto TC1=TTC(0);
      ccsds.start(0x0A,pktseq,TC);
      ccsds.fill16(temperatureRaw);
      ccsds.fill32(pressureRaw);
      ccsds.fill16(temperature);
      ccsds.fill32(pressure);
      ccsds.fill32(TC1);
      ccsds.finish();
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
      Serial.print(",");Serial.print(hx[0], DEC); 
      Serial.print(",");Serial.print(hx[1], DEC); 
      Serial.print(",");Serial.print(hx[2], DEC); 
      Serial.print(",");Serial.print(hx[3], DEC); 
      Serial.print(",");Serial.print(temperature, DEC);    
      Serial.print(",");Serial.print((unsigned int)pressure, DEC); 
      Serial.println();
      phase=0;
    }
  }
  TC=TTC(0);  
  if(pktStore.drain()) {
    unsigned int TC1=TTC(0);
    ccsds.start(0x08,pktseq,TC);
    ccsds.fill32(TC1);
    ccsds.finish();
  }
/*
  if(f.size()>=maxLogSize) {
    closeLog();
    openLog();
  }
*/
}



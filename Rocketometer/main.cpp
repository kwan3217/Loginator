#include <string.h>
#include "StateTwoWire.h"
#include "HardSPI.h"
#include "Serial.h"
#include "bmp180.h"
#include "hmc5883.h"
#include "mpu60x0.h"
#include "adxl345.h"
#include "l3g4200d.h"
#include "Task.h"
#include "Time.h"
#include "LPC214x.h"
#include "gpio.h"
#include "dump.h"
#include "packet.h"
#include "sdhc.h"
#include "Partition.h"
#include "cluster.h"
#include "direntry.h"
#include "file.h"
#include "FileCircular.h"

int temperature, pressure;
int temperatureRaw, pressureRaw;
int16_t ax,ay,az;    //acc
int16_t bx,by,bz;    //compass (bfld)
int16_t gx,gy,gz;    //gyro
uint8_t gt,gs;       //gyro temp and status
int16_t max,may,maz; //MPU60x0 acc
int16_t mgx,mgy,mgz; //MPU60x0 gyro
int16_t mt;          //MPU60x0 temp
char buf[2*SDHC::BLOCK_SIZE];

StateTwoWire Wire1(1);
SDHC sd(&SPI,7);
Partition p(sd);
Cluster fs(p);
File f(fs);
BMP180 bmp180(Wire1);
HMC5883 hmc5883(Wire1);
MPU6050 mpu6050(Wire1,0);
ADXL345 adxl345(&SPI1,20);
L3G4200D l3g4200d(&SPI1,25);
Base85 d(Serial);
FileCircular pktStore(buf,f);
CCSDS ccsds(pktStore);
unsigned short pktseq01,pktseq04,pktseq05,pktseq06,pktseq07;
bool timeToRead;
unsigned int lastTC,nextTC,interval;

void sensorTimingTask(void* stuff) {
  static int lightState=0;
  set_light(0,lightState>0);
  lightState=1-lightState;
//  timeToRead=true;
  taskManager.reschedule(100,0,sensorTimingTask,0);
}

void setup() {
  taskManager.begin();
  Serial.begin(230400);
  Wire1.begin();

  SPI1.begin(1000000,1,1);

  bool worked=sd.begin();
  Serial.print("sd");    Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(sd.errno);

  worked=p.begin(1);
  Serial.print("p");     Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(p.errno);

  worked=fs.begin();  
  Serial.print("fs");    Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(fs.errno);

  char fn[13];
  strcpy(fn,"rkto0000.sds");
  int i=0;
  Serial.println(fn);
  while(i<9999 && f.find(fn)) {
    i++;
    fn[4]='0'+i/1000;
    fn[5]='0'+(i%1000)/100;
    fn[6]='0'+(i%100)/10;
    fn[7]='0'+(i%10);
    Serial.println(fn);
  }
  worked=f.openw(fn,pktStore.headPtr());
  Serial.print("f.openw(\"");Serial.print(fn);Serial.print("\"): ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(f.errno);

  //Dump code to serial port and packet file
  int len=source_end-source_start;
  char* base=source_start;
  unsigned short dumpSeq=0;
  d.begin();
  while(len>0) {
    d.line(base,0,len>120?120:len);
    ccsds.start(0x03,&dumpSeq);
    ccsds.fill16(base-source_start);
    ccsds.fill(base,len>120?120:len);
    ccsds.finish();
    pktStore.drain(); 
    base+=120;
    len-=120;
  }
  d.end();

  adxl345.begin();
  Serial.print("ADXL345 identifier (should be 0o345): 0o");
  Serial.println(adxl345.whoami(),OCT);

  l3g4200d.begin();
  Serial.print("L3G4200D identifier (should be 0xD3): 0x");
  Serial.println(l3g4200d.whoami(),HEX);

  mpu6050.begin();
  Serial.print("MPU6050 identifier (should be 0x68): 0x");
  Serial.println(mpu6050.whoami(),HEX);

  hmc5883.begin();
  char HMCid[4];
  hmc5883.whoami(HMCid);
  Serial.print("HMC5883L identifier (should be 'H43'): ");
  Serial.println(HMCid);

  worked=bmp180.begin();
  Serial.print("bmp180");Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(0);
  Serial.print("BMP180 identifier (should be 0x55): 0x");
  Serial.println(bmp180.whoami(),HEX);

  bmp180.printCalibration(&Serial);

  bmp180.ouf=0;
  ccsds.start(0x02);

  bmp180.fillCalibration(ccsds);

  ccsds.finish();

  Serial.println("t,tc,ax,ay,az,bx,by,bz,gx,gy,gz,gt,max,may,maz,mgx,mgy,mgz,mt,Traw,Praw");
//  Serial.println("t,tc,Traw,Praw");

//  taskManager.schedule(100,0,sensorTimingTask,0); 
  lastTC=TTC(0);
  interval=(PCLK/1000)*10;
  nextTC=(lastTC+interval) % timerInterval;
}

void loop1() {
  static int phase=0;
  unsigned int TC=TTC(0);
  if(nextTC>lastTC) {
    timeToRead=TC>nextTC;
  } else {
    timeToRead=(TC>nextTC) & (TC<lastTC);
  }
  if(timeToRead) {
    lastTC=nextTC;
    nextTC=(lastTC+interval) % timerInterval;
    phase++;
    if(phase>=50) {
      phase=0;
      bmp180.takeMeasurement();
      temperatureRaw=bmp180.getTemperatureRaw();
      pressureRaw=bmp180.getPressureRaw();
      Serial.print(RTCHOUR,DEC,2);
      Serial.print(":");Serial.print(RTCMIN,DEC,2);
      Serial.print(":");Serial.print(TC/PCLK,DEC,2);
      Serial.print(".");Serial.print((TC%PCLK)/(PCLK/1000),DEC,3);
      Serial.print(",,,,,,,,,,,,,,,,,,");Serial.print(temperatureRaw, DEC);    
      Serial.print(",");Serial.print(pressureRaw, DEC); 
      Serial.println();
      ccsds.start(0x07,&pktseq07,TC,0);
      ccsds.fill16(temperatureRaw);
      ccsds.fill32(pressureRaw);
      ccsds.finish();
    } else {
      adxl345.read(ax,ay,az);
      hmc5883.read(bx,by,bz);
      l3g4200d.read(gx,gy,gz,gt,gs);
      mpu6050.read(max,may,maz,mgx,mgy,mgz,mt);
      if(phase%10==0) {
        Serial.print(RTCHOUR,DEC,2);
        Serial.print(":");Serial.print(RTCMIN,DEC,2);
        Serial.print(":");Serial.print(TC/PCLK,DEC,2);
        Serial.print(".");Serial.print((TC%PCLK)/(PCLK/1000),DEC,3);
        Serial.print(",");Serial.print(ax, DEC); 
        Serial.print(",");Serial.print(ay, DEC); 
        Serial.print(",");Serial.print(az, DEC); 
        Serial.print(",");Serial.print(bx, DEC); 
        Serial.print(",");Serial.print(by, DEC); 
        Serial.print(",");Serial.print(bz, DEC); 
        Serial.print(",");Serial.print(gx, DEC); 
        Serial.print(",");Serial.print(gy, DEC); 
        Serial.print(",");Serial.print(gz, DEC); 
        Serial.print(",");Serial.print(gt, DEC); 
        Serial.print(",");Serial.print(max, DEC);
        Serial.print(",");Serial.print(may, DEC);
        Serial.print(",");Serial.print(maz, DEC);
        Serial.print(",");Serial.print(mgx, DEC);
        Serial.print(",");Serial.print(mgy, DEC);
        Serial.print(",");Serial.print(mgz, DEC);
        Serial.print(",");Serial.print(mt, DEC);
        Serial.println();
      }
      ccsds.start(0x01,&pktseq01,TC,0);
      ccsds.fill16(ax); ccsds.fill16(ay);  ccsds.fill16(az);  
      ccsds.finish();
      ccsds.start(0x04,&pktseq04,TC,0);
      ccsds.fill16(bx);  ccsds.fill16(by);  ccsds.fill16(bz);
      ccsds.finish();
      ccsds.start(0x05,&pktseq05,TC,0);
      ccsds.fill16(gx);  ccsds.fill16(gy);  ccsds.fill16(gz);  ccsds.fill16(gt);
      ccsds.finish();
      ccsds.start(0x06,&pktseq06,TC,0);
      ccsds.fill16(max);ccsds.fill16(may); ccsds.fill16(maz); ccsds.fill16(mgx); ccsds.fill16(mgy); ccsds.fill16(mgz); ccsds.fill16(mt);
      ccsds.finish();
    }
    pktStore.drain();
  }
}

void loop2() {
  unsigned int TC=TTC(0);
  adxl345.read(ax,ay,az);
  ccsds.start(0x01,&pktseq01,TC,0);
  ccsds.fill16(ax); ccsds.fill16(ay);  ccsds.fill16(az);  
  ccsds.finish();
  TC=TTC(0);
  hmc5883.read(bx,by,bz);
  ccsds.start(0x04,&pktseq04,TC,0);
  ccsds.fill16(bx);  ccsds.fill16(by);  ccsds.fill16(bz);
  ccsds.finish();
  TC=TTC(0);
  l3g4200d.read(gx,gy,gz,gt,gs);
  ccsds.start(0x05,&pktseq05,TC,0);
  ccsds.fill16(gx);  ccsds.fill16(gy);  ccsds.fill16(gz);  ccsds.fill16(gt);
  ccsds.finish();
  TC=TTC(0);
  mpu6050.read(max,may,maz,mgx,mgy,mgz,mt);
  ccsds.start(0x06,&pktseq06,TC,0);
  ccsds.fill16(max);ccsds.fill16(may); ccsds.fill16(maz); ccsds.fill16(mgx); ccsds.fill16(mgy); ccsds.fill16(mgz); ccsds.fill16(mt);
  ccsds.finish();
  pktStore.drain();
}

void loop() {loop2();}

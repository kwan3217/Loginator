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
#include "DirectTask.h"
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
//Set to 128MiB so that we can test the feature
unsigned int maxLogSize=1024U*1024U*128U;
uint16_t resetFileSkip=100;

//int temperature, pressure;
//int temperatureRaw, pressureRaw;
char buf[SDHC::BLOCK_SIZE]; //Use when you need some space to do a sd write
volatile bool writeDrain=false;
volatile bool writeSd=false;
volatile unsigned int drainTC0,drainTC1;

const int readPeriodMs=3; //Read period in ms

StateTwoWire Wire1(0);
SDHC sd(&SPI,15);
SDHC_info sdinfo;
Partition p(sd);
Cluster fs(p);
File f(fs);
Hd sector(Serial);
BMP180 bmp180(Wire1);
HMC5883 hmc5883(Wire1);
MPU6050 mpu6050(Wire1,0);
AD799x ad799x(Wire1);
const int dumpPktSize=120;
Base85 d(Serial,dumpPktSize);
FileCircular pktStore(f);
CCSDS ccsds(pktStore);
unsigned short pktseq[32];

const char syncMark[]="KwanSync";

void blinklock(int blinkcode) {
  VICIntEnClr=0xFFFFFFFF;
  set_light(0,0);
  set_light(1,0);
  set_light(2,0);
  for(;;) {
    for(int i=0;(blinkcode >> i)>0 && i<32;i++) {
      set_light((blinkcode >> i) & 1,1);
      delay(1000);
      set_light((blinkcode >> i) & 1,0);
      delay(1000);
    }
    set_light(2,1);
    delay(1000);
    set_light(2,0);
    delay(1000);
  }
}

void openLog(uint16_t inc=1) {
  static char fn[13];
  if(fn[0]!='r') strcpy(fn,"rkto0000.sds");
  static int i=0;
  Serial.println(fn);
  while(i<9999 && f.find(fn)) {
    i+=inc;
    fn[4]='0'+i/1000;
    fn[5]='0'+(i%1000)/100;
    fn[6]='0'+(i%100)/10;
    fn[7]='0'+(i%10);
    Serial.println(fn);
  }
  bool worked=f.openw(fn,pktStore.headPtr());
  Serial.print("f.openw(\"");Serial.print(fn);Serial.print("\"): ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(f.errno);
  if(!worked) blinklock(f.errno);
}

void closeLog() {
  f.close(buf); //Sync the directory entry - after this, we may reuse this file
               //object on a new file.
}

static const char version_string[]="Rocketometer v1.01 " __DATE__ " " __TIME__;

int16_t max,may,maz; //MPU60x0 acc
int16_t mgx,mgy,mgz; //MPU60x0 gyro
int16_t mt;          //MPU60x0 temp
int16_t bx,by,bz;    //compass (bfld)
uint16_t hx[4];      //HighAcc
bool wantPrint;
uint32_t TC,TC1;
int temperatureRaw,pressureRaw;
int16_t temperature;
int32_t pressure;

void writeSdPacket() {
  ccsds.start(0x11);
  while(sd.buf.readylen()>0) ccsds.fill(sd.buf.get());
  ccsds.finish();
}

void collectData(void* stuff) {
  static int phase=0;
  static uint32_t bmpTC;
  phase++;
  TC=TTC(0);
  mpu6050.read(max,may,maz,mgx,mgy,mgz,mt);
  ad799x.read(hx);
  TC1=TTC(0);
  ccsds.start(0x10,pktseq,TC);
  ccsds.fill16(max);ccsds.fill16(may); ccsds.fill16(maz); ccsds.fill16(mgx); ccsds.fill16(mgy); ccsds.fill16(mgz); ccsds.fill16(mt);
  ccsds.fill((char*)hx,8);
  ccsds.fill32(TC1);
  ccsds.finish();
  if(0==(phase%20)) {
    //Only read the compass once every n times we read the 6DoF
    TC=TTC(0);
    hmc5883.read(bx,by,bz);
    ccsds.start(0x04,pktseq,TC);
    ccsds.fill16(bx);  ccsds.fill16(by);  ccsds.fill16(bz);
    ccsds.finish();
  }
  if(200==phase) {
    //Only read the pressure sensor once every n times we read the 6DoF
    bmpTC=TTC(0);  
    bmp180.startMeasurement();
  } else if(bmp180.ready) {
    temperatureRaw=bmp180.getTemperatureRaw();
    pressureRaw=bmp180.getPressureRaw();
    temperature=bmp180.getTemperature();
    pressure=bmp180.getPressure();
    bmp180.ready=false;
    TC1=TTC(0);
    ccsds.start(0x0A,pktseq,bmpTC);
    ccsds.fill16(temperatureRaw);
    ccsds.fill32(pressureRaw);
    ccsds.fill16(temperature);
    ccsds.fill32(pressure);
    ccsds.fill32(TC1);
    ccsds.finish();
    wantPrint=true;
    phase=0;
  }
  //Why here? Because since creation of packets is interrupt driven, and there
  //is only a single buffer, only the interrupt routine is allowed to write
  if(writeDrain) {
    ccsds.start(0x08,pktseq,drainTC0);
    ccsds.fill32(drainTC1);
    ccsds.finish();
    writeDrain=false;
  }
  if(writeSd) {
    writeSdPacket();
    writeSd=false;
  }
  directTaskManager.reschedule(1,readPeriodMs,0,collectData,0); 
}

void setup() {
  Serial.begin(230400);
  Serial.println(version_string);
  Wire1.begin();

  SPI1.begin(1000000,1,1);

  bool worked=sd.begin();
  Serial.print("sd");    Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(sd.errno);
  if(!worked) blinklock(sd.errno);

  sd.get_info(sdinfo);
  sdinfo.print(Serial);
  worked=p.begin(1);
  Serial.print("p");     Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(p.errno);
  p.print(Serial);
  if(!worked) blinklock(p.errno);

  worked=fs.begin();  
  Serial.print("fs");    Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(fs.errno);
//  sector.begin();
  fs.print(Serial);//,sector);
  if(!worked) blinklock(fs.errno);

  openLog(resetFileSkip);
  pktStore.fill(syncMark);
  pktStore.mark();
  //Dump code to serial port and packet file
  int len=source_end-source_start;
  char* base=source_start;
  d.begin();
  while(len>0) {
    d.line(base,0,len>dumpPktSize?dumpPktSize:len);
    ccsds.start(0x03,pktseq);
    ccsds.fill16(base-source_start);
    ccsds.fill(base,len>dumpPktSize?dumpPktSize:len);
    ccsds.finish();
    pktStore.drain(); 
    if(sd.buf.readylen()>128) writeSdPacket();
    base+=dumpPktSize;
    len-=dumpPktSize;
  }
  d.end();

  ccsds.start(0x12,pktseq);
  sdinfo.fill(ccsds);
  ccsds.finish();
  pktStore.drain(); 
  if(sd.buf.readylen()>128) writeSdPacket();

  mpu6050.begin(3,3);
  Serial.print("MPU6050 identifier (should be 0x68): 0x");
  Serial.println(mpu6050.whoami(),HEX);
  ccsds.start(0x0F);
  mpu6050.fillConfig(ccsds);
  ccsds.finish();
  pktStore.drain(); 
  if(sd.buf.readylen()>128) writeSdPacket();

  hmc5883.begin();
  char HMCid[4];
  hmc5883.whoami(HMCid);
  Serial.print("HMC5883L identifier (should be 'H43'): ");
  Serial.println(HMCid);
  ccsds.start(0x0E);
  hmc5883.fillConfig(ccsds);
  ccsds.finish();
  pktStore.drain(); 
  if(sd.buf.readylen()>128) writeSdPacket();

  char channels=0x0B;
  worked=ad799x.begin(channels); 
  Serial.print("ad799x");Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(0);
  if(!worked) blinklock(0xAAAA5555);
  ccsds.start(0x0D);
  ccsds.fill(ad799x.getAddress());
  ccsds.fill(channels);
  ccsds.fill(ad799x.getnChannels());
  ccsds.fill((uint8_t)worked);
  ccsds.finish();
  pktStore.drain(); 
  if(sd.buf.readylen()>128) writeSdPacket();

  worked=bmp180.begin(2);
  Serial.print("bmp180");Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(0);
  Serial.print("BMP180 identifier (should be 0x55): 0x");
  Serial.println(bmp180.whoami(),HEX);
  if(!worked) blinklock(0x5555AAAA);
  bmp180.printCalibration(&Serial);

  bmp180.ouf=0;
  ccsds.start(0x02);

  bmp180.fillCalibration(ccsds);

  ccsds.finish();
  pktStore.drain(); 
  if(sd.buf.readylen()>128) writeSdPacket();

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
  pktStore.drain(); 
  if(sd.buf.readylen()>128) writeSdPacket();

  Serial.println("t,tc,bx,by,bz,max,may,maz,mgx,mgy,mgz,mt,h0,h1,h2,h3,T,P");
//  Serial.println("t,tc,Traw,Praw");

  directTaskManager.begin();
  directTaskManager.schedule(1,readPeriodMs,0,collectData,0); 
}

void loop() {
  drainTC0=TTC(0);  
  if(pktStore.drain()) {
    writeDrain=true;
    drainTC1=TTC(0);
    if(sd.buf.readylen()>128) writeSd=true;
    if(f.size()>=maxLogSize) {
      closeLog();
      openLog();
      pktStore.fill(syncMark);
      pktStore.mark();
    }
  }
  if(pktStore.errno!=0) {
    Serial.print("Problem writing file: pktStore.errno=");
    Serial.println(pktStore.errno);
    blinklock(pktStore.errno);
  }
  if(wantPrint) {
    Serial.print(RTCHOUR,DEC,2);
    Serial.print(":");Serial.print(RTCMIN,DEC,2);
    Serial.print(":");Serial.print(((unsigned int)(TC/PCLK)),DEC,2);
    Serial.print(".");Serial.print(((unsigned int)((TC%PCLK)/(PCLK/1000))),DEC,3);
    Serial.print(",");Serial.print(((unsigned int)(TC)),DEC,10);
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
    Serial.print(",");Serial.print(hx[0], HEX,4); 
    Serial.print(",");Serial.print(hx[1], HEX,4); 
    Serial.print(",");Serial.print(hx[2], HEX,4); 
    Serial.print(",");Serial.print(hx[3], HEX,4); 
    Serial.print(",");Serial.print(temperature/10, DEC);    
    Serial.print(".");Serial.print(temperature%10, DEC);    
    Serial.print(",");Serial.print((unsigned int)pressure, DEC); 
    Serial.println();
    wantPrint=false;
  }
}



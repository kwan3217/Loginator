#include <string.h>
#include "gpio.h"
#include "StateTwoWire.h"
#include "HardSPI.h"
#include "Serial.h"
#include "bmp180.h"
#include "hmc5883.h"
#include "l3g4200d.h"
#include "adxl345.h"
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
static const int blockSize=SDHC::BLOCK_SIZE;
#include "FileCircular.h"
#include "gps.h"
#include "LPCduino.h"

//Once the log becomes greater or equal to this length, cycle the log file
//This way we don't violate the FAT file size limit, and don't choke our processing
//program with data.
//Set to 128MiB so that we can test the feature
const unsigned int maxLogSize=1024U*1024U*1024U;
const uint16_t resetFileSkip=1;

volatile bool writeDrain=false;
volatile bool writeSd=false;
volatile unsigned int drainTC0,drainTC1;

const uint32_t readPeriodMs=3;

inline uint32_t abs(int in) {
  return in>0?in:-in;
}

StateTwoWire Wire1(0);
SDHC sd(&SPI,7);
SDHC_info sdinfo;
Partition p(sd);
Cluster fs(p);
File f(fs);
Hd sector(Serial);
#undef HAS_SPI
#ifdef HAS_SPI
ADXL345 adxl345(&SPI1,20);
L3G4200D l3g4200d(&SPI1,25);
#endif
#undef HAS_I2C
#ifdef HAS_I2C
BMP180 bmp180(Wire1);
HMC5883 hmc5883(Wire1);
int16_t bx,by,bz;    //compass (bfld)
int temperatureRaw,pressureRaw;
int16_t temperature;
int32_t pressure;
uint32_t bmpTC;
#endif
int bmpPhase=0;
int hmcPhase=0;
const int bmpMaxPhase=150;
const int hmcMaxPhase=10;
int16_t max,may,maz; //ADXL345 acc
int16_t mgx,mgy,mgz; //L3G4200D gyro
uint8_t mt,ms;       //L3G4200D temperature and status
uint16_t aft,fwd;    //aft and fwd analog data
uint16_t minAft=1024,minFwd=1024;    
uint16_t maxAft,maxFwd;    
uint16_t aftThreshold,fwdThreshold;    
bool old_aft,old_fwd;    //aft and fwd analog data
uint32_t TC,TC1;
uint32_t oldOvr;
const int dumpPktSize=120;
FileCircular sdStore(f);
char measBuf[1024],serialBuf[1024];
Circular measStore(1024,measBuf),serialStore(1024,serialBuf);
CCSDS ccsds;
unsigned short pktseq[80];
bool avgLight=true;
bool wantPrint=false;

static const char syncMark[]="KwanSync";
static const char version_string[]="Loginator Pinewood 1.0 " __DATE__ " " __TIME__;

static uint16_t log_i=0;

void openLog(uint16_t inc=1) {
  Serial.println(inc,DEC);
  Serial.println(log_i,DEC);
  if(inc==0) blinklock(108);
  static char fn[13];
  if(fn[0]!='r') strcpy(fn,"pnwd0000.sds");
  Serial.println(fn);
  while(log_i<9999 && f.find(fn)) {
    log_i+=inc;
    fn[4]='0'+log_i/1000;
    fn[5]='0'+(log_i%1000)/100;
    fn[6]='0'+(log_i%100)/10;
    fn[7]='0'+(log_i%10);
    Serial.println(fn);
  }
  bool worked=f.openw(fn);
  Serial.print("f.openw(\"");Serial.print(fn);Serial.print("\"): ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(f.errnum);
  if(!worked) blinklock(f.errnum);
}

void closeLog() {
  f.close(); //Sync the directory entry - after this, we may reuse this file
               //object on a new file.
}

static void writeSdPacket(Circular &buf) {
  ccsds.start(buf,0x11);
  while(sd.buf.readylen()>0) ccsds.fill(sd.buf.get());
  ccsds.finish(0x11);
}

static void maybeWriteSdPacket(Circular& buf) {
  if(sd.buf.readylen()>128) writeSdPacket(buf);
}

int wheelrev=0;
bool aftOver,oldAft=false;
bool fwdOver,oldFwd=false;
unsigned long t,old_t=0,t0=0;
float wheelRad=0.015; //wheel radius in m, 1.5cm
const float PI=3.14159265358;
float wheelCirc=wheelRad*2*PI;
float dist=0;
float spd=0,maxspd=0;

void checkSpeed() {
  TC=TTC(0);
  aft=analogRead(5);
  fwd=analogRead(6);
  aftOver=(aft>aftThreshold);
  fwdOver=(fwd>fwdThreshold);
  if (fwdOver!=oldFwd) {
    t=TTC(0);
    if(t0==0) t0=t;
    if(aftOver) {
      wheelrev+=(fwdOver?1:-1);
      dist=wheelrev*wheelCirc;
      if(old_t!=0) {
        spd=wheelCirc/(((float)(t-old_t))/((float)PCLK));
        if(maxspd<spd) maxspd=spd;
        ccsds.start(measStore,0x41,pktseq,TC);
        ccsds.fill(wheelrev);
        ccsds.fill(dist);
        ccsds.fill(spd);
        ccsds.fill(maxspd);
        ccsds.finish(0x40);
        wantPrint=true;
      } else {
        old_t=t;
      }
    }
    oldAft=aftOver;
    oldFwd=fwdOver;
    old_t=t;
  }
}

void collectData(void* stuff) {
  if(avgLight) {
    aft=analogRead(5);
    fwd=analogRead(6);
    if(aft<minAft) minAft=aft;
    if(aft>maxAft) maxAft=aft;
    if(fwd<minFwd) minFwd=fwd;
    if(fwd>maxFwd) maxFwd=fwd;
    directTaskManager.reschedule(1,readPeriodMs,0,collectData,0); 
    if(TTC(0)>20*PCLK) {
      aftThreshold=(minAft+maxAft)/2;
      fwdThreshold=(minFwd+maxFwd)/2;
      avgLight=false;
      set_light(0,OFF);
      set_light(1,OFF);
      set_light(2,OFF);
      Serial.begin(57600);
      Serial.print("Self cal - Aft min/mid/max:");Serial.print(minAft,DEC);
      Serial.print("/");Serial.print(aftThreshold,DEC);
      Serial.print("/");Serial.print(maxAft,DEC);
      Serial.print(" Fwd min/mid/max:");Serial.print(minFwd,DEC);
      Serial.print("/");Serial.print(fwdThreshold,DEC);
      Serial.print("/");Serial.println(maxFwd,DEC);
      ccsds.start(measStore,0x41,pktseq,TC);
      ccsds.fill16(minAft);ccsds.fill16(aftThreshold); ccsds.fill16(maxAft);
      ccsds.fill16(minFwd);ccsds.fill16(fwdThreshold); ccsds.fill16(maxFwd);
      ccsds.fill32(TC1);
      ccsds.finish(0x41);
      Serial.print("t,tc");
#ifdef HAS_I2C
      Serial.print(",bx,by,bz");
#endif
#ifdef HAS_SPI
      Serial.print(",max,may,maz,mgx,mgy,mgz,mt");
#endif
#ifdef HAS_I2C
      Serial.print(",T,P");
#endif
      Serial.println(",rev,dist,spd,maxspd");
      //Serial write takes a long time, maybe more than scheduled. Reschedule
      //(by using schedule, not reschedule) one period in the future
      directTaskManager.schedule(1,readPeriodMs,0,collectData,0); 
    }
    return;
  }
  //Don't bother to read the sensors if we can't store the data
  //this way we yield more time back to the writing routine so 
  //hopefully the buffer becomes empty sooner
  if(measStore.isFull()) {
    directTaskManager.reschedule(1,readPeriodMs,0,collectData,0); 
    return; 
  }
  if(oldOvr!=measStore.getBufOverflow()) {
    ccsds.start(measStore,0x15,pktseq,TC);
    ccsds.fill(measStore.getBufOverflow());
    ccsds.finish(0x15);
    oldOvr=measStore.getBufOverflow();
  }
  hmcPhase++;
  bmpPhase++;
#ifdef HAS_SPI
  TC=TTC(0);
  adxl345.read(max,may,maz);
  l3g4200d.read(mgx,mgy,mgz,mt,ms);
  TC1=TTC(0);
  ccsds.start(measStore,0x40,pktseq,TC);
  ccsds.fill16(max);ccsds.fill16(may); ccsds.fill16(maz); ccsds.fill16(mgx); ccsds.fill16(mgy); ccsds.fill16(mgz); ccsds.fill16(mt);
  ccsds.fill32(TC1);
  ccsds.finish(0x40);
#endif
  checkSpeed();

  if(hmcPhase>=hmcMaxPhase) {
    #ifdef HAS_I2C
    //Only read the compass once every n times we read the 6DoF
    TC=TTC(0);
    hmc5883.read(bx,by,bz);
    ccsds.start(measStore,0x04,pktseq,TC);
    //Sensor registers are in X, Z, Y order, not XYZ. Old code didn't know this,
    //wrote sensor registers in order, therefore wrote xzy order unintentionally.
    //We will keep this order to maintain compatibility with old data, including
    //flight 36.290
    ccsds.fill16(bx);  ccsds.fill16(bz);  ccsds.fill16(by);
    ccsds.finish(0x04);
    #endif
    hmcPhase=0;
  }
  if(bmpPhase>=bmpMaxPhase) {
  #ifdef HAS_I2C
    //Only read the pressure sensor once every n times we read the 6DoF
    bmpTC=TTC(0);  
    bmp180.startMeasurement();
  #endif
    wantPrint=true;
    bmpPhase=0;
  }
  #ifdef HAS_I2C
  if(bmp180.ready) {
    temperatureRaw=bmp180.getTemperatureRaw();
    pressureRaw=bmp180.getPressureRaw();
    temperature=bmp180.getTemperature();
    pressure=bmp180.getPressure();
    bmp180.ready=false;
    TC1=TTC(0);
    ccsds.start(measStore,0x0A,pktseq,bmpTC);
    ccsds.fill16(temperatureRaw);
    ccsds.fill32(pressureRaw);
    ccsds.fill16(temperature);
    ccsds.fill32(pressure);
    ccsds.fill32(TC1);
    ccsds.finish(0x0A);
  }
#endif
  //Why here? Because since creation of packets is interrupt driven, and there
  //is only a single buffer, only the interrupt routine is allowed to write
  if(writeDrain) {
    ccsds.start(measStore,0x08,pktseq,drainTC0);
    ccsds.fill32(drainTC1);
    ccsds.finish(0x08);
    writeDrain=false;
  }
  if(writeSd) {
    writeSdPacket(measStore);
    writeSd=false;
  }
  directTaskManager.reschedule(1,readPeriodMs,0,collectData,0); 
}

void setup() {
  Serial.begin(57600);
  Serial.println(version_string);
  //SPI0 is under control of SD driver, since it needs to start low speed and change to high speed during begin()
#ifdef HAS_I2C
  Wire1.begin();
#endif
#ifdef HAS_SPI
  SPI1.begin(1000000,1,1); 
#endif
  bool worked;
  worked=sd.begin();

  Serial.print("sd");    Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(sd.errnum);
  if(!worked) blinklock(sd.errnum);

  sd.get_info(sdinfo);
  sdinfo.print(Serial);
  worked=p.begin(1);
  Serial.print("p");     Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(p.errnum);
  p.print(Serial);
  if(!worked) blinklock(p.errnum);

  worked=fs.begin();  
  Serial.print("fs");    Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(fs.errnum);
//  sector.begin();
  fs.print(Serial);//,sector);
  if(!worked) blinklock(fs.errnum);

  openLog(resetFileSkip);
  sdStore.fill(syncMark);
  sdStore.mark();
  //Dump code to serial port and packet file
  int len=source_end-source_start;
  char* base=source_start;
  while(len>0) {
    ccsds.start(sdStore,0x03,pktseq);
    ccsds.fill16(base-source_start);
    ccsds.fill(base,len>dumpPktSize?dumpPktSize:len);
    ccsds.finish(0x03);
    sdStore.drain(); 
    maybeWriteSdPacket(sdStore);
    base+=dumpPktSize;
    len-=dumpPktSize;
  }

  ccsds.start(sdStore,0x12,pktseq);
  sdinfo.fill(ccsds);
  ccsds.finish(0x12);
  sdStore.drain(); 
  maybeWriteSdPacket(sdStore);

#ifdef HAS_SPI
  adxl345.begin();
  Serial.print("ADXL345 identifier (should be 0o345): 0o");
  Serial.println(adxl345.whoami(),OCT);
  ccsds.start(sdStore,0x20);
  adxl345.fillConfig(ccsds);
  ccsds.finish(0x20);
  sdStore.drain();
  maybeWriteSdPacket(sdStore);

  l3g4200d.begin();
  Serial.print("L3G4200D identifier (should be 0xD3): 0x");
  Serial.println(l3g4200d.whoami(),HEX);
  ccsds.start(sdStore,0x21);
  l3g4200d.fillConfig(ccsds);
  ccsds.finish(0x21);
  sdStore.drain();
  maybeWriteSdPacket(sdStore);
#endif

#ifdef HAS_I2C
  hmc5883.begin();
  char HMCid[4];
  hmc5883.whoami(HMCid);
  Serial.print("HMC5883L identifier (should be 'H43'): ");
  Serial.println(HMCid);
  ccsds.start(sdStore,0x0E);
  hmc5883.fillConfig(ccsds);
  ccsds.finish(0x0E);
  sdStore.drain();
  maybeWriteSdPacket(sdStore);

  worked=bmp180.begin(2);
  Serial.print("bmp180");Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(0);
  Serial.print("BMP180 identifier (should be 0x55): 0x");
  Serial.println(bmp180.whoami(),HEX);
  if(!worked) blinklock(0x5555AAAA);
  bmp180.printCalibration(&Serial);

  bmp180.ouf=0;
  ccsds.start(sdStore,0x02);
  bmp180.fillCalibration(ccsds);
  ccsds.finish(0x02);
  sdStore.drain();
  maybeWriteSdPacket(sdStore);
#endif
  ccsds.start(sdStore,0x0C);
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
  ccsds.finish(0x0C);
  sdStore.drain();
  maybeWriteSdPacket(sdStore);

  directTaskManager.begin();
  directTaskManager.schedule(1,readPeriodMs,0,collectData,0); 
  set_light(0,ON);
  set_light(1,ON);
  set_light(2,ON);
}

void loop() {
  drainTC0=TTC(0);
  measStore.drain(sdStore);
  if(sdStore.drain()) {
    writeDrain=true;
    drainTC1=TTC(0);
    if(sd.buf.readylen()>128) writeSd=true;
    uint32_t logSize;
    logSize=f.size();
    if(logSize>=maxLogSize) {
      closeLog();
      openLog();
      sdStore.fill(syncMark);
      sdStore.mark();
    }
  }
  if(sdStore.errnum!=0) {
    Serial.print("Problem writing file: sdStore.errnum=");
    Serial.println(sdStore.errnum);
    blinklock(sdStore.errnum);
  }
  if(wantPrint) {
    Serial.print(RTCHOUR,DEC,2);
    Serial.print(":");Serial.print(RTCMIN,DEC,2);
    Serial.print(":");Serial.print(RTCSEC,DEC,2);
    Serial.print(".");Serial.print(CTC&0x7FFF,HEX,4);
    Serial.print(",");Serial.print(((unsigned int)(TC)),DEC,10);
#ifdef HAS_I2C
    Serial.print(",");Serial.print(bx, DEC); 
    Serial.print(",");Serial.print(by, DEC); 
    Serial.print(",");Serial.print(bz, DEC); 
#endif
#ifdef HAS_SPI
    Serial.print(",");Serial.print(max, DEC);
    Serial.print(",");Serial.print(may, DEC);
    Serial.print(",");Serial.print(maz, DEC);
    Serial.print(",");Serial.print(mgx, DEC);
    Serial.print(",");Serial.print(mgy, DEC);
    Serial.print(",");Serial.print(mgz, DEC);
    Serial.print(",");Serial.print(mt, DEC);
#endif
#ifdef HAS_I2C
    Serial.print(",");Serial.print(temperature/10, DEC);    
    Serial.print(".");Serial.print(temperature%10, DEC);    
    Serial.print(",");Serial.print((unsigned int)pressure, DEC); 
#endif
    Serial.print(",");Serial.print(wheelrev,DEC);
    Serial.print(",");Serial.print(dist/0.3048); //Print distance in feet
    Serial.print(",");Serial.print(spd*2.23694); //Print speed in mph
    Serial.print(",");Serial.print(maxspd*2.23694); //Print max speed in mph
    Serial.println();
    wantPrint=false;
  }
}



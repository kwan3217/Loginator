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

//Once the log becomes greater or equal to this length, cycle the log file
//This way we don't violate the FAT file size limit, and don't choke our processing
//program with data.
//Set to 128MiB so that we can test the feature
const unsigned int maxLogSize=1024U*1024U*1024U;
const uint16_t resetFileSkip=10;

//int temperature, pressure;
//int temperatureRaw, pressureRaw;
char vbus;
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
BMP180 bmp180(Wire1);
HMC5883 hmc5883(Wire1);
ADXL345 adxl345(&SPI1,20);
L3G4200D l3g4200d(&SPI1,25);
const int dumpPktSize=120;
FileCircular sdStore(f);
char measBuf[1024],serialBuf[1024];
Circular measStore(1024,measBuf),serialStore(1024,serialBuf);
CCSDS ccsds;
unsigned short pktseq[32];

const char syncMark[]="KwanSync";

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
  Serial.print("f.openw(\"");Serial.print(fn);Serial.print("\"): ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(f.errno);
  if(!worked) blinklock(f.errno);
}

void closeLog() {
  f.close(); //Sync the directory entry - after this, we may reuse this file
               //object on a new file.
}

static const char version_string[]="Loginator Pinewood 1.0 " __DATE__ " " __TIME__;

int16_t max,may,maz; //ADXL345 acc
int16_t mgx,mgy,mgz; //L3G4200D gyro
uint8_t mt,ms;       //L3G4200D temperature and status
uint16_t aft,fwd;    //aft and fwd analog data
uint16_t old_aft,old_fwd;    //aft and fwd analog data
int16_t bx,by,bz;    //compass (bfld)
bool wantPrint;
uint32_t TC,TC1;
int temperatureRaw,pressureRaw;
int16_t temperature;
int32_t pressure;

static void writeSdPacket(Circular &buf) {
  ccsds.start(buf,0x11);
  while(sd.buf.readylen()>0) ccsds.fill(sd.buf.get());
  ccsds.finish(0x11);
}

static void maybeWriteSdPacket(Circular& buf) {
  if(sd.buf.readylen()>128) writeSdPacket(buf);
}

uint32_t oldOvr;
int bmpPhase=0;
int hmcPhase=0;
const int bmpMaxPhase=150;
const int hmcMaxPhase=10;
uint32_t bmpTC;
char old_vbus=0;

void collectData(void* stuff) {
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
  TC=TTC(0);
  vbus=gpio_read(23);
  adxl345.read(max,may,maz);
  l3g4200d.read(mgx,mgy,mgz,mt,ms);
  aft=analogRead(5);
  fwd=analogRead(6);
  TC1=TTC(0);
  ccsds.start(measStore,0x30,pktseq,TC);
  ccsds.fill16(max);ccsds.fill16(may); ccsds.fill16(maz); ccsds.fill16(mgx); ccsds.fill16(mgy); ccsds.fill16(mgz); ccsds.fill16(mt); ccsds.fill16(aft); ccsds.fill16(fwd);
  ccsds.fill32(TC1);
  ccsds.finish(0x30);
  if(hmcPhase>=hmcMaxPhase) {
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
    hmcPhase=0;
  }
  if(bmpPhase>=bmpMaxPhase) {
    //Only read the pressure sensor once every n times we read the 6DoF
    bmpTC=TTC(0);  
    bmp180.startMeasurement();
    wantPrint=true;
    bmpPhase=0;
  }
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
  set_light(1,1);
//  Serial.begin(230400);
//  Serial.begin(115200);
//  Serial.begin(4800);
//  Serial.println("$PSRF103,8,0,1,1*2D");
//  delay(500);
//  Serial.println("$PSRF100,1,115200,8,1,0*05");
//  Serial.end();
  Serial.begin(57600);
  Serial.println(version_string);
  Wire1.begin();
  //SPI0 is under control of SD driver, since it needs to start low speed and change to high speed during begin()
  SPI1.begin(1000000,1,1); 

  bool worked;
  worked=sd.begin();

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

  Serial.println("t,tc,bx,by,bz,max,may,maz,mgx,mgy,mgz,mt,T,P,vbus,ovr");
//  Serial.println("t,tc,Traw,Praw");

  directTaskManager.begin();
  directTaskManager.schedule(1,readPeriodMs,0,collectData,0); 
}

void loop() {
  drainTC0=TTC(0);
  measStore.drain(sdStore);
  if(sdStore.drain()) {
    flicker();
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
  if(sdStore.errno!=0) {
    Serial.print("Problem writing file: sdStore.errno=");
    Serial.println(sdStore.errno);
    blinklock(sdStore.errno);
  }
  if(wantPrint) {
    Serial.print(RTCHOUR,DEC,2);
    Serial.print(":");Serial.print(RTCMIN,DEC,2);
    Serial.print(":");Serial.print(RTCSEC,DEC,2);
    Serial.print(".");Serial.print(CTC&0x7FFF,HEX,4);
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
    Serial.print(",");Serial.print(temperature/10, DEC);    
    Serial.print(".");Serial.print(temperature%10, DEC);    
    Serial.print(",");Serial.print((unsigned int)pressure, DEC); 
    Serial.print(",");Serial.print(vbus, DEC); 
    Serial.print(",");Serial.print(DirEntry::packTime(RTCHOUR,RTCMIN,RTCSEC),HEX,4); 
    Serial.print(",");Serial.print(DirEntry::packDate(RTCYEAR,RTCMONTH,RTCDOM),HEX,4); 
    Serial.println();
    wantPrint=false;
  }
}



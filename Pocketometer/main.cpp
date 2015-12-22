#include <string.h>
#include "Time.h"
#include "LPCduino.h"
#include "StateTwoWire.h"
#include "Serial.h"
#include "HardSPI.h"
#include "mpu60x0.h"
#include "bmp180.h"
#include "ak8975.h"
#include "sdhc.h"
#include "Partition.h"
#include "cluster.h"
#include "direntry.h"
#include "file.h"
#include "FileCircular.h"
#include "dump.h"
#include "nmea.h"
#include "readconfig.h"

const char syncMark[]="KwanSync";
static const char version_string[]="Pocketometer v0.0 " __DATE__ " " __TIME__;

StateTwoWire Wire0(0),Wire1(1);
BMP180 bmp180(&Wire1);
SDHC sd;
MPU6050 mpu6050(Wire1,0);
AK8975 ak8975(&Wire1);
Partition p(sd);
Cluster fs(p);
File f(fs);
FileCircular sdStore(f);
char serialBuf[1024];
Circular serialStore(1024,serialBuf);
CCSDS ccsds;
NMEA gps;

int gsens=3;
int asens=3;
int mpuBw=3;
int mpuSample=5;
int mpuReadMs=10;

const unsigned int maxLogSize=1024U*1024U*1024U;

const char* const ReadConfig::tagNames[]={"GSENS","ASENS","MPUBW","MPUSAMPLE","MPUREADMS","\0"};
const int   ReadConfig::tagTypes[]={ReadConfig::typeInt,ReadConfig::typeInt,ReadConfig::typeInt,ReadConfig::typeInt};
void *const ReadConfig::tagDatas[]={&gsens,&asens,&mpuBw,&mpuSample,&mpuReadMs};
int *const  ReadConfig::tagSizes[]={nullptr,nullptr,nullptr,nullptr,nullptr};
ReadConfig readconfig(fs,ccsds);

uint16_t log_i=0;

uint16_t pktseq[16];

static inline bool isRocketometer() {
  return HW_TYPE_ROCKETOMETER==HW_TYPE();
}

void openLog(uint16_t inc=1) {
  Serial.println(inc,DEC);
  Serial.println(log_i,DEC);
  if(inc==0) blinklock(108);
  static char fn[13];
  if(fn[0]!='s') strcpy(fn,"pocket00.sds");
  Serial.println(fn);
  while(log_i<99 && f.find(fn)) {
    log_i+=inc;
    fn[6]='0'+(log_i%100)/10;
    fn[7]='0'+(log_i%10);
    Serial.println(fn);
  }
  Serial.print("f.openw(\"");Serial.print(fn);Serial.print("\"): ");
  bool worked=f.openw(fn);
  Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(f.errnum);
  if(!worked) blinklock(f.errnum);
}

static void closeLog() {
  f.close(); //Sync the directory entry - after this, we may reuse this file
               //object on a new file.
}

static void initSD() {
  bool worked;
  Serial.print("sd");    Serial.print(".begin ");
  worked=sd.begin();
  Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(sd.errnum);
  if(!worked) blinklock(sd.errnum);

//  sd.get_info(sdinfo);
//  sdinfo.print(Serial);
  Serial.print("p");     Serial.print(".begin ");
  worked=p.begin(1);
  Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(p.errnum);
//  p.print(Serial);
  if(!worked) blinklock(p.errnum);

  Serial.print("fs");    Serial.print(".begin ");
  worked=fs.begin();  
  Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(fs.errnum);
//  sector.begin();
//  fs.print(Serial);//,sector);
  if(!worked) blinklock(fs.errnum);

  openLog();
  sdStore.fill(syncMark);
  sdStore.mark();

}

static void writeVersion(int apid) {
  ccsds.start(sdStore,apid,pktseq);
  ccsds.fill32(HW_TYPE());
  ccsds.fill32(HW_SERIAL());
  ccsds.fill(MAMCR());
  ccsds.fill(MAMTIM());
  ccsds.fill16(PLLSTAT(0));
  ccsds.fill(VPBDIV());
  ccsds.fill32(FOSC);                   //Crystal frequency, Hz
  ccsds.fill32(CCLK);                   //Core Clock rate, Hz
  ccsds.fill32(PCLK);                   //Peripheral Clock rate, Hz
  ccsds.fill32(PREINT());
  ccsds.fill32(PREFRAC());
  ccsds.fill(CCR());
  ccsds.fill(version_string);
  ccsds.finish(apid);
  sdStore.drain();
}

/** Dump a section of ROM to packet file
 @param start pointer to first byte to dump
 @param stop  pointer to one byte after last byte to dump
 @param apid  APID to use for the packets of this dump
 @param name  Name to print to serial port when done with dump
 */
void dumpRom(char* start, char* end, int apid, char* name) {
  const int dumpPktSize=256-6-3*sizeof(uint32_t); //Write two of these packets per sector

  int len=end-start;
  char* base=start;
  Serial.print("Dumping ");Serial.print(name);Serial.print("...");
  while(len>0) {
    ccsds.start(sdStore,apid,pktseq);
    ccsds.fill32(uint32_t(start));
    ccsds.fill32(uint32_t(end));
    ccsds.fill32(uint32_t(base));
    ccsds.fill(base,len>dumpPktSize?dumpPktSize:len);
    ccsds.finish(apid);
    sdStore.drain();
    base+=dumpPktSize;
    len-=dumpPktSize;
  }
  Serial.println("done.");
}

static void initMpu(int apid) {
  mpu6050.begin(gsens,asens,mpuBw,mpuSample);
  Serial.print("MPU6050");Serial.print(" identifier (should be '"); Serial.print("68");Serial.print("'): ");
  Serial.println(mpu6050.whoami(),HEX);
  ccsds.start(sdStore,apid,pktseq);
  mpu6050.fillConfig(ccsds);
  ccsds.finish(apid);
  sdStore.drain();

  //Priming read, not written anywhere but clears INT
  int16_t ax,ay,az,gx,gy,gz,mt;
  mpu6050.read(ax,ay,az,mt,gx,gy,gz);
  
  //On Pocketometer, set up CAP0.0 on pin P0.22 to capture (read) MPU INT
  set_pin(22,2,0);
  TCR(0,0)=0;
  TCCR(0)=(0x01 << (0*3)); //channel 3, 3 control bits per channel, capture on rising, not falling, no interrupt
}

static void initMag(int apid) {
  Serial.print("AK8975");
  ak8975.begin(&Wire1);
  Serial.println(" started");
  Serial.print("AK8975");Serial.print(" identifier (should be '");Serial.print("0x48");Serial.print("'): ");
  Serial.println(ak8975.whoami(),HEX,2);
  ccsds.start(sdStore,apid,pktseq);
  ak8975.fillConfig(ccsds);
  ccsds.finish(apid);
  sdStore.drain();
}

static void initBmp(int apid) {
  Serial.print("BMP180");Serial.print(".begin ");
  bool worked=bmp180.begin(&Wire1,2);
  Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(0);
  Serial.print("BMP180");Serial.print(" identifier (should be '"); Serial.print("55");Serial.print("'): ");
  Serial.println(bmp180.whoami(),HEX);

  bmp180.ouf=0;
  ccsds.start(sdStore,apid,pktseq);

  bmp180.fillCalibration(ccsds);
  ccsds.finish(apid);
  sdStore.drain();
}

void setup() {
  Serial.begin(4800);
  Serial.println(version_string);
  Serial.print("Hardware type: ");
  Serial.println(HW_TYPE());
  if(isRocketometer()) {
    Serial.print("Can't control a Rocketometer yet");
    blinklock(0x1234);
  }
  sd.p0=7;
  Serial.print("Using Logomatic hardware, sd.p0=");
  Serial.println(sd.p0);
  initSD();

  Serial.print("Wire1 ");
  Wire1.begin();
  Serial.println(" started");

  //Write processor config and firmware version
  writeVersion(0x01);
  sdStore.drain();

  dumpRom(source_start, source_end, 0x02, "source");
  dumpRom( image_start,  image_end, 0x03, "image" );

  //Read system config
  bool worked;
  ccsds.start(sdStore,0x04,pktseq);
  Serial.print("readconfig");Serial.print(".begin ");
  worked=readconfig.begin();
  Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(readconfig.errnum);
  if(!worked) blinklock(readconfig.errnum);
  ccsds.finish(0x04);
  sdStore.drain();
  
  initMpu(0x05); //MPU6050 has to be started first to set I2 bus to passthrough mode
  initMag(0x06);
  initBmp(0x07);

  ccsds.start(sdStore,0x08,pktseq);
  SDHC_info sdinfo;
  sd.get_info(sdinfo);
  sdinfo.print(Serial);
  sdinfo.fill(ccsds);
  ccsds.finish(0x08);
  sdStore.drain();
  Serial.println("Done with setup");
  Serial.flush();
  
}

//return true if sensor has updated the heading
bool collectMpuData(int apid) {
  static uint32_t gTC; ///< Last TC that we collected a measurement
         uint32_t gTCR; ///< Current TC capture value for this sensor
  gTCR=TCR(0,0);
  bool triggerOnInt=(gTC!=gTC);
  bool triggerOnTimeout=(mpuReadMs<msPassed(gTC));
  if(triggerOnInt | triggerOnTimeout) {
    gTC=triggerOnInt?gTCR:TTC(0);
    //Write an accelerometer fast packet
    int16_t ax,ay,az,gx,gy,gz,mt;
    mpu6050.read(ax,ay,az,gx,gy,gz,mt);
    ccsds.start(sdStore,apid,pktseq,gTC);
    ccsds.fill16(ax);ccsds.fill16(ay); ccsds.fill16(az); ccsds.fill16(gx); ccsds.fill16(gy); ccsds.fill16(gz); ccsds.fill16(mt); ccsds.fill(triggerOnInt?1:0); ccsds.fill(triggerOnTimeout?1:0);
    ccsds.finish(apid);
    return true;
  }
  return false;
}

void collectMagData(int apid) {
  if(ak8975.noblockMeasurement()) {
    uint32_t bTC=TTC(0);
    int16_t bx,by,bz;
    uint8_t st1,st2;
    ccsds.start(sdStore,apid,pktseq,bTC);
    ak8975.read(bx,by,bz,st1,st2);
    //Swap up magnetic axes so that they match MPU axes
    //MPU   Mag
    //+x   +y
    //+y   +x
    //+z   -z
    ccsds.fill(st1); ccsds.fill16(by);  ccsds.fill16(bx); ccsds.fill16(-bz); ccsds.fill(st2);  
    ccsds.finish(apid);
  }
}

void collectBmpData(int apid) {
  if(bmp180.noblockMeasurement()) {
    uint32_t bmpTC=TTC(0);
    uint16_t temperatureRaw=bmp180.getTemperatureRaw();
    uint32_t pressureRaw=bmp180.getPressureRaw();
    int16_t temperature=bmp180.getTemperature();
    uint32_t pressure=bmp180.getPressure();
    ccsds.start(sdStore,apid,pktseq,bmpTC);
    ccsds.fill16(temperatureRaw);
    ccsds.fill32(pressureRaw);
    ccsds.fillfp(fp(temperature)/10.0);
    ccsds.fill32(pressure);
    ccsds.finish(apid);
  }
}

bool collectGPS(int ppsApid, int nmeaApid, int ggaApid, int rmcApid) {
  bool hasGuide=false;
  while(Serial.available()>0) {
    char in=Serial.read();
    if(in=='$') {
      serialStore.mark();
      ccsds.start(sdStore,nmeaApid,pktseq,TTC(0));
      serialStore.drainPartial(sdStore);
      ccsds.finish(nmeaApid);
      Serial.write('p');
    }
    serialStore.fill(in);
//    Serial.write(in);
    gps.process(in);
  }
  if(gps.writeGGA) {
    ccsds.start(sdStore,ggaApid,pktseq,TTC(0));
    ccsds.fillfp(gps.HMS);
    ccsds.fillfp(gps.lat);
    ccsds.fillfp(gps.lon);
    ccsds.fillfp(gps.alt);
    ccsds.finish(ggaApid);
    gps.writeGGA=false;
    Serial.write('g');
  } else if(gps.writeRMC) {
    uint32_t TC=TTC(0);
    ccsds.start(sdStore,rmcApid,pktseq,TC);
    ccsds.fillfp(gps.HMS);
    ccsds.fillfp(gps.lat);
    ccsds.fillfp(gps.lon);
    ccsds.fillfp(gps.spd);
    ccsds.fillfp(gps.hdg);
    ccsds.fill32(gps.DMY);
    ccsds.finish(rmcApid);
    Serial.write('r');
    gps.writeRMC=false;
    hasGuide=true;
  }
  return hasGuide;
}

void loop() {
  if(f.size()>=maxLogSize) {
    closeLog();
    openLog();
    sdStore.fill(syncMark);
    sdStore.mark();
  }
  collectMpuData(0x09);
  collectMagData(0x0A);
  collectBmpData(0x0B);
  collectGPS(0x0C,0x0D,0x0E,0x0F);
  sdStore.drain();
}

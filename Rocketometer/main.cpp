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
#include "ccsds.h"
#include "sdhc.h"
#include "Partition.h"
#include "cluster.h"
#include "direntry.h"
#include "file.h"
#include "FileCircular.h"
#include "nmea.h"

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
volatile unsigned int drainTC0,drainTC1;

const uint32_t readPeriodMs=3;

CCSDS packet;
StateTwoWire<decltype(packet),1> Wire1;
SDHC<decltype(packet),decltype(Serial),decltype(SPI)> sd(SPI);
SDHC_info<decltype(packet),decltype(Serial)> sdinfo;
Partition<decltype(packet),decltype(Serial),decltype(SPI)> p(sd);
Cluster<decltype(packet),decltype(Serial),decltype(SPI)> fs(p);
File<decltype(packet),decltype(Serial),decltype(SPI)> f(fs);
Hd<decltype(Serial)> sector(Serial);
BMP180<decltype(packet),decltype(Serial),StateTwoWire<decltype(packet),1>> bmp180(Wire1);
HMC5883<decltype(packet),decltype(Wire1)> hmc5883(Wire1);
MPU6050<decltype(packet),StateTwoWire<decltype(packet),1>> mpu6050(Wire1,0);
AD799x<decltype(Wire1)> ad799x(Wire1);
const int dumpPktSize=120;
FileCircular<decltype(packet),decltype(Serial),decltype(SPI)> sdStore(f);
char measBuf[1024],serialBuf[1024];
Circular measStore(1024,measBuf),serialStore(1024,serialBuf);
unsigned short pktseq[32];
//NMEA gps;

static const int blockSize=SDHC<decltype(packet),decltype(Serial),decltype(SPI)>::BLOCK_SIZE;

const char syncMark[]="KwanSync";

static uint16_t log_i=0;

void openLog(uint16_t inc=1) {
  Serial.println(inc,DEC);
  Serial.println(log_i,DEC);
  if(inc==0) blinklock(108);
  static char fn[13];
  if(fn[0]!='r') strcpy(fn,"rkto0000.sds");
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

static const char version_string[]="Rocketometer v1.1 using Kwan FAT/SD library " __DATE__ " " __TIME__;

int16_t max,may,maz; //MPU60x0 acc
int16_t mgx,mgy,mgz; //MPU60x0 gyro
int16_t mt;          //MPU60x0 temp
int16_t bx,by,bz;    //compass (bfld)
uint16_t hx[4];      //HighAcc
bool wantPrint,wantBmp;
uint32_t TC,TC1;
int temperatureRaw,pressureRaw;
int16_t temperature;
int32_t pressure;

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
    #include "write_packet_pktOverflow.INC"
    oldOvr=measStore.getBufOverflow();
  }
  hmcPhase++;
  bmpPhase++;
  TC=TTC(0);
  vbus=gpio_read(23);
  mpu6050.read(max,may,maz,mgx,mgy,mgz,mt);
  ad799x.read(hx);
  TC1=TTC(0);
  #include "write_packet_rktoFast.INC"
  if(vbus!=old_vbus) {
    #include "write_packet_vbusPower.INC"
    old_vbus=vbus;
  }
  if(hmcPhase>=hmcMaxPhase) {
    //Only read the compass once every n times we read the 6DoF
    TC=TTC(0);
    hmc5883.read(bx,by,bz);
    //Sensor registers are in X, Z, Y order, not XYZ. Old code didn't know this,
    //wrote sensor registers in order, therefore wrote xzy order unintentionally.
    //We will keep this order to maintain compatibility with old data, including
    //flight 36.290
    #include "write_packet_mag.INC"
    hmcPhase=0;
  }
  if(bmpPhase>=bmpMaxPhase) {
    //Only read the pressure sensor once every n times we read the 6DoF
    bmpTC=TTC(0);  
    bmp180.noblockMeasurement();
    wantPrint=true;
    wantBmp=true;
    bmpPhase=0;
  }
  if(wantBmp && bmp180.noblockMeasurement()) {
    temperatureRaw=bmp180.getTemperatureRaw();
    pressureRaw=bmp180.getPressureRaw();
    temperature=bmp180.getTemperature();
    pressure=bmp180.getPressure();
    wantBmp=false;
    TC1=TTC(0);
    #include "write_packet_bmpData.INC"
  }
  //Why here? Because since creation of packets is interrupt driven, and there
  //is only a single buffer, only the interrupt routine is allowed to write
  if(writeDrain) {
    #include "write_packet_sdDrain.INC"
    writeDrain=false;
  }
  /* No GPS in flight 36.290 Rocketometer
  if(gps.writePPS) {
    packet.start(measStore,0x16,pktseq,gps.PPSTC);
    gps.writePPS=false;
    packet.finish(0x16);
  }
  if(gps.writeZDA) {
    packet.start(measStore,0x17,pktseq,TTC(0));
    packet.fill32(gps.zdaHMS);
    packet.fill(gps.zdaDD);
    packet.fill(gps.zdaMM);
    packet.fill(gps.zdaYYYY-2000);
    gps.writeZDA=false;
    packet.finish(0x17);
  }
  if(gps.writeGGA) {
    packet.start(measStore,0x18,pktseq,TTC(0));
    packet.fill32(gps.lat);
    packet.fill32(gps.lon);
    packet.fill32(gps.alt);
    packet.fill(gps.altScale);
    gps.writeGGA=false;
    packet.finish(0x18);
  }
  if(gps.writeVTG) {
    packet.start(measStore,0x19,pktseq,TTC(0));
    packet.fill32(gps.vtgCourse);
    packet.fill(gps.vtgCourseScale);
    packet.fill32(gps.vtgSpeedKt);
    packet.fill(gps.vtgSpeedKtScale);
    gps.writeVTG=false;
    packet.finish(0x19);
  } */
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
  Serial.begin(115200);
  Serial.println(version_string);
  Wire1.begin();

  bool worked;
  sd.p0=15; //Note that this is different from the Loginator and Logomatic
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
    #include "write_packet_source.INC"
    sdStore.drain(); 
    base+=dumpPktSize;
    len-=dumpPktSize;
  }

  packet.start(sdStore,0x12);
  sdinfo.fill(packet);
  packet.finish(0x12);
  sdStore.drain(); 

  mpu6050.begin(3,3);
  Serial.print("MPU6050 identifier (should be 0x68): 0x");
  Serial.println(mpu6050.whoami(),HEX);
  packet.start(sdStore,0x0F);
  mpu6050.fillConfig(packet);
  packet.finish(0x0F);
  sdStore.drain();

  hmc5883.begin();
  char HMCid[4];
  hmc5883.whoami(HMCid);
  Serial.print("HMC5883L identifier (should be 'H43'): ");
  Serial.println(HMCid);
  packet.start(sdStore,0x0E);
  hmc5883.fillConfig(packet);
  packet.finish(0x0E);
  sdStore.drain();

  char channels=0x0B;
  worked=ad799x.begin(channels);
  Serial.print("ad799x");Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(0);
  if(!worked) blinklock(0xAAAA5555);
  packet.start(sdStore,0x0D);
  packet.fill(ad799x.getAddress());
  packet.fill(channels);
  packet.fill(ad799x.getnChannels());
  packet.fill((uint8_t)worked);
  packet.finish(0x0D);
  sdStore.drain();

  worked=bmp180.begin(2);
  Serial.print("bmp180");Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(0);
  Serial.print("BMP180 identifier (should be 0x55): 0x");
  Serial.println(bmp180.whoami(),HEX);
  if(!worked) blinklock(0x5555AAAA);
  bmp180.printCalibration(Serial);

  bmp180.ouf=0;
  packet.start(sdStore,0x02);

  bmp180.fillCalibration(packet);

  packet.finish(0x02);
  sdStore.drain();

  #include "write_packet_version.INC"
  sdStore.drain();

  //Set USB_ON to GPIO read
  set_pin(23,0,0);
  // gps.begin();
  Serial.println("t,tc,bx,by,bz,max,may,maz,mgx,mgy,mgz,mt,h0,h1,h2,h3,T,P,vbus,ovr");
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
    Serial.print(RTCHOUR(),DEC,2);
    Serial.print(":");Serial.print(RTCMIN(),DEC,2);
    Serial.print(":");Serial.print(RTCSEC(),DEC,2);
    Serial.print(".");Serial.print(CTC()&0x7FFF,HEX,4);
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
    Serial.print(",");Serial.print(vbus, DEC); 
    Serial.print(",");Serial.print(DirEntry<decltype(packet),decltype(Serial),decltype(SPI)>::packTime(RTCHOUR(),RTCMIN(),RTCSEC()),HEX,4);
    Serial.print(",");Serial.print(DirEntry<decltype(packet),decltype(Serial),decltype(SPI)>::packDate(RTCYEAR(),RTCMONTH(),RTCDOM()),HEX,4);
    Serial.println();
    wantPrint=false;
  }
//  gps.process();
}



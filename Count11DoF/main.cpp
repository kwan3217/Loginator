#include <string.h>
#include "Time.h"
#include "LPCduino.h"
#include "StateTwoWire.h"
#include "hmc5883.h"
#include "Serial.h"
#include "HardSPI.h"
#include "l3g4200d.h"
#include "sdhc.h"
#include "Partition.h"
#include "cluster.h"
#include "direntry.h"
#include "file.h"
#include "FileCircular.h"
#include "dump.h"
#include "nmea.h"
const char syncMark[]="KwanSync";
static const char version_string[]="Project Yukari v0.0 " __DATE__ " " __TIME__;
const int dumpPktSize=120;

StateTwoWire Wire1(1);
HMC5883 hmc5883(Wire1);
SDHC sd(&SPI,7);
L3G4200D gyro(&SPI1,25);
SDHC_info sdinfo;
Partition p(sd);
Cluster fs(p);
File f(fs);
FileCircular sdStore(f);
char serialBuf[1024];
Circular serialStore(1024,serialBuf);
CCSDS ccsds;
NMEA gps;

uint16_t log_i=0;

const uint32_t readPeriodMs=100;
unsigned int bTC,gTC;
int16_t bx,by,bz,gx,gy,gz;

void collectMagData() {
  bTC=TTC(0);
  hmc5883.read(bx,by,bz);
  ccsds.start(sdStore,0x04,0,bTC);
  //Sensor registers are in X, Z, Y order, not XYZ. Old code didn't know this,
  //wrote sensor registers in order, therefore wrote xzy order unintentionally.
  //We will keep this order to maintain compatibility with old data, including
  //flight 36.290
  ccsds.fill16(bx);  ccsds.fill16(bz);  ccsds.fill16(by);
  ccsds.finish(0x04);

//  directTaskManager.reschedule(1,readPeriodMs,0,collectData,0); 
}

bool collectGyroData() {
  unsigned int gTCR=TCR(0,3);
  if(gTCR!=gTC) {
    gTC=gTCR;
    uint8_t t,status;
    gyro.read(gx,gy,gz,t,status);
    unsigned int gTC1=TTC(0);
//    if(status!=0) {
      ccsds.start(sdStore,0x21,0,gTC);
      ccsds.fill16(gx);  ccsds.fill16(gy);  ccsds.fill16(gz); ccsds.fill(t); ccsds.fill(status); ccsds.fill32(gTC1);
      ccsds.finish(0x21);
      return true;
//    }
  }
  return false;
}

void openLog(uint16_t inc=1) {
  Serial.println(inc,DEC);
  Serial.println(log_i,DEC);
  if(inc==0) blinklock(108);
  static char fn[13];
  if(fn[0]!='r') strcpy(fn,"yukari00.sds");
  Serial.println(fn);
  while(log_i<99 && f.find(fn)) {
    log_i+=inc;
//    fn[4]='0'+log_i/1000;
//    fn[5]='0'+(log_i%1000)/100;
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

void initSD() {
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

  openLog();
  sdStore.fill(syncMark);
  sdStore.mark();

}

void initHMC5883() {
  hmc5883.begin();
  Serial.println("HMC5883 started");
  char HMCid[4];
  hmc5883.whoami(HMCid);
  Serial.print("HMC5883L identifier (should be 'H43'): ");
  Serial.println(HMCid);
  ccsds.start(sdStore,0x0E);
  hmc5883.fillConfig(ccsds);
  ccsds.finish(0x0E);
  sdStore.drain();
}

void initGyro() {
  gyro.begin(); //Use default sensitivity
  Serial.println("L3G4200D started");
  uint8_t gyroId=gyro.whoami();
  Serial.print("L3G4200D identifier (should be 'D3'): ");
  Serial.println(gyroId,HEX,2);
  ccsds.start(sdStore,0x20);
  gyro.fillConfig(ccsds);
  ccsds.finish(0x20);
  sdStore.drain();
  //Priming read, not written anywhere but clears INT2
  int16_t gx,gy,gz;
  uint8_t t,status;
  gyro.read(gx,gy,gz,t,status);
  //set up CAP0.3 on pin P0.29 (Loginator AD2, 11DoF IG) to capture (read) gyro INT2
  set_pin(29,2,0);
  gTC=0;
  TCR(0,3)=0;
  TCCR(0)=(0x01 << (3*3)); //channel 3, 3 control bits per channel, capture on rising, not falling, no interrupt
}

void setup() {
  Serial.begin(4800);
  Serial.println(version_string);
  initSD();

  //Dump code to serial port and packet file
  int len=source_end-source_start;
  char* base=source_start;
  while(len>0) {
    ccsds.start(sdStore,0x03);
    ccsds.fill16(base-source_start);
    ccsds.fill(base,len>dumpPktSize?dumpPktSize:len);
    ccsds.finish(0x03);
    Serial.print(".");
    sdStore.drain(); 
    base+=dumpPktSize;
    len-=dumpPktSize;
  }
  Serial.println("Done with dump");
  ccsds.start(sdStore,0x12);
  sdinfo.fill(ccsds);
  ccsds.finish(0x12);
  sdStore.drain(); 
  
  //Must be done before the I2C sensors are activated (Compass and BMP)  
  Wire1.begin();
  Serial.println("Wire1 started");

  initHMC5883();

  //Must be done before the SPI sensors are activated (Gyro and Acc)
  SPI1.begin(1000000,1,1);
  Serial.println("SPI1 started");

  initGyro();

  Serial.flush();
}

void flushPackets() {
//  static int flickerState=0;
  if(sdStore.drain()) {
//    flickerState=1-flickerState;
//    set_pin(8,0,1);
//    gpio_write(8,flickerState);
  }
  if(sdStore.errno!=0) { 
    Serial.print("Problem writing file: sdStore.errno=");
    Serial.println(sdStore.errno);
    blinklock(sdStore.errno);
  }
}

void collectGPS() {
  while(Serial.available()>0) {
    char in=Serial.read();
    if(in=='$') {
      serialStore.mark();
      ccsds.start(sdStore,0x1A,0,TTC(0));
      serialStore.drainPartial(sdStore);
      ccsds.finish(0x1A);
      Serial.write('p');
    }
    serialStore.fill(in);
    Serial.write(in);
    gps.process(in);
  }
  if(gps.writeGGA) {
    ccsds.start(sdStore,0x18,0,TTC(0));
    ccsds.fill32(gps.HMS);
    ccsds.fill32(gps.lat);
    ccsds.fill32(gps.lon);
    ccsds.fill32(gps.alt);
    ccsds.fill32(gps.altScale);
    ccsds.finish(0x18);
    gps.writeGGA=false;
    Serial.write('g');
  } else if(gps.writeRMC) {
    ccsds.start(sdStore,0x19,0,TTC(0));
    ccsds.fill32(gps.HMS);
    ccsds.fill32(gps.lat);
    ccsds.fill32(gps.lon);
    ccsds.fill32(gps.spd);
    ccsds.fill32(gps.spdScale);
    ccsds.fill32(gps.hdg);
    ccsds.fill32(gps.hdgScale);
    ccsds.fill32(gps.DMY);
    ccsds.finish(0x19);
    Serial.write('r');
    gps.writeRMC=false;
  }
}

void loop() {
  static unsigned int magPhase=0;
  const unsigned int magMaxPhase=10;
  if(collectGyroData()) {
    if(magPhase>=magMaxPhase) {
      collectMagData();
      magPhase=0;
    }
    magPhase++;
  }
  flushPackets();
  collectGPS();
  flushPackets();
}

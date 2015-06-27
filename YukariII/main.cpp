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
#include "readconfig.h"
#include "config.h"
#include "navigate.h"
#include "guide.h"
#include "control.h"
#include "pwm.h"
#include "kalman.h"

const char syncMark[]="KwanSync";
static const char version_string[]="Project Yukari v1.0 ";// __DATE__ " " __TIME__;
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
ReadConfig readconfig(fs,ccsds);
Config config;
Navigate nav(config);
Guide guide(nav,config);
Control control(nav,guide,config);

const char* const ReadConfig::tagNames[]={"GSENS","GODR","GBW","P","PS","I","IS","D","DS","WPDLAT","WPDLON","THR","YSCL",nullptr};
const int   ReadConfig::tagTypes[]={
ReadConfig::typeInt, //GSENS
ReadConfig::typeInt, //GODR
ReadConfig::typeInt, //GBW
ReadConfig::typeInt, //P
ReadConfig::typeInt, //PS
ReadConfig::typeInt, //I
ReadConfig::typeInt, //IS
ReadConfig::typeInt, //D
ReadConfig::typeInt, //DS
ReadConfig::typeFp,//WPDLAT
ReadConfig::typeFp,//WPDLON
ReadConfig::typeInt, //THR
ReadConfig::typeInt  //YSCL
};
void *const  ReadConfig::tagDatas[]={&config.gyroSens,&config.gyroODR,&config.gyroBW,
&config.P,&config.Ps,
&config.I,&config.Is,
&config.D,&config.Ds,
config.dlatWaypoint,
config.dlonWaypoint,
&config.throttle,
&config.yscl
};
int *const  ReadConfig::tagSizes[]={nullptr,nullptr,nullptr,
nullptr,nullptr,
nullptr,nullptr,
nullptr,nullptr,
&config.nWaypoints,
&config.nWaypoints,
nullptr,
nullptr};

const unsigned char channelSteer=6;
const unsigned char channelThrottle=4;
const int right=-1;
const int left=1;
const unsigned char channelMask=(1<<channelSteer)|(1<<channelThrottle);

uint16_t log_i=0;

const uint32_t readPeriodMs=100;
unsigned int bTC,gTC;
int16_t bx,by,bz,gx,gy,gz;

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

void collectMagData() {
  static unsigned int magPhase=0;
  const unsigned int magMaxPhase=10;
  if(magPhase>=magMaxPhase) {
    bTC=TTC(0);
    hmc5883.read(bx,by,bz);
    ccsds.start(sdStore,0x04,0,bTC);
    //Sensor registers are in X, Z, Y order, not XYZ. Old code didn't know this,
    //wrote sensor registers in order, therefore wrote xzy order unintentionally.
    //We will keep this order to maintain compatibility with old data, including
    //flight 36.290
    ccsds.fill16(bx);  ccsds.fill16(bz);  ccsds.fill16(by);
    ccsds.finish(0x04);
    magPhase=0;
  }
  magPhase++;
}

//return true if sensor has updated the heading
bool collectGyroData() {
  unsigned int gTCR=TCR(0,3);
  if(gTCR!=gTC) {
    gTC=gTCR;
    uint8_t t,status;
    gyro.read(gx,gy,gz,t,status);
    unsigned int gTC1=TTC(0);
    nav.handleGyro(gTC,gx,gy,gz);
    ccsds.start(sdStore,0x21,0,gTC);
    ccsds.fill16(gx);  ccsds.fill16(gy);  ccsds.fill16(gz); ccsds.fill(t); ccsds.fill(status); ccsds.fill32(gTC1);
    ccsds.finish(0x21);
    ccsds.start(sdStore,0x24,0,gTC);
    ccsds.fillfp(nav.gyroT);   ccsds.fillfp(nav.lastT);   ccsds.fillfp(nav.deltaT); ccsds.fillfp(nav.sens);
    ccsds.fillfp(nav.calGx);   ccsds.fillfp(nav.calGy);   ccsds.fillfp(nav.calGz);
    ccsds.fillfp(nav.e.x);     ccsds.fillfp(nav.e.y);     ccsds.fillfp(nav.e.z);     ccsds.fillfp(nav.e.w);
    ccsds.fillfp(nav.nose_r.x);ccsds.fillfp(nav.nose_r.y);ccsds.fillfp(nav.nose_r.z);ccsds.fillfp(nav.nose_r.w);
    ccsds.fillfp(nav.gyroHdg);
    ccsds.fillfp(nav.hdg);
    ccsds.finish(0x24);
    flushPackets();
    return nav.hasInit;
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

//  sd.get_info(sdinfo);
//  sdinfo.print(Serial);
  worked=p.begin(1);
  Serial.print("p");     Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(p.errno);
//  p.print(Serial);
  if(!worked) blinklock(p.errno);

  worked=fs.begin();  
  Serial.print("fs");    Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(fs.errno);
//  sector.begin();
//  fs.print(Serial);//,sector);
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
  gyro.begin(config.gyroSens,config.gyroODR,config.gyroBW); //Use sensitivity specified in config file. Actual
                               //sensitivity is 250*2^config.gyroSens deg/s full scale
  Serial.println("L3G4200D started");
  uint8_t gyroId=gyro.whoami();
  Serial.print("L3G4200D identifier (should be 'D3'): ");
  Serial.println(gyroId,HEX,2);
  nav.setSens(config.gyroSens);
  ccsds.start(sdStore,0x20);
  gyro.fillConfig(ccsds);
  ccsds.finish(0x20);
  sdStore.drain();
  //Priming read, not written anywhere but clears INT2
  uint8_t t,status;
  gyro.read(gx,gy,gz,t,status);
  //set up CAP0.3 on pin P0.29 (Loginator AD2, 11DoF IG) to capture (read) gyro INT2
  set_pin(29,2,0);
  gTC=0;
  TCR(0,3)=0;
  TCCR(0)=(0x01 << (3*3)); //channel 3, 3 control bits per channel, capture on rising, not falling, no interrupt
}

void writeVersion() {
  ccsds.start(sdStore,0x0C);
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
  ccsds.finish(0x0C);
  sdStore.drain();
}

void setupWaypointPacket() {
  ccsds.start(sdStore,0x27,0,TTC(0));
  ccsds.fill(guide.currentBaseWaypoint);
  ccsds.fill(guide.target);
  ccsds.fillfp(config.dlatWaypoint[guide.currentBaseWaypoint]);
  ccsds.fillfp(config.dlonWaypoint[guide.currentBaseWaypoint]);
  ccsds.fillfp(config.dlatWaypoint[guide.target]);
  ccsds.fillfp(config.dlonWaypoint[guide.target]);
  ccsds.fillfp(guide.ddlatBasepath);
  ccsds.fillfp(guide.ddlonBasepath);
  ccsds.fillfp(guide.distBasepath);
  ccsds.fillfp(guide.nlatBasepath);
  ccsds.fillfp(guide.nlonBasepath);
  ccsds.fillfp(guide.dlatSteerTo);
  ccsds.fillfp(guide.dlonSteerTo);
  ccsds.finish(0x27);
  flushPackets();
}

void GyroPacket() {
}

void setup() {
  Serial.begin(4800);
  Serial.println(version_string);
//  volatile int theta=45;
//  Serial.print(sint(theta));
  initSD();

  //Write processor config and firmware version
  writeVersion();
  sdStore.drain(); 

  //Read system config
  bool worked;
  ccsds.start(sdStore,0x22);
  worked=readconfig.begin();
  ccsds.finish(0x22);
  sdStore.drain(); 
  ccsds.start(sdStore,0x28);
  ccsds.fill32(config.gyroSens);
  ccsds.fill32(config.gyroODR);
  ccsds.fill32(config.gyroBW);
  ccsds.fill32(config.P);
  ccsds.fill32(config.Ps);
  ccsds.fill32(config.I);
  ccsds.fill32(config.Is);
  ccsds.fill32(config.D);
  ccsds.fill32(config.Ds);
  ccsds.fill32(config.nWaypoints);
  for(int i=0;i<=config.nWaypoints;i++) {
    config.dlonWaypoint[i]*=clat;
    ccsds.fillfp(config.dlatWaypoint[i]);
    ccsds.fillfp(config.dlonWaypoint[i]);
  }
  ccsds.finish(0x28);
  Serial.print("readconfig");Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(readconfig.errno);
  if(!worked) blinklock(readconfig.errno);

  //Convert PID coefficients into real values
  guide.begin();
  setupWaypointPacket();
  control.begin();

  //Dump code to serial port and packet file
  int len=source_end-source_start;
  char* base=source_start;
  while(len>0) {
    ccsds.start(sdStore,0x03);
    ccsds.fill16(base-source_start);
    ccsds.fill(base,len>dumpPktSize?dumpPktSize:len);
    ccsds.finish(0x03);
//    Serial.print(".");
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

  //Init PWM
  initPWM(channelMask); 

  Serial.flush();
}

bool collectGPS() {
  bool hasGuide=false;
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
    ccsds.start(sdStore,0x38,0,TTC(0));
    ccsds.fillfp(gps.HMS);
    ccsds.fillfp(gps.lat);
    ccsds.fillfp(gps.lon);
    ccsds.fillfp(gps.alt);
    ccsds.finish(0x38);
    gps.writeGGA=false;
    Serial.write('g');
  } else if(gps.writeRMC) {
    uint32_t TC=TTC(0);
    nav.handleRMC(TC,gps.lat,gps.lon,gps.spd,gps.hdg);
    ccsds.start(sdStore,0x39,0,TC);
    ccsds.fillfp(gps.HMS);
    ccsds.fillfp(gps.lat);
    ccsds.fillfp(gps.lon);
    ccsds.fillfp(gps.spd);
    ccsds.fillfp(gps.hdg);
    ccsds.fill32(gps.DMY);
    ccsds.finish(0x39);
    Serial.write('r');
    gps.writeRMC=false;
    hasGuide=true;
  }
  return hasGuide;
}

void navigatePacket() {
  ccsds.start(sdStore,0x25,0,gTC);
  ccsds.fillfp(nav.gyroT);   
  ccsds.fillfp(nav.deltaT); 
  ccsds.fillfp(nav.gyroHdg);
  ccsds.fillfp(nav.rmcHdg);
  ccsds.fillfp(nav.dHdg);
  ccsds.fillfp(nav.hdgOfs);
  ccsds.fillfp(nav.hdg);
  ccsds.fillfp(nav.rmcSpd);
  ccsds.finish(0x25);
  flushPackets();
}

void guidePacket() {
  ccsds.start(sdStore,0x26,0,TTC(0));
  ccsds.fillfp(nav.firstLat);
  ccsds.fillfp(nav.firstLon);
  ccsds.fillfp(nav.lat);
  ccsds.fillfp(nav.lon);
  ccsds.fillfp(nav.dLat);
  ccsds.fillfp(nav.dLon);
  ccsds.fillfp(guide.dlatSteerTo);
  ccsds.fillfp(guide.dlonSteerTo);
  ccsds.fillfp(guide.ddlatSteerTo);
  ccsds.fillfp(guide.ddlonSteerTo);
  ccsds.fillfp(guide.desiredHdg);
  ccsds.fillfp(guide.dotp);
  ccsds.fillfp(guide.ddlatToGo);
  ccsds.fillfp(guide.ddlonToGo);
  ccsds.finish(0x26);
  flushPackets();
}

void controlPacket() {
  ccsds.start(sdStore,0x23,0,TTC(0));
  ccsds.fillfp(nav.hdg);
  ccsds.fillfp(guide.desiredHdg);
  ccsds.fillfp(control.hdgError);
  ccsds.fillfp(control.hdgRate); 
  ccsds.fillfp(control.hdgIntError);
  ccsds.fillfp(control.P);
  ccsds.fillfp(control.I);
  ccsds.fillfp(control.D);
  ccsds.fillfp(control.ufsteerCmd);
  ccsds.fill(control.steerCmd);
  ccsds.finish(0x23);
  flushPackets();
}

void steer() {
  setServo(channelSteer,-control.steerCmd);
  if(nav.hasInit) setServo(channelThrottle,guide.runFinished?0:config.throttle); //Stomp the gas or hit the brakes
}

void loop() {
  bool hasNav=collectGyroData();
  hasNav|=collectGPS();
  if(hasNav) {
    collectMagData();
    navigatePacket();
    if(nav.hasRMC) {
      guide.guide();
      guidePacket();
    }
    control.control();
    steer();
    controlPacket();
  }
}

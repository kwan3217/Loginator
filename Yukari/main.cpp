#include <string.h>
#include "Time.h"
#include "LPCduino.h"
#include "StateTwoWire.h"
#include "hmc5883.h"
#include "float.h"
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
#include "ccsds.h"
#include "guide.h"
#include "control.h"
#include "pwm.h"
//#include "kalman.h"
#include "YukariConfig.h"

const char syncMark[]="KwanSync";
static const char version_string[]="Project Yukari v4.0 " __DATE__ " " __TIME__;
const int dumpPktSize=120;

CCSDS packet;
HMC5883 hmc5883;
SDHC sd;
L3G4200D gyro;
SDHC_info sdinfo;
Partition p(sd);
Cluster fs(p);
File f(fs);
FileCircular sdStore(f);
char serialBuf[1024];
Circular serialStore(1024,serialBuf);
NMEA gps;
ReadConfig readconfig(fs,packet);
Navigate nav(config);
Guide guide(nav,config);
Control control(nav,guide,config);

const unsigned char channelMask=(1<<channelSteer)|(1<<channelThrottle);

uint16_t log_i=0;

const uint32_t readPeriodMs=100;
unsigned int ppsTC,btnTC;
int16_t bx,by,bz,gx,gy,gz;

void flushPackets() {
//  static int flickerState=0;
  if(sdStore.drain()) {
//    flickerState=1-flickerState;
//    set_pin(8,0,1);
//    gpio_write(8,flickerState);
  }
  if(sdStore.errnum!=0) { 
    Serial.print("Problem writing file: sdStore.errnum=");
    Serial.println(sdStore.errnum);
    blinklock(sdStore.errnum);
  }
}

void collectMagData() {
  static unsigned int magPhase=0;
  const unsigned int magMaxPhase=10;
  static unsigned int bTC;
  if(magPhase>=magMaxPhase) {
    bTC=TTC(0);
    hmc5883.read(bx,by,bz);
    //Sensor registers are in X, Z, Y order, not XYZ. Old code didn't know this,
    //wrote sensor registers in order, therefore wrote xzy order unintentionally.
    //We will keep this order to maintain compatibility with old data, including
    //flight 36.290
    #include "write_packet_mag.INC"
    magPhase=0;
  }
  magPhase++;
}

//return true if sensor has updated the heading
bool collectGyroData() {
  unsigned int gTCR=TCR(0,channelTCGyro);
  static unsigned int gTC;
  if(gTCR!=gTC) {
    gTC=gTCR;
    uint8_t t,status;
    gyro.read(gx,gy,gz,t,status);
    unsigned int gTC1=TTC(0);
    Vector<3> meas;
    meas[0]=gx;
    meas[1]=gy;
    meas[2]=gz;
    nav.handleGyro(gTC,meas);
    #include "write_packet_nav2.INC"
    flushPackets();
    return nav.hasButton;
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
  Serial.print("f.openw(\"");Serial.print(fn);Serial.print("\"): ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(f.errnum);
  if(!worked) blinklock(f.errnum);
}

void closeLog() {
  f.close(); //Sync the directory entry - after this, we may reuse this file
               //object on a new file.
}

void initSD() {
  bool worked;
  sd.p0=7;
  Serial.print("sd.p0=");
  Serial.println(sd.p0);
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
  packet.start(sdStore,0x0E);
  hmc5883.fillConfig(packet);
  packet.finish(0x0E);
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
  packet.start(sdStore,0x20);
  gyro.fillConfig(packet);
  packet.finish(0x20);
  sdStore.drain();
  //Priming read, not written anywhere but clears INT2
  uint8_t t,status;
  gyro.read(gx,gy,gz,t,status);
  //set up CAP0.3 on pin P0.29 (Loginator AD2, 11DoF IG) to capture (read) gyro INT2
  setup_cap(29,true,false,false);
}

void initButtons() {
  //PPS is wired to AD5, P0.22, CAP0.0
  setup_cap(22,true,false,false);
  setup_cap(22,0x01);
  ppsTC=0;
  //Use the STOP button to start (I know). STOP is wired to P0.16, CAP0.2
  setup_cap(16,true,false,false);
  btnTC=0;
}

void writeVersion() {
  #include "write_packet_version.INC"
  sdStore.drain();
}

void setupWaypointPacket(int context) {
  #include "write_packet_wpt.INC"
  flushPackets();
}

void setup() {
  setup_clock();
  Serial.begin(4800);
  Serial.println(version_string);
  initSD();

  //Write processor config and firmware version
  writeVersion();
  sdStore.drain();

  //Read system config
  bool worked;
  packet.start(sdStore,0x22);
  worked=readconfig.begin();
  packet.finish(0x22);
  sdStore.drain();
//  volatile int deg=45;
//  Serial.print(sint(deg));
  packet.start(sdStore,0x28);
  #include "write_packet_parseconfig.INC"
  for(int i=0;i<=config.nWaypoints;i++) {
    packet.fillfp(config.waypoint[i][0]);
    packet.fillfp(config.waypoint[i][1]);
  }
  packet.finish(0x28);
  Serial.print("readconfig");Serial.print(".begin ");Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(readconfig.errnum);
  if(!worked) blinklock(readconfig.errnum);

  guide.begin();
  setupWaypointPacket(0);
  control.begin();

  packet.start(sdStore,0x12);
  sdinfo.fill(packet);
  packet.finish(0x12);
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

  initButtons();
}

bool collectGPS() {
  bool hasGuide=false;
  while(Serial.available()>0) {
    char in=Serial.read();
    if(in=='$') {
      serialStore.mark();
      packet.start(sdStore,0x1A,TTC(0));
      serialStore.drainPartial(sdStore);
      packet.finish(0x1A);
      Serial.write('p');
    }
    serialStore.fill(in);
    Serial.write(in);
    gps.process(in);
  }
  if(gps.writeGGA) {
//    #include "write_packet_gga.inc"
    gps.writeGGA=false;
    Serial.write('g');
  } else if(gps.writeRMC) {
    uint32_t TC=TTC(0);
    nav.handleRMC(TC,gps.lat,gps.lon,gps.spd,gps.hdg);
//    #include "write_packet_rmc.inc"
    Serial.write('r');
    gps.writeRMC=false;
    hasGuide=true;
  }
  return hasGuide;
}

bool collectEncoder() {
  return nav.handleEncoder(TTC(0),analogRead(adEncoder0),analogRead(adEncoder1));
}

void guidePacket() {
  #include "write_packet_guide.INC"
  flushPackets();
}

void controlPacket() {
  #include "write_packet_control.INC"
  flushPackets();
}

static fp time() {
  static unsigned int oldTTC=0;
  static fp min=0;
  unsigned int ttc=TTC(0);
  if(oldTTC>ttc) {
    min++;
  }
  oldTTC=ttc;
  return min*60+fp(ttc)/fp(PCLK);
}

void steer() {
  setServo(channelSteer,   control.steerCmd);
  setServo(channelThrottle,control.throttleCmd*forward); //Stomp the gas or hit the brakes
}

void steerSeq() {
  if(time()>0) {
    static bool done=false;
    if(!done) {
      Serial.print(time());
      Serial.println("cmd0");
      setServo(channelThrottle,0);
      setServo(channelSteer,0);
      done=true;
    }
  }
  if(time()>1) {
    static bool done=false;
    if(!done) {
      Serial.print(time());
      Serial.println("cmd1");
      setServo(channelThrottle,config.throttle*forward);
      done=true;
    }
  }
  if(time()>15) {
    static bool done=false;
    if(!done) {
      Serial.print(time());
      Serial.println("cmd15");
      setServo(channelSteer,127*right);
      done=true;
    }
  }
  if(time()>16) {
    static bool done=false;
    if(!done) {
      Serial.print(time());
      Serial.println("cmd16");
      setServo(channelSteer,0);
      done=true;
    }
  }
  if(time()>30) {
    static bool done=false;
    if(!done) {
      Serial.print(time());
      Serial.println("cmd30");
      setServo(channelThrottle,0);
      done=true;
    }
  }
}

void loop() {
  if(ppsTC!=TCR(0,channelTCPPS)) {
    #include "write_packet_pps.INC"
    flushPackets();
    ppsTC=TCR(0,channelTCPPS);
    nav.handlePPS(ppsTC);
  }
  if(btnTC!=TCR(0,channelTCBtn)) {
    Serial.println("Button!");
    #include "write_packet_button.INC"
    flushPackets();
    btnTC=TCR(0,channelTCBtn);
    nav.buttonPress();
    guide.buttonPress();
    control.buttonPress();
  }
  bool hasNav=collectGyroData();
  hasNav|=collectGPS();
  hasNav|=collectEncoder();
#ifndef LPC2148
  static int len=_binary_source_cpio_max1_zpaq_end-_binary_source_cpio_max1_zpaq_start;
  char* const sourceStart=_binary_source_cpio_max1_zpaq_start;
#else
  static int len=source_end-source_start;
  char* const sourceStart=source_start;
#endif
  static char* base=sourceStart;
  steer();
  if(hasNav) {
    collectMagData();
    if(nav.hasRMC) {
      guide.guide();
      if(guide.hasNewWaypoint) {
        setupWaypointPacket(1);
        guide.hasNewWaypoint=false;
      }
      guidePacket();
    }
    control.control();
    steer();
    controlPacket();
    //Dump code to serial port and packet file
  } else if(len>0) {
    #include "write_packet_source.INC"
    sdStore.drain();
    base+=dumpPktSize;
    len-=dumpPktSize;
  }
}



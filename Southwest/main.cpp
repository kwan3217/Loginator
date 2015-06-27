#include <string.h>
#include "Time.h"
#include "LPCduino.h"
#include "StateTwoWire.h"
#include "hmc5883.h"
#include "Serial.h"
#include "HardSPI.h"
#include "l3g4200d.h"
#include "adxl345.h"
#include "mpu60x0.h"
#include "bmp180.h"
#include "sdhc.h"
#include "Partition.h"
#include "cluster.h"
#include "direntry.h"
#include "file.h"
#include "FileCircular.h"
#include "dump.h"
#include "nmea.h"
#include "readconfig.h"
#include "ad799x.h"

const char syncMark[]="KwanSync";
static const char version_string[]="Project Southwest v0.0 " __DATE__ " " __TIME__;
const int dumpPktSize=120;

StateTwoWire Wire0(0),Wire1(1);
HMC5883 hmc5883(&Wire1);
BMP180 bmp180(&Wire1);
AD799x ad799x(Wire0); //If we use this, it is because we are using the rocketometer which only uses i2c0
SDHC sd;
L3G4200D gyro(&SPI1,25);
ADXL345 acc(&SPI1,20);
MPU6050 mpu6050(Wire1,0);
SDHC_info sdinfo;
Partition p(sd);
Cluster fs(p);
File f(fs);
FileCircular sdStore(f);
char serialBuf[1024];
Circular serialStore(1024,serialBuf);
CCSDS ccsds;
NMEA gps;

int gyroSens=3;
int gyroODR=3;
int gyroBW=3;

const char* const ReadConfig::tagNames[]={"GSENS","GODR","GBW","\0"};
const int   ReadConfig::tagTypes[]={ReadConfig::typeInt,ReadConfig::typeInt,ReadConfig::typeInt};
void *const ReadConfig::tagDatas[]={&gyroSens,&gyroODR,&gyroBW};
int *const  ReadConfig::tagSizes[]={nullptr,nullptr,nullptr};
ReadConfig readconfig(fs,ccsds);

uint16_t log_i=0;

const uint32_t readPeriodMs=100;
unsigned int bTC,gTC;
int16_t ax,ay,az,mt,bx,by,bz,gx,gy,gz;
uint16_t hx[4];      //HighAcc

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

static inline bool isRocketometer() {
  return HW_TYPE_ROCKETOMETER==HW_TYPE();
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

static unsigned int bmpPhase=0;
const unsigned int bmpMaxPhase=200;

void collectBmpData() {
  static uint32_t bmpTC;
  if(bmpPhase>=bmpMaxPhase) {
    //Only read the pressure sensor once every n times we read the 6DoF
    bmpTC=TTC(0);
    bmp180.takeMeasurement();
    bmpPhase=0;
    Serial.print("b");
    uint16_t temperatureRaw=bmp180.getTemperatureRaw();
    uint32_t pressureRaw=bmp180.getPressureRaw();
    int16_t temperature=bmp180.getTemperature();
    uint32_t pressure=bmp180.getPressure();
    uint32_t TC1=TTC(0);
    ccsds.start(sdStore,0x0A,0,bmpTC);
    ccsds.fill16(temperatureRaw);
    ccsds.fill32(pressureRaw);
    ccsds.fill16(temperature);
    ccsds.fill32(pressure);
    ccsds.fill32(TC1);
    ccsds.finish(0x0A);
    flushPackets();
//    Serial.print("T*10: ");Serial.println((short)temperature,DEC);
//    Serial.print("P:    ");Serial.println((unsigned int)pressure,DEC);
  }
  bmpPhase++;
}

//return true if sensor has updated the heading
bool collectAccGyroData() {
  unsigned int gTCR;
  if(isRocketometer()) {
    gTCR=timer1_to_timer0(TCR(1,2));
  } else {
    gTCR=TCR(0,3);
  }
  if(gTCR!=gTC) {
    gTC=gTCR;
    uint8_t t,status;
    if(isRocketometer()) {
      //Write an accelerometer fast packet
      mpu6050.read(ax,ay,az,gx,gy,gz,mt);
      ad799x.read(hx);
      uint32_t TC1=TTC(0);
      ccsds.start(sdStore,0x10,0,gTC);
      ccsds.fill16(ax);ccsds.fill16(ay); ccsds.fill16(az); ccsds.fill16(gx); ccsds.fill16(gy); ccsds.fill16(gz); ccsds.fill16(t);
      ccsds.fill((char*)hx,8);
      ccsds.fill32(TC1);
      ccsds.finish(0x10);
    } else {
      gyro.read(gx,gy,gz,t,status);
      acc.read(ax,ay,az);
      unsigned int gTC1=TTC(0);
      ccsds.start(sdStore,0x24,0,gTC);
      ccsds.fill16(gx);  ccsds.fill16(gy);  ccsds.fill16(gz); ccsds.fill16(ax);  ccsds.fill16(ay);  ccsds.fill16(az); ccsds.fill(t); ccsds.fill(status); ccsds.fill32(gTC1);
      ccsds.finish(0x24);
    }
    flushPackets();
    return true;
  }
  return false;
}

void openLog(uint16_t inc=1) {
  Serial.println(inc,DEC);
  Serial.println(log_i,DEC);
  if(inc==0) blinklock(108);
  static char fn[13];
  if(fn[0]!='s') strcpy(fn,"southw00.sds");
  Serial.println(fn);
  while(log_i<99 && f.find(fn)) {
    log_i+=inc;
    fn[6]='0'+(log_i%100)/10;
    fn[7]='0'+(log_i%10);
    Serial.println(fn);
  }
  Serial.print("f.openw(\"");Serial.print(fn);Serial.print("\"): ");
  bool worked=f.openw(fn);
  Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(f.errno);
  if(!worked) blinklock(f.errno);
}

void closeLog() {
  f.close(); //Sync the directory entry - after this, we may reuse this file
               //object on a new file.
}

void initSD() {
  bool worked;
  Serial.print("sd");    Serial.print(".begin ");
  worked=sd.begin();
  Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(sd.errno);
  if(!worked) blinklock(sd.errno);

//  sd.get_info(sdinfo);
//  sdinfo.print(Serial);
  Serial.print("p");     Serial.print(".begin ");
  worked=p.begin(1);
  Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(p.errno);
//  p.print(Serial);
  if(!worked) blinklock(p.errno);

  Serial.print("fs");    Serial.print(".begin ");
  worked=fs.begin();  
  Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(fs.errno);
//  sector.begin();
//  fs.print(Serial);//,sector);
  if(!worked) blinklock(fs.errno);

  openLog();
  sdStore.fill(syncMark);
  sdStore.mark();

}

void initHMC5883() {
  Serial.print("HMC5883L");
  hmc5883.begin(isRocketometer()?&Wire0:&Wire1);
  Serial.println(" started");
  char HMCid[4];
  hmc5883.whoami(HMCid);
  Serial.print("HMC5883L");Serial.print(" identifier (should be '");Serial.print("H43");Serial.print("'): ");
  Serial.println(HMCid);
  ccsds.start(sdStore,0x0E);
  hmc5883.fillConfig(ccsds);
  ccsds.finish(0x0E);
  sdStore.drain();
}

void initGyro() {
  Serial.println("L3G4200D");
  gyro.begin(gyroSens,gyroODR,gyroBW); //Use sensitivity specified in config file. Actual
                               //sensitivity is 250*2^config.gyroSens deg/s full scale
  Serial.println(" started");
  uint8_t gyroId=gyro.whoami();
  Serial.print("L3G4200D");Serial.print(" identifier (should be '"); Serial.print("D3");Serial.print("'): ");
  Serial.println(gyroId,HEX,2);
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

void initAcc() {
  Serial.print("ADXL345");Serial.print(".begin");
  acc.begin();
  uint8_t accId=gyro.whoami();
  Serial.print("ADXL345");Serial.print(" identifier (should be '"); Serial.print("345");Serial.print("'): ");
  Serial.println(accId,OCT,3);
  ccsds.start(sdStore,0x23);
  acc.fillConfig(ccsds);
  ccsds.finish(0x23);
  sdStore.drain();
}

void initMpu() {
  mpu6050.begin(3,3);
  Serial.print("MPU6050");Serial.print(" identifier (should be '"); Serial.print("68");Serial.print("'): ");
  Serial.println(mpu6050.whoami(),HEX);
  ccsds.start(sdStore,0x0F);
  mpu6050.fillConfig(ccsds);
  ccsds.finish(0x0F);
  sdStore.drain();

  //Priming read, not written anywhere but clears INT
  mpu6050.read(ax,ay,az,mt,gx,gy,gz);
  //On Rocketometer, set up CAP1.2 on pin P0.17 to capture (read) MPU INT
  set_pin(17,1,0);
  gTC=0;
  TCR(0,3)=0;
  TCCR(0)=(0x01 << (3*3)); //channel 3, 3 control bits per channel, capture on rising, not falling, no interrupt
}

void initAd() {
  char channels=0x0B;
  Serial.print("ad799x");Serial.print(".begin ");
  bool worked=ad799x.begin(channels);
  Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(0);
  if(!worked) blinklock(0xAAAA5555);
  ccsds.start(sdStore,0x0D);
  ccsds.fill(ad799x.getAddress());
  ccsds.fill(channels);
  ccsds.fill(ad799x.getnChannels());
  ccsds.fill((uint8_t)worked);
  ccsds.finish(0x0D);
  sdStore.drain();
}

void initBmp() {
  Serial.print("BMP180");Serial.print(".begin ");
  bool worked=bmp180.begin(isRocketometer()?&Wire0:&Wire1,2);
  Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(0);
  Serial.print("BMP180");Serial.print(" identifier (should be '"); Serial.print("55");Serial.print("'): ");
  Serial.println(bmp180.whoami(),HEX);
  if(!worked) blinklock(0x5555AAAA);
  bmp180.printCalibration(&Serial);

  bmp180.ouf=0;
  ccsds.start(sdStore,0x02);

  bmp180.fillCalibration(ccsds);

  ccsds.finish(0x02);
  sdStore.drain();
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

void GyroPacket() {

}

void setup() {
  Serial.begin(4800);
  Serial.println(version_string);
  Serial.print("Hardware type: ");
  Serial.println(HW_TYPE());
  Serial.print("Using ");
  if(isRocketometer()) {
    Serial.print("Rocketometer");
    sd.p0=15;
  } else {
    Serial.print("Logomatic");
    sd.p0=7;
  }
  Serial.print(" hardware, sd.p0=");
  Serial.println(sd.p0);
  initSD();

  //Write processor config and firmware version
  writeVersion();
  sdStore.drain();

  //Read system config
  bool worked;
  ccsds.start(sdStore,0x22);
  Serial.print("readconfig");Serial.print(".begin ");
  worked=readconfig.begin();
  Serial.print(worked?"Worked":"didn't work");Serial.print(". Status code ");Serial.println(readconfig.errno);
  ccsds.finish(0x22);
  sdStore.drain();

  ccsds.start(sdStore,0x28);
  ccsds.fill32(gyroSens);
  ccsds.fill32(gyroODR);
  ccsds.fill32(gyroBW);
  ccsds.finish(0x28);
//  if(!worked) blinklock(config.errno);

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
  if(isRocketometer()) {
    Serial.println("Wire0 ");
    Wire0.begin();
  } else {
    Serial.println("Wire1 ");
    Wire1.begin();
  }
  Serial.println(" started");

  initHMC5883();
  initBmp();

  if(isRocketometer()) {
    initMpu();
    initAd();
  } else {
    //Must be done before the SPI sensors are activated (Gyro and Acc)
    SPI1.begin(1000000,1,1);
    Serial.println("SPI1 started");

    initGyro();
    initAcc();
  }

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
//    Serial.write(in);
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

void loop() {
  bool hasNav=collectAccGyroData();
  hasNav|=collectGPS();
  if(hasNav) {
    collectMagData();
    collectBmpData();
  }
}

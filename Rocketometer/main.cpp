#include "HardTwoWire.h"
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
unsigned short pktseq;

void testTask(void* stuff) {
  static int lightState=0;
  set_light(0,(lightState & 1)>0);
  lightState=(lightState+1)%8;
  taskManager.reschedule(500,0,testTask,0);
}

unsigned int tc0;
unsigned int seconds=0;

void setup() {
  set_rtc(2012,11,12,13,14,15);
  taskManager.begin();
//  taskManager.schedule(500,0,testTask,0);
  Serial.begin(115200);
  Wire1.begin();

  bmp180.begin();

  SPI1.begin(1000000,1,1);

  bool worked=sd.begin();
  Serial.printf("%s.begin %s. Status code %d\n","sd",worked?"Worked":"didn't work",sd.errno);
//  if(!worked) return;

  worked=p.begin(1);
  Serial.printf("%s.begin %s. Status code %d\n","p",worked?"Worked":"didn't work",p.errno);
//  if(!worked) return;

  worked=fs.begin();  
  Serial.printf("%s.begin %s. Status code %d\n","fs",worked?"Worked":"didn't work",fs.errno);
//  if(!worked) return;

  worked=f.openw("packet.sds",pktStore.headPtr());
  Serial.printf("%s.openw %s. Status code %d\n","f",worked?"Worked":"didn't work",f.errno);
//  if(!worked) return;

  //Dump code to serial port and packet file
  int len=source_end-source_start;
  char* base=source_start;
  unsigned short dumpSeq=0;
  d.begin();
  while(len>0) {
    d.line(base,0,116);
    ccsds.start(0x03,&dumpSeq);
    ccsds.fill32(base-source_start);
    ccsds.fill16(0);
    ccsds.fill(base,len>116?116:len);
    ccsds.finish();
    pktStore.drain();
    base+=116;
    len-=116;
  }
  d.end();

  bmp180.printCalibration(&Serial);
  ccsds.start(0x02);
  bmp180.fillCalibration(ccsds);
  ccsds.finish();
  bmp180.ouf=0;

  hmc5883.begin();
  char HMCid[4];
  hmc5883.read(HMCid);
  Serial.print("HMC5883L identifier (should be 'H43'): '");
  Serial.print(HMCid);
  Serial.println("'");

  adxl345.begin();
  int ADXLid=adxl345.read();
  Serial.print("ADXL345 identifier (should be 0o345): 0o");
  Serial.println(ADXLid,OCT);

  l3g4200d.begin();
  int L3Gid=l3g4200d.read();
  Serial.print("L3G4200D identifier (should be 0xD3): 0x");
  Serial.println(L3Gid,HEX);

  mpu6050.begin();
  Serial.print("MPU6050 identifier (should be 0x68): 0x");
  Serial.println(mpu6050.whoami(),HEX);

  Serial.println("t,ax,ay,az,bx,by,bz,gx,gy,gz,gt,T,P,Traw,Praw,max,may,maz,mgx,mgy,mgz,mt");

  tc0=TTC(0);
}

void loop() {
  while(TTC(0)>tc0) ;
  while(TTC(0)<tc0) ;
  unsigned int TC=TTC(0);
  bmp180.startMeasurement();
  adxl345.read(ax,ay,az);
  hmc5883.read(bx,by,bz);
  l3g4200d.read(gx,gy,gz,gt,gs);
  mpu6050.read(max,may,maz,mgx,mgy,mgz,mt);
  while(!bmp180.ready) ;
  temperature=bmp180.getTemperature();
  pressure=bmp180.getPressure();
  temperatureRaw=bmp180.getTemperatureRaw();
  pressureRaw=bmp180.getPressureRaw();
  Serial.print(seconds,DEC);
  Serial.print(",");
  Serial.println();
  Serial.print(ax, DEC);
  Serial.print(",");
  Serial.print(ay, DEC);
  Serial.print(",");
  Serial.print(az, DEC);
  Serial.print(",");
  Serial.print(bx, DEC);
  Serial.print(",");
  Serial.print(by, DEC);
  Serial.print(",");
  Serial.print(bz, DEC);
  Serial.print(",");
  Serial.print(gx, DEC);
  Serial.print(",");
  Serial.print(gy, DEC);
  Serial.print(",");
  Serial.print(gz, DEC);
  Serial.print(",");
  Serial.print(gt, DEC);
  Serial.print(",");
  Serial.print(temperature/10, DEC);
  Serial.print(".");
  Serial.print(temperature%10, DEC);
  Serial.print(",");
  Serial.print(pressure, DEC);
  Serial.print(",");
  Serial.print(temperatureRaw, DEC);
  Serial.print(",");
  Serial.print(pressureRaw, DEC);
  Serial.print(",");
  Serial.print(max, DEC);
  Serial.print(",");
  Serial.print(may, DEC);
  Serial.print(",");
  Serial.print(maz, DEC);
  Serial.print(",");
  Serial.print(mgx, DEC);
  Serial.print(",");
  Serial.print(mgy, DEC);
  Serial.print(",");
  Serial.print(mgz, DEC);
  Serial.print(",");
  Serial.print(mt, DEC);
  ccsds.start(0x01,&pktseq,TC,seconds%60);
  ccsds.fill(ax);
  ccsds.fill(ay);
  ccsds.fill(az);
  ccsds.fill(bx);
  ccsds.fill(by);
  ccsds.fill(bz);
  ccsds.fill(gx);
  ccsds.fill(gy);
  ccsds.fill(gz);
  ccsds.fill(gt);
  ccsds.fill(temperatureRaw);
  ccsds.fill(pressureRaw);
  ccsds.fill(max);
  ccsds.fill(may);
  ccsds.fill(maz);
  ccsds.fill(mgx);
  ccsds.fill(mgy);
  ccsds.fill(mgz);
  ccsds.fill(mt);
  ccsds.finish();
  pktStore.drain();
  seconds++;
}



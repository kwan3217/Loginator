#include "HardTwoWire.h"
#include "spi.h"
#include "Serial.h"
#include "bmp180.h"
#include "hmc5883.h"
#include "adxl345.h"
#include "l3g4200d.h"
#include "Task.h"
#include "Time.h"
#include "LPC214x.h"
#include "gpio.h"

int temperature, pressure;
int16_t ax,ay,az; //acc
int16_t bx,by,bz; //compass (bfld)
int16_t gx,gy,gz; //gyro

BMP180 bmp180(&Wire1);
HMC5883 hmc5883(&Wire1);
ADXL345 adxl345(&SPI1,20);
L3G4200D l3g4200d(&SPI1,25);

void testTask(void* stuff) {
  static int lightState=0;
  set_light(0,(lightState & 1)>0);
  lightState=(lightState+1)%8;
  taskManager.reschedule(500,0,testTask,NULL);
}

int tc0;
int seconds=0;

void setup() {
  taskManager.begin();
//  taskManager.schedule(500,0,testTask,NULL);
  Serial.begin(9600);
  Wire1.begin();
  SPI1.begin(1000000,1,1);

  bmp180.begin();
  bmp180.printCalibration(&Serial);
  bmp180.ouf=NULL;

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

  Serial.println("t,ax,ay,az,bx,by,bz,gx,gy,gz,T,P");

  tc0=TTC(0);
}

void loop() {
  while(TTC(0)>tc0) ;
  while(TTC(0)<tc0) ;
  bmp180.startMeasurement();
  adxl345.read(ax,ay,az);
  hmc5883.read(bx,by,bz);
  l3g4200d.read(gx,gy,gz);
  while(!bmp180.ready) ;
  temperature=bmp180.getTemperature();
  pressure=bmp180.getPressure();
  Serial.print(seconds,DEC);
  Serial.print(",");
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
  Serial.print(temperature/10, DEC);
  Serial.print(".");
  Serial.print(temperature%10, DEC);
  Serial.print(",");
  Serial.print(pressure, DEC);
  Serial.println();
  seconds++;
}



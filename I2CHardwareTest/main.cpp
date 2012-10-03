#include "gpio.h"
#include "HardTwoWire.h"
#include "Serial.h"
#include "bmp180.h"
#include "hmc5883.h"
#include "Task.h"

int temperature, pressure;
int16_t x,y,z;

BMP180 bmp180(&Wire1);
HMC5883 hmc5883(&Wire1);

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
  bmp180.begin();
  bmp180.printCalibration(&Serial);
  bmp180.ouf=NULL;
  hmc5883.begin();
  char HMCid[4];
  hmc5883.read(HMCid);
  Serial.print("HMC5883L identifier (should be 'H43'): '");
  Serial.print(HMCid);
  Serial.println("'");
  tc0=TTC(0);
}

void loop() {
  while(TTC(0)>tc0) ;
  while(TTC(0)<tc0) ;
  bmp180.startMeasurement();
  hmc5883.read(x,y,z);
  while(!bmp180.ready) ;
  temperature=bmp180.getTemperature();
  pressure=bmp180.getPressure();
  Serial.print(seconds,DEC);
  Serial.print(",");
  Serial.print(x, DEC);
  Serial.print(",");
  Serial.print(y, DEC);
  Serial.print(",");
  Serial.print(z, DEC);
  Serial.print(",");
  Serial.print(temperature/10, DEC);
  Serial.print(".");
  Serial.print(temperature%10, DEC);
  Serial.print(",");
  Serial.print(pressure, DEC);
  Serial.println();
  seconds++;
}



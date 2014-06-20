#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <libgen.h>
#include "compass.h"

#define PI 3.1415926535897932
#define clat 0.765243525639  //cosine of latitude of AVC2014 start line

void Compass::handleRMC(uint32_t TC, uint32_t hms, int32_t Llat, int32_t Llon, int32_t spd, int32_t spdScale, int32_t hdg, int32_t hdgScale, int32_t dmy) {
  int h=hms/10000;
  int ms=hms % 10000;
  int m=ms/100;
  int s=ms%100;
  sod=h*3600+m*60+s;
  rmcHdg=hdg;
  for(int i=0;i<hdgScale;i++) rmcHdg/=10.0;
  rmcSpd=spd;
  for(int i=0;i<spdScale;i++) rmcSpd/=10.0;
  lat=((float)Llat)/1e7;
  lon=((float)Llon)/1e7;
}

void Compass::handleGyroCfg(uint8_t ctrl4) {
  uint8_t fs=(ctrl4>>4) & 0x03;
  fp FS=((fp)(250 << fs));
  sens=FS/360.0; //rotations per second full scale
  sens*=2*PI;   //radians per second full scale
  sens/=32768;  //radians per second per DN
  sens*=yscl/32768; //include calibration scale factor
} 

void Compass::handleGyro(uint32_t TC, int16_t gx, int16_t gy, int16_t gz) {
  //Gyro packet
  gyroT=TC/60e6;
  if(gyroT<lastT) {
    deltaT=(gyroT-lastT)+60;
    minuteofs++;
  } else {
    deltaT=(gyroT-lastT);
  }
  lastT=gyroT;
  gyroT+=(minuteofs*60);
  if ((avgGMin <= gyroPktCount) && (avgGMax> gyroPktCount)) {
    //Do average G
    avgGx+=gx;
    avgGy+=gy;
    avgGz+=gz;
  } else if(avgGMax == gyroPktCount) {
    avgGx/=(avgGMax-avgGMin);
    avgGy/=(avgGMax-avgGMin);
    avgGz/=(avgGMax-avgGMin);
  } else if(avgGMax < gyroPktCount) {
    //propagate the quaternion
    calGx=sens*(((fp)gx)-avgGx);
    calGy=sens*(((fp)gy)-avgGy);
    calGz=sens*(((fp)gz)-avgGz);
    e.integrate(calGx,calGy,calGz,deltaT);
    //Figure the nose vector
    nose_r=e.b2r(nose);
    gyroHdg=atan2(nose_r.z,nose_r.x)*180.0/PI;
    hdg=gyroHdg;
  }
  gyroPktCount++;
}


#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <libgen.h>
#include "compass.h"

#define PI 3.1415926535897932
#define clat 0.765243525639  //cosine of latitude of AVC2014 start line

Quaternion Quaternion::mul(Quaternion& p, Quaternion& q) {
  Quaternion r(p.w*q.x-p.z*q.y+p.y*q.z+p.x*q.w,
               p.z*q.x+p.w*q.y-p.x*q.z+p.y*q.w,
              -p.y*q.x+p.x*q.y+p.w*q.z+p.z*q.w,
              -p.x*q.x-p.y*q.y-p.z*q.z+p.w*q.w);
  return r;
}

Quaternion Quaternion::add(Quaternion& p, Quaternion& q) {
  Quaternion r(p.x+q.x,
               p.y+q.y,
               p.z+q.z,
               p.w+q.w);
  return r;
}

void Quaternion::add(Quaternion& q) {
  x+=q.x;
  y+=q.y;
  z+=q.z;
  w+=q.w;
}

void Quaternion::mul(fp s) {
  x*=s;
  y*=s;
  z*=s;
  w*=s;
}

void Quaternion::negate() {
  mul(-1);
}

void Quaternion::conjugate() {
  x=-x;
  y=-y;
  z=-z;
}

/** Integrate a rotation. Given a quaternion that represents
    an orientation (this), a rotation rate vector, and a time
    interval to integrate over, integrate the rotation 
    @param wx rotation rate in right-handed sense around
              body x axis in radians per time unit
    @param wy rotation rate in right-handed sense around
              body y axis in radians per time unit
    @param wz rotation rate in right-handed sense around
              body z axis in radians per time unit
    @param dt time interval to integrate over, implies time units
    @param steps number of Euler method steps to use
*/
void Quaternion::integrate(fp wx, fp wy, fp wz, fp dt, int steps) {
  Quaternion wv(wx,wy,wz,0);
  for(int i=0;i<steps;i++) {
    Quaternion edot=mul(*this,wv);
    edot.mul(0.5*dt/steps);
    add(edot);
    normalize();
  }
}

/** Implement direct (reference to body) rotation with quaternions using the convention documented
    in the Kwan Hypertext Library:

v_r=q'*v_b*q

    @param vr Vector in reference frame. Must be a quaternion with zero scalar part
    @return same vector but transformed into body frame. Will be a quaternion
            with zero scalar part
*/    
Quaternion Quaternion::r2b(Quaternion& vr) {
  Quaternion vre=mul(vr,*this);
  Quaternion ep=*this;ep.conjugate();
  Quaternion vb=mul(ep,vre);
  return vb;
}

/** Implement inverse (body to reference) rotation with quaternions using the convention documented
    in the Kwan Hypertext Library:

v_r=q'*v_b*q

    @param vr Vector in reference frame. Must be a quaternion with zero scalar part
    @return same vector but transformed into body frame. Will be a quaternion
            with zero scalar part
*/    
Quaternion Quaternion::b2r(Quaternion& vb) {
  Quaternion ep=*this;ep.conjugate();
  Quaternion vbep=mul(vb,ep);
  Quaternion vr=mul(*this,vbep);
  return vr;
}


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

void Compass::setSens(uint8_t fs) {
  fp FS=((fp)(250 << fs));
  sens=FS/360.0; //rotations per second full scale
  sens*=2*PI;   //radians per second full scale
  sens/=32768;  //radians per second per DN
  sens*=yscl/32768; //include calibration scale factor
}

void Compass::handleGyroCfg(uint8_t ctrl4) {
  uint8_t fs=(ctrl4>>4) & 0x03;
  setSens(fs);
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
    hasHdg=true;
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


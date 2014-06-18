#ifndef compass_h
#define compass_h

#include "float.h"

class Quaternion {
public:
  fp x,y,z,w;
  Quaternion(fp Lx, fp Ly, fp Lz, fp Lw):x(Lx),y(Ly),z(Lz),w(Lw){};
  Quaternion():x(0),y(0),z(0),w(1){};
  Quaternion(fp Lx, fp Ly, fp Lz):x(Lx),y(Ly),z(Lz),w(0){};
  static Quaternion mul(Quaternion& p, Quaternion& q);
  void mul(fp s);
  static Quaternion add(Quaternion& p, Quaternion& q);
  void add(Quaternion& q);
  void negate();
  void conjugate(); //Good night, everybody!
  void integrate(fp wx, fp wy, fp wz, fp dt, int steps=1); 
  fp length() {return sqrt(x*x+y*y+z*z+w*w);};
  void normalize() {mul(1.0/length());};
  Quaternion r2b(Quaternion& vr);
  Quaternion b2r(Quaternion& vb);
};

class Compass {
public:
  Quaternion e;
  Quaternion nose,nose_r;
  fp sens; //Gyro sensitivity in rad/s/DN
  fp hdg,rmcHdg,gyroHdg; //Heading in degrees
  fp rmcSpd; //Speed in knots
  fp gyroT,lastT,deltaT;  //Time of last gyro reading in seconds, derived from TC so wraps every minute
  int32_t sod,minuteofs;
  uint32_t gyroPktCount;
  fp avgGx, avgGy, avgGz;
  fp calGx, calGy, calGz;
  fp lat,lon;
  static const fp clat;
  static const int yscl=37564;
  static const int avgGMin=200;
  static const int avgGMax=100;
  Compass():nose(0,0,-1,0) {};
  void handleRMC(uint32_t TC, uint32_t hms, int32_t lat, int32_t lon, int32_t spd, int32_t spdScale, int32_t hdg, int32_t hdgScale, int32_t dmy);
  void handleGyroCfg(uint8_t ctrl4);
  void handleGyro(uint32_t TC, int16_t gx, int16_t gy, int16_t gz);
};

#endif

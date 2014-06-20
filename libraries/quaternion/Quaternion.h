#ifndef Quaternion_h
#define Quaternion_h

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

#endif

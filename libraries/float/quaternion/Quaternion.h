#ifndef Quaternion_h
#define Quaternion_h

#include "Vector.h"

class Quaternion:public Vector<4,fp> {
public:
  fp& x;
  fp& y;
  fp& z;
  fp& w;
  Quaternion& operator=(Quaternion rhs) {
    for(int i=0;i<4;i++)comp[i]=rhs[i];
    return *this;
  }
  Quaternion(fp Lx, fp Ly, fp Lz, fp Lw):Quaternion(){x=Lx;y=Ly;z=Lz;w=Lw;};
  Quaternion():x(comp[0]),y(comp[1]),z(comp[2]),w(comp[3]){x=0;y=0;z=0;w=1;};
  Quaternion(fp Lx, fp Ly, fp Lz):Quaternion(Lx,Ly,Lz,0){};

  /** Quaternion multiplication */
  Quaternion& operator*=(const Quaternion& rhs);
  void conjugate(); //Good night, everybody!
  void integrate(Vector<3> w, fp dt, int steps=1);
  fp rlength() {return Q_rsqrt(x*x+y*y+z*z+w*w);};
  void normalize() {((Vector<4,fp>)(*this))*=rlength();};
  Quaternion r2b(Quaternion& vr);
  Quaternion b2r(Quaternion& vb);
};

static inline Quaternion operator*(Quaternion lhs, const Quaternion& rhs) {lhs*=rhs;return lhs;};

#endif

#include "Quaternion.h"

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
void Quaternion::integrate(Vector<3> w, fp dt, int steps) {
  Quaternion wv(w[0],w[1],w[2],0);
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



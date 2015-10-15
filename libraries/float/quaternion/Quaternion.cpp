#include "Quaternion.h"

Quaternion& Quaternion::operator*=(const Quaternion& q) {
  fp rx= w*q.x-z*q.y+y*q.z+x*q.w;
  fp ry= z*q.x+w*q.y-x*q.z+y*q.w;
  fp rz=-y*q.x+x*q.y+w*q.z+z*q.w;
  fp rw=-x*q.x-y*q.y-z*q.z+w*q.w;
  x=rx;y=ry;z=rz;w=rw;
  return *this;
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
void Quaternion::integrate(Vector<3> wv, fp dt, int steps) {
  Quaternion wq(wv[0],wv[1],wv[2],0);
  for(int i=0;i<steps;i++) {
    Quaternion edot=*this*wq;
    ((Vector<4,fp>)(edot))*=0.5*dt/steps;
    *this+=edot;
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
  Quaternion vre=vr*(*this);
  Quaternion ep=*this;ep.conjugate();
  Quaternion vb=ep*vre;
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
  Quaternion vbep=vb*ep;
  Quaternion vr=*this*vbep;
  return vr;
}



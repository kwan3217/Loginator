#include "Quaternion.h"

/** Integrate a rotation. Given a quaternion that represents
    an orientation (this), a rotation rate vector, and a time
    interval to integrate over, integrate the rotation 
    @param[in] wv rotation rate vector right-handed sense
    @param[in] dt time interval to integrate over, implies time units
    @param[in] steps number of Euler method steps to use
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

    @param[in] vr Vector in reference frame. Must be a quaternion with zero scalar part
    @return copy of same vector but expressed in components in body frame. Will be a quaternion
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



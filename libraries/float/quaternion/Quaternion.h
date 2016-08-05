#ifndef Quaternion_h
#define Quaternion_h

#include "Vector.h"

/** Quaternion class. This is a specialization of the Vector class, which adds
    the quaternion multiplication operator, along with a couple of methods to
    encourage use of a consistent convention when using this quaternion class
    for 3D orientation. */
class Quaternion:public Vector<4,fp> {
public:
  fp& x; ///< Name of component 0, which is the x vector component by convention
  fp& y; ///< Name of component 1, which is the y vector component by convention
  fp& z; ///< Name of component 2, which is the z vector component by convention
  fp& w; ///< Name of component 3, which is the scalar   component by convention
  /** Assignment operator. Copy the components from the right hand side
      quaternion into this quaternion without affecting the right hand side
   @param[in] rhs Quaternion to copy components from
   */
  Quaternion& operator=(Quaternion rhs) {
    for(int i=0;i<4;i++)comp[i]=rhs[i];
    return *this;
  }
  /** Constructor with components specified
   @param[in] Lx initial x vector component
   @param[in] Ly initial y vector component
   @param[in] Lz initial z vector component
   @param[in] Lw initial scalar   component
  */
  Quaternion(fp Lx, fp Ly, fp Lz, fp Lw):Quaternion(){x=Lx;y=Ly;z=Lz;w=Lw;};
  /** Construct an identity quaternion, IE 0i+0j+0k+1 . This also represents an 
      identity orientation */
  Quaternion():x(comp[0]),y(comp[1]),z(comp[2]),w(comp[3]){x=0;y=0;z=0;w=1;};
  /** Construct a quaternion from the components of a 3D vector. Quaternion
      will be Lx*i+Ly*j+Lz*k+0 . Does not have to be a unit quaternion or unit
      vector. 
   @param[in] Lx initial x vector component
   @param[in] Ly initial y vector component
   @param[in] Lz initial z vector component
   */
  Quaternion(fp Lx, fp Ly, fp Lz):Quaternion(Lx,Ly,Lz,0){};

private:
  /** Quaternion multipy and assign. Don't use this directly, instead use the 
      plain multiply operator built upon this. 

      The operator uses this as the left-hand operand,
      the passed-in quaternion as the right-hand operand, and writes
      the result back to the left-hand side, returning a reference to it. 

      @param q right-hand side quaternion
  */
  Quaternion& operator*=(const Quaternion& q) {
    fp rx= w*q.x-z*q.y+y*q.z+x*q.w;
    fp ry= z*q.x+w*q.y-x*q.z+y*q.w;
    fp rz=-y*q.x+x*q.y+w*q.z+z*q.w;
    fp rw=-x*q.x-y*q.y-z*q.z+w*q.w;
    x=rx;y=ry;z=rz;w=rw;
    return *this;
  }

  friend Quaternion operator*(Quaternion lhs, const Quaternion& rhs);

public:
  /** Quaternion conjugation (Good night, everybody!). This method operates 
      directly on this quaternion, changing its components (as opposed to 
      returning a copy which is conjugated). */
  void conjugate() {x=-x;y=-y;z=-z;}

  void integrate(Vector<3> w, fp dt, int steps=1);
  /** Reciprocal of quaternion magnitude
   @return reciprocal of quaternion magnitude=1/sqrt(x^2+y^2+z^2+w^2)
   */
  fp rlength() {return Q_rsqrt(x*x+y*y+z*z+w*w);};
  /** Force the quaternion to be unit length with the same direction */
  void normalize() {((Vector<4,fp>)(*this))*=rlength();};
  Quaternion r2b(Quaternion& vr);
  Quaternion b2r(Quaternion& vb);
};

/** Quaternion multiply and assign. Not a method because we need to follow the
    operator overloading recipe. Built on multiply-assign member operator.
  @param[in] lhs left-hand side operand
  @param[in] rhs right-hand side operator
  @return The quaternion product */
static inline Quaternion operator*(Quaternion lhs, const Quaternion& rhs) {lhs*=rhs;return lhs;};

#endif

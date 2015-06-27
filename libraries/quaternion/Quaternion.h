#ifndef Quaternion_h
#define Quaternion_h

#include "float.h"

template<int n, typename T=fp>
class Vector {
public:
  T comp[n];
  T& operator[](int i) {return comp[i];};
  Vector<n,T>() {for(int i=0;i<n;i++) comp[i]=0;};
  Vector<n,T>& operator=(Vector<n> rhs) {
    for(int i=0;i<n;i++)comp[i]=rhs[i];
    return *this;
  }  
  /** Vector addition, component by component */
  Vector<n,T>& operator+=(const Vector<n>& rhs) {
    for(int i=0;i<n;i++) comp[i]+=rhs.comp[i];
    return *this;
  }
  /** Vector-scalar addition. Add the scalar to each component of the vector */
  Vector<n,T>& operator+=(T rhs) {
    for(int i=0;i<n;i++) comp[i]+=rhs;
    return *this;
  }
  /** Vector subtraction, component by component */
  Vector<n,T>& operator-=(const Vector<n>& rhs) {
    for(int i=0;i<n;i++) comp[i]-=rhs.comp[i];
    return *this;
  }
  /** Vector-scalar subtraction. Subtract the scalar from each component of the vector */
  Vector<n,T>& operator-=(T rhs) {
    for(int i=0;i<n;i++) comp[i]-=rhs;
    return *this;
  }
  /** Vector component-by-component multiplication (not dot or cross product) */
  Vector<n,T>& operator*=(const Vector<n>& rhs) {
    for(int i=0;i<n;i++) comp[i]*=rhs.comp[i];
    return *this;
  }
  /** Vector-scalar multiplication. Multiply each component by the scalar */
  Vector<n,T>& operator*=(T rhs) {
    for(int i=0;i<n;i++) comp[i]*=rhs;
    return *this;
  }
  /** Vector component-by-component division */
  Vector<n,T>& operator/=(const Vector<n>& rhs) {
    for(int i=0;i<n;i++) comp[i]/=rhs.comp[i];
    return *this;
  }
  /** Vector-scalar division. Divide each component by the scalar */
  Vector<n,T>& operator/=(T rhs) {
    for(int i=0;i<n;i++) comp[i]/=rhs;
    return *this;
  }
};

template<int n, typename T=fp> Vector<n,T> operator+(Vector<n,T> lhs, const Vector<n,T>& rhs) {lhs+=rhs;return lhs;};
template<int n, typename T=fp> Vector<n,T> operator+(Vector<n,T> lhs, const T rhs)            {lhs+=rhs;return lhs;};
template<int n, typename T=fp> Vector<n,T> operator+(const T& lhs, Vector<n,T> rhs)           {rhs+=lhs;return rhs;};

template<int n, typename T=fp> Vector<n,T> operator-(Vector<n,T> lhs, const Vector<n,T>& rhs) {lhs-=rhs;return lhs;};
template<int n, typename T=fp> Vector<n,T> operator-(Vector<n,T> lhs, const T rhs)            {lhs-=rhs;return lhs;};
template<int n, typename T=fp> Vector<n,T> operator-(const T& lhs, Vector<n,T> rhs)           {rhs-=lhs;return rhs;};

template<int n, typename T=fp> Vector<n,T> operator*(Vector<n,T> lhs, const Vector<n,T>& rhs) {lhs*=rhs;return lhs;};
template<int n, typename T=fp> Vector<n,T> operator*(Vector<n,T> lhs, const T rhs)            {lhs*=rhs;return lhs;};
template<int n, typename T=fp> Vector<n,T> operator*(const T& lhs, Vector<n,T> rhs)           {rhs*=lhs;return rhs;};

template<int n, typename T=fp> Vector<n,T> operator/(Vector<n,T> lhs, const Vector<n,T>& rhs) {lhs/=rhs;return lhs;};
template<int n, typename T=fp> Vector<n,T> operator/(Vector<n,T> lhs, const T rhs)            {lhs/=rhs;return lhs;};
// No operator/(fp,Vector). This operation doesn't make sense

template<int n, typename T=fp> T dot(const Vector<n,T>& lhs, const Vector<n,T>& rhs) {T result=0;for(int i=0;i<n;i++) result+=lhs.comp[i]*rhs.comp[i];return result;};

class Quaternion:public Vector<4> {
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
  static Quaternion mul(Quaternion& p, Quaternion& q);
  void mul(fp s);
  static Quaternion add(Quaternion& p, Quaternion& q);
  void add(Quaternion& q);
  void negate();
  void conjugate(); //Good night, everybody!
  void integrate(Vector<3> w, fp dt, int steps=1); 
  fp rlength() {return Q_rsqrt(x*x+y*y+z*z+w*w);};
  void normalize() {mul(rlength());};
  Quaternion r2b(Quaternion& vr);
  Quaternion b2r(Quaternion& vb);
};

#endif

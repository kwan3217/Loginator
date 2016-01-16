#ifndef Vector_h
#define Vector_h

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

template<int n, typename T=fp> static inline Vector<n,T> operator+(Vector<n,T> lhs, const Vector<n,T>& rhs) {lhs+=rhs;return lhs;};
template<int n, typename T=fp> static inline Vector<n,T> operator+(Vector<n,T> lhs, const T rhs)            {lhs+=rhs;return lhs;};
template<int n, typename T=fp> static inline Vector<n,T> operator+(const T& lhs, Vector<n,T> rhs)           {rhs+=lhs;return rhs;};

template<int n, typename T=fp> static inline Vector<n,T> operator-(Vector<n,T> lhs, const Vector<n,T>& rhs) {lhs-=rhs;return lhs;};
template<int n, typename T=fp> static inline Vector<n,T> operator-(Vector<n,T> lhs, const T rhs)            {lhs-=rhs;return lhs;};
template<int n, typename T=fp> static inline Vector<n,T> operator-(const T& lhs, Vector<n,T> rhs)           {rhs-=lhs;return rhs;};

template<int n, typename T=fp> static inline Vector<n,T> operator*(Vector<n,T> lhs, const Vector<n,T>& rhs) {lhs*=rhs;return lhs;};
template<int n, typename T=fp> static inline Vector<n,T> operator*(Vector<n,T> lhs, const T rhs)            {lhs*=rhs;return lhs;};
template<int n, typename T=fp> static inline Vector<n,T> operator*(const T& lhs, Vector<n,T> rhs)           {rhs*=lhs;return rhs;};

template<int n, typename T=fp> static inline Vector<n,T> operator/(Vector<n,T> lhs, const Vector<n,T>& rhs) {lhs/=rhs;return lhs;};
template<int n, typename T=fp> static inline Vector<n,T> operator/(Vector<n,T> lhs, const T rhs)            {lhs/=rhs;return lhs;};
// No operator/(fp,Vector). This operation doesn't make sense

template<int n, typename T=fp> static inline T dot(const Vector<n,T>& lhs, const Vector<n,T>& rhs) {T result=0;for(int i=0;i<n;i++) result+=lhs.comp[i]*rhs.comp[i];return result;};
//If we wanted a cross product, we would include it similar to dot() above
#endif

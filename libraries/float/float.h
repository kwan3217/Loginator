#ifndef FLOAT_H
#define FLOAT_H

#include <inttypes.h>
#include <math.h>

#ifdef DOUBLE
typedef double fp;
#else
typedef float fp;
#define atan2 atan2f
//#define sqrt sqrtf
#endif

/**
Calculate the sine and cosine of the sum of two angles given the sines and cosines of those angles
@param ca cosine of angle a
@param cb cosine of angle b
@param sa sine of angle a
@param sb sine of angle b
@param c cosine of a+b
@param s sine of a+b

Avoid trig-land! Use trig identities!
\f{eqnarray*}{
\cos(a\pm b)=\cos(a)\cos(b)\mp \sin(a)\sin(b) \\
\sin(a\mp b)=\sin(a)\cos(b)\pm \cos(a)\sin(b)
\f}
*/
static inline void trigp(fp ca, fp cb, fp sa, fp sb, fp &c, fp &s) {
  c=ca*cb+sa*sb;
  s=sa*cb-ca*sb;
}

//Calculate the sine and cosine of the sum of difference between two angles
//given the sines and cosines of those angles
static inline void trigm(fp ca, fp cb, fp sa, fp sb, fp &c, fp &s) {
  c=ca*cb-sa*sb;
  s=sa*cb+ca*sb;
}

//An extreme example of trading space for time in an optimization - a big sine
//table and an easy table lookup function
constexpr fp PI=3.1415926535897932;

static inline constexpr fp factorial(int n) {
  fp result=1.0;
  for(int i=1;i<=n;i++) result*=i;
  return result;
}

constexpr fp sinPoly[]={0,1,0,-1.0/factorial(3),0,+1.0/factorial(5),0,-1.0/factorial(7),0,1.0/factorial(9),0,-1.0/factorial(11)};
constexpr fp cosPoly[]={1,0,-1.0/factorial(2),0,+1.0/factorial(4),0,-1.0/factorial(6),0,1.0/factorial(8),0,-1.0/factorial(10)};

static inline constexpr fp poly(const fp x, const fp* P, const int maxorder, const int order) {
  return P[maxorder-order]+x*(order==0?0:poly(x,P,maxorder,order-1));
}

static inline constexpr fp poly(const fp x, const fp* P, const int order) {
  return poly(x,P,order,order);
}

static inline constexpr fp sin(const fp x) {
  return poly(x,sinPoly,sizeof(sinPoly)/sizeof(fp)-1);
}

static inline constexpr fp cos(const fp x) {
  return poly(x,cosPoly,sizeof(cosPoly)/sizeof(fp)-1);
}

extern const fp sinTable[];

//Angle is measured in tenths of a degree, so a full circle has 3600 parts
//Angle must be between 0 and 3599 or else Bad Things happen
static inline fp sint(int angle) {
  if(angle<0) return -sint(-angle);
  if(angle>=1800) return -sint(angle-1800);
  if(angle>900) return sint(1800-angle);
  return sinTable[angle];
}

static inline fp cost(int angle) {
  return sint(900-angle);
}

//From Wikipedia:Fast inverse square root. Must be float, not fp. Edited
//to use correct types and avoid strict-aliasing warning on conversion from float to int
static inline float Q_rsqrt( float number ) {
  union floatint {
    float f;
    int32_t i;
  };

  floatint i;
  float x2, y;
  const float threehalfs = 1.5F;

  x2 = number * 0.5F;
  i.f  = number;        // evil floating point bit level hacking
  i.i  = 0x5f3759df - ( i.i >> 1 );               // what the...?
  y  = i.f;
  y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//  y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

  return y;
}

fp stof(char* s);

#endif



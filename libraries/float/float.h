#ifndef FLOAT_H
#define FLOAT_H

#include <math.h>
#include <inttypes.h>

#ifdef DOUBLE
typedef double fp;
#else
typedef float fp;
#define sin sinf
#define cos cosf
#define atan2 atan2f
#define sqrt sqrtf
#endif
fp poly(fp x, const fp* p, int order);

//Avoid trig-land! Use trig identities!
//cos(a+-b)=cos(a)*cos(b)-+sin(a)*sin(b)
//sin(a+-b)=sin(a)*cos(b)+-cos(a)*sin(b)
//Calculate the sine and cosine of the sum of two angles given
//the sines and cosines of those angles
static inline void trigp(fp ca, fp cb, fp sa, fp sb, fp *c, fp *s) {
  *c=ca*cb+sa*sb;
  *s=sa*cb-ca*sb;
}

//Calculate the sine and cosine of the sum of difference between two angles
//given the sines and cosines of those angles
static inline void trigm(fp ca, fp cb, fp sa, fp sb, fp *c, fp *s) {
  *c=ca*cb-sa*sb;
  *s=sa*cb+ca*sb;
}

//An extreme example of trading space for time in an optimization - a big sine
//table and an easy table lookup function
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


union floatint {
  float f;
  int32_t i;
};

//From Wikipedia:Fast inverse square root. Must be float, not fp. Edited
//to use correct types and avoid strict-aliasing warning on conversion from float to int
static inline float Q_rsqrt( float number ) {
	floatint i;
	float x2, y;
	const float threehalfs = 1.5F;
 
	x2 = number * 0.5F;
	i.f  = number;        // evil floating point bit level hacking
	i.i  = 0x5f3759df - ( i.i >> 1 );               // what the...?
	y  = i.f;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//      y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed
 
	return y;
}

#endif



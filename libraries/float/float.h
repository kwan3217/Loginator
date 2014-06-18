#ifndef FLOAT_H
#define FLOAT_H

#include <math.h>

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

#endif



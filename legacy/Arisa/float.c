#include "float.h"

fp poly(fp x, const fp* P, int order) {
  fp acc=P[order];
  for(int i=order-1;i>=0;i--) {
    acc=acc*x+P[i];
  }
  return acc;
}
#include "FileCircular.h"

bool FileCircular::drainCore() {
  if(!ouf.append(buf+tail)) FAIL(ouf.errnum*100+1);
  fullState=false;
  tail=(tail+blockSize)%N;
  return true;
}

#include "FileCircular.h"

bool FileCircular::drainCore() {
  if(!ouf.append(buf+tail)) FAIL(ouf.errno*100+1);
  fullState=false;
  tail=(tail+blockSize)%N;
  return true;
}

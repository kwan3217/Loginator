#include "FileCircular.h"

bool FileCircular::drainCore() {
#ifdef RIEGEL
#else
  if(!ouf.append(buf+tail)) FAIL(ouf.errno*100+1);
#endif
  tail=(tail+blockSize)%N;
  return true;
}

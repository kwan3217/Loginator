#include "FileCircular.h"

bool FileCircular::drainCore() {
  if(!ouf.append(buf+tail)) FAIL(ouf.errno*100+1)
  tail=(tail+SDHC::BLOCK_SIZE)%N;
  return true;
}

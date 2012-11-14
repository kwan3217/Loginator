#include "FileCircular.h"

bool FileCircular::drainCore() {
  if(!ouf.append(buf+tail)) return false;
  tail=(tail+SDHC::BLOCK_SIZE)%N;
}

#ifndef FILECIRCULAR_H
#define FILECIRCULAR_H

#ifdef RIEGEL
#include "fat.h"
#else
#include "file.h"
#endif
#include "Circular.h"

class FileCircular: public Circular {
private:
  bool drainCore();
  static const blockSize=
#ifdef RIEGEL
512
#else
SDHC::BLOCK_SIZE
#endif
;
  static const bufSize=blockSize*6;
  char buf[bufSize];
protected:
#ifdef RIEGEL
  struct fat_file_struct*
#else
  File &
#endif
 ouf;
public:
  unsigned int errno;
  FileCircular(
#ifdef RIEGEL
#else
File &
#endif
 Louf):Circular(sizeof(buf),buf),ouf(Louf) {};
  bool drain() {errno=0;if(readylen()>=blockSize) return drainCore();return false;};
};

#endif

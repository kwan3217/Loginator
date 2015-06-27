#ifndef FILECIRCULAR_H
#define FILECIRCULAR_H

#include "file.h"
#include "Circular.h"

class FileCircular: public Circular {
private:
  bool drainCore();
  static const int blockSize=SDHC::BLOCK_SIZE;
  static const int bufSize=blockSize*8;
  char buf[bufSize];
protected:
  File& ouf;
public:
  unsigned int errno;
  FileCircular(File& Louf):Circular(sizeof(buf),buf),ouf(Louf) {};

  bool drain() {errno=0;if(readylen()>=blockSize) return drainCore();return false;};
};

#endif

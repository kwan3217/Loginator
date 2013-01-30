#ifndef FILECIRCULAR_H
#define FILECIRCULAR_H

#include "file.h"
#include "Circular.h"

class FileCircular: public Circular {
private:
  bool drainCore();
  char buf[SDHC::BLOCK_SIZE*2];
protected:
  File& ouf;
public:
  unsigned int errno;
  FileCircular(File& Louf):Circular(sizeof(buf),buf),ouf(Louf) {};
  bool drain() {if(readylen()>=SDHC::BLOCK_SIZE) return drainCore();return false;};
};

#endif

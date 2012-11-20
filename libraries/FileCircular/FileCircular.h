#ifndef FILECIRCULAR_H
#define FILECIRCULAR_H

#include "file.h"
#include "Circular.h"

class FileCircular: public Circular {
private:
  bool drainCore();
protected:
  File& ouf;
public:
  unsigned int errno;
  FileCircular(char* Lbuf,File& Louf):Circular(1024,Lbuf),ouf(Louf) {};
  bool drain() {if(readylen()>=SDHC::BLOCK_SIZE) return drainCore();return false;};
};

#endif

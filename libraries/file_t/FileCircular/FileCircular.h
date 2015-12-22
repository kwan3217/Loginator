#ifndef FILECIRCULAR_H
#define FILECIRCULAR_H

#include "file.h"
#include "Circular.h"

template<class Pk, class Pr, class S>
class FileCircular: public Circular {
private:
  bool drainCore();
  static const int blockSize=SDHC<Pk,Pr,S>::BLOCK_SIZE;
  static const int bufSize=blockSize*8;
  char buf[bufSize];
protected:
  File<Pk,Pr,S>& ouf;
public:
  unsigned int errnum;
  FileCircular(File<Pk,Pr,S>& Louf):Circular(sizeof(buf),buf),ouf(Louf) {};

  bool drain() {errnum=0;if(readylen()>=blockSize) return drainCore();return false;};
};

#include "FileCircular.inc"

#endif

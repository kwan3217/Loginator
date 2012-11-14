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
  FileCircular(File& Louf):Circular(),ouf(Louf) {};
  bool drain() {if(readylen()>=SDHC::BLOCK_SIZE) return drainCore();return true;};
};

#endif

#ifndef file_h
#define file_h

#include "cluster.h"
#include "direntry.h"

class File {
  Cluster& c;
  DirEntry de;
  uint32_t cluster,sector;
public:
  int errno;
  File(Cluster& Lc):c(Lc),de(c),errno(0) {};
  bool find(const char* fn, uint32_t dir_cluster=0) {return de.find(fn,dir_cluster);};
  bool create(const char* filename, char* buf,uint32_t dir_cluster=0);
  bool openr(const char* name, uint32_t dir_cluster=0);
  bool openw(const char* name, char* buf,uint32_t dir_cluster=0);
  bool read(char* buf);
  bool append(char* buf, char* fastBuf=0);
  bool remove(const char* filename, char* buf,uint32_t dir_cluster=0);
  bool wipeChain(char* buf);
  bool sync(char* buf);
  bool close(char* buf) {return sync(buf);};
  unsigned int size() {return de.size;};
};

#endif


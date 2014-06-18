#ifndef file_h
#define file_h

#include "cluster.h"
#include "direntry.h"

class File {
  Cluster& c;
  uint32_t cluster,sector,last_cluster;
public:
  int errno;
  DirEntry de;
  File(Cluster& Lc):c(Lc),last_cluster(1),errno(0),de(c) {};
  bool find(const char* fn, uint32_t dir_cluster=0) {return de.find(fn,dir_cluster);};
  bool create(const char* filename, uint32_t dir_cluster=0);
  bool openr(const char* name, uint32_t dir_cluster=0);
  bool openw(const char* name, uint32_t dir_cluster=0);
  bool read(char* buf);
  bool append(char* buf);
  bool remove(const char* filename, char* buf,uint32_t dir_cluster=0);
  bool wipeChain();
  bool sync();
  bool close() {return sync();};
  unsigned int size() {return de.size;};
};

#endif


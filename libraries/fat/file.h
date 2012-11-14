#ifndef file_h
#define file_h

#include "cluster.h"
#include "direntry.h"

class File {
  Cluster& c;
  DirEntry de;
  uint32_t cluster,sector;
public:
  int errno=0;
  File(Cluster& Lc):c(Lc),de(c) {};
  bool create(const char* filename, char* buf,uint32_t dir_cluster=0);
  bool openr(const char* name, uint32_t dir_cluster=0);
  bool openw(const char* name, char* buf,uint32_t dir_cluster=0);
  bool read(char* buf);
  bool append(char* buf);
  bool remove(const char* filename, char* buf,uint32_t dir_cluster=0);
  bool wipeChain(char* buf);
};

#endif


#ifndef file_h
#define file_h

#include "cluster.h"
#include "direntry.h"

class File {
  Cluster& c;
  DirEntry de;
  uint32_t cluster,sector;
public:
  File(Cluster& Lc):c(Lc),de(c) {};
  bool create(uint32_t dir_cluster,const char* filename, char* buf);
  bool openr(uint32_t dir_cluster, const char* name);
  bool openr(const char* name) {return openr(0,name);};
  bool openw(uint32_t dir_cluster, const char* name, char* buf);
  bool openw(const char* name,char* buf) {return openw(0,name,buf);};
  bool read(char* buf);
  bool append(char* buf);
  bool remove(uint32_t dir_cluster,const char* filename, char* buf);
  bool remove(const char* name, char* buf) {return remove(0,name,buf);};
  bool wipeChain(char* buf);
};

#endif


#ifndef file_h
#define file_h

#include "cluster.h"
#include "direntry.h"

/**
Class uses as the read/write interface to a file. This is the closest approximation to
the concept of "file" in UNIX, and as such includes such concepts as open, read, write,
and close.

@tparam Pk Packet class used to write debug information to a packet (passed through to members c and de)
@tparam Pr Print class used to write debug information to a Print object (passed through to members c and de)

*/
template<class Pk, class Pr, class S>
class File {
  Cluster<Pk,Pr,S>& c;
  uint32_t cluster,sector,last_cluster;
public:
  int errno;
  DirEntry<Pk,Pr,S> de;
  File(Cluster<Pk,Pr,S>& Lc):c(Lc),last_cluster(1),errno(0),de(c) {};
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

#include "file.inc"

#endif


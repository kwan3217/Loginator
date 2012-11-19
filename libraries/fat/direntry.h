#ifndef direntry_h
#define direntry_h

#include <inttypes.h>
#include "Print.h"
#include "cluster.h"

class DirEntry {
private:
  Cluster& f;  
public:
  DirEntry(Cluster& Lf):f(Lf) {};
  int errno;
  static const uint8_t ATTR_READONLY = (1<<0); 
  static const uint8_t ATTR_HIDDEN   = (1<<1); 
  static const uint8_t ATTR_SYSTEM   = (1<<2); 
  static const uint8_t ATTR_VOLUME   = (1<<3); 
  static const uint8_t ATTR_SUBDIR   = (1<<4); 
  static const uint8_t ATTR_ARCHIVE  = (1<<5); 
  static const uint8_t ATTR_RESERVE6 = (1<<6); 
  static const uint8_t ATTR_RESERVE7 = (1<<7);
  union {
    struct {
      union {
        struct {
          char shortBase[8] __attribute__((packed));
          char shortExt[3]  __attribute__((packed));
        };
        char shortName[11] __attribute__((packed));
      };
      uint8_t attr      __attribute__((packed));
      uint8_t reserved1 __attribute__((packed));
      uint8_t ctenths   __attribute__((packed));
      uint16_t ctime    __attribute__((packed));
      uint16_t cdate    __attribute__((packed));
      uint16_t adate    __attribute__((packed));
      uint16_t clusterM __attribute__((packed));
      uint16_t wtime    __attribute__((packed));
      uint16_t wdate    __attribute__((packed));
      uint16_t clusterL __attribute__((packed));
      uint32_t size     __attribute__((packed));
    };
    struct {
      uint8_t ordinal       __attribute__((packed));
      char name1[10]        __attribute__((packed));
      uint8_t longAttr      __attribute__((packed));
      uint8_t type          __attribute__((packed));
      uint8_t checksum      __attribute__((packed));
      char name2[12]        __attribute__((packed));
      uint16_t longClusterL __attribute__((packed));
      char name3[4]         __attribute__((packed));
    };
    char entry[32];
  };
  uint32_t entrySector,entryOffset,entryCluster;
  uint32_t cluster() {return ((uint32_t)clusterM)<<16 | clusterL;};
  void setCluster(uint32_t c) {clusterM=(c>>16) & 0xFFFF;clusterL=c & 0xFFFF;};
  bool isLFN() {return (attr & (ATTR_HIDDEN|ATTR_VOLUME|ATTR_SYSTEM))==(ATTR_HIDDEN|ATTR_VOLUME|ATTR_SYSTEM);};
  void print(Print &out);
  static void canonFileName(const char* fn, char* canon);
  bool find(const char* fn,uint32_t dir_cluster=0);
  bool findEmpty(uint32_t dir_cluster=0);
  bool writeBack(char* buf);
};

 
#endif


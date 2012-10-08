#ifndef PARTITION_H
#define PARTITION_H

#include <inttypes.h>
#include "sdhc.h"

class Partition {
  //We completely ignore the CHS addresses - SD cards don't even have heads 
  //Read just for interest
private:
  union {
    struct {
      uint8_t status __attribute__((packed));
      uint8_t h0 __attribute__((packed));
      uint8_t cs0 __attribute__((packed));
      uint8_t c0 __attribute__((packed));
      uint8_t type __attribute__((packed));
      uint8_t h1 __attribute__((packed));
      uint8_t cs1 __attribute__((packed));
      uint8_t c1 __attribute__((packed));
      uint32_t lba_start __attribute__((packed));
      uint32_t lba_length __attribute__((packed));
    };
    char mbr[];
  };
  SDHC &sd;
public:
  Partition(SDHC &Lsd):sd(Lsd) {};
  uint16_t first_cylinder() {return ((uint16_t)(cs0&0xC0))<<2 | c0;};
  uint8_t first_head() {return h0;};
  uint8_t first_sector() {return cs0 & 0x3F;};
  uint16_t last_cylinder() {return ((uint16_t)(cs1&0xC0))<<2 | c1;};
  uint8_t last_head() {return h1;};
  uint8_t last_sector() {return cs1 & 0x3F;};
  bool begin(int index);
  void print(Print &out);
  bool read(const uint32_t block, char* buf) {return sd.read(block+lba_start,buf);};
  bool read(const uint32_t block, char* buf, int start, int len) {return sd.read(block+lba_start,buf,start,len);};
  bool write(const uint32_t block, const char* buf){return sd.write(block+lba_start,buf);};
};

#endif

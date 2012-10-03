#ifndef PARTITION_H
#define PARTITION_H

#include <inttypes.h>
#include "sdhc.h"

class Partition {
  //We completely ignore the CHS addresses - SD cards don't even have heads 
  //Read just for interest
private:
  uint8_t status;
  uint16_t first_cylinder;
  uint8_t first_head;
  uint8_t first_sector;
  uint16_t last_cylinder;
  uint8_t type;
  uint8_t last_head;
  uint8_t last_sector;
  uint32_t lba_start;
  uint32_t lba_length;
  SDHC *sd;
  uint16_t read_uint16(char* buf, int pos) {return ((uint16_t)buf[pos+0]) << 0 | ((uint16_t)buf[pos+1]) << 8;};
  uint32_t read_uint32(char* buf, int pos) {return ((uint32_t)buf[pos+0]) << 0 | ((uint32_t)buf[pos+1]) << 8 | ((uint32_t)buf[pos+2]) << 16 | ((uint32_t)buf[pos+3]) << 24;};
  int16_t read_int16(char* buf, int pos) {return (int16_t)read_uint16(buf,pos);};
  int32_t read_int32(char* buf, int pos) {return (int32_t)read_uint32(buf,pos);};
public:
  Partition(SDHC *Lsd):sd(Lsd) {};
  bool begin(int index);
  void print(Print &out);
  bool read(const uint32_t block, char* buf) {return sd->read(block+lba_start,buf);};
  bool read(const uint32_t block, char* buf, int start, int len) {return sd->read(block+lba_start,buf,start,len);};
  bool write(const uint32_t block, const char* buf){return sd->write(block+lba_start,buf);};
};

#endif

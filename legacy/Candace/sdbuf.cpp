#include <stdio.h>
#include <string.h>
#include "sdbuf.h"
extern "C" {
#include "rootdir.h"
#include "partition.h"
#include "sd_raw.h"
}
#include "main.h"
#include "load.h"

#define SECTOR_LEN 512

struct fat16_file_struct ouf;
circular sdBuf;

int openSD(char* fn) {
  int result=root_open_new(&ouf,fn);
  return result;
}

//returns 0 on success, negative on failure
static int flushSD(void) {
  if(fat16_write_file(&ouf, sdBuf.tailPtr(), SECTOR_LEN) < 0) return -1;
  sdBuf.tailSwitch(SECTOR_LEN);
  return 0;
}

static int isFlushSDNeeded(void) {
  return sdBuf.readylen()>=SECTOR_LEN;
}

void flushAsNeeded() {
  if(isFlushSDNeeded()) {
    hasLoad(LOAD_FLUSH);
    flushSD();
  }
}

void drainToSD(circular& buf) {
  if(&buf!=&sdBuf) buf.drain(&sdBuf);
  flushAsNeeded();
}

//After running this, the sdBuf is ruined (tail pointer not on a sector boundary)
//so don't call flushSD or flushSDLast again
int flushSDLast() {
  sdBuf.mark();
  int len=sdBuf.readylen();
  if(len==0) return 0;
  len=len>SECTOR_LEN?SECTOR_LEN:len;
  if(fat16_write_file(&ouf, sdBuf.tailPtr(), len) < 0) return -1;
  sdBuf.tailSwitch(SECTOR_LEN);
  len=sdBuf.readylen();
  if(len>0) return fat16_write_file(&ouf, sdBuf.tailPtr(), len);
  return 0;
}


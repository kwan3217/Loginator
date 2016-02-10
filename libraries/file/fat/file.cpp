#include "file.h"
#include "registers.h" //Need RTC registers for file date

bool File::openr(const char* filename,uint32_t dir_cluster) {
  if(!de.find(filename,dir_cluster)) FAIL(100*de.errnum+1);
  cluster=de.cluster();
  sector=0;
  return true;
}

bool File::openw(const char* filename,uint32_t dir_cluster) {
  if(!openr(filename,dir_cluster)) {
    errnum=0; //Not opening the file is not necessarily an error
    if(!create(filename,dir_cluster)) FAIL(errnum*100+2);
  }
  de.size=0;
  if(!wipeChain()) FAIL(errnum*100+3);
  cluster=de.cluster();
  return true;
}

bool File::create(const char* filename,uint32_t dir_cluster) {
  if(!de.findEmpty(dir_cluster)) FAIL(100*de.errnum+4);    
  de.canonFileName(filename,de.shortName);
  uint32_t cl=c.findFreeCluster(last_cluster);
  if(cl==c.BAD_cluster) FAIL(100*c.errnum+5);
  de.setCluster(cl);
  return true;
}

bool File::read(char* buf) {
  if(cluster==c.EOF_cluster) return false;
  while(sector>=c.sectorsPerCluster()) {
    sector-=c.sectorsPerCluster();
    cluster=c.readTable(cluster);
    if(cluster==c.EOF_cluster) FAIL(6);
  }
  if(!c.read(cluster,sector,buf)) FAIL(100*c.errnum+7);
  sector++;
  return true;
}

bool File::sync() {
  if(!de.writeBack()) FAIL(de.errnum*100+8);
  return true;
}

bool File::append(char* buf) {
//  uint32_t write_cluster=c.BAD;
  if(de.size==0) {
    //Need to allocate first cluster
    sector=0;
    if(c.BAD_cluster==(cluster=c.findFreeCluster())) FAIL(c.errnum*100+8);
    de.setCluster(cluster);
    if(!c.write(cluster,sector,buf)) FAIL(c.errnum*100+9);
    if(!c.writeTable(cluster,c.EOF_cluster)) FAIL(c.errnum*100+10);
    last_cluster=cluster;
  } else if(sector>=c.sectorsPerCluster()) {
    //Need to allocate a new cluster
    uint32_t next_cluster=c.readTable(cluster);
    if(c.EOF_cluster==next_cluster) next_cluster=c.findFreeCluster(cluster);
    if(c.BAD_cluster==next_cluster) FAIL(c.errnum*100+16);
    sector=0;
    if(!c.write(next_cluster,sector,buf)) FAIL(c.errnum*100+11);
    if(!c.writeTable(cluster,next_cluster)) FAIL(c.errnum*100+12);
    if(!c.writeTable(next_cluster,c.EOF_cluster)) FAIL(c.errnum*100+13);
    cluster=next_cluster;
    last_cluster=cluster;
  } else {
    if(!c.write(cluster,sector,buf)) FAIL(c.errnum*100+14);
  }
  sector++;
  de.size+=c.sectorSize();
  de.wtime=de.packTime(RTCHOUR(),RTCMIN(),  RTCSEC());
  de.wdate=de.packDate(RTCYEAR(),RTCMONTH(),RTCDOM());
  if(!de.writeBack()) FAIL(de.errnum*100+15);
  return true;
}

bool File::wipeChain() {
  cluster=de.cluster();
  while(cluster!=0 && cluster!=c.EOF_cluster) {
    uint32_t next_cluster=c.readTable(cluster);
    if(next_cluster!=0) if(!c.writeTable(cluster,0)) FAIL(c.errnum*100+16);
    cluster=next_cluster;
  }
  return true;
}

bool File::remove(const char* filename, char* buf,uint32_t dir_cluster) {
  if(!de.find(filename,dir_cluster)) FAIL(de.errnum*100+17);
  de.entry[0]=0xE5;
  if(!de.writeBack()) FAIL(de.errnum*100+18);
  if(!wipeChain()) FAIL(errnum*100+19);
  return true;
}


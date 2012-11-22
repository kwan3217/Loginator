#include "file.h"

bool File::openr(const char* filename,uint32_t dir_cluster) {
  if(!de.find(filename,dir_cluster)) FAIL(100*de.errno+1);
  cluster=de.cluster();
  sector=0;
  return true;
}

bool File::openw(const char* filename,char* buf,uint32_t dir_cluster) {
  if(!openr(filename,dir_cluster)) {
    errno=0; //Not opening the file is not necessarily an error
    if(!create(filename,buf,dir_cluster)) FAIL(errno*100+2);
  }
  de.size=0;
  if(!wipeChain(buf)) FAIL(errno*100+3);
  cluster=de.cluster();
  return true;
}

bool File::create(const char* filename, char* buf,uint32_t dir_cluster) {
  if(!de.findEmpty(dir_cluster)) FAIL(100*de.errno+4);    
  de.canonFileName(filename,de.shortName);
  uint32_t cl=c.findFreeCluster(buf);
  if(cl==c.BAD) FAIL(100*c.errno+5);
  de.setCluster(cl);
  return true;
}

bool File::read(char* buf) {
  if(cluster==c.EOF) return false;
  while(sector>=c.sectorsPerCluster()) {
    sector-=c.sectorsPerCluster();
    cluster=c.readTable(cluster);
    if(cluster==c.EOF) FAIL(6);
  }
  if(!c.read(cluster,sector,buf)) FAIL(100*c.errno+7);
  sector++;
  return true;
}

bool File::append(char* buf, char* fastBuf) {
//  uint32_t write_cluster=c.BAD;
  if(de.size==0) {
    sector=0;
    if(c.BAD==(cluster=c.findFreeCluster(fastBuf))) FAIL(c.errno*100+8);
    de.setCluster(cluster);
    if(!c.write(cluster,sector,buf)) FAIL(c.errno*100+9);
    //Now the buffer is ours
    if(!c.writeTable(cluster,buf,c.EOF)) FAIL(c.errno*100+10);
  } else if(sector>=c.sectorsPerCluster()) {
    uint32_t next_cluster=c.readTable(cluster);
    if(c.EOF==next_cluster) next_cluster=c.findFreeCluster(fastBuf,cluster);
    if(c.BAD==next_cluster) FAIL(c.errno*100+16);
    sector=0;
    if(!c.write(next_cluster,sector,buf)) FAIL(c.errno*100+11);
    //Now the buffer is ours
    if(!c.writeTable(cluster,buf,next_cluster)) FAIL(c.errno*100+12);
    if(!c.writeTable(next_cluster,buf,c.EOF)) FAIL(c.errno*100+13);
    cluster=next_cluster;
  } else {
    if(!c.write(cluster,sector,buf)) FAIL(c.errno*100+14);
  }
  sector++;
  de.size+=c.sectorSize();
  if(!de.writeBack(buf)) FAIL(de.errno*100+15);
  return true;
}

bool File::wipeChain(char* buf) {
  cluster=de.cluster();
  while(cluster!=0 && cluster!=c.EOF) {
    uint32_t next_cluster=c.readTable(cluster);
    if(next_cluster!=0) if(!c.writeTable(cluster,buf,0)) FAIL(c.errno*100+16);
    cluster=next_cluster;
  }
  return true;
}

bool File::remove(const char* filename, char* buf,uint32_t dir_cluster) {
  if(!de.find(filename,dir_cluster)) FAIL(de.errno*100+17);
  de.entry[0]=0xE5;
  if(!de.writeBack(buf)) FAIL(de.errno*100+18);
  if(!wipeChain(buf)) FAIL(errno*100+19);
  return true;
}


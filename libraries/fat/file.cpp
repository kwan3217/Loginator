#include "file.h"

bool File::openr(const char* filename,uint32_t dir_cluster) {
  if(!de.find(dir_cluster,filename)) return false;
  cluster=de.cluster();
  sector=0;
  return true;
}

bool File::openw(const char* filename,char* buf,uint32_t dir_cluster) {
  if(!openr(filename,dir_cluster)) {
    if(!create(filename,buf,dir_cluster)) {
      errno=errno*100+1;
      return false;
    }
  }
  de.size=0;
  if(!wipeChain(buf)) {
    errno=errno*100+2;
    return false;
  }
  cluster=de.cluster();
}

bool File::create(const char* filename, char* buf,uint32_t dir_cluster) {
  if(!de.findEmpty(dir_cluster)) {
    
    return false;
  }
  de.canonFileName(filename,de.shortName);
  uint32_t cl=c.findFreeCluster();
  if(cl==c.BAD) return false;
  de.setCluster(cl);
}

bool File::read(char* buf) {
  if(cluster==c.EOF) return false;
  while(sector>=c.sectorsPerCluster()) {
    sector-=c.sectorsPerCluster();
    cluster=c.readTable(cluster);
    if(cluster==c.EOF) return false;
  }
  if(!c.read(cluster,sector,buf)) return false;
  sector++;
  return true;
}

bool File::append(char* buf) {
//  uint32_t write_cluster=c.BAD;
  if(de.size==0) {
    sector=0;
    if(c.BAD==(cluster=c.findFreeCluster())) return false;
    de.setCluster(cluster);
    if(!c.write(cluster,sector,buf)) return false;
    //Now the buffer is ours
    if(!c.writeTable(cluster,buf,c.EOF)) return false;
  } else if(sector>=c.sectorsPerCluster()) {
    uint32_t next_cluster=c.readTable(cluster);
    if(next_cluster==c.EOF) next_cluster=c.findFreeCluster(cluster);
    sector=0;
    if(!c.write(next_cluster,sector,buf)) return false;
    //Now the buffer is ours
    if(!c.writeTable(cluster,buf,next_cluster)) return false;
    if(!c.writeTable(next_cluster,buf,c.EOF)) return false;
    cluster=next_cluster;
  } else {
    if(!c.write(cluster,sector,buf)) return false;
    //Now the buffer is ours
  }
  sector++;
  de.size+=c.sectorSize();
  if(!de.writeBack(buf)) return false;
  return true;
}

bool File::wipeChain(char* buf) {
  cluster=de.cluster();
  while(cluster!=0 && cluster!=c.EOF) {
    uint32_t next_cluster=c.readTable(cluster);
    if(next_cluster!=0) if(!c.writeTable(cluster,buf,0)) {
      errno=1;
      return false;
    }
    cluster=next_cluster;
  }
  return true;
}

bool File::remove(const char* filename, char* buf,uint32_t dir_cluster) {
  if(!de.find(dir_cluster,filename)) return false;
  de.entry[0]=0xE5;
  if(!de.writeBack(buf)) return false;
  return wipeChain(buf);
}


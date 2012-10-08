#include "file.h"

bool File::openr(uint32_t dir_cluster, const char* filename) {
  if(!de.find(dir_cluster,filename)) return false;
  cluster=de.cluster();
  sector=0;
  return true;
}

bool File::openw(uint32_t dir_cluster, const char* filename,char* buf) {
  if(!openr(dir_cluster,filename)) {
    if(!create(dir_cluster,filename,buf)) return false;
  }
  de.size=0;
  if(!wipeChain(buf)) return false;
  cluster=de.cluster();
}

bool File::create(uint32_t dir_cluster,const char* filename, char* buf) {

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
  uint32_t write_cluster=c.BAD;
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
    if(!c.writeTable(cluster,buf,0)) return false;
    cluster=next_cluster;
  }
  return true;
}

bool File::remove(uint32_t dir_cluster,const char* filename, char* buf) {
  if(!de.find(dir_cluster,filename)) return false;
  de.entry[0]=0xE5;
  if(!de.writeBack(buf)) return false;
  return wipeChain(buf);
}


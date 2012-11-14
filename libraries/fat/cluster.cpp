#include "cluster.h"

bool Cluster::begin() {
  if(!p.read(0,bpb,510,2)) {
    errno=100*p.errno+1;
    return false;
  }
  if(bpb[0]!=0x55) {
    errno=2;
    return false;
  }
  if(bpb[1]!=0xAA) FAIL(3);
  if(!p.read(0,bpb,0x0B,sizeof(bpb))) FAIL(p.errno*100+4);
  if(bytesPerSector!=512) FAIL(p.errno+5);

  //The following implements ceil(numRootEntries*32/bytesPerSector);
  uint32_t rootDirSectors=((numRootEntries16*32)+(bytesPerSector-1))/bytesPerSector;

  uint32_t sectorsPerTable=(sectorsPerTable16!=0)?sectorsPerTable16:sectorsPerTable32;
  uint32_t totalSectors=(numSectors16!=0)?numSectors16:numSectors32;
  numClusters=totalSectors/sectorsPerCluster8;
  if(numClusters<4085) {
    tableEntrySize=12;
  } else if(numClusters<65525) {
    tableEntrySize=16;
  } else {
    tableEntrySize=32;
  }
  firstRootSector=reservedSectors+(numTables*sectorsPerTable);
  firstDataSector=firstRootSector+rootDirSectors;
  if(tableEntrySize==32) {
    firstRootSector=clusterFirstSector(rootCluster);
  }
  return true;
}

void Cluster::print(Print& out, Dump &d) {
  d.region(bpb,0,sizeof(bpb),16);
  out.print("bytesPerSector:    ");
  out.println(bytesPerSector);
  out.print("sectorsPerCluster: ");
  out.println(sectorsPerCluster8,DEC);
  out.print("reservedSectors:   ");
  out.println(reservedSectors);
  out.print("numTables:         ");
  out.println(numTables,DEC);
  out.print("numRootEntries:    ");
  out.println(numRootEntries16);
  out.print("numSectors16:      ");
  out.println(numSectors16  );
  out.print("mediaDesc:         ");
  out.println(mediaDesc    ,DEC);
  out.print("sectorsPerTable16: ");
  out.println(sectorsPerTable16);
  out.print("sectorsPerTrack:   ");
  out.println(sectorsPerTrack );
  out.print("numHeads:          ");
  out.println(numHeads   );
  out.print("numHiddenSectors:  ");
  out.println((unsigned int)numHiddenSectors);
  out.print("numSectors32:      ");
  out.println((unsigned int)numSectors32  );
  out.print("sectorsPerTable32: ");
  out.println((unsigned int)sectorsPerTable32);
  out.print("flags1:            ");
  out.println(flags1);
  out.print("version:           ");
  out.println(version);
  out.print("rootCluster:       ");
  out.println((unsigned int)rootCluster);
  out.print("fsInfoSector:      ");
  out.println(fsInfoSector);
  out.print("bootCopySector:    ");
  out.println(bootCopySector);
  out.print("reserved2:         ");
  out.write(reserved2,sizeof(reserved2));
  out.println();
  out.print("numClusters:       ");
  out.println((unsigned int)numClusters);
  out.print("tableEntrySize:    ");
  out.println((unsigned int)tableEntrySize);
  out.print("firstDataSector:   ");
  out.println((unsigned int)firstDataSector);
  out.print("firstRootSector:   ");
  out.println((unsigned int)firstRootSector);
}

void Cluster::calcTableCluster(uint32_t cluster, uint32_t& sectorsPerTable, uint32_t& entrySector, uint32_t& entryOffset) {
  const int tableNumber=0; //If for some reason you want to read another table, change this
  sectorsPerTable=(sectorsPerTable16!=0)?sectorsPerTable16:sectorsPerTable32;
  uint32_t tableOffset=cluster*(tableEntrySize/8);
  entrySector=reservedSectors+(tableOffset/bytesPerSector)+tableNumber*sectorsPerTable;
  entryOffset=tableOffset % bytesPerSector;
}

uint32_t Cluster::readTable(uint32_t cluster) {
  if(tableEntrySize!=12) {
    uint32_t sectorsPerTable, entrySector, entryOffset;
    calcTableCluster(cluster, sectorsPerTable, entrySector, entryOffset);
    uint32_t result=0;
    if(!p.read(entrySector,(char*)&result,entryOffset,tableEntrySize/8)) {
      errno=p.errno*100+6;
      return BAD;
    }
    if(tableEntrySize==16) result+=0x0FFF0000; //Homogenize 16-bit and 32-bit table entries
    result &= 0x0FFFFFFF;                      //chop off reserved bits
    if(result>=0x0FFFFFF8) result =0x0FFFFFFF; //Homogenize end-of-chain marker
    return result;
  }
  errno=7;
  return BAD;
}

/** Write a particular value in a slot in the file allocation table(s).
This involves reading the whole sector containing this slot, changing the value in the
 slot, then writing the whole sector back out. As a result, this function needs
 a 512-byte buffer that it is allowed to write on

\param cluster Cluster number to write to. Remember that the first usable cluster
 is numbered 2. If you want that cluster, pass 2, not zero. Clusters 1 and 0 do not exist.
\param buf Writable buffer large enough to hold 1 sector (512 bytes). Buffer 
contents are not saved and are liable to be overwritten.
\param entry Value to write in this slot in the table. If you want to write one of the special
cluster values such as eof or bad, write them in 32 bit form - 0x0FFFFFFF is end of chain
and bad sector is 0x0FFFFFFF7

This code presumes that the filesystem has multiple tables, that the tables were
in sync when the system was started, and makes sure that they staty that way. 
It reads from the first table only, writes back to each table, and the 
assumptions made above make this valid.
*/
bool Cluster::writeTable(uint32_t cluster, char* buf, uint32_t entry) {
  if(tableEntrySize!=12) {
    uint32_t sectorsPerTable, entrySector, entryOffset;
    calcTableCluster(cluster, sectorsPerTable, entrySector, entryOffset);
    if(!p.read(entrySector,buf)) FAIL(p.errno*100+8);
    if(tableEntrySize==16) {
      buf[entryOffset+0]=(uint8_t)(entry & 0xFF);
      buf[entryOffset+1]=(uint8_t)((entry>>8) & 0xFF);
    } else if(tableEntrySize==32) {
      buf[entryOffset+0]=(uint8_t)(entry & 0xFF);
      buf[entryOffset+1]=(uint8_t)((entry>>8) & 0xFF);
      buf[entryOffset+2]=(uint8_t)((entry>>16) & 0xFF);
      buf[entryOffset+3]=(buf[entryOffset+3]&0xC0)|(uint8_t)((entry>>24) & 0x3F);
    }
    for(int i=0;i<numTables;i++) if(!p.write(entrySector+i*sectorsPerTable,buf)) FAIL(p.errno*100+9);
    return true;
  } else {
    errno=10;
    return false;
  }
}

/** Find a free cluster in the table, starting at the cluster after the cluster 
given and searching until a free cluster is found or we wrap back around to the
start cluster.

This is spectacularly time-inefficient, reading a full sector each time it 
checks a cluster, but OK in the normal case when writing a log
on an not-too-heavily fragmented device. It is memory-efficient in that it only
reads one entry into memory each time, so it doesn't need a buffer.
\param startCluster If you have just filled up a cluster and want to find the 
next available cluster after that, pass the number of the cluster you have just
filled. The default value starts searching at the beginning of the table.
*/
uint32_t Cluster::findFreeCluster(uint32_t startCluster) {
  for(uint32_t i=startCluster+1;i<numClusters+2;i++) {
    uint32_t result=readTable(i);
    if(errno!=0) FAIL(errno*100+11);
    if(result==0) return i;
  }
  for(uint32_t i=2;i<=startCluster;i++) {
    uint32_t result=readTable(i);
    if(errno!=0) FAIL(errno*100+12);
    if(result==0) return i;
  }
  errno=13;
  return BAD;
}

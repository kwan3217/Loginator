#ifndef CLUSTER_H
#define CLUSTER_H

#include "Partition.h"
#include "Print.h"
#include "dump.h"

/** Extended BIOS parameter block. Included separately because it could
appear at one of two places in the block.
*/
struct BPBExtension {
  uint8_t  __attribute__ ((packed)) numDrive;           ///< Physical drive number, for information purposes only
  uint8_t  __attribute__ ((packed)) reserved1;          ///< Doesn't mean anything to us
  uint8_t  __attribute__ ((packed)) extBootSig;         ///< Magic number, sould be 0x29, could be 0x28, both are valid
  uint32_t volumeID __attribute__ ((packed));           ///< 32-bit Volume ID, normally printed 1234-ABCD in directory listings
  char* volumeLabel[11] __attribute__ ((packed));       ///< Partition volume label - should be a copy of the normal volume label
                                                        ///< but this is accessible without parsing the file structure to find it
  char* fsDisplayType[8] __attribute__ ((packed));      ///< File System Display type, 
                                                        ///< but this is accessible without parsing the file structure to find it
};

/**
  Class to provide access to a FAT formatted filesystem by cluster and sector. 
  Also provides services to read and write the file allocation table.

*/
class Cluster {
private:
  Partition& p;
  union {
    struct {
      //Original BIOS Parameter Block (BPB)
      uint16_t __attribute__ ((packed)) bytesPerSector;     ///< Size of the unit "sector" used elsewhere in the table, in bytes. This code supports only 512-byte sectors
      uint8_t  __attribute__ ((packed)) sectorsPerCluster8;  ///< Size of the unit "cluster" in sectors
      uint16_t __attribute__ ((packed)) reservedSectors;    ///< Number of sectors before first file allocation table
      uint8_t  __attribute__ ((packed)) numTables;          ///< Number of file allocation tables
      uint16_t __attribute__ ((packed)) numRootEntries16;   ///< Number of root directory entries, ignored on FAT32 since a root directory is an ordinary file
      uint16_t __attribute__ ((packed)) numSectors16;       ///< Total number of logical sectors if it will fit in 16 bits, otherwise \see numSectors32
      uint8_t  __attribute__ ((packed)) mediaDesc;          ///< Media descriptor, for information purposes only
      uint16_t __attribute__ ((packed)) sectorsPerTable16;  ///< Sectors per table, ignored on FAT32
      //DOS 3.31 BPB. Information to be trusted if and only if numSectors16 is 0
      uint16_t __attribute__ ((packed)) sectorsPerTrack;    ///< Sectors per track, for information purposes only
      uint16_t __attribute__ ((packed)) numHeads;           ///< Number of heads, for information purposes only
      uint32_t __attribute__ ((packed)) numHiddenSectors;   ///< Number of heads, for information purposes only
      uint32_t __attribute__ ((packed)) numSectors32;       ///< Total number of logical sectors
      //Extended BPB
      union {
        struct {
          //FAT32 BPB extension slipped in between DOS3.31 BPB and extended BPB
          uint32_t __attribute__ ((packed)) sectorsPerTable32;  ///< Sectors per table if more than 16 bit
          uint16_t __attribute__ ((packed)) flags1;             ///< Supposedly bit 7 clear means that tables are mirrored as usual
          uint16_t __attribute__ ((packed)) version;            ///< Filesystem version number, should be 0
          uint32_t __attribute__ ((packed)) rootCluster;        ///< First cluster of root directory, should be 2
          uint16_t __attribute__ ((packed)) fsInfoSector;       ///< Logical sector number of FS information sector
          uint16_t __attribute__ ((packed)) bootCopySector;     ///< Logical sector number of first sector of a copy of the three FAT32 boot sectors. Typically 6.  
          char     __attribute__ ((packed)) reserved2[12];      ///< Doesn't mean anything to us
          BPBExtension __attribute__ ((packed)) EBPB32;         ///< Extended BPB continues here
        };
        BPBExtension EBPB;
      };       
    };
    char bpb[79];
  };
  uint32_t numClusters;       ///< Number of clusters in the filesystem
  uint32_t tableEntrySize;    ///< Number of bits in each entry of file allocation table
  uint32_t firstDataSector;   ///< Sector number of first sector in data section
  uint32_t clusterFirstSector(uint32_t cluster) {
    //As a special case, treat cluster 0 as first cluster of rootdir, whether fat12/16 where it is
    //out of the data area, or fat32, where it really is a cluster (probably 2)
    if(cluster==0) return firstRootSector; //This is set properly in begin() for both fat12/16 and fat32
    return ((cluster-2)*sectorsPerCluster8)+firstDataSector;
  };
  void calcTableCluster(uint32_t cluster, uint32_t& sectorsPerTable, uint32_t& entrySector, uint32_t& entryOffset);
public:
  Cluster(Partition &Lp):p(Lp) {};
  uint8_t sectorsPerCluster() {return sectorsPerCluster8;};
  uint16_t sectorSize() {return bytesPerSector;};
  uint32_t numRootEntries() {if(tableEntrySize==32) return bytesPerSector*sectorsPerCluster8/32; else return numRootEntries16;};
  uint32_t firstRootSector;   ///< Sector number of first sector of root directory, valid only for FAT12 and FAT16
  bool begin();
  bool read(uint32_t cluster, uint8_t sector, char* buf, int start, int len) {return p.read(clusterFirstSector(cluster)+sector,buf,start,len);};
  bool read(uint32_t cluster, uint8_t sector, char* buf) {return p.read(clusterFirstSector(cluster)+sector,buf);};
  bool write(uint32_t cluster, uint8_t sector, char* buf) {return p.write(clusterFirstSector(cluster)+sector,buf);};
  void print(Print &out, Dump &d);
  uint32_t readTable(uint32_t cluster);
  bool writeTable(uint32_t cluster, char* buf, uint32_t entry);
  uint32_t findFreeCluster(uint32_t clusterToStart=1);
  static const uint32_t BAD=0x0FFFFFF7;
  static const uint32_t EOF=0x0FFFFFFF;
};

#endif

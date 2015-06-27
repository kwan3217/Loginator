#include <string.h>
#include "direntry.h"

uint16_t DirEntry::packTime(int h, int n, int s) {
                      //width        //pos
  return (( h    & ((1 << (5+1))-1)) << 11) |
         (( n    & ((1 << (6+1))-1)) <<  5) |
         (((s/2) & ((1 << (5+1))-1)) <<  0) ;
}

uint16_t DirEntry::packDate(int y, int m, int d) {
  return (((y+(y>1980?-1980:20)) & ((1 << (7+1))-1)) << 9) |
         ((m                     & ((1 << (4+1))-1)) << 5) |
         ((d                     & ((1 << (5+1))-1)) << 0) ;
}

void DirEntry::print(Print& out) {
  if(((unsigned char)(shortName[0]))==0xE5) {
    out.println("Entry represents deleted file");
  } else if(shortName[0]==0x00) {
    out.println("Entry is empty - no entries past this one");
  } else if(isLFN()) {
    out.println("Entry is part of a long filename");
  } else {
    out.println("Entry has a valid filename");
  }
  if(isLFN()) {
    out.print("Ordinal:      ");
    out.println(ordinal,DEC);
    out.print("Name:         ");
    for(int i=0;i<10;i+=2) out.print(name1[i]);
    for(int i=0;i<12;i+=2) out.print(name2[i]);
    for(int i=0;i<4;i+=2)  out.print(name3[i]);
    out.println();
    out.print("Attributes:   0x");
    out.print(longAttr,HEX,2);
    out.print(' ');
    out.print((longAttr & ATTR_READONLY)?'R':'-');
    out.print((longAttr & ATTR_HIDDEN)?'H':'-');
    out.print((longAttr & ATTR_SYSTEM)?'S':'-');
    out.print((longAttr & ATTR_VOLUME)?'V':'-');
    out.print((longAttr & ATTR_SUBDIR)?'D':'-');
    out.print((longAttr & ATTR_ARCHIVE)?'A':'-');
    out.print((longAttr & ATTR_RESERVE6)?'6':'-');
    out.print((longAttr & ATTR_RESERVE7)?'7':'-');
    out.println();
    out.print("Type:         ");
    out.println(type,DEC);
    out.print("Checksum:     ");
    out.println(type,DEC);
    out.print("longClusterL: ");
    out.println(type,DEC);
  } else {
    out.print("Name:       ");
    out.write(shortBase,sizeof(shortBase));
    out.print('.');
    out.write(shortExt,sizeof(shortExt));
    out.println();
    out.print("Attributes: 0x");
    out.print(attr,HEX,2);
    out.print(' ');
    out.print((attr & ATTR_READONLY)?'R':'-');
    out.print((attr & ATTR_HIDDEN)?'H':'-');
    out.print((attr & ATTR_SYSTEM)?'S':'-');
    out.print((attr & ATTR_VOLUME)?'V':'-');
    out.print((attr & ATTR_SUBDIR)?'D':'-');
    out.print((attr & ATTR_ARCHIVE)?'A':'-');
    out.print((attr & ATTR_RESERVE6)?'6':'-');
    out.print((attr & ATTR_RESERVE7)?'7':'-');
    out.println();
    out.print("Cluster:    ");
    out.println((unsigned int)cluster());
    out.print("Size:       ");
    out.println((unsigned int)size);
  }
  out.print("EntryCluster: ");
  out.println((unsigned int)entryCluster);
  out.print("EntrySector:  ");
  out.println((unsigned int)entrySector);
  out.print("EntryOffset:  ");
  out.println((unsigned int)entryOffset);
}

/** Convert a filename from the normal dotted form to the form used in directory
entries, with an implied dot between filename and extension, and with the name
and extension both padded with spaces to 8 and 3 characters respectively. Also
converts to upper case. 

\param fn Input filename, in normal human-readable format
\param canon output filename, all uppercase, implied dot, padded with spaces. 
Buffer must be provided, writable, and at least 11 characters long. The first
11 characters, no more, no less, will always be written by this function
*/
void DirEntry::canonFileName(const char* fn, char* canon) {
  bool foundDot=false;
  bool foundEnd=false;
  int j=0; //Pointer within fn, will stop advancing once dot or end is hit
  for(int i=0;i<8;i++) {
    if(fn[j]==0) foundEnd=true;
    if(fn[j]=='.') foundDot=true;
    if(foundEnd || foundDot) {
      canon[i]=' ';
    } else {
      canon[i]=fn[j];
      if(canon[i]>='a' && canon[i]<='z') canon[i]-=32;
      j++;
    }
  }
  do {
    if(fn[j]==0) foundEnd=true;
    if(fn[j]=='.') foundDot=true;
    j++;
  } while(!foundDot&&!foundEnd);
  for(int i=8;i<11;i++) {
    if(fn[j]==0) foundEnd=true;
    if(foundEnd) {
      canon[i]=' ';
    } else {
      canon[i]=fn[j];
      if(canon[i]>='a' && canon[i]<='z') canon[i]-=32;
      j++;
    }
  }      
}

bool DirEntry::find(const char* fn,uint32_t dir_cluster) {
  char canon[11];
  canonFileName(fn,canon);
  uint32_t entryIndex=0;
  entryCluster=dir_cluster;
  bool match=false;
  while(entryIndex<f.numRootEntries()) {
    entrySector=entryIndex*32;
    entryOffset=entrySector%512;
    entrySector=entrySector/512;
    if(!f.read(entryCluster,entrySector,entry,entryOffset,sizeof(entry))) FAIL(f.errno*100+1);
    if(entry[0]==0) return false;
    if(!isLFN()) {
      int i=0;
      match=true;
      while((i<11)&&match) {
        match=(canon[i]==entry[i]);
        i++;
      }
      if(match) return true; 
    }
    entryIndex++;
  }
  return false;
}

bool DirEntry::findEmpty(uint32_t dir_cluster) {
  entryCluster=dir_cluster;
  //See if there is an unused entry first, preserve deleted entries
  for(uint32_t entryIndex=0;entryIndex<f.numRootEntries();entryIndex++) {
    entrySector=entryIndex*32;
    entryOffset=entrySector%512;
    entrySector=entrySector/512;
    if(!f.read(entryCluster,entrySector,entry,entryOffset,sizeof(entry))) FAIL(f.errno*100+2);
    if(entry[0]==0) return true;
  }
  //No unused entries, have to use a deleted entry
  for(uint32_t entryIndex=0;entryIndex<f.numRootEntries();entryIndex++) {
    entrySector=entryIndex*32;
    entryOffset=entrySector%512;
    entrySector=entrySector/512;
    if(!f.read(entryCluster,entrySector,entry,entryOffset,sizeof(entry))) FAIL(f.errno*100+3);
    if(((unsigned char)(entry[0]))==0xE5) return true;
  }
  return false;
}

bool DirEntry::writeBack() {
  
  if(!f.read(entryCluster,entrySector,buf)) FAIL(f.errno*100+4);
  char* pentry=(char*)&entry;
  for(unsigned int i=0;i<sizeof(entry);i++) {
    buf[entryOffset+i]=pentry[i];
  }
  if(!f.write(entryCluster,entrySector,buf)) FAIL(f.errno*100+5);
  return true;
}

    


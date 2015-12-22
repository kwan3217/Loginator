#include "Partition.h"

template<class Pk, class Pr, class S> inline
bool Partition<Pk,Pr,S>::begin(int index) {
  if(!sd.read(0,mbr,0x1fe,2)) FAIL(sd.errnum*100+1);
  if(mbr[0x0]!=0x55) FAIL(2);
  if((unsigned char)mbr[0x1]!=0xAA) FAIL(3);
  if(!sd.read(0,mbr,(index-1)*0x10+0x1be,16)) FAIL(sd.errnum*100+4);
  return true;
}

template<class Pk, class Pr, class S> inline
void Partition<Pk,Pr,S>::print(Pr &out) {
  out.print("Status:         ");
  out.println(status,HEX);
  out.print("First cylinder: ");
  out.println(first_cylinder(),DEC);
  out.print("First head:     ");
  out.println(first_head(),DEC);
  out.print("First sector:   ");
  out.println(first_sector(),DEC);
  out.print("Type:         ");
  out.println(type,HEX);
  out.print("Last cylinder:  ");
  out.println(last_cylinder(),DEC);
  out.print("Last head:      ");
  out.println(last_head(),DEC);
  out.print("Last sector:    ");
  out.println(last_sector(),DEC);
  out.print("LBA start:      ");
  out.println((int)lba_start,HEX);
  out.print("LBA length:     ");
  out.println((int)lba_length,HEX);
}

template<class Pk, class Pr, class S>
bool Partition<Pk,Pr,S>::write(const uint32_t block, const char* buf, uint32_t trace) {
  ASSERT(sd.write(block+lba_start,buf,trace),sd.errnum*100+7);
};


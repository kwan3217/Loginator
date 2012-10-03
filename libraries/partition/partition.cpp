#include "partition.h"

bool Partition::begin(int index) {
  char buf[16];
  int start=index*0x10+0x1be;
  if(!sd->read(0,buf,start,2)) return false;
  if(buf[0x0]!=0x55) return false;
  if(buf[0x1]!=0xAA) return false;
  if(!sd->read(0,buf,start,16)) return false;
  status=buf[0x00];
  type=buf[0x04];
  first_cylinder=((uint16_t)(buf[0x02]&0xC0))<<2 | buf[0x03];
  first_head=buf[start+0x01];
  first_sector=buf[start+0x02] & 0x3F;
  last_cylinder=((uint16_t)(buf[0x02+0x04]&0xC0))<<2 | buf[0x03+0x04];
  last_head=buf[0x01+0x04];
  last_sector=buf[0x02+0x04] & 0x3F;
  lba_start=read_uint32(buf,0x08);
  lba_length=read_uint32(buf,0x0C);
  
  return true;
}

void Partition::print(Print &out) {
  out.print("Status:         ");
  out.println(status,HEX);
  out.print("First cylinder: ");
  out.println(first_cylinder,DEC);
  out.print("First head:     ");
  out.println(first_head,DEC);
  out.print("First sector:   ");
  out.println(first_sector,DEC);
  out.print("Type:         ");
  out.println(type,HEX);
  out.print("Last cylinder:  ");
  out.println(last_cylinder,DEC);
  out.print("Last head:      ");
  out.println(last_head,DEC);
  out.print("Last sector:    ");
  out.println(last_sector,DEC);
  out.print("LBA start:      ");
  out.println((int)lba_start,HEX);
  out.print("LBA length:     ");
  out.println((int)lba_length,HEX);

}

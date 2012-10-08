#include "Partition.h"

bool Partition::begin(int index) {
  if(!sd.read(0,mbr,0x1fe,2)) return false;
  if(mbr[0x0]!=0x55) return false;
  if(mbr[0x1]!=0xAA) return false;
  if(!sd.read(0,mbr,(index-1)*0x10+0x1be,16)) return false;
  return true;
}

void Partition::print(Print &out) {
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

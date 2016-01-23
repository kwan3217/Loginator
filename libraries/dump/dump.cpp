#include "dump.h"

void IntelHex::print_byte(unsigned char b) {
  checksum+=b;
  out.print(b>>4,HEX);
  out.print(b & 0x0F,HEX);
}

void IntelHex::print16(unsigned short b) {
  print_byte((b >> (8*0)) & 0xFF);
  print_byte((b >> (8*1)) & 0xFF);
}

void IntelHex::print32(unsigned int b) {
  print_byte((b >> (8*0)) & 0xFF);
  print_byte((b >> (8*1)) & 0xFF);
  print_byte((b >> (8*2)) & 0xFF);
  print_byte((b >> (8*3)) & 0xFF);
}

void IntelHex::begin_line(unsigned char len, unsigned short a, unsigned char type) {
  checksum=0;
  out.print(":");
  print_byte(len);
  print_byte(a>>8);
  print_byte(a & 0xFF);
  print_byte(type);
}

void IntelHex::end_line() {
  print_byte(256-checksum);
  out.println();
}

void IntelHex::begin() {
  addr=0;
}

void IntelHex::end() {
  begin_line(0,0,1);
  end_line();
}

void IntelHex::address(size_t ia) {
  if((ia & 0xFFFF0000) != (addr & 0xFFFF0000)) {
    addr=ia;
    begin_line(2,0,4);
    print_byte((addr>>24) & 0xFF);
    print_byte((addr>>16) & 0xFF);
    end_line();
  }
}

void IntelHex::line(const char* start, size_t base, size_t len) {
  address(base);
  begin_line(len,((unsigned int)base)&0xFFFF,0);
  for(int i=0;i<len;i++) print_byte(start[i]);
  end_line();
}

void Base85::print_group(const char* p, size_t len) {
  unsigned int group=0;
  for(int i=0;i<4;i++) {
    group<<=8;
    if(i<len) group |= p[i];
  }
  char group_c[5];
  for(int i=0;i<5;i++) {
    group_c[4-i]=(group % 85)+33;
    group/=85;
  }
  for(int i=0;i<len+1;i++) out.print(group_c[i]);
}

void Base85::line(const char* start, size_t base, size_t len) {
  while(len>0) {
    print_group(start,len>4?4:len);
    start+=4;
    len-=4;
  }
  out.println();
}

void Hd::line(const char* start, size_t base, size_t len) {
  out.print((unsigned int)base,HEX,4);
  out.print(' ');
  for(int i=0;i<len;i++) {
    out.print(start[i],HEX,2);
    if(i%4==3) out.print(' ');
  }
  for(int i=len;i<preferredLen;i++) {
    out.print("  ");
    if(i%4==3) out.print(' ');
  }
  out.print(' ');
  for(int i=0;i<len;i++) out.print((start[i]>=32 && start[i]<127)?start[i]:'.');
  out.println();
}



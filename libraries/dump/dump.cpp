#include "dump.h"

extern char _btext[],_etext[];

void dump::dumpText() {
  unsigned int btext=(unsigned int)(&_btext);
  unsigned int etext=(unsigned int)(&_etext);
  region(_btext,etext-btext,16);
}

void IntelHex::print_byte(unsigned char b) {
  checksum+=b;
  out->print(b>>4,HEX);
  out->print(b & 0x0F,HEX);
}

void IntelHex::begin_line(unsigned char len, unsigned short a, unsigned char type) {
  checksum=0;
  out->print(":");
  print_byte(len);
  print_byte(a>>8);
  print_byte(a & 0xFF);
  print_byte(type);
}

void IntelHex::end_line() {
  print_byte(256-checksum);
  out->println();
}

void IntelHex::begin() {
  addr=0;
}

void IntelHex::end() {
  begin_line(0,0,1);
  end_line();
}

void IntelHex::address(int ia) {
  if((ia & 0xFFFF0000) != (addr & 0xFFFF0000)) {
    addr=ia;
    begin_line(2,0,4);
    print_byte((addr>>24) & 0xFF);
    print_byte((addr>>16) & 0xFF);
    end_line();
  }
}

void IntelHex::line(const char* start, int base, int len) {
  address(base);
  begin_line(len,((unsigned int)base)&0xFFFF,0);
  for(int i=0;i<len;i++) print_byte(start[i]);
  end_line();
}

void IntelHex::region(const char* p, int base, int len, int rec_len) {
  begin();
  while(len>0) {
    if(rec_len>len) rec_len=len;
    line(p,base,rec_len);
    base+=rec_len;
    p+=rec_len;
    len-=rec_len;
  }
  end();
}

void Base85::print_group(const char* p, int len) {
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
  for(int i=0;i<len+1;i++) out->print(group_c[i]);
}

void Base85::line(const char* start, int base, int len) {
  while(len>0) {
    print_group(start,len>4?4:len);
    start+=4;
    len-=4;
  }
  out->println();
}

void Base85::region(const char* p, int base, int len, int rec_len) {
  while(len>0) {
    if(rec_len>len) rec_len=len;
    line(p,base,rec_len);
    base+=rec_len;
    p+=rec_len;
    len-=rec_len;
  }
}





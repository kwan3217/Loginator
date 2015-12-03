template<class T,class P> inline
void Dump<T,P>::region(const char* p, size_t base, size_t len, unsigned int rec_len) {
  begin();
  while(len>0) {
    if(rec_len>len) rec_len=len;
    static_cast<T*>(this)->line(p,base,rec_len);
    base+=rec_len;
    p+=rec_len;
    len-=rec_len;
  }
  end();
}

template<class P> inline
void IntelHex<P>::print_byte(unsigned char b) {
  checksum+=b;
  this->out.print(b>>4,HEX);
  this->out.print(b & 0x0F,HEX);
}

template<class P> inline
void IntelHex<P>::begin_line(unsigned char len, unsigned short a, unsigned char type) {
  checksum=0;
  this->out.print(":");
  print_byte(len);
  print_byte(a>>8);
  print_byte(a & 0xFF);
  print_byte(type);
}

template<class P> inline
void IntelHex<P>::end_line() {
  print_byte(256-checksum);
  this->out.println();
}

template<class P> inline
void IntelHex<P>::begin() {
  addr=0;
}

template<class P> inline
void IntelHex<P>::end() {
  begin_line(0,0,1);
  end_line();
}

template<class P> inline
void IntelHex<P>::address(size_t ia) {
  if((ia & 0xFFFF0000) != (addr & 0xFFFF0000)) {
    addr=ia;
    begin_line(2,0,4);
    print_byte((addr>>24) & 0xFF);
    print_byte((addr>>16) & 0xFF);
    end_line();
  }
}

template<class P> inline
void IntelHex<P>::line(const char* start, size_t base, size_t len) {
  address(base);
  begin_line(len,((unsigned int)base)&0xFFFF,0);
  for(int i=0;i<len;i++) print_byte(start[i]);
  end_line();
}

template<class P> inline
void Base85<P>::print_group(const char* p, size_t len) {
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
  for(int i=0;i<len+1;i++) this->out.print(group_c[i]);
}

template<class P> inline
void Base85<P>::line(const char* start, size_t base, size_t len) {
  while(len>0) {
    print_group(start,len>4?4:len);
    start+=4;
    len-=4;
  }
  this->out.println();
}

template<class P> inline
void Hd<P>::line(const char* start, size_t base, size_t len) {
  this->out.print((unsigned int)base,HEX,4);
  this->out.print(' ');
  for(int i=0;i<len;i++) {
    this->out.print(start[i],HEX,2);
    if(i%4==3) this->out.print(' ');
  }
  for(int i=len;i<this->preferredLen;i++) {
    this->out.print("  ");
    if(i%4==3) this->out.print(' ');
  }
  this->out.print(' ');
  for(int i=0;i<len;i++) this->out.print((start[i]>=32 && start[i]<127)?start[i]:'.');
  this->out.println();
}



#include <stdlib.h>
#include "circular.h"
#include "stringex.h"
#include "conparse.h"
#include "nmeawrite.h"
#include "serial.h"

int circular::isFull() {
  return (head+1)%1024==tail;
}

int circular::isEmpty() {
  return head==tail;
}

//returns 0 if character written, negative
//if character could not be written
int circular::fill(char in) {
  if(!isFull()) {
    buf[head]=in;
    head=(head+1)%1024;
    return 0;
  }
  return -1;
}

char circular::get() {
  if(isEmpty()) return 0;
  char result=buf[tail];
  tail=(tail+1)%1024;
  return result;
}

int circular::fillString(const char* in) {
  int i=0;
  while(in[i]!=0) {
    int result=fill(in[i]);
    if (result<0) return result;
    i++;
  }
  return 0;
}

int circular::fillDec(int in) {
  char dbuf[10];
  toDec(dbuf,in);
  fillString(dbuf);
  return 0;
}

int circular::fill0Dec(int in, int len) {
  char dbuf[10];
  to0Dec(dbuf,in,len);
  fillString(dbuf);
  return 0;
}

int circular::fillHex(unsigned int in, int len) {
  int hexit;
  int result;
  for(int i=0;i<len;i++) {
    hexit=(in>>(4*(len-i-1))) & 0x0F;
    result=fill(hexDigits[hexit]);
    if(result!=0) return result;
  }
  return 0;
}

int circular::fillStringn(const char* in, int len) {
  for(int i=0;i<len;i++) {
    int result=fill(in[i]);
    if(result<0) return result;
  }
  return 0;
}

int circular::fillShort(short in) {
  int result=fill((in>> 8) & 0xFF);
  if(result<0) return result;
  return     fill((in>> 0) & 0xFF);
}

int circular::fillInt(int in) {
  int result=fillShort((in>> 16) & 0xFFFF);
  if(result<0) return result;
  return     fillShort((in>>  0) & 0xFFFF);
}

char circular::peekMid(int pos) {
  pos+=mid;
  pos&=0x3FF;
  return buf[pos];
}

short circular::peekMidShort(int pos) {
  return (((short)peekMid(pos))<<8) | peekMid(pos+1);
}

int circular::peekMidInt(int pos) {
  return (((int)peekMidShort(pos))<<16) | peekMidShort(pos+2);
}
  
char circular::peekHead(int pos) {
  pos+=head;
  pos&=0x3FF;
  return buf[pos];
}

char circular::peekTail(int pos) {
  pos+=tail;
  pos&=0x3FF;
  return buf[pos];
}

void circular::pokeMid(int pos, char poke) {
  pos+=mid;
  pos&=0x3FF;
  buf[pos]=poke;
}

void circular::pokeHead(int pos, char poke) {
  pos+=head;
  pos&=0x3FF;
  buf[pos]=poke;
}

void circular::pokeTail(int pos, char poke) {
  pos+=tail;
  pos&=0x3FF;
  buf[pos]=poke;
}

void circular::mark() {
  mid=head;
}

int circular::unreadylen() {
  int h=head;
  int m=mid;
  if(h<m) h+=1024;
  return h-m;
}

int circular::readylen() {
  int m=mid;
  int t=tail;
  if(m<t) m+=1024;
  return m-t;
}

void circular::fillError(const char* msg, unsigned int a, unsigned int b) {
  circular& This=*this;
  fillPktStart(This,PT_ERROR);
  fillPktString(This,msg);
  dataDec=0;
  dataDigits=8;
  fillPktInt(This,a);
  fillPktInt(This,b);
  fillPktFinish(This);
}

int circular::drain(circular* to) {
  while(readylen()>0) {
    if(to->isFull()) {
      to->empty();
      empty();
      to->fillError("Buffer full",(unsigned int)this,(unsigned int)to);
      return -1;
    }
    to->fill(get());
  }
  to->mark();
  return 0;
}

void circular::empty() {
  head=0;
  tail=0;
  mid=0;
}


void circular::send(int port) {
  char* pos;
  if(head>mid) {
    pos=midPtr();
    tx_serial(port,pos,unreadylen());
  } else {
    pos=midPtr();
    tx_serial(port,pos,1024-mid);
    pos=buf;
    tx_serial(port,pos,head);
  }
  mark();
}


#include <stdlib.h>
#include "Circular.h"
#include "Stringex.h"

int Circular::isFull() {
  return (head+1)%1024==tail;
}

int Circular::isEmpty() {
  return head==tail;
}

//returns 0 if character written, negative
//if character could not be written
int Circular::fill(char in) {
  if(!isFull()) {
    buf[head]=in;
    head=(head+1)%1024;
    return 0;
  }
  return -1;
}

char Circular::get() {
  if(isEmpty()) return 0;
  char result=buf[tail];
  tail=(tail+1)%1024;
  return result;
}

int Circular::fillString(const char* in) {
  int i=0;
  while(in[i]!=0) {
    int result=fill(in[i]);
    if (result<0) return result;
    i++;
  }
  return 0;
}

int Circular::fillDec(int in) {
  char dbuf[10];
  toDec(dbuf,in);
  fillString(dbuf);
  return 0;
}

int Circular::fill0Dec(int in, int len) {
  char dbuf[10];
  to0Dec(dbuf,in,len);
  fillString(dbuf);
  return 0;
}

int Circular::fillHex(unsigned int in, int len) {
  int hexit;
  int result;
  for(int i=0;i<len;i++) {
    hexit=(in>>(4*(len-i-1))) & 0x0F;
    result=fill(hexDigits[hexit]);
    if(result!=0) return result;
  }
  return 0;
}

int Circular::fillStringn(const char* in, int len) {
  for(int i=0;i<len;i++) {
    int result=fill(in[i]);
    if(result<0) return result;
  }
  return 0;
}

int Circular::fillShort(short in) {
  int result=fill((in>> 8) & 0xFF);
  if(result<0) return result;
  return     fill((in>> 0) & 0xFF);
}

int Circular::fillInt(int in) {
  int result=fillShort((in>> 16) & 0xFFFF);
  if(result<0) return result;
  return     fillShort((in>>  0) & 0xFFFF);
}

char Circular::peekMid(int pos) {
  pos+=mid;
  pos&=0x3FF;
  return buf[pos];
}

short Circular::peekMidShort(int pos) {
  return (((short)peekMid(pos))<<8) | peekMid(pos+1);
}

int Circular::peekMidInt(int pos) {
  return (((int)peekMidShort(pos))<<16) | peekMidShort(pos+2);
}
  
char Circular::peekHead(int pos) {
  pos+=head;
  pos&=0x3FF;
  return buf[pos];
}

char Circular::peekTail(int pos) {
  pos+=tail;
  pos&=0x3FF;
  return buf[pos];
}

void Circular::pokeMid(int pos, char poke) {
  pos+=mid;
  pos&=0x3FF;
  buf[pos]=poke;
}

void Circular::pokeHead(int pos, char poke) {
  pos+=head;
  pos&=0x3FF;
  buf[pos]=poke;
}

void Circular::pokeTail(int pos, char poke) {
  pos+=tail;
  pos&=0x3FF;
  buf[pos]=poke;
}

void Circular::mark() {
  mid=head;
}

int Circular::unreadylen() {
  int h=head;
  int m=mid;
  if(h<m) h+=1024;
  return h-m;
}

int Circular::readylen() {
  int m=mid;
  int t=tail;
  if(m<t) m+=1024;
  return m-t;
}

int Circular::drain(Circular* to) {
  while(readylen()>0) {
    if(to->isFull()) {
      to->empty();
      empty();
      return -1;
    }
    to->fill(get());
  }
  to->mark();
  return 0;
}

void Circular::empty() {
  head=0;
  tail=0;
  mid=0;
}


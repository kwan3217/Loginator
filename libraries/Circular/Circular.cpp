#include "Circular.h"

bool Circular::isFull() {
  return fullState || ((head+1)%N==tail);
}

bool Circular::isEmpty() {
  return head==tail;
}

//returns true if character written, false 
//if character could not be written
bool Circular::fill(char in) {
  if(!isFull()) {
    buf[head]=in;
    head=(head+1)%N;
    return true;
  }
  //if buffer is full, throw away all unmarked data
  head=mid;
  fullState=true;
  return false;
}

char Circular::get() {
  if(isEmpty()) return 0;
  char result=buf[tail];
  tail=(tail+1)%N;
  return result;
}

bool Circular::fill(const char* in) {
  while(*in) {
    if(!fill(*in)) return false;
    in++;
  }
  return true;
}

bool Circular::fill(const char* in, int len) {
  for(int i=0;i<len;i++) {
    if(!fill(in[i])) return false;;
  }
  return true;
}

char Circular::peekMid(int pos) {
  pos+=mid;
  pos%=N;
  return buf[pos];
}

char Circular::peekHead(int pos) {
  pos+=head;
  pos%=N;
  return buf[pos];
}

char Circular::peekTail(int pos) {
  pos+=tail;
  pos%=N;
  return buf[pos];
}

void Circular::pokeMid(int pos, char poke) {
  pos+=mid;
  pos%=N;
  buf[pos]=poke;
}

void Circular::pokeHead(int pos, char poke) {
  pos+=head;
  pos%=N;
  buf[pos]=poke;
}

void Circular::pokeTail(int pos, char poke) {
  pos+=tail;
  pos%=N;
  buf[pos]=poke;
}

void Circular::mark() {
  mid=head;
}

int Circular::unreadylen() {
  int h=head;
  int m=mid;
  if(h<m) h+=N;
  return h-m;
}

int Circular::readylen() {
  int m=mid;
  int t=tail;
  if(m<t) m+=N;
  return m-t;
}

bool Circular::drain(Circular& to) {
  while(readylen()>0) {
    if(to.isFull()) {
      to.empty();
      empty();
      return false;
    }
    to.fill(get());
  }
  fullState=false;
  to.mark();
  return true;
}

void Circular::empty() {
  head=0;
  tail=0;
  mid=0;
}


#include "float.h"
#include <string.h>
#include "Stringex.h"

#define makeSinTableA(x) makeSinTableB(x) makeSinTableB(x + 450)
#define makeSinTableB(x) makeSinTableC(x) makeSinTableC(x + 225)
#define makeSinTableC(x) makeSinTableD(x) makeSinTableD(x +  75) makeSinTableD(x+150)
#define makeSinTableD(x) makeSinTableE(x) makeSinTableE(x +  25) makeSinTableE(x+ 50)
#define makeSinTableE(x) makeSinTableF(x) makeSinTableF(x +   5) makeSinTableF(x+ 10) makeSinTableF(x+ 15) makeSinTableF(x+ 20)
#define makeSinTableF(x) makeSinTableG(x) makeSinTableG(x +   1) makeSinTableG(x+  2) makeSinTableG(x+  3) makeSinTableG(x+  4)
#define makeSinTableG(x) sin(fp(x)/fp(10.0)*PI/fp(180.0)) ,

const fp sinTable[] = { makeSinTableA(0) 1.0 };

//Given a string representing number with a decimal point, return the number
fp stof(char* in) {
  char buf[16];
  int len=strlen(in);
  int decimal=0;
  while(in[decimal]!='.' && in[decimal]!=0) decimal++;
  in[decimal]=0;
  if(len==decimal) {
    return fp(stoi(in));
  }
  int fraclen=len-decimal-1;
  int shift=fraclen;
  for(int i=0;i<decimal;i++) buf[i]=in[i];
  for(int i=decimal+1;i<len;i++) buf[i-1]=in[i];
  buf[len-1]=0;
  fp result=stoi(buf);
  for(int i=0;i<shift;i++) result/=10;
  return result;
}



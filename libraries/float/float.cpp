#include "float.h"
#include <string.h>
#include "Stringex.h"

// Generate an entry to the CRC lookup table
//template <int c> struct gen_sin_table{enum {value = sin(fp(c)/10.0*PI/180.0)};};


//#include "sinShort.h"
// Old makeSinShort.pl
//printf("const fp sinTable[]={\n");
//my $pi=3.1415926535897932385;
//for(my $i=0;$i<=900;$i++) {
//  printf("/*%05.1f*/  %20.18f%s\n",$i/10.0,sin($i/10.0*$pi/180.0),$i==900?"":",");
//}
//printf("};\n")

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



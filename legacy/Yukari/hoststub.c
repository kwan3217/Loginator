#include <stdio.h>
#include "circular.h"
#include "hoststub.h"

const int CCLK=60000000;
const int gyroPeriod=100;



static char headerNMEASIRF[]={'C',0x2A,
                              'G',0x33,
                              'D',0x2F,
			                        'A',0x2C,
         						          'M',0x17,
		         				          'F',0x20,
				         		          'L',0x15,
								              'P',0x19,
								              'V',0x1A,
								              'U',0x2B,
								              'S',0x18,
								              'B',0x16,
                              'H',0x40,
                              'E',0x41,
                              'I',0x42,
                              'J',0x43};

int fill(circular* buf, char c) {
  printf("%c",c);
  return 0;
}
int fillDec(circular* buf, int in) {
  printf("%d",in);
  return 0;
}

int fill0Dec(circular* buf, int in, int len) {
  printf("%0*d",len,in);
  return 0;
}

void i2c_tx_string() {
}

void i2c_txrx_string() {
}



static char hexDigits[]="0123456789ABCDEF";

int fillHex(circular* buf, unsigned int in, int len) {
  int hexit;
  int result;
  for(int i=0;i<len;i++) {
    hexit=(in>>(4*(len-i-1))) & 0x0F;
    result=fill(buf,hexDigits[hexit]);
    if(result!=0) return result;
  }
  return 0;
}


int fillString(circular* buf, char* s) {
  printf("%s",s);
  return 0;
}

void fillPktStart(circular* buf,int type) {
  fillString(buf,"$PKWN");
  fill(buf,headerNMEASIRF[type*2]);

  //Reset all formatting parameters so that 
  //it doesn't matter what the last packet was
  buf->dataDigits=0;
  buf->dataDec=1;
  buf->dataAsciiz=0;
  buf->dataComma=1;
}

void fillPktFinish(circular* buf) {
  printf("\n");
}

int fillPktByte(circular* buf, char in) {
  if(buf->dataComma)fill(buf,',');
  if(buf->dataDec) {
    if(buf->dataDigits>0) return fill0Dec(buf,in,buf->dataDigits);
    return fillDec(buf,in);
  } else {
    return fillHex(buf,in,buf->dataDigits>0?buf->dataDigits:2);
  }
}

int fillPktChar(circular* buf, char in) {
  if(buf->dataComma)fill(buf,',');
  return fill(buf,in);
}

int fillPktShort(circular* buf, short in) {
  if(buf->dataComma)fill(buf,',');
  if(buf->dataDec) {
    if(buf->dataDigits>0) return fill0Dec(buf,in,buf->dataDigits);
    return fillDec(buf,in);
  } else {
    return fillHex(buf,in,buf->dataDigits>0?buf->dataDigits:4);
  }
}

int fillPktInt(circular* buf, int in) {
  if(buf->dataComma)fill(buf,',');
  if(buf->dataDec) {
    if(buf->dataDigits>0) return fill0Dec(buf,in,buf->dataDigits);
    return fillDec(buf,in);
  } else {
    return fillHex(buf,in,buf->dataDigits>0?buf->dataDigits:8);
  }
}

int fillPktString(circular* buf, char* in) {
  if(buf->dataComma) fill(buf,',');
  int i=0;
  while(in[i]!=0) {
    int result=fill(buf,in[i]);
    if (result<0) return result;
    i++;
  }
  return 0;
}

int fillPktFP(circular* buf, fp f) {
  printf(",%15.8e",f);

  return 0;

}

typedef union {
  float f;
  unsigned int i;
  struct {
    unsigned int sign:1;
    unsigned int exp:8;
    unsigned int mant:24;
  } p;
} float_parse;

typedef union {
  float f;
  unsigned long int i;
  struct {
    unsigned int sign:1;
    unsigned int exp:11;
    unsigned long int mant:52;
  } p;
} double_parse;

int fillPktFloatExact(circular* buf, float f) {
  if(buf->dataComma)fill(buf,',');
  int oldDigits=buf->dataDigits;
  int oldDec=buf->dataDec;
  int oldComma=buf->dataComma;
  buf->dataDec=0;
  buf->dataComma=0;
  buf->dataDigits=8;
  float_parse ff;
  ff.f=f;
  fillPktInt(buf,ff.i);
  buf->dataDigits=6;
  fill(buf,',');
  fill(buf,ff.p.sign?'-':'+');
  fill(buf,'0');
  fill(buf,'x');
  fill(buf,ff.p.exp==0?'0':'1');
  fill(buf,'.');
  fillPktInt(buf,ff.p.mant);
  fill(buf,'p');
  buf->dataDec=1;
  buf->dataDigits=2;
  fillPktInt(buf,ff.p.exp);
  buf->dataDigits=oldDigits;
  buf->dataDec=oldDec;
  buf->dataComma=oldComma;
  return 0;
}
  
int fillPktFPexact(circular* buf, fp f) {
   if(sizeof(fp)==4) {
    return fillPktFloatExact(buf,f);
  } else {
  }
}
  

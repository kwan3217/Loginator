#include "nmeawrite.h"
#include "main.h"

nmeawrite NMEAWrite("txt");

static const char headerNMEA[]="CGDAMFLPVUSBHEIJR";

void nmeawrite::fillPktStartCore(circular& buf,int type) {
  buf.fill('$');
  if(type>=0) {
    buf.fillString("PKWN");
    buf.fill(headerNMEA[type]);
  }
}

void nmeawrite::fillPktFinishCore(circular& buf) {
//  blinklock(0,buf.head);
  int len=buf.unreadylen()-1;
  short checksum=0;
  for(int i=0;i<len;i++) {
    checksum^=buf.peekMid(i+1);
  }
  buf.fill('*');
  buf.fillHex(checksum,2);
  buf.fillShort(0x0D0A);
}

int nmeawrite::fillPktByte(circular& buf, char in) {
  if(buf.dataComma)buf.fill(buf.dataDelim);
  if(buf.dataDec) {
    if(buf.dataDigits>0) return buf.fill0Dec(in,buf.dataDigits);
    return buf.fillDec(in);
  } else {
    return buf.fillHex(in,buf.dataDigits>0?buf.dataDigits:2);
  }
}

int nmeawrite::fillPktChar(circular& buf, char in) {
  if(buf.dataComma) buf.fill(buf.dataDelim);
  return buf.fill(in);
}

int nmeawrite::fillPktShort(circular& buf, short in) {
  if(buf.dataComma)buf.fill(buf.dataDelim);
  if(buf.dataDec) {
    if(buf.dataDigits>0) return buf.fill0Dec(in,buf.dataDigits);
    return buf.fillDec(in);
  } else {
    return buf.fillHex(in,buf.dataDigits>0?buf.dataDigits:4);
  }
}

int nmeawrite::fillPktInt(circular& buf, int in) {
  if(buf.dataComma)buf.fill(buf.dataDelim);
  if(buf.dataDec) {
    if(buf.dataDigits>0) return buf.fill0Dec(in,buf.dataDigits);
    return buf.fillDec(in);
  } else {
    return buf.fillHex(in,buf.dataDigits>0?buf.dataDigits:8);
  }
}

int nmeawrite::fillPktString(circular& buf, const char* in) {
  if(buf.dataComma) buf.fill(buf.dataDelim);
  int i=0;
  while(in[i]!=0) {
    int result=buf.fill(in[i]);
    if (result<0) return result;
    i++;
  }
  return 0;
}

int nmeawrite::fillPktStringn(circular& buf, const char* in, int length) {
  if(buf.dataComma) buf.fill(buf.dataDelim);
  if(length==0) return 0; //If told to write 0 bytes, do it and return successfully
  for(int i=0;i<length;i++) {
    int result=buf.fill(in[i]);
    if (result<0) return result;
  }
  return 0;
}

const char base85[]="0123456789"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "abcdefghijklmnopqrstuvwxyz"
                    "!#$%&()+-/;<=>?@^_`{|}~";
const char base85All0='[';
const char base85All1='\\';
const char base85AllSp=']';

int nmeawrite::fillPktBlock(circular& buf, const char* in, int length) {
  if(length==0) return 0; //If told to write 0 bytes, do it and return successfully
  if(buf.dataBase85) {
    if(buf.dataComma) buf.fill(buf.dataDelim);
    buf.fill('G');
    int i=0;
    while(i<(length-3)) {
      unsigned int acc=0;
      for(int j=3;j>=0;j--) {
        acc=(acc<<8)+(unsigned char)(in[i*4+j]);
      }
      if(acc==0) {
        buf.fill(base85All0);
      } else if(acc==0xFFFFFFFF) {
        buf.fill(base85All1);
      } else if(acc==0x20202020) {
        buf.fill(base85AllSp);
      } else for(int j=0;j<5;j++) {
        buf.fill(base85[acc % 85]);
        acc=acc/85;
      }
      i+=4;
    }/*
    unsigned int acc=0;
    for(int j=(length-i-1);j>=0;j--) {
      acc=(acc<<8)+(unsigned char)(in[i*4+j]);
    }
    for(int j=0;j<length-i-2;j++) {
      buf.fill(base85[acc % 85]);
      acc=acc/85;
    }*/
    return 0;
  } else {
    int oldDec=buf.dataDec;
    int oldDigits=buf.dataDigits;
    int oldComma=buf.dataComma;
    buf.dataDec=0;
    buf.dataDigits=2;
    fillPktByte(buf,in[0]);
    buf.dataComma=0;
    for(int j=1;j<length;j++) {
      int result=fillPktByte(buf,in[j]);
      if(result<0) return result;
    }
    buf.dataDec=oldDec;
    buf.dataDigits=oldDigits;
    buf.dataComma=oldComma;
    return 0;
  }
}

int nmeawrite::fillPktFP(circular& buf, fp f) {
  int oldComma=buf.dataComma;
  buf.dataComma=0;
  int oldDigits=buf.dataDigits;
  if(oldComma) buf.fill(buf.dataDelim);
  if(f<0 && f>-1) buf.fill('-');
  fillPktInt(buf,(int)f);
  buf.fill('.');
  if(f<0) f=-f;
  f=f-(int)f;
  buf.dataDigits=6;
  int result=fillPktInt(buf,(int)(f*1e6));
  buf.dataComma=oldComma;
  buf.dataDigits=oldDigits;
  return result;
}



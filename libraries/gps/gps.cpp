#include <stdlib.h>
#include <string.h>
#include "gps.h"
#include "Stringex.h"
#include "Time.h"

//Given a string representing number with a decimal point, return the number multiplied by 10^(shift)
int NMEAParser::parseNumber(char* in, int& shift) {
  char buf[16];
  int len=strlen(in);
  int decimal=0;
  while(in[decimal]!='.' && in[decimal]!=0) decimal++;
  in[decimal]=0;
  int fraclen=len-decimal-1;
  shift=fraclen;
  for(int i=0;i<decimal;i++) buf[i]=in[i];
  for(int i=decimal+1;i<len;i++) buf[i-1]=in[i];
  buf[len-1]=0;
  return stoi(buf);
}

//Given a string representing a number in the form dddmm.mmmm, return
//an integer representing that number in just degrees multiplied by 10^7
int NMEAParser::parseMin(char* buf) {
  int shift;
  int num=parseNumber(buf,shift);
  int shift10=1;
  for(int i=0;i<shift;i++) shift10*=10;
  int rshift=7-shift;
  int rshift10=1;
  for(int i=0;i<rshift;i++) rshift10*=10;
  int intpart=num/shift10;
  int fracpart=num%shift10;
  int degpart=intpart/100;
  int minintpart=intpart%100;
  int minpart=(minintpart*shift10+fracpart)*rshift10/60;
  return degpart*10000000+minpart;
}

void NMEAParser::expectDollar(char in) {
  if(in=='$') {
    checksum=0;
    nmeaState=&NMEAParser::expectG;
  }
}

void NMEAParser::expectG(char in) {
  if(in=='G') {
    nmeaState=&NMEAParser::expectP;
    return;
  }
  nmeaState=&NMEAParser::expectDollar;
}

void NMEAParser::expectP(char in) {
  if(in=='P') {
    nmeaState=&NMEAParser::ThirdLetter;
    return;
  }
  nmeaState=&NMEAParser::expectDollar;
}

void NMEAParser::ThirdLetter(char in) {
  if(in=='Z') {
    nmeaState=&NMEAParser::expectZDA2;
    return;
//  } else if(in=='R') {
//    nmeaState=expectRMC2;
//    return;
//  } else if(in=='G') {
//    nmeaState=expectG4;
//    return;
  }
  nmeaState=&NMEAParser::expectDollar;
}

/*
void expectRMC2(char in) {
  if(in=='M') {
    nmeaState=expectRMC3;
    return;
  }
  nmeaState=expectDollar;
}

void expectRMC3(char in) {
  if(in=='C') {
    nmeaState=expectRMCComma0;
    return;
  }
  state=expectDollar;
}

*/
void NMEAParser::expectZDA2(char in) {
  if(in=='D') {
    nmeaState=&NMEAParser::expectZDA3;
    return;
  }
  nmeaState=&NMEAParser::expectDollar;
}

void NMEAParser::expectZDA3(char in) {
  if(in=='A') {
    pktState=0;
    nmeaState=&NMEAParser::parseZDA;
    return;
  }
  nmeaState=&NMEAParser::expectDollar;
}

void NMEAParser::parseZDA(char in) {
  static char numBuf[128];
  static int numPtr=0;
  switch(pktState) {
    case 0:
    case 2:
      //waiting for comma
      if(in==',') {
        numPtr=0;
        pktState++;
        return;
      }
      break;
    case 1:
      //in field 1, UTC hhmmss.000
      if(in==',') {
        //we've seen the whole number
        int shift;
        numBuf[numPtr]=0;
        zdaHMS=parseNumber(numBuf,shift);
        for(int i=0;i<shift;i++) zdaHMS/=10;
        pktState++;
        return;
      } else if((in>='0' && in<='9') || (in=='.')) {
        numBuf[numPtr]=in;
        numPtr++;
        return;
      }
      break;
    case 3:
      //in field 2, day of month
      if(in==',') {
        //we've seen the whole number
        numBuf[numPtr]=0;
        zdaDD=stoi(numBuf);
        pktState++;
        return;
      } else if((in>='0' && in<='9')) {
        numBuf[numPtr]=in;
        numPtr++;
        return;
      }
      break;
    case 5:
      //in field 3, month number
      if(in==',') {
        //we've seen the whole number
        numBuf[numPtr]=0;
        zdaMM=stoi(numBuf);
        pktState++;
        return;
      } else if((in>='0' && in<='9')) {
        numBuf[numPtr]=in;
        numPtr++;
        return;
      }
      break;
    case 7:
      //in field 4, year number
      if(in==',') {
        //we've seen the whole number
        numBuf[numPtr]=0;
        zdaYYYY=stoi(numBuf);
        pktState=0;
        nmeaState=&NMEAParser::waitChecksum;
        finishPacket=&NMEAParser::finishZDA;
        return;
      } else if((in>='0' && in<='9')) {
        numBuf[numPtr]=in;
        numPtr++;
        return;
      }
      break;
  }
  //Error in packet, ignore packet and go back to start
  nmeaState=&NMEAParser::expectDollar;
}

void NMEAParser::waitChecksum(char in) {
  switch(pktState) {
    case 0: //waiting for *
      if(in=='*') {
        checksum^=in; //Don't include this character in the checksum
        pktState++;
      }
      return; //no possible error condition here, so get outta here.
    case 1:
      checksum^=in;
      if((in>='0' && in<='9') || (in>='A' && in<='F')) {
        int digit=(in<='9')?in-'0':in-'A';
        if((checksum & 0xF0)!=(digit<<4)) break; //If checksum is bad, go directly to the end of the case statement
        pktState++;
      }
      return;
    case 2:
      checksum^=in;
      if((in>='0' && in<='9') || (in>='A' && in<='F')) {
        int digit=(in<='9')?in-'0':in-'A';
        if((checksum & 0x0F)!=(digit)) break;
        if(finishPacket) PTMF(finishPacket)();
      }
  }
  nmeaState=&NMEAParser::expectDollar;
}

void NMEAParser::finishZDA() {
  set_rtc(zdaYYYY,zdaMM,zdaDD,zdaHMS/10000,(zdaHMS%10000)/100,zdaHMS%100);
}



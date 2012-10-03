#include <stdlib.h>
#include <string.h>
#include "gps.h"
#include "conparse.h"
#include "circular.h"
#include "LPC214x.h"
#include "main.h"
#include "serial.h"
#include "stringex.h"
#include "pktwrite.h"

//SiRF packet processing and GPS status
int GPSLight;
int GPShasPos;
int hasRMC;
int hasNewGPS=0;
//Latitude and longitude in Kdeg (1e7 Kdeg in 1 deg)
//Alt in cm
//Course in deg*10, 3600 in full circle
//Spd in m/s*10
int lat,lon,alt,GPScourse,GPSspd;
int satMask,satNum;
fp scourse,ccourse;

int countBits(int b) {
  int c=0;
  for(int i=0;i<32;i++) if(b & (1<<i)) c++;
  return c;
}

void parseSirf(circular& sirfBuf) {
  if(sirfBuf.peekMid(4)==0x29) {
    //Check if we have a position
    GPShasPos=(sirfBuf.peekMid(6)==0) & (sirfBuf.peekMid(5)==0);
    if(GPShasPos) {
      GPSLight=1;
      YEAR=(unsigned)sirfBuf.peekMidShort(15);
      MONTH=sirfBuf.peekMid(17);
      DOM=sirfBuf.peekMid(18);
      HOUR=sirfBuf.peekMid(19);
      MIN=sirfBuf.peekMid(20);
      SEC=((unsigned)sirfBuf.peekMidShort(21))/1000;
      satMask=sirfBuf.peekMidInt(23);
      satNum=sirfBuf.peekMid(92); //This may not be right
      lat=sirfBuf.peekMidInt(27);
      lon=sirfBuf.peekMidInt(31);
    }
  } else if(!GPShasPos && sirfBuf.peekMid(4)==4) {
    //Check if we have at least one sat
    if(sirfBuf.peekMid(12)>0) {
      GPSLight=1-GPSLight;
    } else {
      GPSLight=0;
    }
  }
}

int parseCommaPart(circular& buf, int* p0, char* out) {
  int p=*p0;
  char b=buf.peekMid(p);
  while(p-*p0<15 && b!=',') {
    out[p-*p0]=b;
    p++;
    b=buf.peekMid(p);
  }
  //terminate the string
  out[p-*p0]=0;  
  //skip the comma
  p++;
  int result=p-*p0;
  *p0=p;
  return result;
}

int skipCommaPart(circular& buf, int* p0) {
  int p=*p0;
  char b=buf.peekMid(p);
  while(p-*p0<15 && b!=',') {
    p++;
    b=buf.peekMid(p);
  }
  //skip the comma
  p++;
  int result=p-*p0;
  *p0=p;
  return result;
}

char parseChar(circular& buf, int* p0) {
  //Hemisphere
  char result=buf.peekMid(*p0);
  if(result!=',') {
    (*p0)++;
  } else {
    result=0;
  }
  //skip the comma
  (*p0)++;
  return result;
}

//Given a string representing number with a decimal point, return the number multiplied by 10^(shift)
int parseNumber(char* in, int* shift) {
  char buf[16];
  int len=strlen((char*)in);
  int decimal=0;
  while(in[decimal]!='.' && in[decimal]!=0) decimal++;
  in[decimal]=0;
  int fraclen=len-decimal-1;
  *shift=fraclen;
  for(int i=0;i<decimal;i++) buf[i]=in[i];
  for(int i=decimal+1;i<len;i++) buf[i-1]=in[i];
  buf[len-1]=0;
  return stoi(buf);
}

//Given a string representing a number in the form dddmm.mmmm, return
//an integer representing that number in just degrees multiplied by 10^7
int parseMin(char* buf) {
  int shift;
  int num=parseNumber(buf,&shift);
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

void parseGPRMC(circular& buf, int p) {
  char time[15]; //Good for at least microsecond precision
  char status;
  char latb[15]; //Good to absurd precision
  char latHem;
  char lonb[15]; //Good to absurd precision
  char lonHem;
  char date[7]; //It promises only 6 digits
  char spdb[15]; //Good to absurd precision
  char courseb[15]; //Good to absurd precision
  int shift;
  
  parseCommaPart(buf,&p,time);
  status=parseChar(buf,&p);
  parseCommaPart(buf,&p,latb);  
  latHem=parseChar(buf,&p);
  parseCommaPart(buf,&p,lonb);
  lonHem=parseChar(buf,&p);
  parseCommaPart(buf,&p,spdb);
  parseCommaPart(buf,&p,courseb);
  parseCommaPart(buf,&p,date);

  if(status=='A') {
    if(strlen((char*)latb)>0) {
      GPShasPos=1;
      GPSLight=1;
    
      lat=parseMin(latb);
      if(latHem=='S') lat*=-1;
      lon=parseMin(lonb);
      if(lonHem=='W') lon*=-1;
      GPSspd=parseNumber(spdb,&shift);       //speed in kts
      GPScourse=parseNumber(courseb,&shift); //course in tenths of a degree, true, zero north, increasing east
      for(int i=1;i<shift;i++) GPScourse/=10;
      scourse=sint(GPScourse);
      ccourse=cost(GPScourse);
      fillPktStart(buf,PT_I2C);
      fillPktString(buf,"GPS Parse");
      fillPktInt(buf,lat);
      fillPktInt(buf,lon);
      fillPktInt(buf,GPSspd);
      fillPktInt(buf,GPScourse);
      fillPktFP(buf,scourse);
      fillPktFP(buf,ccourse);
      fillPktFinish(buf);
      hasNewGPS=1;
    } else {
      GPSLight=1-GPSLight;
    }
  } else {
    GPShasPos=0;
    GPSLight=0;
  }

  //Record the time in the calendar registers  
  time[6]=0;
  int hns=stoi(time);
  int dmy=stoi(date);    
  HOUR=hns/10000;
  MIN=(hns%10000)/100;
  SEC=hns%100;
    
  DOM=dmy/10000;
  MONTH=(dmy%10000)/100;
  YEAR=dmy%100+2000;
	
}

void parseNmea(circular& buf) {
  int p=0;
  char tag[10];
  char b=buf.peekMid(p);
  while(p<10 && b!=',') {
    tag[p]=b;
    p++;
    b=buf.peekMid(p);
  }
  tag[p]=0;
  if(strcmp(tag,"$GPRMC")==0) {
    parseGPRMC(buf,p+1);
  } 
}


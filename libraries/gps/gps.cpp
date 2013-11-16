#include <stdlib.h>
#include <string.h>
#include "gps.h"
#include "Stringex.h"
#include "LPC214x.h"
#include "Time.h"
#include "irq.h"


//Given a string representing number with a decimal point, return the number multiplied by 10^(shift)
int GPS::parseNumber(char* in, int& shift) {
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

static GPS* selectedGPS;

static void handleEINT1() {
  selectedGPS->handlePPS();
  EXTINT=(1<<1); //clear EINT1 flag
  //Acknowledge the VIC
  VICVectAddr = 0;
}

void GPS::begin() {
  selectedGPS=this;
  EXTMODE=(1 << 1); //EINT1 edge sensitive
  EXTPOLAR=(1 << 1); //EINT1 on rising edge
  EXTINT=(1<<1); //clear EINT1 flag
  IRQHandler::install(IRQHandler::EINT1,handleEINT1);
  set_pin(14,2,0); //Set BSL (PPS input) to EINT1
};  

//Given a string representing a number in the form dddmm.mmmm, return
//an integer representing that number in just degrees multiplied by 10^7
int GPS::parseMin(char* buf) {
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

void GPS::handlePPS() {
  uint32_t thisPPSTC=TTC(0);
  time_mark();
  if(!writePPS) {
    PPSTC=thisPPSTC;
    writePPS=true;
  }
}

void GPS::process() {
  if(inf.available()>0) {
    char in=inf.read();
    checksum ^= in;
//      Serial.print(in);
//      Serial.print(",");Serial.print(nmeaState->num,DEC);
//      Serial.print(",");Serial.print(pktState,DEC);
    nmeaState->act(*this,in);
//      Serial.print(",");Serial.print(nmeaState->num,DEC);
//      Serial.print(","),Serial.print(pktState,DEC);
//      Serial.print(",");Serial.println(checksum,HEX,2);
  }
}

void ExpectDollar::act(GPS& that, char in) {
  if(in=='$') {
    that.checksum=0;
    that.nmeaState=&that.expectG;
  }
}

void ExpectG::act(GPS& that, char in) {
  if(in=='G') {
    that.nmeaState=&that.expectP;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectP::act(GPS& that, char in) {
  if(in=='P') {
    that.nmeaState=&that.thirdLetter;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ThirdLetter::act(GPS& that, char in) {
  switch(in) {
    case 'Z':
      that.nmeaState=&that.expectZDA2;
      return;
    case 'G':
      that.nmeaState=&that.expectGGA2;
      return;
    case 'V':
      that.nmeaState=&that.expectVTG2;
      return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectZDA2::act(GPS& that, char in) {
  if(in=='D') {
    that.nmeaState=&that.expectZDA3;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectZDA3::act(GPS& that, char in) {
  if(in=='A') {
    that.pktState=0;
    that.nmeaState=&that.parseZDA;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectGGA2::act(GPS& that, char in) {
  if(in=='G') {
    that.nmeaState=&that.expectGGA3;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectGGA3::act(GPS& that, char in) {
  if(in=='A') {
    that.pktState=0;
    that.nmeaState=&that.parseGGA;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectVTG2::act(GPS& that, char in) {
  if(in=='T') {
    that.nmeaState=&that.expectVTG3;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectVTG3::act(GPS& that, char in) {
  if(in=='G') {
    that.pktState=0;
    that.nmeaState=&that.parseVTG;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

int GPS::handleHMS() {
  int shift,result;
  numBuf[numPtr]=0;
  numPtr=0;
  result=parseNumber(numBuf,shift);
  for(int i=0;i<shift;i++) result/=10;
  pktState++;
  return result;
}

int GPS::handlestoi() {
  int result;
  numBuf[numPtr]=0;
  numPtr=0;
  result=stoi(numBuf);
  pktState++;
  return result;
}

int GPS::handleMin() {
  int result;
  numBuf[numPtr]=0;
  numPtr=0;
  result=parseMin(numBuf);
  pktState++;
  return result;
}

int GPS::handleNum(int& shift) {
  int result;
  numBuf[numPtr]=0;
  numPtr=0;
  result=parseNumber(numBuf,shift);
  pktState++;
  return result;
}

void GPS::acc(char in) {
  numBuf[numPtr]=in;
  numPtr++;
}

bool GPS::parseFieldHMS(char in, int& result) {
  if(in==',') {
    //we've seen the whole number
    result=handleHMS();
    return true;
  } else if((in>='0' && in<='9') || (in=='.')) {
    acc(in);
    return true;
  }
  return false;
}

bool GPS::parseFieldMin(char in, int& result) {
  if(in==',') {
    //we've seen the whole number
    result=handleMin();
    return true;
  } else if((in>='0' && in<='9') || (in=='.')) {
    acc(in);
    return true;
  }
  return false;
}

bool GPS::parseFieldstoi(char in, int& result) {
  if(in==',') {
    //we've seen the whole number
    result=handlestoi();
    return true;
  } else if((in>='0' && in<='9')) {
    acc(in);
    return true;
  }
  return false;
}

void ParseZDA::act(GPS& that, char in) {
  switch(that.pktState) {
    case 0:
      //waiting for comma
      if(in==',') {
        that.numPtr=0;
        that.pktState++;
        return;
      }
      break;
    case 1: //HHNNSS.###
      if(that.parseFieldHMS(in,that.zdaHMS)) return;
      break;
    case 2: //DD
      if(that.parseFieldstoi(in,that.zdaDD)) return;
      break;
    case 3:
      if(that.parseFieldstoi(in,that.zdaMM)) return;
      break;
    case 4:
      //in field 4, year number
      if(in==',') {
        that.zdaYYYY=that.handlestoi();
        that.pktState=0;
        that.nmeaState=&that.waitChecksum;
        that.finishPacket=&that.finishZDA;
        return;
      } else if((in>='0' && in<='9')) {
        that.acc(in);
        return;
      }
      break;
  }
  //Error in packet, ignore packet and go back to start
  that.nmeaState=&that.expectDollar;
}

void ParseGGA::act(GPS& that, char in) {
  switch(that.pktState) {
    case 0:
    case 4:
    case 7:
      //waiting for comma
      if(in==',') {
        that.numPtr=0;
        that.pktState++;
        return;
      }
      break;
    case 1: //UTC hhmmss.000
      if(that.parseFieldHMS(in,that.zdaHMS)) return;
      break;
    case 2: //Latitude
      if(that.parseFieldMin(in,that.ggaLat)) return;
      break;
    case 3: //in field 3, N/S hemisphere
      if((in=='N' || in=='S')) {
        if(in=='S') that.ggaLat=-that.ggaLat;
        that.pktState++;
        return;
      }
      break;
    case 5: //Longitude
      if(that.parseFieldMin(in,that.ggaLon)) return;
      break;
    case 6: //in field 3, E/W hemisphere
      if((in=='E' || in=='W')) {
        if(in=='W') that.ggaLon=-that.ggaLon;
        that.pktState++;
        return;
      }
      break;
  }
  //Error, ignore packet
  that.nmeaState=&that.expectDollar;
}

void ParseVTG::act(GPS& that, char in) {
  //We don't handle vtg packets yet
  that.nmeaState=&that.expectDollar;
}

void WaitChecksum::act(GPS& that, char in) {
  switch(that.pktState) {
    case 0: //waiting for *
      if(in=='*') {
        that.checksum^=in; //Don't include this character in the checksum
        that.pktState++;
      } else if(in==0x0D) {
        //No checksum is coming
        if(that.finishPacket) (that.finishPacket->act(that));
        that.nmeaState=&that.expectDollar;
      }  
      return; //no possible error condition here, so get outta here.
    case 1:
      that.checksum^=in;
      if((in>='0' && in<='9') || (in>='A' && in<='F')) {
        int digit=(in<='9')?in-'0':in-'A';
        if((that.checksum & 0xF0)!=(digit<<4)) break; //If checksum is bad, go directly to the end of the case statement
        that.pktState++;
      }
      return;
    case 2:
      that.checksum^=in;
      if((in>='0' && in<='9') || (in>='A' && in<='F')) {
        int digit=(in<='9')?in-'0':in-'A';
        if((that.checksum & 0x0F)!=(digit)) break;
        if(that.finishPacket) (that.finishPacket->act(that));
      }
  }
  that.nmeaState=&that.expectDollar;
}

void FinishZDA::act(GPS& that) {
  set_rtc(that.zdaYYYY,that.zdaMM,that.zdaDD,that.zdaHMS/10000,(that.zdaHMS%10000)/100,that.zdaHMS%100);
}

void FinishVTG::act(GPS& that) {

}

void FinishGGA::act(GPS& that) {
  that.lat=that.ggaLat;
  that.lon=that.ggaLon;
  that.alt=that.ggaAlt;
  that.altScale=that.ggaAltScale;
}




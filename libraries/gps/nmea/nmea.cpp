#include <stdlib.h>
#include <string.h>
#include "nmea.h"
#include "Stringex.h"

//Given a string representing number with a decimal point, return the number
fp NMEA::parseNumber(char* in) {
  char buf[16];
  int len=strlen(in);
  int decimal=0;
  while(in[decimal]!='.' && in[decimal]!=0) decimal++;
  in[decimal]=0;
  int fraclen=len-decimal-1;
  int shift=fraclen;
  for(int i=0;i<decimal;i++) buf[i]=in[i];
  for(int i=decimal+1;i<len;i++) buf[i-1]=in[i];
  buf[len-1]=0;
  fp result=stoi(buf);
  for(int i=0;i<shift;i++) result/=10;
  return result;
}

//Given a string representing a number in the form dddmm.mmmm, return
//an integer representing that number in just degrees
fp NMEA::parseMin(char* buf, int junk) {
  fp num=parseNumber(buf);
  int degpart=num/100;
  fp minpart=num-degpart*100;
  fp result=degpart+minpart/60;
  return result;
}

void NMEA::process(const char in) {
    checksum ^= in;
//    Serial.print(in);
//    Serial.print(",");Serial.print(nmeaState->num,DEC);
//    Serial.print(",");Serial.print(pktState,DEC);
    nmeaState->act(*this,in);
//    Serial.print(",");Serial.print(nmeaState->num,DEC);
//    Serial.print(","),Serial.print(pktState,DEC);
//    Serial.print(",");Serial.println(checksum,HEX,2);
}

void ExpectDollar::act(NMEA& that, char in) {
  if(in=='$') {
    that.checksum=0;
    that.nmeaState=&that.expectG;
  }
}

void ExpectG::act(NMEA& that, char in) {
  if(in=='G') {
    that.nmeaState=&that.expectP;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectP::act(NMEA& that, char in) {
  if(in=='P') {
    that.nmeaState=&that.thirdLetter;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ThirdLetter::act(NMEA& that, char in) {
  switch(in) {
    case 'G':
      that.nmeaState=&that.expectGGA2;
      return;
    case 'R':
      that.nmeaState=&that.expectRMC2;
      return;
    case 'V':
      that.nmeaState=&that.expectVTG2;
      return;
    case 'Z':
      that.nmeaState=&that.expectZDA2;
      return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectZDA2::act(NMEA& that, char in) {
  if(in=='D') {
    that.nmeaState=&that.expectZDA3;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectZDA3::act(NMEA& that, char in) {
  if(in=='A') {
    that.pktState=0;
    that.nmeaState=&that.parseZDA;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectGGA2::act(NMEA& that, char in) {
  if(in=='G') {
    that.nmeaState=&that.expectGGA3;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectGGA3::act(NMEA& that, char in) {
  if(in=='A') {
    that.pktState=0;
    that.nmeaState=&that.parseGGA;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectVTG2::act(NMEA& that, char in) {
  if(in=='T') {
    that.nmeaState=&that.expectVTG3;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectVTG3::act(NMEA& that, char in) {
  if(in=='G') {
    that.pktState=0;
    that.nmeaState=&that.parseVTG;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectRMC2::act(NMEA& that, char in) {
  if(in=='M') {
    that.nmeaState=&that.expectRMC3;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

void ExpectRMC3::act(NMEA& that, char in) {
  if(in=='C') {
    that.pktState=0;
    that.nmeaState=&that.parseRMC;
    return;
  }
  that.nmeaState=&that.expectDollar;
}

fp NMEA::handleHMS() {
  int result;
  numBuf[numPtr]=0;
  numPtr=0;
  result=parseNumber(numBuf);
  pktState++;
  return result;
}

int NMEA::handlestoi() {
  int result;
  numBuf[numPtr]=0;
  numPtr=0;
  result=stoi(numBuf);
  pktState++;
  return result;
}

fp NMEA::handleMin() {
  fp result;
  numBuf[numPtr]=0;
  numPtr=0;
  result=parseMin(numBuf,0);
  pktState++;
  return result;
}

fp NMEA::handleNum() {
  fp result;
  numBuf[numPtr]=0;
  numPtr=0;
  result=parseNumber(numBuf);
  pktState++;
  return result;
}

void NMEA::acc(char in) {
  numBuf[numPtr]=in;
  numPtr++;
}

bool NMEA::parseFieldHMS(char in, fp& result) {
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

bool NMEA::parseFieldMin(char in, fp& result) {
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

bool NMEA::parseFieldstoi(char in, int& result) {
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

bool NMEA::parseFieldNum(char in, fp& result) {
  if(in==',') {
    //we've seen the whole number
    result=handleNum();
    return true;
  } else if((in>='0' && in<='9')||in=='.') {
    acc(in);
    return true;
  }
  return false;
}

bool NMEA::discardFieldstoi(char in) {
  if(in==',') {
    //we've seen the whole number
    return true;
  } else if((in>='0' && in<='9')) {
    acc(in);
    return true;
  }
  return false;
}

void ParseZDA::act(NMEA& that, char in) {
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

void ParseGGA::act(NMEA& that, char in) {
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
      if(that.parseFieldHMS(in,that.ggaHMS)) return;
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
    case 8: //Fix quality
      //in field 4, year number
      if(in==',') {
        if(that.handlestoi()==0) break;
        that.pktState++;
        return;
      } else if((in>='0' && in<='9')) {
        that.acc(in);
        return;
      }
      break;
    case 9: //HDOP
      if(that.discardFieldstoi(in)) return;
      break;
    case 10: //MSL Altitude
      //in field 4, year number
      if(in==',') {
        that.ggaAlt=that.handleNum();
        that.pktState=0;
        that.nmeaState=&that.waitChecksum;
        that.finishPacket=&that.finishGGA;
        return;
      } else if((in>='0' && in<='9')||in=='.') {
        that.acc(in);
        return;
      }
      break;
  }
  //Error, ignore packet
  that.nmeaState=&that.expectDollar;
}

void ParseRMC::act(NMEA& that, char in) {
  switch(that.pktState) {
    case 0:
    case 3:
    case 6:
    case 9:
      //waiting for comma
      if(in==',') {
        that.numPtr=0;
        that.pktState++;
        return;
      }
      break;
    case 1: //UTC hhmmss.000
      if(that.parseFieldHMS(in,that.rmcHMS)) return;
      break;
    case 2: //Fix flag, must be A to be a valid fix
      if(in=='A') {
        that.pktState++;
        return;
      }
    case 4: //Latitude
      if(that.parseFieldMin(in,that.rmcLat)) return;
      break;
    case 5: //in field 3, N/S hemisphere
      if((in=='N' || in=='S')) {
        if(in=='S') that.rmcLat=-that.ggaLat;
        that.pktState++;
        return;
      }
      break;
    case 7: //Longitude
      if(that.parseFieldMin(in,that.rmcLon)) return;
      break;
    case 8: //in field 3, E/W hemisphere
      if((in=='E' || in=='W')) {
        if(in=='W') that.rmcLon=-that.rmcLon;
        that.pktState++;
        return;
      }
      break;
    case 10: //Speed in knots
      if(that.parseFieldNum(in,that.rmcSpd)) return;
      break;
    case 11: //Heading in deg true
      if(that.parseFieldNum(in,that.rmcHdg)) return;
      break;
    case 12: //UTC DDMMYY
      if(in==',') {
        that.rmcDMY=that.handleHMS();
        that.pktState=0;
        that.nmeaState=&that.waitChecksum;
        that.finishPacket=&that.finishRMC;
        return;
      } else if((in>='0' && in<='9')) {
        that.acc(in);
        return;
      }
      break;
  }
  //Error, ignore packet
  that.nmeaState=&that.expectDollar;
}

void ParseVTG::act(NMEA& that, char in) {
  switch(that.pktState) {
    case 0:
    case 3:
    case 4:
      //waiting for comma
      if(in==',') {
        that.numPtr=0;
        that.pktState++;
        return;
      }
      break;
    case 1: //Course
      if(that.parseFieldNum(in,that.vtgCourse)) return;
      break;
    case 2: //Course unit marker
      if(in=='T') {
        that.pktState++;
        return;
      }
      break;
    case 5: //Course unit marker
      if(in=='M') {
        that.pktState++;
        return;
      }
      break;
    case 6: //Speed
      if(in==',') {
        that.vtgSpeedKt=that.handleNum();
        that.pktState=0;
        that.nmeaState=&that.waitChecksum;
        that.finishPacket=&that.finishVTG;
        return;
      } else if((in>='0' && in<='9')) {
        that.acc(in);
        return;
      }
      break;
  }
  //Error, ignore packet
  that.nmeaState=&that.expectDollar;
}

void WaitChecksum::act(NMEA& that, char in) {
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

void FinishZDA::act(NMEA& that) {
//  set_rtc(that.zdaYYYY,that.zdaMM,that.zdaDD,that.zdaHMS/10000,(that.zdaHMS%10000)/100,that.zdaHMS%100);
  that.writeZDA=true;
}

void FinishVTG::act(NMEA& that) {
  that.writeVTG=true;
}

void FinishGGA::act(NMEA& that) {
  if(!that.writeGGA) {
    that.HMS=that.ggaHMS;
    that.lat=that.ggaLat;
    that.lon=that.ggaLon;
    that.alt=that.ggaAlt;
    that.writeGGA=true;
  }
}

void FinishRMC::act(NMEA& that) {
//  set_rtc(that.rmcDMY/10000+2000,(that.rmcDMY%10000)/100,that.rmcDMY%100,that.zdaHMS/10000,(that.zdaHMS%10000)/100,that.zdaHMS%100);
  if(!that.writeRMC) {
    that.HMS=that.rmcHMS;
    that.lat=that.rmcLat;
    that.lon=that.rmcLon;
    that.spd=that.rmcSpd;
    that.hdg=that.rmcHdg;
    that.DMY=that.rmcDMY;
    that.writeRMC=true;
  }
}




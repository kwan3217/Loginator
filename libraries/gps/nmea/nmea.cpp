#include <stdlib.h>
#include <string.h>
#include "nmea.h"
#include "Stringex.h"
#include "Time.h"
//#include "Serial.h" 

//Given a string representing a number in the form dddmm.mmmm, return
//an integer representing that number in cm' . This routine can't handle
//negative signs, which is ok since it is used for NMEA sentences which 
//encode the hemisphere with a separate N/S or E/W field, not a negative sign.
int NMEA::parseMin(char* in) {
  //Find the decimal point
  int len=strlen(in);
  int decimal=0;
  while(in[decimal]!='.' && in[decimal]!=0) decimal++;
  in[decimal]=0;
  int dm=stoi(in); //This is the dddmm part, to the left of the decimal
  //Force the number to have a certain number of digits after the decimal point
  static const int nDigits=5;
  if(len>(decimal+nDigits)) {      //Be super-careful of off-by-one errors here
    //Cut off a number which is longer than we want (don't do anything special here)
  } else {
    //Stretch a number that is shorter
    for(int i=len;i<(decimal+nDigits+1);i++) {
      in[i]='0';
    }
  }
  //In either case, we now put the string termination exactly where we want
  in[decimal+nDigits+1]=0; 
  int mf=stoi(&in[decimal+1]);
  int d=dm/100;
  int m=dm%100;
  for(int i=0;i<nDigits;i++) m*=10;
  m+=mf;
  int df=m*10;
  df=df/6;
  return d*10000000+df;  
}

void NMEA::process(const char in) {
    checksum ^= in;
//    Serial.print(in);
//    Serial.print(",");Serial.print(nmeaState->num,DEC);
//    Serial.print(",");Serial.print(pktState,DEC);
    nmeaState(*this,in);
//    Serial.print(",");Serial.print(nmeaState->num,DEC);
//    Serial.print(","),Serial.print(pktState,DEC);
//    Serial.print(",");Serial.println(checksum,HEX,2);
}

void expectDollar(NMEA& that, char in) {
  if(in=='$') {
    that.checksum=0;
    that.nmeaState=expectG;
  }
}

void expectG(NMEA& that, char in) {
  if(in=='G') {
    that.nmeaState=expectP;
    return;
  }
  that.nmeaState=expectDollar;
}

void expectP(NMEA& that, char in) {
  if(in=='P') {
    that.nmeaState=thirdLetter;
    return;
  }
  that.nmeaState=expectDollar;
}

void thirdLetter(NMEA& that, char in) {
  if(in=='G') {
    that.nmeaState=expectGGA2;
  } else if(in=='R') {
    that.nmeaState=expectRMC2;
  } else if(in=='V') {
    that.nmeaState=expectVTG2;
  } else if(in=='Z') {
    that.nmeaState=expectZDA2;
  } else {
    that.nmeaState=expectDollar;
  }
/*  switch(in) {
    case 'G':
      that.nmeaState=expectGGA2;
      return;
    case 'R':
      that.nmeaState=expectRMC2;
      return;
    case 'V':
      that.nmeaState=expectVTG2;
      return;
    case 'Z':
      that.nmeaState=expectZDA2;
      return;
  }
  that.nmeaState=expectDollar;
*/
}

void expectZDA2(NMEA& that, char in) {
  if(in=='D') {
    that.nmeaState=expectZDA3;
    return;
  }
  that.nmeaState=expectDollar;
}

void expectZDA3(NMEA& that, char in) {
  if(in=='A') {
    that.pktState=0;
    that.nmeaState=parseZDA;
    return;
  }
  that.nmeaState=expectDollar;
}

void expectGGA2(NMEA& that, char in) {
  if(in=='G') {
    that.nmeaState=expectGGA3;
    return;
  }
  that.nmeaState=expectDollar;
}

void expectGGA3(NMEA& that, char in) {
  if(in=='A') {
    that.pktState=0;
    that.nmeaState=parseGGA;
    return;
  }
  that.nmeaState=expectDollar;
}

void expectVTG2(NMEA& that, char in) {
  if(in=='T') {
    that.nmeaState=expectVTG3;
    return;
  }
  that.nmeaState=expectDollar;
}

void expectVTG3(NMEA& that, char in) {
  if(in=='G') {
    that.pktState=0;
    that.nmeaState=parseVTG;
    return;
  }
  that.nmeaState=expectDollar;
}

void expectRMC2(NMEA& that, char in) {
  if(in=='M') {
    that.nmeaState=expectRMC3;
    return;
  }
  that.nmeaState=expectDollar;
}

void expectRMC3(NMEA& that, char in) {
  if(in=='C') {
    that.pktState=0;
    that.nmeaState=parseRMC;
    return;
  }
  that.nmeaState=expectDollar;
}

fp NMEA::handleHMS() {
  int result;
  numBuf[numPtr]=0;
  numPtr=0;
  result=stof(numBuf);
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

int NMEA::handleMin() {
  int result;
  numBuf[numPtr]=0;
  numPtr=0;
  result=parseMin(numBuf);
  pktState++;
  return result;
}

fp NMEA::handleNum() {
  fp result;
  numBuf[numPtr]=0;
  numPtr=0;
  result=stof(numBuf);
  pktState++;
  return result;
}

/** Accumulate a character into the number buffer */
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

bool NMEA::parseFieldMin(char in, int& result) {
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

void parseZDA(NMEA& that, char in) {
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
        that.nmeaState=waitChecksum;
        that.finishPacket=finishZDA;
        return;
      } else if((in>='0' && in<='9')) {
        that.acc(in);
        return;
      }
      break;
  }
  //Error in packet, ignore packet and go back to start
  that.nmeaState=expectDollar;
}

void parseGGA(NMEA& that, char in) {
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
        that.nmeaState=waitChecksum;
        that.finishPacket=finishGGA;
        return;
      } else if((in>='0' && in<='9')||in=='.') {
        that.acc(in);
        return;
      }
      break;
  }
  //Error, ignore packet
  that.nmeaState=expectDollar;
}

void parseRMC(NMEA& that, char in) {
//  Serial.println(that.pktState);
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
        that.nmeaState=waitChecksum;
        that.finishPacket=finishRMC;
        return;
      } else if((in>='0' && in<='9')) {
        that.acc(in);
        return;
      }
      break;
  }
  //Error, ignore packet
//  Serial.print("Error in packet");
  that.nmeaState=expectDollar;
}

void parseVTG(NMEA& that, char in) {
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
        that.nmeaState=waitChecksum;
        that.finishPacket=finishVTG;
        return;
      } else if((in>='0' && in<='9')) {
        that.acc(in);
        return;
      }
      break;
  }
  //Error, ignore packet
  that.nmeaState=expectDollar;
}

void waitChecksum(NMEA& that, char in) {
// Serial.print(that.pktState);
//  Serial.println("waitChecksum");
  switch(that.pktState) {
    case 0: //waiting for *
      if(in=='*') {
        that.checksum^=in; //Don't include this character in the checksum
        that.pktState++;
//        Serial.print("Calculated Checksum: ");
  //      Serial.println(that.checksum,HEX,2);
      } else if(in==0x0D) {
        //No checksum is coming
  //      Serial.println("No checksum, shortcut exit");
        if(that.finishPacket) (that.finishPacket(that));
        that.nmeaState=expectDollar;
      }  
      return; //no possible error condition here, so get outta here.
    case 1:
      that.checksum^=in;
      if((in>='0' && in<='9') || (in>='A' && in<='F')) {
        int digit=(in<='9')?in-'0':in-'A'+10;
        if((that.checksum & 0xF0)!=(digit<<4)) {
//          Serial.println("Checksum bad");
  //        Serial.print(that.checksum & 0xF0);
    //      Serial.print(",");
      //    Serial.println(digit<<4);
	  break; //If checksum is bad, go directly to the end of the case statement
	}
        that.pktState++;
      }
      return;
    case 2:
      that.checksum^=in;
      if((in>='0' && in<='9') || (in>='A' && in<='F')) {
        int digit=(in<='9')?in-'0':in-'A'+10;
        if((that.checksum & 0x0F)!=(digit)) {
//          Serial.println("Checksum bad");
  //        Serial.print(that.checksum & 0x0F);
    //      Serial.print(",");
      //    Serial.println(digit);
	  break; //If checksum is bad, go directly to the end of the case statement
	}
//	Serial.println("Checksum ok");
        if(that.finishPacket) (that.finishPacket(that));
      }
  }
  that.nmeaState=expectDollar;
}

void finishZDA(NMEA& that) {
//  set_rtc(that.zdaYYYY,that.zdaMM,that.zdaDD,that.zdaHMS/10000,(that.zdaHMS%10000)/100,that.zdaHMS%100);
  that.writeZDA=true;
}

void finishVTG(NMEA& that) {
  that.writeVTG=true;
}

void finishGGA(NMEA& that) {
  if(!that.writeGGA) {
    that.HMS=that.ggaHMS;
    that.lat=that.ggaLat;
    that.lon=that.ggaLon;
    that.alt=that.ggaAlt;
    that.writeGGA=true;
  }
}

void finishRMC(NMEA& that) {
  int thatrmcDMY=that.rmcDMY;
  int thatrmcHMS=that.rmcHMS;
  set_rtc(thatrmcDMY%100+2000,(thatrmcDMY%10000)/100,thatrmcDMY/10000,thatrmcHMS/10000,(thatrmcHMS%10000)/100,thatrmcHMS%100);
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




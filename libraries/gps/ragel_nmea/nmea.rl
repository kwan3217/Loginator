#ifndef LPC2148
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#endif
#include "nmea.h"
#include "Time.h"
//#include "Serial.h" 

#ifdef LPC2148
#define printAction(x)
#else
#define printAction(x) printf(x "\n")
#endif

//Since we have a parser right here, handle conversion from decimal to integer here as 
//well. Don't call stoi or stof, which require another pass over the data, and extra storage.
%%{
  machine nmea;

  action handleDollar {printAction("handleDollar");checksum=0;countChecksum=true;}
  action startInt {printAction("startInt");buildInt=0;}
  action handleIntDigit {printAction("handleIntDigit");buildInt=buildInt*10+(fc-'0');}
  action startFrac {printAction("startFrac");buildFrac=0;fracDigits=1;}
  action handleFracDigit {printAction("handleFracDigit");buildFrac=buildFrac*10+(fc-'0');fracDigits*=10;}
  action finishFrac {printAction("finishFrac");buildFrac/=fracDigits;buildFloat=buildInt+buildFrac;}
  action finishIntFloat {printAction("finishIntFloat");buildFrac=0;buildFloat=buildInt;}
  action handleHH {printAction("handleHH");shadowHH=buildInt;}
  action handleNN {printAction("handleNN");shadowNN=buildInt;}
  action handleSS {printAction("handleSS");shadowSS=buildFloat;}
  action handleDD {printAction("handleDD");shadowDD=buildInt;}
  action handleMM {printAction("handleMM");shadowMM=buildInt;}
  action handleYY {printAction("handleYYYY");shadowYYYY=buildInt+1900;if(shadowYYYY<1950)shadowYYYY+=100;}
  action handleYYYY {printAction("handleYYYY");shadowYYYY=buildInt;}
  action handleStar {printAction("handleStar");checksum^='*';countChecksum=false;}
  action checkChecksum1 {printAction("handleChecksum1");
    if((fc<'0' || fc>'9') && (fc<'A' || fc>'F')) {fgoto main;} 
    int digit=(fc<='9')?fc-'0':fc-'A'+10;
    if((checksum & 0xF0)!=(digit<<4)) {fgoto main;}
  }
  action checkChecksum2 {printAction("handleChecksum2");
    if((fc<'0' || fc>'9') && (fc<'A' || fc>'F')) {fgoto main;} 
    int digit=(fc<='9')?fc-'0':fc-'A'+10;
    if((checksum & 0x0F)!=(digit)) {fgoto main;}
  }
  action finishZDA {printAction("finishZDA");finishZDA();fgoto main;}
  action finishRMC {printAction("finishRMC");finishRMC();fgoto main;}
  action finishGGA {printAction("finishGGA");finishGGA();fgoto main;}
  action finishVTG {printAction("finishVTG");finishVTG();fgoto main;}
  action handleDeg {printAction("handleDeg");shadowDeg=buildInt;}
  action handleLat {printAction("handleLat");shadowLat=shadowDeg*M10+(buildInt*M10+int(buildFrac*1e7))/60;}
  action invertLat {printAction("invertLat");shadowLat=-shadowLat;}
  action handleLon {printAction("handleLon");shadowLon=shadowDeg*M10+(buildInt*M10+int(buildFrac*1e7))/60;}
  action invertLon {printAction("invertLon");shadowLon=-shadowLon;}
  action handleSpd {printAction("handleSpd");shadowSpd=buildFloat;}
  action handleHdg {printAction("handleHdg");shadowHdg=buildFloat;}
  action handleAlt {printAction("handleAlt");shadowAlt=buildFloat;}

  int = ([0-9]+)                 $ handleIntDigit > startInt;  #Integer of any length, pick it up at buildInt
  int2 = ([0-9][0-9])            $ handleIntDigit > startInt;  #Integer specifically with two characters
  int3 = ([0-9][0-9][0-9])       $ handleIntDigit > startInt;  #Integer specifically with three characters
  int4 = ([0-9][0-9][0-9][0-9])  $ handleIntDigit > startInt;  #Integer specifically with four characters
  intIgn = ([0-9]+);                                           #Integer of any length, ignored to save on performing actions
  int2Ign = ([0-9][0-9]);                                      #Ignored integer specifically with two characters
  int3Ign = ([0-9][0-9][0-9])   ;                              #Ignored integer specifically with three characters
  int4Ign = ([0-9][0-9][0-9][0-9]) ;                           #Ignored integer specifically with four characters
  frac = (('.')>startFrac)(([0-9]+) $ handleFracDigit % finishFrac) ; #Fractional part of floating point number, pick it up in buildFrac 
  float = (int)((frac)?|('' % finishIntFloat)) ;                      #Float, pick it up in buildFloat, parts are still in buildInt and buildFrac
  floatIgn = (int)(('.'[0-9]+)?) ;                                         #Ignored float
  lat=(((int2 % handleDeg)(float)) % handleLat)(',N'|(',S' % invertLat)) ; #Latitude  in  DDMM.MMMMM,[NS], pick up degrees in shadowDeg, minutes in buildInt+buildFrac
  lon=(((int3 % handleDeg)(float)) % handleLon)(',E'|(',W' % invertLon)) ; #Longitude in DDDMM.MMMMM,[EW], pick up degrees in shadowDeg, minutes in buildInt+buildFrac
  checksum = [^*]* ( '*' $ handleStar ) (([A-F0-9] $ checkChecksum1)([A-F0-9] $ checkChecksum2 ))? ; #Handle (optional) checksum by skipping characters until the star, then skipping the star.
                                                                                                     #If checksum is present, check it one digit at a time.

  main := ('$' $ handleDollar (
         (('GPZDA,' 
                    (int2 % handleHH) (int2 % handleNN) (float % handleSS) ','  #UTC time of last PPS in HHMMSS.SSS 
                    (int2 % handleDD)','   #Day of month of last PPS
                    (int2 % handleMM)','   #Month of last PPS
                    (int4 % handleYYYY)',' #Year of last PPS
                    checksum ) %finishZDA)       
       | 
         (('GPRMC,' 
                    (int2 % handleHH) (int2 % handleNN) (float % handleSS) ','  #UTC time of fix in HHMMSS.SSS 
                    'A,' #Fix valid flag
                    (lat)',' #Latitude in   DDMM.MMMMM
                    (lon)',' #Longitude in DDDMM.MMMMM
                    (float % handleSpd)',' #Speed in knots
                    (float % handleHdg)',' #Heading in degrees East of North
                    (int2 % handleDD) (int2 % handleMM) (int2 % handleYY) ','#Date of fix in DDMMYY
                    checksum ) %finishRMC) 
       | 
         (('GPGGA,' 
                    (int2 % handleHH) (int2 % handleNN) (float % handleSS) ','  #UTC time of fix in HHMMSS.SSS 
                    (lat)(',N'|(',S' % invertLat))',' #Latitude in   DDMM.MMMMM
                    (lon)(',E'|(',W' % invertLon))',' #Longitude in DDDMM.MMMMM
                    '1,'                              #Fix valid flag
                    (intIgn)','                       #Number of satellites in fix (not used)
                    (floatIgn)','                     #HDOP (not used)
                    (float % handleAlt)',M'           #Altitude above MSL in m
                    checksum ) %finishGGA) 
       | 
         (('GPVTG,' (float % handleHdg)',T,'          #True heading in degrees East of North
                    (floatIgn)',M,'                   #Magnetic heading in degrees East of North
                    (float % handleSpd)',N,'          #Ground speed in knots
                    (floatIgn)',K'                    #Ground speed in km/hr
                    checksum ) %finishVTG) 
      )[\r\n]+)+ $err{fgoto main;};

}%%

%% write data;

NMEA::NMEA() :checksum(0),writeZDA(false),writeGGA(false),writeVTG(false),writeRMC(false) {
  %% write init;
};

void NMEA::process(const char in) {
#ifndef LPC2148
  printf("%c %d %02x %d\n",in,countChecksum,checksum,cs);
#endif
  if(countChecksum) checksum ^= in;
  const char *p=&in;
  const char *pe=p+1;
  const char *eof=NULL;
  %% write exec;
  if(cs==0) {
    %% write init;
  }
}

void NMEA::finishZDA() {
//  set_rtc(that.zdaYYYY,that.zdaMM,that.zdaDD,that.zdaHMS/10000,(that.zdaHMS%10000)/100,that.zdaHMS%100);
  if(!writeZDA) {
    HH=shadowHH;
    NN=shadowNN;
    SS=shadowSS;
    DD=shadowDD;
    MM=shadowMM;
    YYYY=shadowYYYY;
    writeZDA=true;
  }
}

void NMEA::finishVTG() {
  writeVTG=true;
}

void NMEA::finishGGA() {
  if(!writeGGA) {
    HH=shadowHH;
    NN=shadowNN;
    SS=shadowSS;
    lat=shadowLat;
    lon=shadowLon;
    alt=shadowAlt;
    writeGGA=true;
  }
}

void NMEA::finishRMC() {
//  set_rtc(thatrmcDMY%100+2000,(thatrmcDMY%10000)/100,thatrmcDMY/10000,thatrmcHMS/10000,(thatrmcHMS%10000)/100,thatrmcHMS%100);
  if(!writeRMC) {
    HH=shadowHH;
    NN=shadowNN;
    SS=shadowSS;
    lat=shadowLat;
    lon=shadowLon;
    spd=shadowSpd;
    hdg=shadowHdg;
    DD=shadowDD;
    MM=shadowMM;
    YYYY=shadowYYYY;
    writeRMC=true;
  }
}




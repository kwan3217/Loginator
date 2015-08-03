#include "nmea.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
  NMEA nmea;
  char sentences[]="$GPRMC,081836,A,3751.65,S,14507.36,E,000.0,360.0,130998,011.3,E*62\n"
                   "$GPRMC,225446,A,4916.45,N,12311.12,W,000.5,054.7,191194,020.3,E*68\n"
                   "$GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70\n"
                   "$GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70\n"
                   "$GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70\n"
                   "$GPZDA,214905.123,30,07,2015,00,00*\n"
                   "$GPZDA,214905.123,30,07,2015,00,00*\n";

  int len=strlen(sentences);
  for(int i=0;i<len;i++) {
    nmea.process(sentences[i]);
    if(nmea.writeRMC) {
      nmea.writeRMC=false;
      printf("RMC %02d:%02d:%06.3f lat:%d (%02d'%08.5f%c) lon:%d (%02d'%08.5f%c) Spd: %f kts Hdg: %f True %02d/%02d/%04d\n",nmea.HH,nmea.NN,nmea.SS,
       nmea.lat,abs(nmea.lat)/nmea.M10,60*fp((abs(nmea.lat)%nmea.M10)/1e7),nmea.lat>0?'N':'S',
       nmea.lon,abs(nmea.lon)/nmea.M10,60*fp((abs(nmea.lon)%nmea.M10)/1e7),nmea.lon>0?'E':'W',
       nmea.spd,nmea.hdg,nmea.MM,nmea.DD,nmea.YYYY);
    }
    if(nmea.writeZDA) {
      nmea.writeZDA=false;
      printf("ZDA %02d:%02d:%06.3f  %02d/%02d/%04d\n",nmea.HH,nmea.NN,nmea.SS,
       nmea.MM,nmea.DD,nmea.YYYY);
    }
  }
}


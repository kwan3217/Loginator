#include "playback.h"
//#include <time.h>
#include <string.h>
#include <stdio.h>
#include "navigate.h" //for clat
#include "LPC214x.h"

#include "ccsds.h"
#include "extract_str.inc"
int16_t ntohs(int16_t in) {
  return ((in&0xFF00)>>8) | ((in&0x00FF)<<8);
}

int32_t ntohl(int32_t in) {
  return ((in&0xFF000000)>>24) |
         ((in&0x00FF0000)>> 8) |
         ((in&0x0000FF00)<< 8) |
         ((in&0x000000FF)<<24);
}

float ntohf(float in) {
  int iin=*(int*)(&in);
  iin=ntohl(iin);
  return *(float*)(&iin);
}


RobotState* state;

int main(int argc, char** argv) {
  PlaybackState playbackState(argv[1],0);
  state=&playbackState;
  setup(); //Run robot setup code
  while(!playbackState.done()) {
    playbackState.propagate(1);
    loop();
  }
}

char buf[65536+7];
PlaybackState::PlaybackState(char* infn,int fs) {
  inf=fopen(infn,"rb");
  fread(buf,1,8,inf);
  fp FS=((fp)(250 << fs));
  double sens=FS/360.0; //rotations per second full scale
  sens*=2*PI;   //radians per second full scale
  sens/=32768;  //radians per second per DN
  sensX=sens; //Everyone gets nominal sensitivity
  sensY=sens;
  sensZ=sens;
}

void PlaybackState::propagate(int ms) {
  struct ccsdsHeader* ccsds=(struct ccsdsHeader*)buf;
  #include "extract_vars.inc" 

  fread(buf,1,6,inf);
  ccsds->apid=ntohs(ccsds->apid) & 0x07FF;
  ccsds->seq=ntohs(ccsds->seq);
  ccsds->length=ntohs(ccsds->length);
  fread(buf+6,1,ccsds->length+1,inf);
  buf[ccsds->length+7]=0; //terminate a char* at the end of a packet
  if(ccsds->apid==0x1A) {
    #include "reverse_packet_nmea.inc"
    int nmeaDataLength=ccsds->length+7-(nmea->nmeaData-buf);
    memcpy(gpsBuf,nmea->nmeaData,nmeaDataLength);
    ttc=nmea->TC;
    gpsBuf[nmeaDataLength]=0;
    gpsTransPointer=0;
  }
  if(ccsds->apid==0x24) {
    #include "reverse_packet_nav2.inc"
    xRate=-nav2->gx*sensX;
    yRate=-nav2->gy*sensY;
    zRate=-nav2->gz*sensZ;
    ttc=nav2->TC;
    TCR[0][channelTCGyro]=TTC(0); //Trigger a gyro reading
  }
  if(ccsds->apid==0x1C) {
    #include "reverse_packet_button.inc"
    ttc=button->TC;
    TCR[0][channelTCBtn]=TTC(0);
  }
}

bool PlaybackState::done() {
  return feof(inf);
}


#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <libgen.h>

#include "float.h"
#include "ccsds.h"
#include "direntry.h"

char buf[65536+7];
struct ccsdsHeader* ccsds=(struct ccsdsHeader*)buf;

#include "extract_str.INC"
#include "extract_vars.INC"

char* KwanSync="KwanSync";
uint32_t lastPPSTC;

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

unsigned int hour_packets=1200000*6;
unsigned int hour=0;
unsigned int fast_packets=0;
unsigned int min=0;
unsigned int lastTC=0;
void adjustMin(unsigned int TC) {
  if(TC<lastTC) {
    min++;
  }
  lastTC=TC;
}
int main(int argc, char** argv) {
  char oufn[1024],base[1024];
  strcpy(base,basename(argv[1]));
  base[8]=0;
  FILE* inf=fopen(argv[1],"rb");
  FILE* ouf[2048];
  
  for(int i=0;i<2048;i++) ouf[i]=NULL;
  fread(buf,1,8,inf);
  int seq=0;
  int qes=ntohl(seq);
  while(!feof(inf)) {
    fread(buf,1,6,inf);
    ccsds->apid=ntohs(ccsds->apid) & 0x07FF;
    ccsds->seq=ntohs(ccsds->seq);
    ccsds->length=ntohs(ccsds->length);
    fread(buf+6,1,ccsds->length+1,inf);
//    fprintf(stdout,"apid 0x%03x, length %5d\n",ccsds->apid,ccsds->length);
    if(ouf[ccsds->apid]==NULL) {
      sprintf(oufn,"%s_%03x_%02d.sds",base,ccsds->apid,hour);
      ouf[ccsds->apid]=fopen(oufn,"wb");
    }
    fwrite(&qes,sizeof(qes),1,ouf[ccsds->apid]);
    fwrite(buf+6,1,ccsds->length+1,ouf[ccsds->apid]);
    #include "extract_body.INC"
/*    if(ccsds->apid==0x03) {
      if(tarzpaq==NULL) {
        sprintf(oufn,"%s.cpio.zpaq",base);
        tarzpaq=fopen(oufn,"wb");
      }
      fwrite(buf+8,1,ccsds->length-1,tarzpaq);
    }
*/
    seq++;
    qes=ntohl(seq);
  }
  return 0;
}

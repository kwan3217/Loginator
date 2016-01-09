#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <libgen.h>
#include <fstream>
#include <ios>

#include "float.h"
#include "ccsds.h"
#include "direntry.h"

#include "tlmDb.h"

using std::string;
using std::ofstream;

char* KwanSync="KwanSync";
uint32_t lastPPSTC;

int16_t ntohs(int16_t in) {
  return ((in&0xFF00)>>8) |
		 ((in&0x00FF)<<8);
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

//From http://stackoverflow.com/a/21625151
char* print(char* fmt, ...) {
  static char buffer[80] = "";
  va_list argptr;
  va_start(argptr,fmt);
  vsprintf(buffer, fmt, argptr);
  va_end(argptr);
  return buffer;
}


int main(int argc, char** argv) {
  unsigned char buf[65536+7];
  struct ccsdsHeader* ccsds=(struct ccsdsHeader*)buf;
  char oufn[1024],base[1024];
  auto packets=tlmDb::read(argv[2]);
  std::map<int,tlmDb::Packet> packetsA;
  for(auto i=packets.begin();i!=packets.end();++i) {
	packetsA[i->apid]=*i;
  }
  strcpy(base,basename(argv[1]));
  base[8]=0;
  string base2(base);
  FILE* inf=fopen(argv[1],"rb");
  FILE* ouf[2048];
  ofstream csv[2048];
  static int min[2048];
  static uint32_t lastTC[2048];
  
  for(int i=0;i<2048;i++) ouf[i]=NULL;
  fread(buf,1,8,inf);
  int seq=0;
  int qes=ntohl(seq);
  long int fileptr;
  while(!feof(inf)) {
	fileptr=ftell(inf);
    fread(buf,1,6,inf);
    ccsds->apid=ntohs(ccsds->apid) & 0x07FF; //Throw away version and cmd/tlm
    ccsds->seq=ntohs(ccsds->seq) & 0x3FFF; //Throw away segmentation flags
    ccsds->length=ntohs(ccsds->length);
    fread(buf+6,1,ccsds->length+1,inf);
    buf[7+ccsds->length]=0; //Null-terminate the packet in case it ends with a string
    if(ouf[ccsds->apid]==NULL) {
      sprintf(oufn,"%s_%03x_%02d.sds",base,ccsds->apid,hour);
      ouf[ccsds->apid]=fopen(oufn,"wb");
    }
    fwrite(&qes,sizeof(qes),1,ouf[ccsds->apid]);
    fwrite(buf+6,1,ccsds->length+1,ouf[ccsds->apid]);
    tlmDb::Packet packet=packetsA[ccsds->apid];

    if(!csv[ccsds->apid].is_open()) {
      if(packet.shortName!="") {
        string csvfn=base2+"."+packet.shortName;
	    if(packet.shortName!=packet.fileExt) csvfn+=("."+packet.fileExt);
        csv[ccsds->apid].open(csvfn,std::ios::binary | std::ios::out);
        if(packet.extractor=="csv") {
          //Print the header
          int i=0;
          if(packet.hasTC()) {
        	csv[ccsds->apid] << "TC,t,";
        	i++;
          }
          csv[ccsds->apid] << packet.fields[i].name;
          i++;
          for(;i<packet.fields.size();i++) {
        	csv[ccsds->apid] << "," << packet.fields[i].name;
          }
          csv[ccsds->apid] << std::endl;
        }
      } else {
    	printf("No packet description for APID 0x%03x\n",ccsds->apid);
      }
    }
    if(packet.extractor=="dump") {
      //Dump packets currently ignore offsets, etc and just write the last field
      //of the packet into the file.
      int pos=packet.fieldStart[packet.fields.size()-1];

      csv[ccsds->apid].write((char*)(buf+pos),ccsds->length+7-pos);
    } else if(packet.extractor=="csv") {
      int i=0;
      if(packet.hasTC()) {
    	uint32_t TC=*((uint32_t*)(buf+packet.fieldStart[i]));
    	TC=ntohl(TC);
    	if(TC<lastTC[ccsds->apid]) {
    	  min[ccsds->apid]++;
    	}
    	lastTC[ccsds->apid]=TC;
    	csv[ccsds->apid] << print("%10u",TC) << ",";
    	csv[ccsds->apid] << print("%f",(double)(min[ccsds->apid]*60)+(double)(TC)/60'000'000)<<",";
    	i++;
      }
      for(;i<packet.fields.size();i++) {
    	auto field=packet.fields[i];
    	unsigned char* fieldStart=buf+packet.fieldStart[i];
        if(field.ntohType=="fp") {
          fp val=*((fp*)fieldStart);
          if(!field.le) val=ntohf(val);
          csv[ccsds->apid] << print("%f",val);
        } else if(packet.fields[i].ntohType=="int8_t") {
          int8_t val=*(int8_t*)fieldStart;
          csv[ccsds->apid] << (int)val;
        } else if(packet.fields[i].ntohType=="int16_t") {
          int16_t val=*((int16_t*)(buf+packet.fieldStart[i]));
          if(!field.le) val=ntohs(val);
          csv[ccsds->apid] << val;
        } else if(packet.fields[i].ntohType=="int32_t") {
          int32_t val=*(int32_t*)fieldStart;
          if(!field.le) val=ntohl(val);
          csv[ccsds->apid] << val;
        } else if(packet.fields[i].ntohType=="uint8_t") {
          uint8_t val=*(uint8_t*)fieldStart;
          csv[ccsds->apid] << (unsigned int)val;
        } else if(packet.fields[i].ntohType=="uint16_t") {
          uint16_t val=*(uint16_t*)fieldStart;
          if(!field.le) val=ntohs(val);
          csv[ccsds->apid] << val;
        } else if(packet.fields[i].ntohType=="uint32_t") {
          uint32_t val=*(uint32_t*)fieldStart;
          if(!field.le) val=ntohl(val);
          csv[ccsds->apid] << val;
        } else if(packet.fields[i].ntohType=="char[") {
          string val((char*)fieldStart);
          if(field.arraySize()>0) val=val.substr(0,field.arraySize());
          csv[ccsds->apid] << val;
        } else {
          printf("Unrecognized field type %s\n",field.ntohType.c_str());
        }
        if(i+1<packet.fields.size()) csv[ccsds->apid] << ",";
      }
      csv[ccsds->apid] << std::endl;
    }

    seq++;
    qes=ntohl(seq);
  }
  return 0;
}

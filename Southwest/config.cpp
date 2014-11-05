#include "config.h"
#include "Stringex.h"
#include <string.h>
#undef DEBUG
#ifdef DEBUG
#include "Serial.h"
#endif
#include "navigate.h" //for clat

const char Config::configFilename[]="CONFIG.TXT";
#ifdef LPC2148

bool Config::begin() {
  //Config file is in the form:
  //0) Zero or more non-printables <34 (takes care of remainder of 0D0A line ending)
  //1) One or more non-space printables of a tag, or a # which makes the rest of the line (to nonprintable <32) a comment
  //2) One space
  //3) One or more characters of data, ended by a nonprintable <32 (intended to be 0A or 0D0A)
  //data structure is determined by the tag. The tag is not case
  //sensitive (converted to uppercase internally). Total config file size is
  //limited to 1 sector (512 bytes). Tags have a maximum of 10 characters.
  //This code doesn't have to be particularly fast, since it is called once before
  //the run. It doesn't need to conserve stack too much since nothing else is 
  //running while this is. Prefer using stack for temporaries, so memory is 
  //reclaimed and not statically allocated.
  char buf[512]; //Put this on the stack, we are called early in the processing
                 //before much stack is used, and don't want to hold it
                 //permanently.
#else
#include <stdio.h>
#define FAIL(x) {printf("Failed, code %d\n",(x));return false;}
bool Config::begin() {
  FILE* inf=fopen(configFilename,"r");
  char buf[512];
  int len=fread(buf,1,512,inf);
  fclose(inf);
  return begin(buf,len);
}
bool Config::begin(char* buf, int size) {
#endif
  char tag[11];  //Storage for the tag
  int tagp=0;
  int partindata=0;
  int datastart=0;
  int tagid=0;
#ifdef LPC2148
  if(!f.openr(configFilename)) FAIL(100*f.errno+21);
  int size=f.size()>512?512:f.size();
  if(!f.read(buf)) FAIL(100*f.errno+22);
  //Dump the whole file into a packet, including its direntry. This will capture
  //the file name, date and size. The FileCircular buffer is big enough to 
  //handle it if the calling function calls dump right after
  ccsds.fill(f.de.entry,sizeof(f.de.entry));
  ccsds.fill(buf,size);
#endif
  for(int i=0;i<size;i++) {
#ifdef DEBUG
    Serial.print("Parsing char ");
    Serial.print(buf[i],HEX,2);
    if(buf[i]>=32) {
      Serial.print("(");
      Serial.write(buf[i]);
      Serial.print(")");
    }
    Serial.println();
#endif
    switch(partindata) {
      case 0: //Handle part 0, line break before tag
        if(buf[i]=='#') {
#ifdef DEBUG
          Serial.println("Found comment start");
#endif
          partindata=4;
          break;
        } else if(buf[i]>=33) {
#ifdef DEBUG
          Serial.println("Found the tag start");
#endif
          partindata=1;
          //Intentionally fall through so tag handler below gets to look at this char
        }
      case 1: //tag
        if(buf[i]!=' ') {
#ifdef DEBUG
          Serial.println("Found more tag");
#endif
          if(buf[i]>='a' && buf[i] <='z') buf[i]-=32; //Force upper case
          tag[tagp]=buf[i];
          tagp++;
        } else {
          //Skip to state 3
#ifdef DEBUG
          Serial.println("Found the tag end");
#endif
          partindata=3;
          datastart=i;
          //Finish this tag and reset for the next one
          tag[tagp]=0;
          tagp=0;
#ifdef DEBUG
          Serial.print("Tag is: ");
          Serial.println(tag);
#endif
          //Identify the tag, assign a number for easier case handling below
          if(strcmp(tag,"GSENS")==0) {
            tagid= 1;            
          } else if(strcmp(tag,"GODR")==0) {
            tagid= 2;            
          } else if(strcmp(tag,"GBW")==0) {
            tagid= 3;            
          } else if(strcmp(tag,"P")==0) {
            tagid= 4;            
          } else if(strcmp(tag,"PS")==0) {
            tagid= 5;            
          } else if(strcmp(tag,"I")==0) {
            tagid= 6;            
          } else if(strcmp(tag,"IS")==0) {
            tagid= 7;            
          } else if(strcmp(tag,"D")==0) {
            tagid= 8;            
          } else if(strcmp(tag,"DS")==0) {
            tagid= 9;            
          } else if(strcmp(tag,"WPDLAT")==0) {
            tagid=10;            
          } else if(strcmp(tag,"WPDLON")==0) {
            tagid=11;            
          } else if(strcmp(tag,"THR")==0) {
            tagid=12;
          } else if(strcmp(tag,"YSCL")==0) {
            tagid=13;
          } else {
#ifdef DEBUG
            Serial.print("Unrecognized tag");
#endif
            FAIL(23);
          }
        }
        break;
      case 3: //data
        if(buf[i]<32) {
          //Found the end of the data
          buf[i]=0; 
          partindata=0;
          if(!handleData(tagid,buf+datastart)) return false; //handleData will set errno if necessary
        }
        break;
      case 4: //comment
        if(buf[i]<32) {
          //Found the end of the data
          partindata=0;
        }
        break;
    }
  }
  if(partindata==3) if(!handleData(tagid,buf+datastart)) return false; //handleData will set errno if necessary
  return true;
}

bool Config::handleData(int tagid, char* buf) {
#ifdef DEBUG
  Serial.print("Tag ID: ");
  Serial.println(tagid,DEC);
  Serial.print("Data:");
  Serial.println(buf);
#endif
  switch(tagid) {
    case 1:
      if(!handleInt(buf,gyroSens)) return false; //handler will set errno if necessary
      break;
    case 2:
      if(!handleInt(buf,gyroODR)) return false; 
      break;
    case 3:
      if(!handleInt(buf,gyroBW)) return false;
      break;
    case 4:
      if(!handleInt(buf,P)) return false; 
      break;
    case 5:
      if(!handleInt(buf,Ps)) return false;
      break;
    case 6:
      if(!handleInt(buf,I)) return false; 
      break;
    case 7:
      if(!handleInt(buf,Is)) return false;
      break;
    case 8:
      if(!handleInt(buf,D)) return false; 
      break;
    case 9:
      if(!handleInt(buf,Ds)) return false;
      break;
    case 10:
      if(!handleWaypoint(buf,dlatWaypoint,1)) return false;
      break;
    case 11:
      if(!handleWaypoint(buf,dlonWaypoint,clat)) return false;
      break;
    case 12:
      if(!handleInt(buf,throttle)) return false;
      break;
    case 13:
      if(!handleInt(buf,yscl)) return false;
      break;
    default:
      FAIL(24);
  }
  return true;
}

bool Config::handleInt(char* buf, int& out) {
  trim(buf);
#ifdef DEBUG
  Serial.println("handleInt()");
  Serial.print("Trimmed data: ");
  Serial.println(buf);
#endif  
  out=stoi(buf);
#ifdef DEBUG
  Serial.print("Value: ");
  Serial.println(out);
#endif
  return true;
}

bool Config::handleWaypoint(char* buf, fp* out, fp scale) {
  trim(buf);
  nWaypoints=0;
  int n=strlen(buf);
  int bp=0;
  for(int i=0;i<n;i++) {
    if(buf[i]==' ') {
      buf[i]=0;
      out[nWaypoints]=stoi(buf+bp)*scale/1e7;
      bp=i+1;
      nWaypoints++;
    }
  }
  out[nWaypoints]=stoi(buf+bp)*scale/1e7;
  nWaypoints++;
  out[nWaypoints]=out[0]; //close the loop
  return true;
}

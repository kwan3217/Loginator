#include "config.h"
#include "Stringex.h"
#include <string.h>
#define DEBUG
#ifdef DEBUG
#include "Serial.h"
#endif

const char Config::configFilename[]="YUKARI.CFG";

bool Config::begin() {
  //Config file is in the form:
  //0) Zero or more non-printables <34 (takes care of remainder of 0D0A line ending)
  //1) One or more non-space printables of a tag
  //2) One space
  //3) One or more characters of data, ended by a nonprintable <33 (intended to be 0A or 0D0A)
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
  char tag[11];  //Storage for the tag
  int tagp=0;
  int partindata=0;
  int datastart=0;
  int tagid=0;
  if(!f.openr(configFilename)) {
#ifdef DEBUG
    Serial.print("Couldn't open file: ");
    Serial.println(configFilename);
#endif
    FAIL(100*f.errno+21);
  }
  int size=f.size()>512?512:f.size();
  if(!f.read(buf)) {
#ifdef DEBUG
    Serial.print("Couldn't read file: ");
    Serial.println(100*f.errno+22);
#endif
    FAIL(100*f.errno+22);
  }
  //Dump the whole file into a packet, including its direntry. This will capture
  //the file name, date and size. The FileCircular buffer is big enough to 
  //handle it if the calling function calls dump right after
  ccsds.fill(f.de.entry,sizeof(f.de.entry));
  ccsds.fill(buf,size);
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
        if(buf[i]>=33) {
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
          Serial.print("Tag is: ");
          Serial.println(tag);
          //Identify the tag, assign a number for easier case handling below
          if(strcmp(tag,"GSENS")==0) {
            tagid=1;            
          } else if(strcmp(tag,"GODR")==0) {
            tagid=2;            
          } else if(strcmp(tag,"GBW")==0) {
            tagid=3;            
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


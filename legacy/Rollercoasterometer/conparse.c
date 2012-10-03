#include <stdio.h>
#include <string.h>
#include "conparse.h"
#include "stringex.h"
#include "fat16.h"
#include "rootdir.h"
#include "sd_raw.h"
#include "debug.h"
#include "setup.h"
#include "main.h"
#include "sdbuf.h"
#include "pktwrite.h"
#include "flash.h"

#define LOGCON_NAME "RollerCon.txt"

//actual definitions of config parameters
int uartMode[2]={1,1};
int baud[2]={9600,9600};
char trigger[2]={'$','$'};
char frameEnd[2]={'\n','\n'};
int rawSize[2]={64,64};
char timestamp[2]={0,0};
int writeMode=1;
int adcFreq=100;
int adcBin=1;
char channelActive[9]={1,1,1,1,1,1,1,1,1};
int nChannels=9;
int GPSSyncMode=0;
int powerSave=0;
int GSense=0;
int autoGSense=0;
int dumpFirmware=0;
int firmAddress=0;
int writeFirm=0;
char firmFile[80];

//Log configuration tags and default values, used to write the LogCon.txt if not present
//and to read LogCon.txt.
//Specified as one long string here, but used as a ragged array of strings terminated by a double \0
static char LogConTags[]="UART1 mode\0"     //NMEA, Text, Raw, SiRF, off (anything else)
                         "UART1 baud\0"       //Baud code, or Auto
                         "UART1 trigger\0"    //Trigger, same as before (Only effective in NMEA)
                         "UART1 end\0"        //Frame end character (Only effective in NMEA)
                         "UART1 size\0"        
                         "Write mode\0"       //Text, Binary, SiRF, off (anything else)
                         "ADC frequency\0"    //ADC read rate, Hz
                         "ADC binning\0"      //ADC binning rate
                         "GPS Sync\0"         //0=none, 1=GPRMC only, 2=PPS
                         "GSense\0"           //Accelerometer sensitivity, 0=1.5G, 1=2G, 2=4G, 3=6G, or Auto
                         "Powersave\0"        //0=no powersave, 1=powersave
                         "Firmdump\0"         //0=Dump firmware into log file, 1=don't
                         "loadFirm\0"         //name of file to load into firmware
                         "loadAddr\0";        //Address in flash to load. Both of these have to be present before it tries to load firmware.
static char LogConDefault[]="None\0"
                            "0\0"
                            "$\0"
                            "0x0A\0"
                            "64\0"
                            "Text\0"
                            "100\0"
                            "1\0"
                            "1\0"
                            "3\0"
                            "1\0"
                            "\0"
                            "\0"
                            "\0";
        
static int processLine(char* keyword, char* value) {
  trim(keyword);
  trim(value);
  int whichKeyword=0;
  int whereInTags=0;
  char done=0;
  while(!done) {
    if(0==strcasecmp(keyword,&LogConTags[whereInTags])) {
      done=1;
    } else {
      whichKeyword++;
      while(LogConTags[whereInTags]!=0) whereInTags++;
      whereInTags++;
      if(LogConTags[whereInTags]==0) return 0; //Didn't find the keyword
    }
  }
  switch(whichKeyword) {
    case  0: //"UART0 mode\0"       //NMEA, Hex, Raw, SiRF, off (anything else)
      if(0==strcasecmp(value,"NMEA")) {
        uartMode[1]=PKT_NMEA;
      } else if(0==strcasecmp(value,"Text")) {
        uartMode[1]=PKT_TEXT;
      } else if(0==strcasecmp(value,"Binary")) {
        uartMode[1]=PKT_BINARY;
      } else if(0==strcasecmp(value,"SiRF")) {
        uartMode[1]=PKT_SIRF;
      } else if(0==strcasecmp(value,"Both")) {
        uartMode[1]=PKT_BOTH;
      } else {
        uartMode[1]=PKT_NONE;
      }
      break;
    case  1: //"UART0 baud\0"
      if(0==strcasecmp(value,"Auto")) {
        baud[1]=-1;
      } else {
        baud[1]=stoi(value);
      }
      break;
    case  2: //"UART0 trigger\0"    //Trigger, same as before (Only effective in NMEA)
      if(strlen(value)>1) {
        trigger[1]=stoi(value);
      } else {
        trigger[1]=value[0];
      }
      break;
    case  3: //"UART0 end\0"        //Frame end character (Only effective in NMEA)
      if(strlen(value)>1) {
        frameEnd[1]=stoi(value);
      } else {
        frameEnd[1]=value[0];
      }
      break;
    case  4: //"UART0 size\0"        
      rawSize[1]=stoi(value);
      break;
    case  5:
      if(0==strcasecmp(value,"NMEA")) {
        writeMode=PKT_NMEA;
      } else if(0==strcasecmp(value,"Binary")) {
        writeMode=PKT_BINARY;
      } else if(0==strcasecmp(value,"SiRF")) {
        writeMode=PKT_SIRF;
      } else if(0==strcasecmp(value,"Text")) {
        writeMode=PKT_TEXT;
      } else {
        writeMode=PKT_NONE;
      }
      break;
    case  6:
      adcFreq=stoi(value);
      break;
    case  7:
      adcBin=stoi(value);
      break;
    case  8:
      GPSSyncMode=stoi(value);
      break;
    case  9:
      if(0==strcasecmp(value,"Auto")) {
        autoGSense=1;
        GSense=0;
      } else {
        GSense=stoi(value);
      }
      break;
    case 10:
      powerSave=stoi(value);
	  break;
  	case 11:
	    if(strlen(value)>0) {
	      dumpFirmware=1;
   	  } else {
	      dumpFirmware=0;
	    }
	    break;
  	case 12:
  	  if(strlen(value)>0) {
	      strcpy(firmFile,value);
        writeFirm++;
  	  }
  	  break;
	  case 13:
  	  if(strlen(value)>0) {
	      firmAddress=stoi(value);
        writeFirm++;
  	  }
	    break;
  }
  return 0;
}

//0 for success, negative for failure
int readLogCon(void) {
  char keyword[64];
  char value[64];                            
  char stringBuf[512];

  struct fat16_file_struct fd;
  int len;
  int s=0;
  int t=0;
  int d=0;
  keyword[0]=0;
  value[0]=0;
  int result;
  if(root_file_exists(LOGCON_NAME)) {
    result = root_open(&fd,LOGCON_NAME);
    
    if(result<0) return result;
    len = fat16_read_file(&fd, (unsigned char *)stringBuf, 512);
    if(len<0) return -1;
    fat16_close_file(&fd);
  } else {
    result=root_open_new(&fd,LOGCON_NAME);
    if(result<0) return result;

    //I hate pointer math!    
    while(LogConTags[t]!=0) {
      while(LogConTags[t]!=0) {
        stringBuf[s]=LogConTags[t];
        s++;t++;
      }
      t++; //skip the \0 at the end of the tag
      stringBuf[s]='=';s++;
      while(LogConDefault[d]!=0) {
        stringBuf[s]=LogConDefault[d];
        s++;d++;
      }
      d++; //skip the \0 at the end of the default
      stringBuf[s]='\r';s++;
      stringBuf[s]='\n';s++;
    }
    len=s;
    fat16_write_file(&fd, (unsigned char*)stringBuf, len);
    fat16_close_file(&fd);
    sd_raw_sync();
  }
  
  s=0;
  t=0;
  char inValue=0;
  for(s = 0; s < len; s++) {
    if(stringBuf[s] == '=') {
      keyword[t]=0;
      inValue=1;
      t=0;
    } else if(stringBuf[s]=='\n') {
      value[t]=0;
      inValue=0;
      t=0;
      result=processLine(keyword,value);
      if(result<0) return result;
    } else {
      if(inValue) {
        value[t]=stringBuf[s];
      } else {
        keyword[t]=stringBuf[s];
      }
      t++;
    }
  }
  return 0;
}

//0 for success, negative for failure
int writeLogCon(void) {
  char line[64];
  char stringBuf[512];

  struct fat16_file_struct fd;
  int len;
  int s=0;
  int t=0;
  line[0]=0;
  int result;

  result = root_open(&fd,LOGCON_NAME);
    
  if(result<0) return result;
  len = fat16_read_file(&fd, (unsigned char *)stringBuf, 512);
  if(len<0) return -1;
  fat16_close_file(&fd);
  
  s=0;
  t=0;
  for(s = 0; s < len; s++) {
    if(stringBuf[s]=='\n') {
      line[t]=0;
	  if(line[t-1]=='\r') line[t-1]=0;
      t=0;
      fillPktStart(&sdBuf,PT_FILE);
	  fillPktString(&sdBuf,line);
	  fillPktFinish(&sdBuf);
      if(result<0) return result;
    } else {
      line[t]=stringBuf[s];
      t++;
    }
  }
  
  return 0;
}


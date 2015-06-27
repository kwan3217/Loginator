#include <stdio.h>
#include <string.h>
#include "conparse.h"
#include "stringex.h"
#include "fat16.h"
#include "rootdir.h"
#include "sd_raw.h"
#include "setup.h"
#include "main.h"
#include "sdbuf.h"
#include "pktwrite.h"
#include "flash.h"

#define LOGCON_NAME "RocketCon.txt"

//actual definitions of config parameters
int uartMode[2]={1,1};
int baud[2]={9600,9600};
char trigger[2]={'$','$'};
char frameEnd[2]={'\n','\n'};
int rawSize[2]={64,64};
int writeMode=1;
int adcFreq=100;
int GSense=0;
int dumpFirmware=1;
int firmAddress=0;
int writeFirm=0;
int accBW;
int gyroPeriod;
char firmFile[80];
char ropeFile[80];
int ropeReady=0;

typedef void(*conparseFunc)(char*,int*);

void f_uartMode1(char* value, int* dummy) {
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
}

void f_uartBaud1(char* value, int* dummy) {
  if(0==strcasecmp(value,"Auto")) {
    baud[1]=-1;
  } else {
    baud[1]=stoi(value);
  }
}

void f_uartTrigger1(char* value, int* dummy) {
  if(strlen(value)>1) {
    trigger[1]=stoi(value);
  } else {
    trigger[1]=value[0];
  }
}

void f_uartEnd1(char* value, int* dummy) {
  if(strlen(value)>1) { 
    frameEnd[1]=stoi(value);
  } else {
    frameEnd[1]=value[0];
  }
}        

void f_stoi(char* value, int* p) {
  if(strlen(value)>0) {
    *p=stoi(value);
  } else {
    *p=0;
  }
}

void f_writeMode(char* value, int* dummy) {
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
}

void f_firmFile(char* value, int* dummy) {
  if(strlen(value)>0) {
    strcpy(firmFile,value);
    writeFirm++;
  }
}

void f_firmAddr(char* value, int* dummy) {
  if(strlen(value)>0) {
    firmAddress=stoi(value);
    writeFirm++;
  }
}

typedef struct {
  char* tag;
  char* def;
  conparseFunc f;
  int* val;
} logcon_t;

//Log configuration tags and default values, used to write the LogCon.txt if not present
//and to read LogCon.txt.
const logcon_t fTable[]={
    {"UART1 mode",   "NMEA",f_uartMode1,   NULL},          //NMEA, Text, Raw, SiRF, off (anything else)
    {"UART1 baud",   "9600",f_uartBaud1,   NULL},          //Baud rate, or Auto
    {"UART1 trigger","$",   f_uartTrigger1,NULL},          //Trigger, same as before (Only effective in NMEA)
    {"UART1 end",    "0x0A",f_uartEnd1,    NULL},          //Frame end character (Only effective in NMEA)
    {"UART1 size",   "64",  f_stoi,        &rawSize[1]},
    {"Write mode",   "NMEA",f_writeMode,   NULL},          //Text, Binary, SiRF, off (anything else)
    {"Sensor freq",  "100", f_stoi,        &adcFreq},      //ADC read rate, Hz
    {"Acc bandwidth","4",   f_stoi,        &accBW},        //Accelerometer bandwidth, 0=10Hz, 1=20Hz, 2=40Hz, 3=75Hz, 4=150Hz, 5=300Hz, 6=600Hz, 7=1200Hz
    {"Gyro Period",  "10",  f_stoi,        &gyroPeriod},   //Gyroscope period, ms
    {"GSense",       "3",   f_stoi,        &GSense},          //Accelerometer sensitivity, 0=1G, 1=1.5G, 2=2G, 3=3G, 4=4G, 5=8G, 6=16G
    {"Firmdump",     "0",   f_stoi,        &dumpFirmware}, //1=Dump firmware into log file, 0=don't
    {"loadFirm",     "",    f_firmFile,    NULL},          //name of file to load into firmware
    {"loadAddr",     "",    f_firmAddr,    NULL},          //Address in flash to load. Both of these have to be present before it tries to load firmware.
    {"","",NULL,NULL}                                      //End marker
};

int processLine(char* keyword, char* value) {
  trim(keyword);
  trim(value);
  int whichKeyword=0;
  char done=0;
  while(!done) {
    if(0==strcasecmp(keyword,fTable[whichKeyword].tag)) {
      done=1;
    } else {
      whichKeyword++;
      if(fTable[whichKeyword].tag==0) return 0; //Didn't find the keyword
    }
  }
  fTable[whichKeyword].f(value,fTable[whichKeyword].val);
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
  keyword[0]=0;
  value[0]=0;
  int result;
  if(root_file_exists(LOGCON_NAME)) {
    //There already is a logcon file, open it and suck int in
    result = root_open(&fd,LOGCON_NAME);
    
    if(result<0) return result;
    len = fat16_read_file(&fd, (unsigned char *)stringBuf, 512);
    if(len<0) return -1;
    fat16_close_file(&fd);
  } else {
    //No existing logcon file, write a fresh one
    result=root_open_new(&fd,LOGCON_NAME);
    if(result<0) return result;
    stringBuf[0]=0;

    //Write out each default line
    for(int i=0;strlen(fTable[i].tag)>0;i++) {
      strcat(stringBuf,fTable[i].tag);
      strcat(stringBuf,"=");
      strcat(stringBuf,fTable[i].def);
      strcat(stringBuf,"\r\n");
    }

    //write it out to the file
    len=strlen(stringBuf);
    fat16_write_file(&fd, (unsigned char*)stringBuf, len);
    fat16_close_file(&fd);
    sd_raw_sync();
  }

  //Either way, the logcon file is now in stringBuf and
  //its length is in len
  
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


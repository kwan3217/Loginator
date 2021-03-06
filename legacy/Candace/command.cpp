#include <stdio.h>
#include "command.h"
extern "C" {
#include "fat16.h"
#include "rootdir.h"
}

#include "stringex.h"
#include "sdbuf.h"
#include "pktwrite.h"
#include "sirfwrite.h"
#include "nmeawrite.h"
#include "uart.h"
#include "setup.h"

int commandLoops=0;

#define COMMAND_NUM 10

int commandAt[COMMAND_NUM];
char commandFired[COMMAND_NUM];
char commandType[COMMAND_NUM];
char commandLen[COMMAND_NUM];
int commandStart[COMMAND_NUM];
int commandPort[COMMAND_NUM];
char commands[512];
                       
int loadCommands(void) {
  for(int i=0;i<10;i++) {
    commandAt[i]=0;
  }
  struct fat16_file_struct fd;
  if(!root_file_exists("COMMAND.txt")) {
    root_open_new(&fd,"COMMAND.txt");
    fat16_close_file(&fd);
    return 0;
  }
  if(root_open(&fd,"COMMAND.txt")<0) return -1;
  int len = fat16_read_file(&fd, commands, 512);
  if(len<0) return -1;
  fat16_close_file(&fd);

  //Here we do a fancy read and write in the same buffer. OK
  //since no encoding we use compresses data, so read will always
  //be ahead of write
  int q=0; //Location of write
  int atPtr=0;
  char mode=0;
  int j=0; //Which command we're on
  char hex=0;
  char inhex=0;
  commandStart[0]=0;
  
  for(int s = 0; s < len; s++) {
    switch(mode) {
      case 0:
        commandType[j]=upper(commands[s]);
        mode=(commandType[j]==';'?6:1); //Skip comments
        break;
      case 1:
        //skip the space(s) between the type and at
        if(!isBlank(commands[s+1])) {
          mode=2;
          atPtr=s+1;
        }
        break;
      case 2:
        //count the number of characters in the at
        if(isBlank(commands[s])) {
          mode=3;
          commands[s]=0;
          commandAt[j]=stoi(&commands[atPtr]);
        }
        break;
      case 3:
        //skip the space(s) between the at and port
        mode=4;
        atPtr=s;
        break;
      case 4:
        //count the number of characters in the port
        if(isBlank(commands[s])) {
          mode=5;
          commands[s]=0;
          commandPort[j]=stoi(&commands[atPtr]);
          commandFired[j]=0;
          commandLen[j]=0;
        }
        break;
      case 5:
        //Deal with the command itself
        switch(commandType[j]) {
          case 'T':
            //Text mode, just back the data up
            commands[q]=commands[s];
            commandLen[j]++;
            q++;
            break;
          case 'B':
          case 'N':
            //NMEA or bare, don't put in unprintables
            if(commands[s]>=' ') {
              commands[q]=commands[s];
              commandLen[j]++;
              q++;
            }
            break;
          case 'H':
          case 'S':
            if(!isBlank(commands[s])) {
              hex=hex<<4 | ((commands[s]<'A')?(commands[s]-'0'):(commands[s]-'A'+10));
              inhex++;
              if(inhex==2) {
                //Write this hex char back
                commands[q]=hex;
                commandLen[j]++;
                q++;
                hex=0;
                inhex=0;
              }
            }
            break;
        }
        if(commands[s]=='\n') {
          mode=0;
     		  commands[q]=0;
          j++;
          commandStart[j]=q;
        }
        break;
      case 6:
        if(commands[s]=='\n') {
          mode=0;
          commandStart[j]=q;
        }
        break;
    }
  }
  return 0;
}                       

void writeCommand() {
  for(int j=0;j<COMMAND_NUM;j++) {
    if((commandAt[j]>0) && (!commandFired[j]) && (commandLoops>=commandAt[j])) {
      commandFired[j]=1;
      pktwrite* pw;
      if('S'==commandType[j]) pw=&SiRFWrite; else pw=&NMEAWrite;
      pw->fillPktPayload(sdBuf,commands+commandStart[j],commandLen[j],commandPort[j]);
    }
  }
  commandLoops++;
}							 


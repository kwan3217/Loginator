#ifndef pktwrite_h
#define pktwrite_h
#include "circular.h"
#include "setup.h"

//These must be in the same order as headerNMEASIRF in pktwrite.c
#define PT_COLUMNS 0 
#define PT_GAINADJ 1
#define PT_DECIDE  2
#define PT_ANALOG  3
#define PT_MAM     4
#define PT_FILE    5
#define PT_LOAD    6
#define PT_PPS     7
#define PT_VERSION 8
#define PT_UART    9
#define PT_SDINFO 10
#define PT_BAUD   11
#define PT_FLASH  12
#define PT_ERROR  13

void fillPktStart(circular* buf,int type);
void fillPktFinishCore(circular* buf);
int fillPktByte(circular* buf, char in);
int fillPktShort(circular* buf, short in);
int fillPktInt(circular* buf, int in);
int fillPktString(circular* buf, char* in);
void fillPktFinish(circular* buf);

#ifdef COMMAND_SYS
void fillPktFinishSend(circular* buf, int port);
#endif

#endif


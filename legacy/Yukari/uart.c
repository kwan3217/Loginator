#include "LPC214x.h"
#include "armVIC.h"
#include "uart.h"
#include "main.h"
#include "conparse.h"
#include "setup.h"
#include "load.h"
#include "serial.h"
#include "gps.h"
#include "setup.h"
#include "pktwrite.h"
#include "sdbuf.h"

circular uartbuf[2];
volatile int uartInPkt[2];

#define STANDARD_BAUD_NUM 9
static int standardBaud[STANDARD_BAUD_NUM]={1200,2400,4800,9600,14400,19200,38400,57600,115200};
             
inline static int isFrameStart(int port, char next) {
  if(PKT_NMEA==uartMode[port]) {
    return (next==trigger[port]);
  } else if(PKT_SIRF==uartMode[port]) {
    return (next==0xA0);
  }
  return 1;
}

inline static int isFrameEndNMEA(int port) {
  return (peekHead(&uartbuf[port],-1)==frameEnd[port]);
}

inline static int isFrameEndSiRF(int port) {
  return (peekHead(&uartbuf[port],-1)==0xB3 && peekHead(&uartbuf[port],-2)==0xB0);
}
 
inline static int isFrameEnd(int port) {
  if(PKT_NMEA==uartMode[port] || PKT_TEXT==uartMode[port]) {
    return isFrameEndNMEA(port);  
  } else if(PKT_SIRF==uartMode[port]) {
    return isFrameEndSiRF(port);
  }
  return unreadylen(&uartbuf[port])<rawSize[port];
}

static void UARTISR(int port) {
  char next,status;
  
  //Read the whole FIFO
  hasLoad(LOAD_UART);
  status=ULSR(port);
  while(status & 0x01) {
    next=URBR(port);
    if(uartInPkt[port] || isFrameStart(port,next)) { //If already in the frame, or this character starts it
      uartInPkt[port]=1; //We're still in the frame
      fill(&uartbuf[port],next);
      if(isFrameEnd(port) || unreadylen(&uartbuf[port])>=912) {
        //But not after this character
        uartInPkt[port]=0;
        if(PKT_SIRF==uartMode[port]) {
          parseSirf(&uartbuf[port]);
        } else if(PKT_NMEA==uartMode[port]) {
          parseNmea(&uartbuf[port]);
        }
//        sendBuf(1-port,&uartbuf[port]);
        mark(&uartbuf[port]);
      }
    }
    status=ULSR(port);
  }

}

void setup_uart(int port, int setbaud, int want_ints) {
  setup_serial(port,setbaud,want_ints?UARTISR:(void*)0);

  fillPktStart(&uartbuf[port],PT_UART);
  fillPktByte(&uartbuf[port],port & 0xFF);
  fillPktInt(&uartbuf[port],setbaud);
  fillPktFinish(&uartbuf[port]);
}

void startRecordUART(void) {
  if (PKT_NONE!=uartMode[0]) {
    if(baud[0]<0) autobaud(0);
    if(baud[0]>0) setup_uart(0,baud[0],1);
  }
  if (PKT_NONE!=uartMode[1]) {
    if(baud[1]<0) autobaud(1);
    if(baud[1]>0) setup_uart(1,baud[1],1);
  }
}


void sendBuf(int port, circular* buf) {
  char* pos;
  if(buf->head>buf->mid) {
    pos=buf->buf+buf->mid;
    tx_serial(port,pos,unreadylen(buf));
  } else {
    pos=buf->buf+buf->mid;
    tx_serial(port,pos,1024-buf->mid);
    pos=buf->buf;
    tx_serial(port,pos,buf->head);
  }
  mark(buf);
}

//Returns positive if it looks like this is a good baud
//Returns negative otherwise
//Good baud has at least 20 characters in a second
//and less than 1/100 of characters have an error
static int countCharBaud(int port, int trybaud, unsigned int* chars, unsigned int* errors) {
  *chars=0;
  *errors=0;
  setup_uart(port,trybaud,0);
  
  unsigned int start=T0TC;
  while((T0TC-start)<PCLK) {
    int status=ULSR(port);
    if(status & 0x01) {
      char junk=URBR(port);
      (*chars)++;
      if(status & 0x80) (*errors)++;
    }
  }
  int result;
  if (*chars<20) {
    result=-1;
  } else if (*errors>(*chars/100)) {
    result=-2;
  } else {
    result=1;
  }
  fillPktStart(&uartbuf[port],PT_BAUD);
  fillPktByte(&uartbuf[port],port);
  fillPktInt(&uartbuf[port],trybaud);
  fillPktInt(&uartbuf[port],*chars);
  fillPktInt(&uartbuf[port],*errors);
  fillPktInt(&uartbuf[port],result);
  fillPktFinish(&uartbuf[port]);
  return result;
}

int autobaud(int port) {
  int lighton=0;
  int leastErrors=99999;
  int leastBaud=-1;
  unsigned int chars,errors;
  for(int i=0;i<STANDARD_BAUD_NUM;i++) {
    lighton=1-lighton;
    set_light(port,lighton);
    int result=countCharBaud(port,standardBaud[i],&chars,&errors);
    if(result>0) {
      baud[port]=standardBaud[i];
      set_light(port,OFF);
      return result;
    }
  	if(result==-2) {
	    if(errors<(chars/10) && errors<leastErrors) {
	      leastErrors=errors;
        leastBaud=i;
      }
	  }
	  drainToSD(&uartbuf[port]);
  }
  //No spped was perfect, use the best speed
  if(leastBaud>=0) {
    baud[port]=standardBaud[leastBaud];
	set_light(port,OFF);
	return 2;
  }
  //No speed was close enough, don't use the port
  uartMode[port]=PKT_NONE;
  baud[port]=0;
  set_light(port,OFF);
  return -1;
}

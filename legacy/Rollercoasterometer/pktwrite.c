#include "pktwrite.h"
#include "circular.h"
#include "setup.h"
#include "uart.h"
#include "conparse.h"
#include "sirfwrite.h"
#include "nmeawrite.h"

static char headerNMEASIRF[]={'C',0x2A,
                                'G',0x33,
				         		'D',0x2F,
						        'A',0x2C,
         						'M',0x17,
		         				'F',0x20,
				         		'L',0x15,
								'P',0x19,
								'V',0x1A,
								'U',0x2B,
								'S',0x18,
								'B',0x16,
                'H',0x40,
                'E',0x41};

void fillPktStart(circular* buf,int type) {
  switch(writeMode) {
    case PKT_NMEA:
      fillString(buf,"$PKWN");
      fill(buf,headerNMEASIRF[type*2]);
	  break;
	case PKT_SIRF:
      fillStartSirf(buf, headerNMEASIRF[type*2+1]);
      break;
  }
  //Reset all formatting parameters so that 
  //it doesn't matter what the last packet was
  buf->dataDigits=0;
  buf->dataDec=1;
  buf->dataAsciiz=0;
  buf->dataComma=1;
}

void fillPktFinish(circular* buf) {
  switch(writeMode) {
    case PKT_NMEA:
	  fillFinishNMEA(buf);
	  break;
    case PKT_SIRF:
	  fillFinishSirf(buf);
	  break;
  }
}

int fillPktByte(circular* buf, char in) {
  switch(writeMode) {
    case PKT_NMEA:
	  if(buf->dataComma)fill(buf,',');
	case PKT_TEXT:
	  if(buf->dataDec) {
	    if(buf->dataDigits>0) return fill0Dec(buf,in,buf->dataDigits);
		return fillDec(buf,in);
	  } else {
	    return fillHex(buf,in,buf->dataDigits>0?buf->dataDigits:2);
	  }
	case PKT_SIRF:
	case PKT_BINARY:
	  return fill(buf,in);
  }
  return 0;
}

int fillPktShort(circular* buf, short in) {
  switch(writeMode) {
    case PKT_NMEA:
	  if(buf->dataComma)fill(buf,',');
	case PKT_TEXT:
	  if(buf->dataDec) {
	    if(buf->dataDigits>0) return fill0Dec(buf,in,buf->dataDigits);
		return fillDec(buf,in);
	  } else {
	    return fillHex(buf,in,buf->dataDigits>0?buf->dataDigits:4);
	  }
	case PKT_SIRF:
	case PKT_BINARY:
	  return fillShort(buf,in);
  }
  return 0;
}

int fillPktInt(circular* buf, int in) {
  switch(writeMode) {
    case PKT_NMEA:
	  if(buf->dataComma)fill(buf,',');
	case PKT_TEXT:
	  if(buf->dataDec) {
	    if(buf->dataDigits>0) return fill0Dec(buf,in,buf->dataDigits);
		return fillDec(buf,in);
	  } else {
	    return fillHex(buf,in,buf->dataDigits>0?buf->dataDigits:8);
	  }
	case PKT_SIRF:
	case PKT_BINARY:
	  return fillShort(buf,in);
  }
  return 0;
}

int fillPktString(circular* buf, char* in) {
  if(PKT_NMEA==writeMode && buf->dataComma) fill(buf,',');
  int i=0;
  while(in[i]!=0) {
    int result=fill(buf,in[i]);
    if (result<0) return result;
    i++;
  }
  if(buf->dataAsciiz && (PKT_BINARY==writeMode || PKT_SIRF==writeMode)) {
    return fill(buf,0); 
  } else {
    return 0;
  }
}

#ifdef COMMAND_SYS
void fillPktFinishSend(circular* buf, int port) {
  switch(writeMode) {
    case PKT_NMEA:
	  fillFinishNMEASend(buf,port);
	  break;
    case PKT_SIRF:
	  fillFinishSirfSend(buf,port);
	  break;
  }
}
#endif

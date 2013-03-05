#ifndef USB_H
#define USB_H

#include "LPC214x.h"

#define DEBUG
#ifdef DEBUG
#include "Serial.h"
#define DBG Serial.println
#else
#define DBG(x ...) 
#endif

class USB;

class FrameHandler {
public:
  virtual void handle(USB* that, char frameCount)=0;
};

class DeviceHandler {
  public:
  virtual void handle(USB* that, char status)=0;
};

class USBEndpoint {
public:
  int packetSize;
  USBEndpoint(int LpacketSize):packetSize(LpacketSize) {};
  virtual void handle(USB* usb, int physical, char status)=0;
};

class SetupPacket {
public:
  unsigned char requestType;
  unsigned char request;
  unsigned short wValue;
  unsigned short wIndex;
  unsigned short wLength;
  static const unsigned int REQTYPE_DIR_TO_HOST=1;
  static const unsigned int REQTYPE_DIR_TO_DEVICE=0;
  unsigned int getType() {return (requestType>>5) & 0x03;};
  unsigned int getDir() {return (requestType>>7) & 0x01;};
  unsigned int getRecip() {return (requestType>>0) & 0x1F;};
  void debug();
};

class RequestHandler {
public:
  virtual bool handle(SetupPacket& setup, unsigned short* len, char** data)=0;
};

class StandardRequestHandler:public RequestHandler {
private:
  char stdReqData[8];
  bool handleDevice(SetupPacket& setup, unsigned short* len, char** data);
  bool handleInterface(SetupPacket& setup, unsigned short* len, char** data) {return false;};
  bool handleEndpoint(SetupPacket& setup, unsigned short* len, char** data) {return false;};
public:
  static const unsigned int REQTYPE_RECIP_DEVICE=0;
  static const unsigned int REQTYPE_RECIP_INTERFACE=1;
  static const unsigned int REQTYPE_RECIP_ENDPOINT=2;
  static const unsigned int REQTYPE_RECIP_OTHER=3;
  static const unsigned int REQ_GET_STATUS        = 0x00;
  static const unsigned int REQ_CLEAR_FEATURE     = 0x01;
  static const unsigned int REQ_SET_FEATURE       = 0x03;
  static const unsigned int REQ_SET_ADDRESS       = 0x05;
  static const unsigned int REQ_GET_DESCRIPTOR    = 0x06;
  static const unsigned int REQ_SET_DESCRIPTOR    = 0x07;
  static const unsigned int REQ_GET_CONFIGURATION = 0x08;
  static const unsigned int REQ_SET_CONFIGURATION = 0x09;
  static const unsigned int REQ_GET_INTERFACE     = 0x0A;
  static const unsigned int REQ_SET_INTERFACE     = 0x0B;
  static const unsigned int REQ_SYNCH_FRAME       = 0x0C;
  virtual bool handle(SetupPacket& setup, unsigned short* len, char** data);
};

class ControlEndpoint:public USBEndpoint {
private:
  SetupPacket setup;
  RequestHandler* requestHandler[4];
  StandardRequestHandler standardRequestHandler;
public:
  static const unsigned int REQTYPE_TYPE_STANDARD=0;
  static const unsigned int REQTYPE_TYPE_CLASS=1;
  static const unsigned int REQTYPE_TYPE_VENDOR=2;
  static const unsigned int REQTYPE_TYPE_RESERVED=3;
  ControlEndpoint():USBEndpoint(64) {requestHandler[REQTYPE_TYPE_STANDARD]=&standardRequestHandler;};
  virtual void handle(USB* usb, int physical, char status);
};

#define leShort(x) (x)& 0xFF,((x)>>8)&0xFF
class USB {
private:
  //One handler for each logical endpoint
  USBEndpoint* endpoint[16];
  ControlEndpoint ep0;
  static const char descDevice[];
public:
  static const unsigned char CMD_EP_SELECT=0x00;
  static const unsigned char CMD_EP_SET_STATUS=0x40;
  static const unsigned char CMD_EP_CLEAR_BUFFER=0xF2;
  static const unsigned char CMD_DEV_SET_MODE=0xF3;
  static const unsigned char CMD_DEV_READ_CUR_FRAME_NR=0xF5;
  static const unsigned char CMD_EP_VALIDATE_BUFFER=0xFA;
  static const unsigned char CMD_DEV_STATUS=0xFE;
  static const unsigned int FRAME=(1<<0);
  static const unsigned int EP_SLOW=(1<<2);
  static const unsigned int DEV_STAT=(1<<3);
  static const unsigned int CCEMTY=(1<<4);
  static const unsigned int CDFULL=(1<<5);
  static const unsigned int EP_RLZED=(1<<8);

  static const unsigned int EP_STATUS_SETUP=(1<<2);

  static const unsigned int CON=(1<<0);
  static const unsigned int CON_CH=(1<<1);
  static const unsigned int SUS_CH=(1<<3);
  static const unsigned int RST=(1<<4);
  static const unsigned int PKT_RDY=(1<<11);
  static const unsigned int DV=(1<<10);

  static const int OUT=0;
  static const int IN=1;
  static const int EP_OUT=(1 << OUT);
  static const int EP_IN =(1 << IN);
  static const unsigned int RD_EN=(1<<OUT);
  static const unsigned int WR_EN=(1<<IN);
  static const int EP_BOTH=EP_OUT | EP_IN;

  virtual bool begin();
  void cmd(char cmdCode);
  void cmdWrite(char cmdCode, char arg);
  char cmdRead(char cmdCode);
  void waitDevInt(int bit) {while(!(USBDevIntSt & bit)); USBDevIntClr=bit;};
  void HwInt();
  void registerEndpoint(USBEndpoint *ep, int logicalNum, int dir);
  void loop();
  void realize(int physical, int size);
  int readEndpoint(int physical, char* buf, unsigned int maxLen);
  void writeEndpoint(int physical, char* buf, unsigned int len);
  void stall(int physical, bool stalled);
  void registerDevDescriptor(const char* desc, int len);
  void SoftConnect(bool connected) { cmdWrite(CMD_DEV_STATUS, connected?CON:0);}
  FrameHandler* frameHandler;
  DeviceHandler* deviceHandler;
};


#endif

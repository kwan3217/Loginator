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

//Remember that USB defines IN and OUT from the point of view of the host, so
//OUT is HOST->DEVICE and IN is HOST<-DEVICE
static const int OUT=0;
static const int IN=1;

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

static const int EP_OUT=(1 << OUT);
static const int EP_IN =(1 << IN);
static const unsigned int RD_EN=(1<<OUT);
static const unsigned int WR_EN=(1<<IN);
static const int EP_BOTH=EP_OUT | EP_IN;

class FrameHandler {
public:
  virtual void handle(USB* that, char frameCount)=0;
};

class DeviceHandler {
  public:
  virtual void handle(USB* that, char status)=0;
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
  /** @param [in] setup reference to setup structure, used to determine what to
                  do with the request
      @param [out] len length of data returned
      @param [out] data pointer to data to be returned. Data itself may be in 
                   read-only memory, so no one is allowed to modify the data.
  */
  virtual bool handle(SetupPacket& setup, unsigned short& len, const char*& data)=0;
};

class HandlerTree: public RequestHandler {
protected:
  //Pointer to array of pointers to request handlers
  RequestHandler** leaf;
public:
  HandlerTree():leaf() {};
  virtual bool handle(SetupPacket& setup, unsigned short& len, const char*& data) {
    int d=dispatch(setup);
    if(d<0) return false;
    leaf[d]->handle(setup,len,data);
  };
  virtual int dispatch(SetupPacket& setup);
};

class StandardDeviceHandler:public RequestHandler {
  static const unsigned int REQ_GET_DESCRIPTOR    = 0x06;
  static const unsigned int REQ_SET_DESCRIPTOR    = 0x07;
  static const char descDevice[];
  virtual bool handle(SetupPacket& setup, unsigned short& len, const char*& data);
};

class StandardInterfaceHandler:public RequestHandler {
  virtual bool handle(SetupPacket& setup, unsigned short& len, const char*& data) {return false;};
};

class StandardEndpointHandler:public RequestHandler {
  virtual bool handle(SetupPacket& setup, unsigned short& len, const char*& data) {return false;};
};

class StandardRequestHandler:public HandlerTree {
private:
  //Array of pointers to request handlers
  StandardDeviceHandler handleDevice;
  StandardInterfaceHandler handleInterface;
  StandardEndpointHandler handleEndpoint;
  RequestHandler* thisLeaf[3];
public:
  StandardRequestHandler() {
    leaf=thisLeaf;
    thisLeaf[0]=&handleDevice;
    thisLeaf[1]=&handleInterface;
    thisLeaf[2]=&handleEndpoint;
  };
  static const unsigned int REQTYPE_RECIP_DEVICE=0;
  static const unsigned int REQTYPE_RECIP_INTERFACE=1;
  static const unsigned int REQTYPE_RECIP_ENDPOINT=2;
  static const unsigned int REQTYPE_RECIP_OTHER=3;
  static const unsigned int REQ_GET_STATUS        = 0x00;
  static const unsigned int REQ_CLEAR_FEATURE     = 0x01;
  static const unsigned int REQ_SET_FEATURE       = 0x03;
  static const unsigned int REQ_SET_ADDRESS       = 0x05;
  static const unsigned int REQ_GET_CONFIGURATION = 0x08;
  static const unsigned int REQ_SET_CONFIGURATION = 0x09;
  static const unsigned int REQ_GET_INTERFACE     = 0x0A;
  static const unsigned int REQ_SET_INTERFACE     = 0x0B;
  static const unsigned int REQ_SYNCH_FRAME       = 0x0C;
  virtual int dispatch(SetupPacket& setup);
};

class USBEndpoint {
private:
  int physEPNumOut,physEPNumIn,logEPNum;
  int packetSize;
protected:
  int iResidue,iLen;
  const char* pbData;
public:
  USBEndpoint(int LlogEPNum, int LpacketSize):
    physEPNumOut(LlogEPNum<<2|OUT),
    physEPNumIn(LlogEPNum<<2|IN),
    logEPNum(LlogEPNum),
    packetSize(LpacketSize) 
  {};
  int getEPNumIn()  {return physEPNumIn;};
  int getEPNumOut() {return physEPNumOut;};
  int getEPNum()    {return logEPNum;};
  int getPacketSize() {return packetSize;};
  virtual void handle(USB* usb, int physical, char status)=0;
  //Transfer of bytes from physical endpoint buffer in USB peripheral to main memory
  //Always use the OUT physical endpoint
  int  read(USB* that, char* buf, unsigned int maxLen);
  //Transfer of bytes from main memory to physical endpoint buffer
  //Always use the IN physical endpoint
  void write(USB* that, char* buf, unsigned int len);
  //Stall or unstall the endpoint in the indicated direction
  void stall(USB* that, int dir, bool stalled);

  void setupIn(int LiLen, const char* LpbData) {iResidue=iLen=LiLen;pbData=LpbData;};
  void in(USB* that);
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
  ControlEndpoint():USBEndpoint(0,64) {requestHandler[REQTYPE_TYPE_STANDARD]=&standardRequestHandler;};
  virtual void handle(USB* usb, int physical, char status);
};

#define leShort(x) (x)& 0xFF,((x)>>8)&0xFF
class USB {
private:
  //One handler for each logical endpoint
  USBEndpoint* endpoint[16];
  ControlEndpoint ep0;
public:
  virtual bool begin();
  void cmd(char cmdCode);
  void cmdWrite(char cmdCode, char arg);
  char cmdRead(char cmdCode);
  void waitDevInt(int bit) {while(!(USBDevIntSt & bit)); USBDevIntClr=bit;};
  void HwInt();
  void registerEndpoint(USBEndpoint *ep, int logicalNum, int dir);
  void loop();
  void realize(int physical, int size);
  void registerDevDescriptor(const char* desc, int len);
  void SoftConnect(bool connected) { cmdWrite(CMD_DEV_STATUS, connected?CON:0);}
  FrameHandler* frameHandler;
  DeviceHandler* deviceHandler;
};

static const int DESC_DEVICE            = 1;
static const int DESC_CONFIGURATION     = 2;
static const int DESC_STRING            = 3;
static const int DESC_INTERFACE         = 4;
static const int DESC_ENDPOINT          = 5;
static const int DESC_DEVICE_QUALIFIER  = 6;
static const int DESC_OTHER_SPEED       = 7;
static const int DESC_INTERFACE_POWER   = 8;

#endif

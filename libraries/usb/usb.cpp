#include "usb.h"
#include "gpio.h"
#include "Time.h"

const DevDescriptor StandardDeviceHandler::devDescriptor(0x3217,0x0001,0x0001);

/** Get the USB hardware ready, up to but not including turning on SoftConnect. This includes
setting pin 23 to USB Vbus detect,
setting pin 31 to USB SoftConnect,
setting up PLL1 to provide 48MHz,
Turning on the USB system
Acking, clearing, and disabling all interrupts,
Disabling NAK interrupts from the protocol engine
registering the handler for endpoint 0.

Doesn't turn on SoftConnect because subclasses of this may wish to do additional
things, including registering more endpoint handlers, before talking to the 
host. Subclasses should call USB::begin first, then do their thing.
*/
bool USB::begin() {
  //Initialize the hardware
  set_pin(23,1);     //Setup USB Vbus detect pin
  set_pin(31,2);     //Setup USB_Softconnect pin
  setup_pll(1,4);    //Set up PLL1 to run at 4x crystal rate. With 12MHz crystal, we get the 48MHz we need.
  PCONP |= (1<<31);  //Turn on the USB subsystem
  USBDevIntEn=0;               //Disable all device interrupts
  USBDevIntPri=0;              //Set all device interrupts to SLOW
  USBDevIntClr=0xFFFFFFFF;     //Ack and clear any pending interrupts
  USBEpIntEn=0;                //Disable all endpoint interrupts
  USBEpIntPri=0;               //Set all endpoint interrupts to SLOW
  USBEpIntClr=0xFFFFFFFF;      //Ack and clear any pending interrupts
  cmdWrite(CMD_DEV_SET_MODE,0);   //Disable NAK interrupts
  registerEndpoint(&ep0,0,EP_BOTH);   //Register control endpoint handler
  DBG("Got through USB::begin");
  return true;
}

/** Protocol engine command
*/
void USB::cmd(char cmdCode) {
  USBDevIntClr=CCEMTY | CDFULL;
  USBCmdCode=((unsigned int)cmdCode)<<16 | 0x05<<8;
  waitDevInt(CCEMTY);
}

/** Protocol engine command with 1 byte argument
*/
void USB::cmdWrite(char cmdCode, char arg) {
  cmd(cmdCode);
  USBCmdCode=((unsigned int)arg)<<16 | 0x01<<8;
  waitDevInt(CCEMTY);
}

/** Protocol Engine Command with 1 byte read
*/
char USB::cmdRead(char cmdCode) {
  cmd(cmdCode);
  USBCmdCode=((unsigned int)cmdCode)<<16| 0x02<<8;
  waitDevInt(CDFULL);
  return USBCmdData;
}

/** Handle hardware interrupts
*/
void USB::HwInt() {
  if(USBDevIntSt & FRAME) {
    //The FRAME device interrupt is set
    USBDevIntClr=FRAME; //Ack the interrupt
    char frameNumber=cmdRead(CMD_DEV_READ_CUR_FRAME_NR); //Read the data whether there is a handler or not
    if(frameHandler) frameHandler->handle(this,frameNumber); //Call the handler if any
  }
  if(USBDevIntSt & DEV_STAT) {
    //The DEV_STAT device interrupt is set
    USBDevIntClr=DEV_STAT; //Ack the interrupt
    int status=cmdRead(CMD_DEV_STATUS);
    if(status & (CON_CH | SUS_CH | RST)) deviceHandle(status);
  }
  if(USBDevIntSt & EP_SLOW) {
    USBDevIntClr=EP_SLOW;
    for(unsigned int physical=0;physical<32;physical++) {
      if(USBEpIntSt & (1 << physical)) {
        DBG("HwInt:EP_SLOW");
        DBG(physical,DEC);
        USBEpIntClr=(1<<physical);
        waitDevInt(CDFULL);
        char data=USBCmdData;
        if(endpoint[physical/2]) endpoint[physical/2]->handle(this,physical,data);
      }
    }
  }
}

void USB::deviceHandle(char status) {
  if(status & RST) {
    DBG("Reset");
    //I think that RST is handled automatically
  }
}

int USBEndpoint::out(USB* that, char* buf, unsigned int maxLen) {
  if(buf==0) return -1;
  USBCtrl=RD_EN | (logEPNum<<1);
  unsigned int len;
  do {
    len=USBRxPLen;
  } while ((len & PKT_RDY)==0);
  //Is the packet valid?
  if((len & DV)==0) {
    return -1;
  }
  len&=0x3FF; //lower 10 bits are the actual length, bits 10 and 11 are flags
  unsigned int data=0;
  for(unsigned int i=0;i<len;i++) {
    if((i%4)==0) data=USBRxData;
    if(i<maxLen) buf[i]=data & 0xFF; //Note that this throws away bytes in a packet which would overrun the buffer
    data>>=8;
  }
  //Clear RD_EN
  USBCtrl=0; 
  //Clear endpoint buffer
  that->cmd(CMD_EP_SELECT | physEPNumOut);
  that->cmd(CMD_EP_CLEAR_BUFFER);
  return len;
}

void USBEndpoint::write(USB* that, char* buf, unsigned int len) {
  USBCtrl=WR_EN | logEPNum;
  USBTxPLen=len;
  while(USBCtrl & WR_EN) {
    USBTxData=((unsigned int)buf[0])<< 0 
             |((unsigned int)buf[1])<< 8
             |((unsigned int)buf[2])<<16
             |((unsigned int)buf[3])<<24;
    buf+=4;
  }
  that->cmd(CMD_EP_SELECT|logEPNum);
  that->cmd(CMD_EP_VALIDATE_BUFFER);
}

/** Record an endpoint handler for a certain logical endpoint and realize
  the buffer(s) for it.

\param ep Endpoint handler
\param logicalNum Logical endpoint number, 0 to 15
\param dir Bit field, with bit 0 set if we care about the OUT physical 
 endpoint and bit 1 set if we care about the IN physical endpoint
*/
void USB::registerEndpoint(USBEndpoint *ep, int logicalNum, int dir) {
  endpoint[logicalNum]=ep;
  USBEpIntEn|=(dir << logicalNum*2);
  USBDevIntEn|=(1<<2);
  if(dir & EP_OUT) realize(logicalNum<<2 | OUT,ep->getPacketSize());
  if(dir & EP_IN)  realize(logicalNum<<2 | IN ,ep->getPacketSize());
};

void USB::loop() {
  SoftConnect(1);  //Tell the host that we are ready to communicate by turning on USB_SoftConnect
  DBG("Got through USB_Softconnect");
  while(gpio_read(23)==1) {   //While USB is still plugged in...
    HwInt();                  //Handle any "interrupts" that come in
  }  
  DBG("Done with HwInt");
}

void USB::realize(int physical, int size) {
  USBReEp|=(1<<physical);
  USBEpInd=physical;
  USBMaxPSize=size;
  waitDevInt(EP_RLZED);
  cmdWrite(CMD_EP_SET_STATUS|physical,0);
}

void USBEndpoint::stall(USB* that, int dir, bool stalled) {
  that->cmdWrite(CMD_EP_SET_STATUS | (dir==OUT?physEPNumOut:physEPNumIn),stalled?1:0);
}

int USBEndpoint::in(USB* that, const char* pbBuf, int iLen) {
  // set write enable for specific endpoint
  USBCtrl = WR_EN | ((logEPNum & 0xF) << 2);
  DBG("USBCtrl");
  DBG(iLen,HEX,8);
        
  // set packet length
  USBTxPLen = iLen;
        
  // write data
  while (USBCtrl & WR_EN) {
    USBTxData = (pbBuf[3] << 24) | (pbBuf[2] << 16) | (pbBuf[1] << 8) | pbBuf[0];
    pbBuf += 4;
  }

  // select endpoint and validate buffer
  that->cmd(CMD_EP_SELECT | physEPNumIn);
  that->cmd(CMD_EP_VALIDATE_BUFFER);
        
  return iLen;
}

void USBEndpoint::in(USB* that) {
  DBG("IN");
  DBG(inResidue,DEC);
  if(inResidue<=0) return; //early exit if no data to send

  int iChunk=packetSize<inResidue?packetSize:inResidue;
  in(that,pbInData,iChunk);
  pbInData+=iChunk;
  inResidue-=iChunk;
}

void ControlEndpoint::handle(USB* that, int physical, char status) {
  if((physical & 0x01)==OUT) {
    //This is an OUT (host to device) transfer
    if(status & EP_STATUS_SETUP) {
      out(that, (char*)&setup, sizeof(setup));
      setup.debug();
      int iType=setup.getType();
      if((setup.wLength==0) || (setup.getDir()==SetupPacket::REQTYPE_DIR_TO_HOST)) {
        bool handled=false;
        if(requestHandler[setup.getType()]==0) {
          DBG("No handler for reqtype");
          DBG(setup.getType());
          stall(that,OUT,true);
        } else {
          handled=requestHandler[setup.getType()]->handle(setup, setup.wLength,pbInData);
          if(!handled) {
            DBG("Handler failed");
            stall(that,OUT,true);
          } else {
            setupIn(setup.wLength,pbInData);
          }
        }
      }
    }
  } else {
    //This is an IN (device to host) transfer
    in(that);
  }
}

void SetupPacket::debug() {
  DBG("setup.requestType ");
  DBG(requestType,HEX,2);
  DBG("setup.request ");
  DBG(request,HEX,2);
  DBG("setup.wValue ");
  DBG(wValue,HEX,4);
  DBG("setup.wIndex ");
  DBG(wIndex,HEX,4);
  DBG("setup.wLength ");
  DBG(wLength,HEX,4);
}

int StandardRequestHandler::dispatch(SetupPacket& setup) {
  return setup.getRecip();
}

bool StandardDeviceHandler::handle(SetupPacket& setup, unsigned short& len, const char*& data) {
  switch(setup.request) {
    case REQ_GET_DESCRIPTOR:
      DBG("Desc");DBG(setup.wValue,HEX);
      len=devDescriptor.bLength;
      data=(const char*)&devDescriptor;
      return true;
  }
  return false;
}

//A descriptor is an array of bytes. The USB protocol requires a length byte
//first. All implementations I have seen have one long array with multiple 
//descriptors, which the implementation then has to parse out. I'm not going to 
//do that.





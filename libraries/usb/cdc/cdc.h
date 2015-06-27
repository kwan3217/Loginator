#ifndef CDC_H
#define CDC_H

#include "usb.h"

struct HeaderFuncDescriptor:public ClassSpecDescriptor {
  uint16_t bcdCDC __attribute__((packed));
  HeaderFuncDescriptor():ClassSpecDescriptor(5,CS_INTERFACE,0),bcdCDC(0x0120) {};
};

struct CallMgmtFuncDescriptor:public ClassSpecDescriptor {
  uint8_t bmCapabilities __attribute__((packed));
  uint8_t bDataInterface __attribute__((packed));
  CallMgmtFuncDescriptor():ClassSpecDescriptor(5,CS_INTERFACE,1) {};
};

struct ACMFuncDescriptor:public ClassSpecDescriptor {
  uint8_t bmCapabilities __attribute__((packed));
  uint8_t bDataInterface __attribute__((packed));
  ACMFuncDescriptor():ClassSpecDescriptor(4,CS_INTERFACE,2) {};
};

struct UnionFuncDescriptor:public ClassSpecDescriptor {
  uint8_t bControlInterface __attribute__((packed));
  uint8_t bSubordinateInterface0 __attribute__((packed));
  UnionFuncDescriptor():ClassSpecDescriptor(5,CS_INTERFACE,6) {};
};

class CDC {
private:
  USB& usb;
/*
  InterfaceDescriptor iDescControl;
  InterfaceDescriptor iDescData;
  HeaderFuncDescriptor hDesc;
  CallMgmtFuncDescriptor cmDesc;
  ACMFuncDescriptor acmDesc;
  UnionFuncDescriptor uDesc;
*/
public:
  CDC(USB& Lusb):usb(Lusb) {};
  virtual bool begin();
};

#endif



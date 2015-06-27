#include "msc.h"

bool MSC::begin() {
  if(!USB::begin()) return false;
  cmdWrite(CMD_DEV_SET_MODE,(1<<5));   //Enable NAK interrupts for INACK_BI (Interrupt on NACK for Bulk In)
  DBG("Got through MSC::begin");
  return true;
}

const char MSC::descDevice[]={leShort(0x0200), //USB standard 2.00
                              0x00, //Device Class (defined by Interface)
                              0x00, //Device subclass (zero since class is zero)
                              0x00, //Device protocol (defined by interface)
                              64,   //max packet size
                              leShort(0x3217),   //Vendor ID (not in usb.if as of 12 Oct 2012)
                              leShort(0x0001),   //Product ID
                              leShort(0x0001),   //Device version number 0.01
                              0x01, //Index of manufacturer name
                              0x00, //Index of product name
                              0x00, //Index of device serial number
                              1};   //Number of configurations



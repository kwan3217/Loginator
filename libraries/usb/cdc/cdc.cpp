#include "cdc.h"

bool CDC::begin() {
  usb.cmdWrite(CMD_DEV_SET_MODE,(1<<5));   //Enable NAK interrupts for INACK_BI (Interrupt on NACK for Bulk In)
  DBG("Got through CDC::begin");
  return true;
}



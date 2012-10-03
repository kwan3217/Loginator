#ifndef HARDTWOWIRE_H
#define HARDTWOWIRE_H

#include "Wire.h"

class HardTwoWire:public TwoWire {
  private:
    int port;
    virtual void twi_init(void);
    virtual uint8_t twi_readFrom(uint8_t address, char* data, uint8_t length);
    virtual uint8_t twi_writeTo(uint8_t address, const char* data, uint8_t length, uint8_t wait);
    virtual void twi_stop(void);
    virtual void twi_releaseBus(void);
  public:
    HardTwoWire(int);
};

extern HardTwoWire Wire;
extern HardTwoWire Wire1;

#endif

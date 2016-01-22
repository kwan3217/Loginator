#ifndef HARDTWOWIRE_H
#define HARDTWOWIRE_H

#include "Wire.h"
#include "LPC214x.h"

class HardTwoWire:public TwoWire {
  private:
    int port;
    virtual void twi_init(unsigned int freq);
    virtual uint8_t twi_readFrom(uint8_t address, char* data, uint8_t length);
    virtual uint8_t twi_writeTo(uint8_t address, const char* data, uint8_t length, uint8_t wait);
    void twi_stop();
    void twi_releaseBus();
    static const int AA   =(1 << 2);
    static const int SI   =(1 << 3);
    static const int STO  =(1 << 4);
    static const int STA  =(1 << 5);
    static const int EN =(1 << 6);
    void wait_si() {while(!(I2CCONSET(port) & SI)) ;}
  public:
    HardTwoWire(int);
};

extern HardTwoWire Wire;
extern HardTwoWire Wire1;

#endif

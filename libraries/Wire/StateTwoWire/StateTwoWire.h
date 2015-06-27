#ifndef HARDTWOWIRE_H
#define HARDTWOWIRE_H

#include "Wire.h"
#include "LPC214x.h"

class StateTwoWire:public TwoWire {
  public:
    int port;
    virtual void twi_init(unsigned int freq);
    uint8_t address;
    const char* dataWrite;
    char* dataRead;
    uint8_t lengthWrite;
    uint8_t lengthRead;
    volatile bool done;
    virtual uint8_t twi_readFrom(uint8_t Laddress, char* Ldata, uint8_t Llength);
    virtual uint8_t twi_writeTo(uint8_t Laddress, const char* Ldata, uint8_t Llength, uint8_t wait);
    static const int AA   =(1 << 2);
    static const int SI   =(1 << 3);
    static const int STO  =(1 << 4);
    static const int STA  =(1 << 5);
    static const int EN =(1 << 6);
    static StateTwoWire *thisPtr[2];
    static void IntHandler0();
    static void IntHandler1();
    void wait_si() {while(!(I2CCONSET(port) & SI)) ;}
    void stateDriver();
  public:
    StateTwoWire(int);
};

#endif

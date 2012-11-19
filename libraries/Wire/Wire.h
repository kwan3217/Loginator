#ifndef WIRE_H
#define WIRE_H
#include <stdint.h>
#include "Stream.h"

#define BUFFER_LENGTH 32

class TwoWire:public Stream {
  private:
    char rxBuffer[BUFFER_LENGTH];
    uint8_t rxBufferIndex;
    uint8_t rxBufferLength;

    uint8_t txAddress;
    char txBuffer[BUFFER_LENGTH];
    uint8_t txBufferIndex;
    uint8_t txBufferLength;

    uint8_t transmitting;	

    //These things are implementation-specific
    //Claim and set up the hardware in the controller to do I2C
    virtual void twi_init(unsigned int freq)=0;
    //Set up the hardware top be a slave with a given address
    //Read given number of bytes from a slave at a given address to the given buffer
    virtual uint8_t twi_readFrom(uint8_t address, char* data, uint8_t length)=0;
    //Write given number of bytes from given buffer to slave at given address
    virtual uint8_t twi_writeTo(uint8_t address, const char* data, uint8_t length, uint8_t wait)=0;
    static const int I2CFREQ=400000;
  public:
    TwoWire();
    //Initialize the I2C peripheral as a master
    virtual void begin(unsigned int freq=I2CFREQ);
    //Start transmitting to a particular slave address
    void beginTransmission(uint8_t address);

    uint8_t endTransmission(void);
    uint8_t requestFrom(uint8_t, uint8_t);
    virtual void write(uint8_t);
    virtual int available(void);
    virtual int read(void);
    virtual int peek(void);
    virtual void flush(void) {};
};

#endif

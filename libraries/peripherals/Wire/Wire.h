#ifndef WIRE_H
#define WIRE_H
#include <stdint.h>
#include "Stream.h"

/** Abstract driver for I2C bus. Concrete implementations of this class can implement
  the bus with bit-banging or with any hardware support the chip provides.
*/
class TwoWire:public Stream {
  public:
    static const int BUFFER_LENGTH=32;
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
    /**Claim and set up the hardware in the controller to do I2C
       @param freq Bus frequency in Hz
    */
    virtual void twi_init(unsigned int freq)=0;
    //Set up the hardware to be a slave with a given address
    //Read given number of bytes from a slave at a given address to the given buffer
    virtual uint8_t twi_readFrom(uint8_t address, char* data, uint8_t length)=0;
    /** Write given number of bytes from given buffer to slave at given address
       @param address register address on the device to start writing to
       @param data pointer to data to send
       @param length number of bytes to send */
    virtual uint8_t twi_writeTo(uint8_t address, const char* data, uint8_t length, uint8_t wait)=0;
    static const int I2CFREQ=400000; ///< Default I2C frequency in Hz
  public:
    TwoWire() {};
    /** Initialize the I2C peripheral as a master */
    virtual void begin(unsigned int freq=I2CFREQ);
    /** Start transmitting to a particular slave address */
    void beginTransmission(uint8_t address);

    uint8_t endTransmission(void);
    uint8_t requestFrom(uint8_t, uint8_t);
    virtual void write(uint8_t);
    virtual int available(void);
    virtual int read(void);
    virtual int peek(void);
    virtual void flush(void) {};
};

//No ambient Wire ports

#endif

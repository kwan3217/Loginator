#ifndef WIRE_H
#define WIRE_H
#include <stdint.h>

#define BUFFER_LENGTH 32

class TwoWire {
  private:
    char rxBuffer[BUFFER_LENGTH];
    uint8_t rxBufferIndex;
    uint8_t rxBufferLength;

    uint8_t txAddress;
    char txBuffer[BUFFER_LENGTH];
    uint8_t txBufferIndex;
    uint8_t txBufferLength;

    uint8_t transmitting;	
    //void onRequestService(void);
    //void onReceiveService(uint8_t*, int);
    static void (*user_onRequest)(void);
    static void (*user_onReceive)(int);
    void (*twi_onSlaveTransmit)(void);
    void (*twi_onSlaveReceive)(uint8_t*, int);

    void twi_attachSlaveRxEvent( void (*)(uint8_t*, int) );
    void twi_attachSlaveTxEvent( void (*)(void) );

    //These things are implementation-specific
    //Claim and set up the hardware in the controller to do I2C
    virtual void twi_init(void)=0;
    //Set up the hardware top be a slave with a given address
    //Read given number of bytes from a slave at a given address to the given buffer
    virtual uint8_t twi_readFrom(uint8_t address, char* data, uint8_t length)=0;
    //Write given number of bytes from given buffer to slave at given address
    virtual uint8_t twi_writeTo(uint8_t address, const char* data, uint8_t length, uint8_t wait)=0;
    //Send I2C stop condition then relinquish bus master
    virtual void twi_stop(void)=0;
    //Release hardware in the controller from doing I2C
    virtual void twi_releaseBus(void)=0;
    //Some slave things we aren't doing yet
    virtual uint8_t twi_transmit(uint8_t*, uint8_t);
    virtual void twi_reply(uint8_t);
    virtual void twi_setAddress(uint8_t address);
  public:
    TwoWire();
    //Initialize the I2C peripheral as a master
    void begin();
    //Initialize the I2C peripheral as a slave
    void begin(uint8_t address);
    void begin(int address);
    //Start transmitting to a particular slave address
    void beginTransmission(uint8_t);
    void beginTransmission(int address);

    uint8_t endTransmission(void);
    uint8_t requestFrom(uint8_t, uint8_t);
    uint8_t requestFrom(int, int);
    void send(uint8_t);
    void send(uint8_t*, uint8_t);
    void send(int);
    void send(char*);
    uint8_t available(void);
    uint8_t receive(void);
    void onReceive( void (*)(int) );
    void onRequest( void (*)(void) );

};

#endif

#ifndef spi_h
#define spi_h

#include <inttypes.h>

class HardSPI {
  public:
    HardSPI() {};
//Setup the SPI1 port, including claiming the pins. May be called repeatedly, whenever
//any of these input parameters change
//input
//  freq - Approximate bus frequency in Hz
//  CPOL - Clock Polarity. 0 indicates clock is low when idle, 1 conversely
//  CPHA - Clock Phase. 0 indicates data is sampled on rising edge, 1 conversely
    virtual void begin(unsigned int freq, unsigned char CPOL, unsigned char CPHA)=0;

//Send a string to the slave, block until finished
    void tx_block(int p0, const char* data, int count) {select_cs(p0);tx_block(data,count);deselect_cs(p0);};
//Send, but presume that cs is controlled externally
    virtual void tx_block(const char* data, int count)=0;
    virtual void send_byte(uint8_t b)=0;
//Receive a string from the slave, block until finished
    void rx_block(int p0, uint8_t tx, char* data, int count) {select_cs(p0);rx_block(tx,data,count);deselect_cs(p0);};
//Receive, but presume that cs is controlled externally
    virtual void rx_block(uint8_t tx, char* data, int count)=0;
    virtual uint8_t rec_byte()=0;
//Release the SPI1 port, setting the used pins back to GPIO input
    virtual void stop(void)=0;
    virtual void setfreq(unsigned int freq)=0;
    void claim_cs(int p0);
    void release_cs(int p0);
    void select_cs(int p0);
    void deselect_cs(int p0);
};

class HardSPI1:public HardSPI {
  public:
    HardSPI1() {};
    virtual void begin(unsigned int freq, unsigned char CPOL, unsigned char CPHA);
    virtual void setfreq(unsigned int freq);
    virtual void tx_block(const char* data, int count);
    virtual void send_byte(uint8_t b);
    virtual void rx_block(uint8_t tx, char* data, int count);
    virtual uint8_t rec_byte();
    virtual void stop(void);
};

class HardSPI0:public HardSPI {
  public:
    HardSPI0() {};
    virtual void begin(unsigned int freq, unsigned char CPOL, unsigned char CPHA);
    virtual void setfreq(unsigned int freq);
    virtual void tx_block(const char* data, int count);
    virtual void send_byte(uint8_t b);
    virtual void rx_block(uint8_t tx, char* data, int count);
    virtual uint8_t rec_byte();
    virtual void stop(void);
};

//Ambient SPI objects
extern HardSPI0& SPI;
extern HardSPI1& SPI1;
extern HardSpi*  SPIA[];

#endif

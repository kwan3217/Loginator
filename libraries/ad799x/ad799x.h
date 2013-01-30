#ifndef ad799x_H
#define ad799x_H

#include <inttypes.h>
#include "Wire.h"
#include "Serial.h"
#include "packet.h"

class AD799x {
  private:
    TwoWire &port;
    bool read(uint8_t *data, int num);
    bool write(uint8_t data);
    unsigned char ADDRESS;  // I2C address of AD799x
    int nChannels;
    uint8_t channels;
  public:
    AD799x(TwoWire &Lport, int a0=0);
    bool begin(uint8_t Lchannels, bool vref=false);
    bool read(uint16_t ch[]);
    /**
     vref - if set, ch3 is used as a voltage reference. 
            if clear, ch3 is a normal input channel and vdd is reference
     */
    bool writeConfig(uint8_t Lchannels, bool vref=false);
};

#endif

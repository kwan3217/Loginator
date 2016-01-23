#ifndef ad799x_H
#define ad799x_H

#include <inttypes.h>
#include "Wire.h"
#include "Serial.h"
#include "packet.h"

/* The Analog Devices AD7991/5/9 are 12/10/8 bit Analog-digital converters
   which are read and controlled via the I2C bus. Other than the significance                           
   of the lower bits of the readout (the 10/8 bit parts read out as 12 bit with
   the lower bits zero) the parts are used interchangably. It is not even         
   possible to identify whether you have a '7991/5/9 over the bus.

   The AD799x is unlike most I2C parts in that it does not use a register           
   interface. You can write (but not read) configuration to the device, but the                 
   normal operation is to command a read, which immediately takes a measurement
   and reports it to the bus.

   The device has four input channels, and can be configured to read any  
   combination of them in turn. It can also be configured to use one of the
   inputs as a voltage reference.

   The programming model then is this: You configure the part once, then
   each time you want to sample, you call read(), which sends the I2C commands
   to read enough bytes from the bus to trigger a read on the selected channels.
   The read blocks until the conversion and I2C read is complete. Even at 1MHz,
   the conversion and read takes less than 2 bit times, done in parallel with 
   the read, and I have a sneaking suspicion that this is the main reason for 
   the two zero bits in the raw readout, and the reason there isn't a 14-bit
   part in this form factor. Because the sample and conversion happen in parallel
   with the read, no special handling is needed.

   The read is raw - the bits the device produces
   are recorded directly in the readout parameter. This means three things:
   1) Only enough array slots to complete the read are used. If you asked for 
   two channels, only the lowest two array slots are used, regardless of which
   two channels they are
   2) The outputs contain two zero bits in the MSB to fill 16 bits, then two
   channel ID bits, then 12 data bits
   3) The part is big-endian, so interpretaion on an ARM processor will require
   an endian-flip. However, no endian flip is required if the data is just going
   to be written to a CCSDS packet.

   The AD799x comes with a hard-coded I2C address, but comes in two flavors, one
   with its lowest address bit cleared and one with it set. The part therefore
   has a 7-bit address of either 0x28 or 0x29.  
   */

class AD799x {
  private:
    TwoWire *port;
    bool read(uint8_t *data, int num);
    bool write(uint8_t data);
    uint8_t ADDRESS;  // I2C address of AD799x
    static const char addr_msb=0x28; //ORed with low bit of a0 to get actual address
    uint8_t nChannels;
    uint8_t channels;
    static int i_hwDesc;
  public:
    /** Init part directly
      @param Lport I2C port to use with this part
      @param a0 least significant bit of part address */
    AD799x(TwoWire &Lport, int a0=0);
    /** Init part based on hardware description */
    AD799x();
    bool begin(uint8_t Lchannels, bool vref=false);
    bool read(uint16_t ch[]);
    void format(uint16_t ch[]);
    uint8_t getAddress() {return ADDRESS;};
    uint8_t getnChannels() {return nChannels;};
    /**
     vref - if set, ch3 is used as a voltage reference. 
            if clear, ch3 is a normal input channel and vdd is reference
     */
    bool writeConfig(uint8_t Lchannels, bool vref=false);
};

#endif

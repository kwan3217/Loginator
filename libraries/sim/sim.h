#ifndef sim_h
#define sim_h

#include <inttypes.h>
#include <stdio.h>
#include <vector>

//Macros which define a register with zero levels of addressing, IE only
//one instance on the part.
#define ro0(junk,name) \
protected: uint32_t name; \
public: virtual uint32_t read_##name() {fprintf(stdout,#name " read, value=0x%08x (%d)\n",name,name);return name;}
#define wo0(junk,name) \
protected: uint32_t name; \
public: virtual void write_##name(uint32_t value) {fprintf(stdout,#name " written, value=0x%08x (%d)\n",value,value);name=value;}
#define rw0(junk,name) \
protected: uint32_t name; \
public: virtual uint32_t read_##name() {fprintf(stdout,#name " read, value=0x%08x (%d)\n",name,name);return name;}; \
virtual void write_##name(uint32_t value) {fprintf(stdout,#name " written, value=0x%08x (%d)\n",value,value);name=value;}
 
//Macros which define a register with one levels of addressing, IE multiple 
//instances on the part, but no channels in each instance. This includes such
//things as GPIO and I2C
#define ro1(junk,name) \
protected: uint32_t name[16]; \
public: virtual uint32_t read_##name(int instance) {fprintf(stdout,#name "[%d] read, value=0x%08x (%d)\n",instance,name[instance],name[instance]);return name[instance];}
#define wo1(junk,name) \
protected: uint32_t name[16]; \
public: virtual void write_##name(int instance, uint32_t value) {fprintf(stdout,#name "[%d] written, value=0x%08x (%d)\n",instance,value,value);name[instance]=value;}
#define rw1(junk,name) \
protected: uint32_t name[16]; \
public: virtual uint32_t read_##name(int instance) {fprintf(stdout,#name "[%d] read, value=0x%08x (%d)\n",instance,name[instance],name[instance]);return name[instance];};\
virtual void write_##name(int instance, uint32_t value) {fprintf(stdout,#name "[%d] written, value=0x%08x (%d)\n",instance,value,value);name[instance]=value;}
 
//Macros which define a register with two levels of addressing, IE multiple 
//instances on the part, and multiple channels in each instance. This includes
//such things as timers and PWMs
#define ro2(junk,name) \
protected: uint32_t name[16][16]; \
public: virtual uint32_t read_##name(int instance, int channel) {return name[instance][channel];}
#define wo2(junk,name) \
protected: uint32_t name[16][16]; \
public: virtual void write_##name(int instance, int channel, uint32_t value) {name[instance][channel=value;}
#define rw2(junk,name) \
protected: uint32_t name[16][16]; \
public: virtual uint32_t read_##name(int instance, int channel) {return name[instance][channel];};\
virtual void write_##name(int instance, int channel, uint32_t value) {name[instance][channel]=value;}
 
/** Class to back up the simulated registers
Make a subclass to handle details of how your simulated system interacts with
its environment.

Don't worry about efficiency (inline etc) because this will only be run on a 
big beefy host, not the actual embedded system.
*/

class SimGpioListener {
public:
  /** Called when the pin is in output mode and is written to
   @param port First number in PX.YY GPIO numbering system
   @param pin Second number in PX.YY GPIO numbering system (not package pin number)
   @param value value written to pin, either 1 or 0
   */
  virtual void pinOut(int port, int pin, int value)=0;
  /** Called when the pin is in input mode and is read
   @param port First number in PX.YY GPIO numbering system
   @param pin Second number in PX.YY GPIO numbering system (not package pin number)
   @return value read from pin, either 1 or 0
   */
  virtual int pinIn(int port, int pin)=0;
  /** Called when the pin in/out mode is changed. In principle, this shouldn't be
   observable, but the SD protocol requires this. 
   @param port First number in PX.YY GPIO numbering system
   @param pin Second number in PX.YY GPIO numbering system (not package pin number)
   @param out true if the pin is becoming an output, false otherwise.
   */
  virtual void pinMode(int port, int pin, bool out)=0;
};

class SimSubGpio {
public:
  #include "gpio_registers.inc"
};

class SimGpio: public SimSubGpio {
private:
  std::vector<SimGpioListener*> listeners;
  std::vector<std::vector<int>> pins0;
  std::vector<std::vector<int>> pins1;
  bool listenerCares(int index, int port, int pin);
public:
  /** Add a listener to the GPIO
   @param object which will be listening to the GPIO
   @param pins0 pins on GPIO port 0 that this object is interested in.
   @param pins1 pins on GPIO port 1 that this object is interested in.
   */
  void addListener(SimGpioListener& listener, std::vector<int> Lpins0={}, std::vector<int> Lpins1={});
  /** Interface for external devices to drive a particular pin. If the 
   pin is in input mode, IOPIN will be set as appropriate so that the firmware
   can see it. If the pin is in output mode, a warning will be printed as this
   can cause the internal and external drivers to fight for the pin.
   This will never activate the listeners, as presumably it is this listener
   that set the pin in the first place.
   @param port First number in PX.YY GPIO numbering system
   @param pin Second number in PX.YY GPIO numbering system (not package pin number)
   @param value value to drive the pin at, either 0 or 1
  */
  void drivePin(int port, int pin, int value);
  virtual uint32_t read_IOPIN(int port) override;
  virtual void write_IOPIN(int port, uint32_t value) override;
  virtual void write_IOSET(int port, uint32_t value) override;
  virtual void write_IOCLR(int port, uint32_t value) override;
  virtual void write_IODIR(int port, uint32_t value) override;
};

class SimSubUart {
public:
  #include "uart_registers.inc"
};

class SimUart: public SimSubUart {
private:
  int FDR[2];
  int dataBits[2],stopBits[2],parityMode[2];
  bool DLAB[2],parityOn[2];
  int baud(int Lport);
public:
  virtual void write_UTHR(int Lport, uint32_t write) override;
  virtual void write_UDLL(int Lport, uint32_t write) override;
  virtual void write_UDLM(int Lport, uint32_t write) override;
  virtual uint32_t read_ULCR(int Lport) override;
  virtual void write_ULCR(int Lport, uint32_t write) override;
};

class SimI2c {
public:
  #include "i2c_registers.inc"
};

class SimSubSpi {
public:
  #include "spi_registers.inc"
};

class SimSpi:public SimSubSpi {
private:
  int BitEnable,CPHA,CPOL,MSTR,LSBF,SPIE,Bits;
  static const int ABRT=0;
  static const int MODF=0;
  static const int ROVR=0;
  static const int WCOL=0;
  int SPIF;
  bool SPIF_read;
public:
  virtual void write_S0SPCR(uint32_t value) override;
  virtual uint32_t read_S0SPCR() override;
  virtual void write_S0SPCCR(uint32_t value) override;
  virtual uint32_t read_S0SPSR() override;
  virtual void write_S0SPDR(uint32_t value) override;
  virtual uint32_t read_S0SPDR() override;
};

class SimSsp {
public:
  #include "ssp_registers.inc"
};

class SimTime {
public:
  #include "timer_registers.inc"
};

class SimPwm {
public:
  #include "pwm_registers.inc"
};

class SimRtc {
public:
  #include "rtc_registers.inc"
};

class SimVic {
public:
  #include "vic_registers.inc"
};

class SimSubScb {
public:
  #include "scb_registers.inc"
};

class SimScb:public SimSubScb {
private:
  int PLLE,PLLC,MSEL,PSEL;
  const int PLOCK=1;
public:
  virtual uint32_t read_PLLSTAT(int port);
  virtual void write_PLLCFG(int port, uint32_t value);
};

class SimAdc {
public:
  #include "adc_registers.inc"
};

class SimPeripherals {
public:
  SimGpio& gpio;
  SimUart& uart; //Both UARTs
  SimI2c& i2c;
  SimSpi& spi;
  SimSsp& ssp;
  SimTime& time;
  SimRtc& rtc;
  SimPwm& pwm;
  SimAdc& adc;
  SimScb scb; //If it's not overridden, we can use it directly here
  SimVic vic;
  SimPeripherals(SimGpio& Lgpio, 
                 SimUart& Luart,
                 SimI2c& Li2c,
                 SimSpi& Lspi,
                 SimSsp& Lssp,
                 SimTime& Ltime,
                 SimRtc& Lrtc,
                 SimPwm& Lpwm,
                 SimAdc& Ladc
  ):gpio(Lgpio),uart(Luart),i2c(Li2c),spi(Lspi),ssp(Lssp),time(Ltime),rtc(Lrtc),pwm(Lpwm),adc(Ladc) {};
};

extern SimPeripherals peripherals;

#undef ro0
#undef rw0
#undef wo0
#undef ro1
#undef rw1
#undef wo1
#undef ro2
#undef rw2
#undef wo2


#endif

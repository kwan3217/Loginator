#ifndef sim_h
#define sim_h

#include <inttypes.h>
#include <stdio.h>
#include <vector>
#include <unordered_map>

enum DebugStream {SIMSPI,SIMSD,SIMREG,SIMSD_TRANSFER,SIMSCB,SIMUART,SIMSPI_TRANSFER,SIMI2C,SIMI2C_TRANSFER,SIMHMC,SIMSSP,SIMSSP_TRANSFER,SIMGYRO,PLAYBACK};
void dprintf(DebugStream stream, const char* pattern, ...);
void dnprintf(const char* pattern, ...);

//Macros which define a register with zero levels of addressing, IE only
//one instance on the part.
#define ro0(junk,name,addr) \
protected: uint32_t name; \
public: virtual uint32_t read_##name() {dprintf(SIMREG,#name " read, value=0x%08x (%d)\n",name,name);return name;}
#define wo0(junk,name,addr) \
protected: uint32_t name; \
public: virtual void write_##name(uint32_t value) {dprintf(SIMREG,#name " written, value=0x%08x (%d)\n",value,value);name=value;}
#define rw0(junk,name,addr) \
protected: uint32_t name; \
public: virtual uint32_t read_##name() {dprintf(SIMREG,#name " read, value=0x%08x (%d)\n",name,name);return name;}; \
virtual void write_##name(uint32_t value) {dprintf(SIMREG,#name " written, value=0x%08x (%d)\n",value,value);name=value;}
 
//Macros which define a register with one levels of addressing, IE multiple 
//instances on the part, but no channels in each instance. This includes such
//things as GPIO and I2C
#define ro1(junk,name,N,addr) \
protected: uint32_t name[N]; \
public: virtual uint32_t read_##name(int i) {dprintf(SIMREG,#name "[%d] read, value=0x%08x (%d)\n",i,name[i],name[i]);return name[i];}
#define wo1(junk,name,N,addr) \
protected: uint32_t name[N]; \
public: virtual void write_##name(int i, uint32_t value) {dprintf(SIMREG,#name "[%d] written, value=0x%08x (%d)\n",i,value,value);name[i]=value;}
#define rw1(junk,name,N,addr) \
protected: uint32_t name[N]; \
public: virtual uint32_t read_##name(int i) {dprintf(SIMREG,#name "[%d] read, value=0x%08x (%d)\n",i,name[i],name[i]);return name[i];};\
virtual void write_##name(int i, uint32_t value) {dprintf(SIMREG,#name "[%d] written, value=0x%08x (%d)\n",i,value,value);name[i]=value;}
 
//Macros which define a register with two levels of addressing, IE multiple 
//instances on the part, and multiple channels in each instance. This includes
//such things as timers and PWMs
#define ro2(junk,name,M,N,addr) \
protected: uint32_t name[M][N]; \
public: virtual uint32_t read_##name(int i, int j) {dprintf(SIMREG,#name "[%d][%d] read, value=0x%08x (%d)\n",i,j,name[i][j],name[i][j]);return name[i][j];}
#define wo2(junk,name,M,N,addr) \
protected: uint32_t name[M][N]; \
public: virtual void write_##name(int i, int j, uint32_t value) {dprintf(SIMREG,#name "[%d][%d] written, value=0x%08x (%d)\n",i,j,value,value);name[i][j]=value;}
#define rw2(junk,name,M,N,addr) \
protected: uint32_t name[M][N]; \
public: virtual uint32_t read_##name(int i, int j) {dprintf(SIMREG,#name "[%d][%d] read, value=0x%08x (%d)\n",i,j,name[i][j],name[i][j]);return name[i][j];};\
virtual void write_##name(int i, int j, uint32_t value) {dprintf(SIMREG,#name "[%d][%d] written, value=0x%08x (%d)\n",i,j,value,value);name[i][j]=value;}
 
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

class SimFio {
public:
  #include "fio_registers.inc"
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

class SimSubI2c {
public:
  #include "i2c_registers.inc"
};

class SimI2cSlave {
public:
  virtual void start()=0;
  virtual void stop()=0;
  virtual void repeatStart()=0;
  virtual uint8_t readByte(bool ack)=0;
  virtual void writeByte(uint8_t write, bool& ack)=0;
};

class SimI2c: public SimSubI2c {
private:
  std::unordered_map<int,SimI2cSlave*> slaves;
  SimI2cSlave* slave;
public:
  void addSlave(int addr, SimI2cSlave& listener);
  SimI2c() {for(int i=0;i<2;i++) I2CSTAT[i]=0xF8;};
  int AA  (int port) {return (I2CCONSET[port]>>2) & 0x01;};
  int SI  (int port) {return (I2CCONSET[port]>>3) & 0x01;};
  int STO (int port) {return (I2CCONSET[port]>>4) & 0x01;};
  int STA (int port) {return (I2CCONSET[port]>>5) & 0x01;};
  int I2EN(int port) {return (I2CCONSET[port]>>6) & 0x01;};
  void AA  (int port, int val) {I2CCONSET[port]=(I2CCONSET[port] & ~(1<<2)) | ((val & 0x01) << 2);};
  void SI  (int port, int val) {I2CCONSET[port]=(I2CCONSET[port] & ~(1<<3)) | ((val & 0x01) << 3);};
  void STO (int port, int val) {I2CCONSET[port]=(I2CCONSET[port] & ~(1<<4)) | ((val & 0x01) << 4);};
  void STA (int port, int val) {I2CCONSET[port]=(I2CCONSET[port] & ~(1<<5)) | ((val & 0x01) << 5);};
  void I2EN(int port, int val) {I2CCONSET[port]=(I2CCONSET[port] & ~(1<<6)) | ((val & 0x01) << 6);};
  void runStateMachine(int port);
  void printConStatus(int port);
  virtual void write_I2CCONSET(int port, uint32_t write) override;
  virtual void write_I2CCONCLR(int port, uint32_t write) override;
};

class SimSubId {
public:
  #include "id_registers.inc"
};

class SimId: public SimSubId {
public:
  virtual uint32_t read_HW_TYPE()   override { return HW_TYPE_SIMULATOR; };
  virtual uint32_t read_HW_SERIAL() override { return 1; };
};

class SimSubSpi {
public:
  #include "spi_registers.inc"
};

class SimSpiSlave {
public:
  virtual void csOut(int value)=0;
  virtual int csIn()=0;
  virtual void csMode(bool out)=0;
  virtual uint8_t transfer(uint8_t value)=0;
};

class SimSpi:public SimSubSpi, public SimGpioListener {
private:
  int BitEnable,CPHA,CPOL,MSTR,LSBF,SPIE,Bits;
  static const int ABRT=0;
  static const int MODF=0;
  static const int ROVR=0;
  static const int WCOL=0;
  int SPIF;
  bool SPIF_read;
  SimSpiSlave* slaves[64];
  bool selected[64];
public:
  void addSlave(int port, int pin, SimSpiSlave& Lslave);
  virtual void pinOut(int port, int pin, int value) override;
  virtual int pinIn(int port, int pin) override;
  virtual void pinMode(int port, int pin, bool out) override;
  virtual void write_S0SPCR(uint32_t value) override;
  virtual uint32_t read_S0SPCR() override;
  virtual void write_S0SPCCR(uint32_t value) override;
  /**The master triggers an SPI transfer by writing to the data register.
     On real hardware, the transfer takes a certain amount of time, after
     which the SPIF bit in the status register is set. At this point, a read
     of the data register gets the data just received by the master during this
     transfer, and the SPIF bit is cleared. In the simulation, the transfer
     takes no time. The SPIF bit is always set and writing to the data register
     should trigger the calculation of the byte to be received by the host.
     *That* data, not the input value argument, should be written to the S0SPDR
     internal variable. */
  virtual uint32_t read_S0SPSR() override;
  virtual void write_S0SPDR(uint32_t value) override;
  virtual uint32_t read_S0SPDR() override;
};

class SimSubSsp {
public:
  #include "ssp_registers.inc"
};

class SimSsp:public SimSubSsp, public SimGpioListener {
private:
  int DSS,FRF,CPOL,CPHA,SCR,LBM,SSE,MS,SOD;
  static const int TFE=1;
  static const int TNF=1;
  static const int RNE=1;
  static const int RFF=0;
  static const int BSY=0;
  bool SPIF_read;
  SimSpiSlave* slaves[64];
  bool selected[64];
  static const int PCLK=60'000'000;
  int Hz() {int result=-1; if((SSPCPSR*(SCR+1))>0) result=PCLK/(SSPCPSR*(SCR+1));return result;};

public:
  void addSlave(int port, int pin, SimSpiSlave& Lslave);
  virtual void pinOut(int port, int pin, int value) override;
  virtual int pinIn(int port, int pin) override;
  virtual void pinMode(int port, int pin, bool out) override;
  virtual void write_SSPCR0(uint32_t value) override;
  virtual uint32_t read_SSPCR0() override;
  virtual void write_SSPCR1(uint32_t value) override;
  virtual uint32_t read_SSPCR1() override;
  virtual void write_SSPCPSR(uint32_t value) override;
  /**The master triggers an SPI transfer by writing to the data register.
     On real hardware, the transfer takes a certain amount of time, after
     which the SPIF bit in the status register is set. At this point, a read
     of the data register gets the data just received by the master during this
     transfer, and the SPIF bit is cleared. In the simulation, the transfer
     takes no time. The SPIF bit is always set and writing to the data register
     should trigger the calculation of the byte to be received by the host.
     *That* data, not the input value argument, should be written to the S0SPDR
     internal variable. */
  virtual uint32_t read_SSPSR() override;
  virtual void write_SSPDR(uint32_t value) override;
  virtual uint32_t read_SSPDR() override;
};

class SimWdog {
public:
  #include "wdog_registers.inc"
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
  virtual uint32_t read_PLLSTAT(int port) override;
  virtual void write_PLLCFG(int port, uint32_t value) override;
};

class SimAdc {
public:
  #include "adc_registers.inc"
};

class SimUsb {
public:
  #include "usb_registers.inc"
};

class SimPeripherals {
public:
  SimGpio& gpio;
  SimUart& uart; //Both UARTs
  SimTime& time;
  SimRtc& rtc;
  SimPwm& pwm;
  SimAdc& adc;
  SimScb scb; //If it's not overridden, we can use it directly here
  SimI2c i2c;
  SimSpi spi;
  SimSsp ssp;
  SimVic vic;
  SimId id;
  SimFio fio;
  SimUsb usb;
  SimWdog wdog;
  SimPeripherals(SimGpio& Lgpio, 
                 SimUart& Luart,
                 SimTime& Ltime,
                 SimRtc& Lrtc,
                 SimPwm& Lpwm,
                 SimAdc& Ladc
  ):gpio(Lgpio),uart(Luart),time(Ltime),rtc(Lrtc),pwm(Lpwm),adc(Ladc) {};
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

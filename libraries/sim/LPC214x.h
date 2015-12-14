#ifndef LPC214x_h 
#define LPC214x_h

#include "sim.h"

/**
   \file LPC214x.h
   \brief Header file for SIMULATED hardware control. 
   
   This matches the syntax of the embedded LPC214x.h, while having radically
   different semantics. The embedded version declares a bunch of static
   inline functions which return values (for read-only registers) or references
   to values (for read/write or write-only registers) which resolve to reading
   or writing a constant address. This code instead defines a static function
   (for read-only) or a class and object instance (for read/write and write-only
   registers). The read-only version closely matches the interface for the 
   embedded version, but instead of returning a value at a particular memory 
   address, it calls the simulation support code to find the value. The 
   write-only version is a full-blown class, with an overridden parenthesis
   operator which just returns the object, and a overridden assignment operator
   which executes the simulation support code to write the value. This means
   that while code to call this is exactly the same as the code to call the 
   embedded version, this code instead has the opportunity to call code to react
   to the assignment. The read/write version also overrides the int typecast,
   which gives the opportunity to call code on read. This version also overrides
   several of the assignment arithmetic operators also, since some simulation 
   code uses things like |= to do read-modify-write.  
*/

#define ro0(part,name) static inline uint32_t name() {return peripherals.part.read_##name();} 
#define rw0(part,name) class name##_class { \
  public:                              \
    name##_class& operator()() {return *this;};        \
    operator int() {return peripherals.part.read_##name();};                    \
    void operator=(uint32_t write) {peripherals.part.write_##name(write);};         \
    name##_class& operator&=(int write) {(*this)=(*this) & write;return *this;}; \
    name##_class& operator|=(int write) {(*this)=(*this) | write;return *this;}; \
    name##_class& operator-=(int write) {(*this)=(*this) - write;return *this;}; \
    name##_class& operator+=(int write) {(*this)=(*this) + write;return *this;}; \
    name##_class& operator*=(int write) {(*this)=(*this) * write;return *this;}; \
    name##_class& operator/=(int write) {(*this)=(*this) / write;return *this;}; \
    name##_class& operator++() {(*this)=(*this) +1;return *this;}; \
    uint32_t      operator++(int) {uint32_t tmp=(*this);(*this)=(*this) +1;return tmp;}; \
};                                     \
extern name##_class name
#define wo0(part,name) class name##_class { \
  public:                              \
    name##_class& operator()() {return *this;};        \
    void operator=(uint32_t write) {peripherals.part.write_##name(write);};         \
};                                     \
extern name##_class name
#define ro1(part,name) static inline uint32_t name(int port) {return peripherals.part.read_##name(port);}
#define rw1(part,name) class name##_class { \
  private:                             \
    int i_port;                          \
  public:                              \
    name##_class& operator()(int Lport) {i_port=Lport;return *this;};        \
    operator int() {return peripherals.part.read_##name(i_port);};                    \
    void operator=(uint32_t write) {peripherals.part.write_##name(i_port,write);};         \
    name##_class& operator&=(int write) {(*this)=(*this) & write;return *this;}; \
    name##_class& operator|=(int write) {(*this)=(*this) | write;return *this;}; \
    name##_class& operator-=(int write) {(*this)=(*this) - write;return *this;}; \
    name##_class& operator+=(int write) {(*this)=(*this) + write;return *this;}; \
    name##_class& operator*=(int write) {(*this)=(*this) * write;return *this;}; \
    name##_class& operator/=(int write) {(*this)=(*this) / write;return *this;}; \
    name##_class& operator++() {(*this)=(*this) +1;return *this;}; \
    uint32_t      operator++(int) {uint32_t tmp=(*this);(*this)=(*this) +1;return tmp;}; \
};                                     \
extern name##_class name
#define wo1(part,name) class name##_class { \
  private:                             \
    int i_port;                          \
  public:                              \
    name##_class& operator()(int Lport) {i_port=Lport;return *this;};        \
    void operator=(uint32_t write) {peripherals.part.write_##name(i_port,write);};         \
};                                     \
extern name##_class name
#define ro2(part,name) static inline uint32_t name(int port, int channel) {return peripherals.part.read_##name(port,channel);}
#define rw2(part,name) class name##_class { \
  private:                             \
    int i_port,i_channel;                          \
  public:                              \
    name##_class& operator()(int Lport,int Lchannel) {i_port=Lport;i_channel=Lchannel;return *this;};        \
    operator int() {return peripherals.part.read_##name(i_port,i_channel);};                    \
    void operator=(uint32_t write) {peripherals.part.write_##name(i_port,i_channel,write);};         \
    name##_class& operator&=(int write) {(*this)=(*this) & write;return *this;}; \
    name##_class& operator|=(int write) {(*this)=(*this) | write;return *this;}; \
    name##_class& operator-=(int write) {(*this)=(*this) - write;return *this;}; \
    name##_class& operator+=(int write) {(*this)=(*this) + write;return *this;}; \
    name##_class& operator*=(int write) {(*this)=(*this) * write;return *this;}; \
    name##_class& operator/=(int write) {(*this)=(*this) / write;return *this;}; \
    name##_class& operator++() {(*this)=(*this) +1;return *this;}; \
    uint32_t      operator++(int) {uint32_t tmp=(*this);(*this)=(*this) +1;return tmp;}; \
};                                     \
extern name##_class name
#define wo2(part,name) class name##_class { \
  private:                             \
    int i_port,i_channel;                          \
  public:                              \
    name##_class& operator()(int Lport,int Lchannel) {i_port=Lport;i_channel=Lchannel;return *this;};        \
    void operator=(uint32_t write) {peripherals.part.write_##name(i_port,i_channel,write);};         \
};                                     \
extern name##_class name

static inline unsigned int HW_TYPE() {return 3;}
static inline unsigned int HW_SERIAL() {return 1;}

//Serial UART
#include "uart_registers.inc"

//I2C (Wire)
#include "i2c_registers.inc"

//SPI (SPI0)
#include "spi_registers.inc"

//SSP (SPI1)
#include "ssp_registers.inc"

//Timers
#include "timer_registers.inc"

//PWM
#include "pwm_registers.inc"

//RTC
#include "rtc_registers.inc"

//GPIO
#include "gpio_registers.inc"

//VIC
#include "vic_registers.inc"

//System Control Block
#include "scb_registers.inc"

//Analog/Digital Converter
#include "adc_registers.inc"

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

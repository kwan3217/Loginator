#include "registers.h"

#define ro0(part,name    ,addr)
#define rw0(part,name    ,addr) name##_class name
#define wo0(part,name    ,addr) name##_class name
#define ro1(part,name,N  ,addr)
#define rw1(part,name,N  ,addr) name##_class name
#define wo1(part,name,N  ,addr) name##_class name
#define ro2(part,name,M,N,addr)
#define rw2(part,name,M,N,addr) name##_class name
#define wo2(part,name,M,N,addr) name##_class name

#include "uart_registers.inc"
#include "id_registers.inc"
#include "i2c_registers.inc"
#include "spi_registers.inc"
#include "ssp_registers.inc"
#include "timer_registers.inc"
#include "rtc_registers.inc"
#include "pwm_registers.inc"
#include "gpio_registers.inc"
#include "vic_registers.inc"
#include "scb_registers.inc"
#include "adc_registers.inc"

//PLL feed for simulation code. The PLL has no real effect, so this is a no-op.
void feed(int channel) {}

#undef ro0
#undef rw0
#undef wo0
#undef ro1
#undef rw1
#undef wo1
#undef ro2
#undef rw2
#undef wo2



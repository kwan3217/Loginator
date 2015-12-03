#include "StateTwoWire.h"
#include "gpio.h"
#include "Time.h"
#include "irq.h"
#include "Serial.h"

//Define this to use interrupts - really doesn't gain 
#undef I2C_IRQ

//StateTwoWire: Use LPC214x hardware I2C port to implement TwoWire object
StateTwoWire::StateTwoWire(int Lport):port(Lport) {

}

void StateTwoWire::twi_init(unsigned int freq) {}
uint8_t StateTwoWire::twi_readFrom(uint8_t Laddress, char* Ldata, uint8_t Llength) {return 0;}
uint8_t StateTwoWire::twi_writeTo(uint8_t Laddress, const char* Ldata, uint8_t Llength, uint8_t wait) {return 0;}


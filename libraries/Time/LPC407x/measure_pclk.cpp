#include "registers.cpp"
#include "Time.cpp"

/** Measure the PCLK and CCLK rate. This is done by knowing the oscillator
    frequency and looking at the PLL registers for CCLK and the peripheral bus
    clock divider register for PCLK. This is idempotent so it can (should) be
    called in all the setup or begin routines which need PCLK, like Serial. */
void measurePCLK(void) {
  CCLK=FOSC;//*((PLLSTAT(0) & 0x1F)+1);
  PCLK=CCLK/(PCLKSEL() & 0x03);
}


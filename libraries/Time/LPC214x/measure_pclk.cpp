/** Measure the PCLK and CCLK rate. This is done by knowing the oscillator
    frequency and looking at the PLL registers for CCLK and the peripheral bus
    clock divider register for PCLK. This is idempotent so it can (should) be
    called in all the setup or begin routines which need PCLK, like Serial. */
void measurePCLK(void) {
  CCLK=FOSC*((PLLSTAT(0) & 0x1F)+1);
  switch (VPBDIV() & 0x03) {
    case 0:
      PCLK=CCLK/4;
      break;
    case 1:
      PCLK=CCLK;
      break;
    case 2:
      PCLK=CCLK/2;
      break;
    case 3:
      break;
  }
}


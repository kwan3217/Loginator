#include "pll.h"
#include "registers.h"

void setup_pll(unsigned int channel, unsigned int M) {
  //Figure out N, exponent for PLL divider value, P=2^N. Intermediate frequency will be
  //FOSC*M*2*P=FOSC*M*2*2^N, and must be between 156MHz and 320MHz. This selects the lowest
  //N which satisfies the frequency constraint
  unsigned int N=0;
  while(FOSC*M*2*(1<<N)<156'000'000) N++;
  // Set Multiplier and Divider values
  PLLCFG(channel)=(M-1)|(N<<5);
  feed(channel);

  // Enable the PLL */
  PLLCON(channel)=0x1;
  feed(channel);

  // Wait for the PLL to lock to set frequency
  while(!(PLLSTAT(channel) & (1 << 10))) ;

  // Connect the PLL as the clock source
  PLLCON(channel)=0x3;
  feed(channel);

}


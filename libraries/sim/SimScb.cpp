#include "sim.h"

uint32_t SimScb::read_PLLSTAT(int port) {
    PLLSTAT[port]=((MSEL  & ((1<<5)-1)) << 0) |
                  ((PSEL  & ((1<<2)-1)) << 5) |
                  ((PLLE  & ((1<<1)-1)) << 8) |
                  ((PLLC  & ((1<<1)-1)) << 9) |
                  ((PLOCK & ((1<<1)-1)) <<10) ;
  dprintf(SIMSCB,"PLLSTAT[%d] read, 0x%04x (%d), MSEL=%d, PSEL=%d, PLLE=%d, PLLC=%d, PLOCK=%d\n",
    port,PLLSTAT[port],PLLSTAT[port],MSEL,PSEL,PLLE,PLLC,PLOCK);
  return PLLSTAT[port];
}

void SimScb::write_PLLCFG(int port, uint32_t value) {
  SimSubScb::write_PLLCFG(port,value);
  MSEL= (value>>0) & ((1<<4)-1);
  PSEL= (value>>5) & ((1<<2)-1);
  dprintf(SIMSCB,"PLLCFG[%d] written, 0x%04x (%d), MSEL=%d, PSEL=%d\n",
    port,PLLSTAT[port],PLLSTAT[port],MSEL,PSEL);
}

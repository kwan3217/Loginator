#include "sim.h"
#include <stdio.h>

void SimSsp::pinOut (int port, int pin, int value) {
  int addr=((port & 0x1)<<5) | (pin & 0x1F);
  selected[addr]=(value==0);
  slaves[addr]->csOut(value);
}
int  SimSsp::pinIn  (int port, int pin) {
  int addr=((port & 0x1)<<5) | (pin & 0x1F);
  return slaves[addr]->csIn();
}
void SimSsp::pinMode(int port, int pin, bool out) {
  int addr=((port & 0x1)<<5) | (pin & 0x1F);
  slaves[addr]->csMode(out);
}

void SimSsp::addSlave(int port, int pin, SimSpiSlave& Lslave) {
  int addr=((port & 0x0F)<<5) | (pin & 0x1F);
  slaves[addr]=&Lslave;
}

void SimSsp::write_SSPCR0(uint32_t value) {
  DSS = ((value>>0) & ((1<<4)-1))+1; //DSS then is the actual number of bits
  FRF = (value>>4) & ((1<<2)-1);
  CPOL= (value>>6) & ((1<<1)-1);
  CPHA= (value>>7) & ((1<<1)-1);
  SCR = (value>>8) & ((1<<8)-1);
  dprintf(SIMSSP,"SSPCR0 written, 0x%04x (%d), DSS=%d, FRF=%d, CPOL=%d, CPHA=%d, SCR=%d, actual clock rate=%dHz\n",
    value,value,DSS,FRF,CPOL,CPHA,SCR,Hz());
}

uint32_t SimSsp::read_SSPCR0() {
  uint32_t value;
  value=     (((DSS-1) & ((1<<4)-1)) << 0)  |
             ((FRF     & ((1<<2)-1)) << 4)  |
             ((CPOL    & ((1<<1)-1)) << 6)  |
             ((CPHA    & ((1<<1)-1)) << 7)  |
             ((SCR     & ((1<<8)-1)) << 8) ;
  const int PCLK=60'000'000;
  dprintf(SIMSSP,"SSPCR0 read, 0x%04x (%d), DSS=%d, FRF=%d, CPOL=%d, CPHA=%d, SCR=%d, actual clock rate=%dHz\n",
    value,value,DSS,FRF,CPOL,CPHA,SCR,Hz());
  return value;
}

void SimSsp::write_SSPCR1(uint32_t value) {
  LBM = (value>>0) & ((1<<1)-1);
  SSE = (value>>1) & ((1<<1)-1);
  MS  = (value>>2) & ((1<<1)-1);
  SOD = (value>>3) & ((1<<1)-1);
  dprintf(SIMSSP,"SSPCR1 written, 0x%02x (%d), LBM=%d, SSE=%d, MS=%d, SOD=%d\n",
    value,value,LBM,SSE,MS,SOD);
}

uint32_t SimSsp::read_SSPCR1() {
  uint32_t value;
  value=     ((LBM & ((1<<1)-1)) << 0)  |
             ((SSE & ((1<<1)-1)) << 1)  |
             ((MS  & ((1<<1)-1)) << 2)  |
             ((SOD & ((1<<1)-1)) << 3)  ;
  dprintf(SIMSSP,"SSPCR1 read, 0x%02x (%d), LBM=%d, SSE=%d, MS=%d, SOD=%d\n",
    value,value,LBM,SSE,MS,SOD);
  return value;
}

void SimSsp::write_SSPCPSR(uint32_t value) {
  SimSubSsp::write_SSPCPSR(value & 0xF8); //bit 0 is always 0
  dprintf(SIMSSP,"SSPCPSR written, %d, actual clock rate=%dHz",value,Hz());
}

uint32_t SimSsp::read_SSPSR() {
  uint32_t value;
  value=     ((TFE & ((1<<1)-1)) << 0)  |
             ((TNF & ((1<<1)-1)) << 1)  |
             ((RNE & ((1<<1)-1)) << 2)  |
             ((RFF & ((1<<1)-1)) << 3)  |
             ((BSY & ((1<<1)-1)) << 4)  ;
  dprintf(SIMSSP_TRANSFER,"SSPSR read, 0x%02x (%d), TFE=%d, TNF=%d, RNE=%d, RFF=%d, BSY=%d\n",
    value,value,TFE,TNF,RNE,RFF,BSY);
  return value;
}

void SimSsp::write_SSPDR(uint32_t value) {
  int selectedSlaves=0;
  dprintf(SIMSSP_TRANSFER,"Transfer, sending %d (0x%02x)\n",value,value);
  SSPDR=0xFF; //Default value (as if there is a pullup on MISO)
  for(int i=0;i<32;i++) if(selected[i]) {
	SSPDR=slaves[i]->transfer(value);
	selectedSlaves++;
  }
  if(selectedSlaves!=1) {
	dprintf(SIMSSP_TRANSFER,"Warning: Exactly one slave should be selected, but %d are\n",selectedSlaves);
  }
  dprintf(SIMSSP_TRANSFER,"Transfer, received %d (0x%02x)\n",SSPDR,SSPDR);
}

uint32_t SimSsp::read_SSPDR() {
  dprintf(SIMSSP_TRANSFER,"SSPDR read, value %d (0x%02x)\n",SSPDR,SSPDR);
  return SSPDR;
}



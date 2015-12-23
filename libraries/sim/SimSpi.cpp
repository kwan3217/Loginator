#include "sim.h"
#include <stdio.h>

void SimSpi::pinOut (int port, int pin, int value) {
  int addr=((port & 0x0F)<<4) | (pin & 0x0F);
  selected[addr]=(value==0);
  slaves[addr]->csOut(value);
}
int  SimSpi::pinIn  (int port, int pin) {
  int addr=((port & 0x0F)<<4) | (pin & 0x0F);
  return slaves[addr]->csIn();
}
void SimSpi::pinMode(int port, int pin, bool out) {
  int addr=((port & 0x0F)<<4) | (pin & 0x0F);
  slaves[addr]->csMode(out);
}

void SimSpi::addSlave(int port, int pin, SimSpiSlave& Lslave) {
  int addr=((port & 0x0F)<<4) | (pin & 0x0F);
  slaves[addr]=&Lslave;
}

void SimSpi::write_S0SPCR(uint32_t value) {
  BitEnable= (value>>2) & ((1<<1)-1);
  CPHA     = (value>>3) & ((1<<1)-1);
  CPOL     = (value>>4) & ((1<<1)-1);
  MSTR     = (value>>5) & ((1<<1)-1);
  LSBF     = (value>>6) & ((1<<1)-1);
  SPIE     = (value>>7) & ((1<<1)-1);
  Bits     = (value>>8) & ((1<<4)-1);
  if(Bits==0) Bits=16;
  dprintf(SIMSPI,"S0SPCR written, 0x%04x (%d), BitEn=%d, CPHA=%d, CPOL=%d, MSTR=%d, LSBF=%d, SPIE=%d, Bits=%d\n",
    value,value,BitEnable,CPHA,CPOL,MSTR,LSBF,SPIE,Bits);
}

uint32_t SimSpi::read_S0SPCR() {
  uint32_t value;
  value=     ((BitEnable         & ((1<<1)-1)) << 2)  |
             ((CPHA              & ((1<<1)-1)) << 3)  |
             ((CPOL              & ((1<<1)-1)) << 4)  |
             ((MSTR              & ((1<<1)-1)) << 5)  |
             ((LSBF              & ((1<<1)-1)) << 6)  |
             ((SPIE              & ((1<<1)-1)) << 7)  |
             (((Bits==16?0:Bits) & ((1<<4)-1)) << 8)  ;
  dprintf(SIMSPI,"S0SPCR read, 0x%04x (%d), BitEn=%d, CPHA=%d, CPOL=%d, MSTR=%d, LSBF=%d, SPIE=%d, Bits=%d\n",
    value,value,BitEnable,CPHA,CPOL,MSTR,LSBF,SPIE,Bits);
  return value;
}

void SimSpi::write_S0SPCCR(uint32_t value) {
  SimSubSpi::write_S0SPCCR(value);
  const int PCLK=60'000'000;
  dprintf(SIMSPI,"S0SPCCR written, %d, actual clock rate=%dHz",value,PCLK/value);
}

uint32_t SimSpi::read_S0SPSR() {
  uint32_t value;
  value=     ((ABRT & ((1<<1)-1)) << 3)  | //ABRT, bit 2
             ((MODF & ((1<<1)-1)) << 4)  | //MODF, bit 3
             ((ROVR & ((1<<1)-1)) << 5)  | //ROVR, bit 4
             ((WCOL & ((1<<1)-1)) << 6)  | //WCOL, bit 5
             ((SPIF & ((1<<1)-1)) << 7)  ; //SPIF, bit 6
  dprintf(SIMSPI_TRANSFER,"S0SPSR read, 0x%02x (%d), ABRT=%d, MODF=%d, ROVR=%d, WCOL=%d, SPIF=%d\n",
    value,value,ABRT,MODF,ROVR,WCOL,SPIF);
  SPIF_read=true;
  return value;
}

void SimSpi::write_S0SPDR(uint32_t value) {
  SPIF_read=false;  //The SPIF flag has not been read since the last transfer
  SPIF=1;           //The transfer has (instantly) completed
  int selectedSlaves=0;
  dprintf(SIMSPI_TRANSFER,"Transfer, sending %d (0x%02x)\n",value,value);
  S0SPDR=0xFF; //Default value (as if there is a pullup on MISO)
  for(int i=0;i<32;i++) if(selected[i]) {
	S0SPDR=slaves[i]->transfer(value);
	selectedSlaves++;
  }
  if(selectedSlaves!=1) {
	dprintf(SIMSPI_TRANSFER,"Warning: Exactly one slave should be selected, but %d are\n",selectedSlaves);
  }
  dprintf(SIMSPI_TRANSFER,"Transfer, received %d (0x%02x)\n",S0SPDR,S0SPDR);
}

uint32_t SimSpi::read_S0SPDR() {
  //This code does the right thing with the SPIF flag. "This bit is cleared by
  //first reading [S0SPSR] then accessing the SPI data register." Presumably
  //on real hardware, the access above could be a write, meaning the read was
  //ignored. Such a write on the real thing would trigger an SPI transfer,
  //and the SPIF flag would be appropriately cleared until it finished. In the
  //sim, the transfer happens instantly, so a write access actually sets the 
  //flag, but this is correct for simulated purposes.
  if(SPIF_read) {
    SPIF=0;
    SPIF_read=false;
  };
  dprintf(SIMSPI_TRANSFER,"S0SPDR read, value %d (0x%02x)\n",S0SPDR,S0SPDR);
  return S0SPDR;
}



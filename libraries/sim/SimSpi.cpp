#include "sim.h"
#include <stdio.h>

void SimSpi::write_S0SPCR(uint32_t value) {
  BitEnable= (value>>2) & ((1<<1)-1);
  CPHA     = (value>>3) & ((1<<1)-1);
  CPOL     = (value>>4) & ((1<<1)-1);
  MSTR     = (value>>5) & ((1<<1)-1);
  LSBF     = (value>>6) & ((1<<1)-1);
  SPIE     = (value>>7) & ((1<<1)-1);
  Bits     = (value>>8) & ((1<<4)-1);
  if(Bits==0) Bits=16;
  ::fprintf(stderr,"S0SPCR written, 0x%04x (%d), BitEn=%d, CPHA=%d, CPOL=%d, MSTR=%d, LSBF=%d, SPIE=%d, Bits=%d\n",
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
  ::fprintf(stderr,"S0SPCR read, 0x%04x (%d), BitEn=%d, CPHA=%d, CPOL=%d, MSTR=%d, LSBF=%d, SPIE=%d, Bits=%d\n",
    value,value,BitEnable,CPHA,CPOL,MSTR,LSBF,SPIE,Bits);
  return value;
}

void SimSpi::write_S0SPCCR(uint32_t value) {
  SimSubSpi::write_S0SPCCR(value);
  const int PCLK=60'000'000;
  fprintf(stderr,"S0SPCCR written, %d, actual clock rate=%dHz",value,PCLK/value);
}

uint32_t SimSpi::read_S0SPSR() {
  uint32_t value;
  value=     ((ABRT & ((1<<1)-1)) << 3)  | //ABRT, bit 2
             ((MODF & ((1<<1)-1)) << 4)  | //MODF, bit 3
             ((ROVR & ((1<<1)-1)) << 5)  | //ROVR, bit 4
             ((WCOL & ((1<<1)-1)) << 6)  | //WCOL, bit 5
             ((SPIF & ((1<<1)-1)) << 7)  ; //SPIF, bit 6. This is SPI transfer finished, and is permanently 1 since SPI transfers take no simulated time
/*  ::fprintf(stderr,"S0SPSR read, 0x%02x (%d), ABRT=%d, MODF=%d, ROVR=%d, WCOL=%d, SPIF=%d\n",
    value,value,ABRT,MODF,ROVR,WCOL,SPIF); */
  return value;
}
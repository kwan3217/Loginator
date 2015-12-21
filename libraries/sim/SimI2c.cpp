#include "sim.h"
#include <stdio.h>

void SimI2c::write_I2CCONSET(int Lport, uint32_t write) {
  I2CCONSET[Lport]|=write;
  ::printf("I2CCON(%d) %02x (%d) AA=%d SI=%d STO=%d STA=%d I2EN=%d\n",Lport,I2CCONSET[Lport],I2CCONSET[Lport],AA(Lport),SI(Lport),STO(Lport),STA(Lport),I2EN(Lport));
  switch(I2CSTAT[Lport]) {
    case 0xF8:
	  if(STA(Lport)) {
        ::printf("Transmission on I2C%d started\n",Lport);
        I2CCONSET[Lport]|=1<<3;
        I2CSTAT[Lport]=0x08;
	  }
	  break;
  }
}

void SimI2c::write_I2CCONCLR(int Lport, uint32_t write) {
  I2CCONSET[Lport]&=~write;
  ::printf("I2CCON(%d) %02x (%d) AA=%d SI=%d STO=%d STA=%d I2EN=%d\n",Lport,I2CCONSET[Lport],I2CCONSET[Lport],AA(Lport),SI(Lport),STO(Lport),STA(Lport),I2EN(Lport));
}

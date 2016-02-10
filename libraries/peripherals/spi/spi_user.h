#ifndef SPI_USER_H
#define SPI_USER_H

#include "HardSPI.h"

class spi_user {
protected:
  HardSPI* s;
  int p0;
  void set_port(HardSPI& Ls) {s=&Ls;};
  spi_user() {};
public:
  spi_user(HardSPI& Ls, int Lp0):s(&Ls),p0(Lp0){};
  void set_p0(int Lp0) {s->release_cs(p0); p0=Lp0;};
  int get_p0() {return p0;};
};

#endif

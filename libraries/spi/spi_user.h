#ifndef SPI_USER_H
#define SPI_USER_H

#include "spi.h"

class spi_user {
protected:
  HardSPI *s;
  int p0;
public:
  spi_user(HardSPI *Ls, int Lp0):s(Ls),p0(Lp0){};
};

#endif

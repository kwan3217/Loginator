#ifndef SPI_USER_H
#define SPI_USER_H

#include "HardSPI.h"

template<class T>
class spi_user {
protected:
  T& s;
  int p0;
public:
  spi_user(T& Ls, int Lp0):s(Ls),p0(Lp0){};
  void set_p0(int Lp0) {s.release_cs(p0); p0=Lp0;};
  int get_p0() {return p0;};
};

#endif

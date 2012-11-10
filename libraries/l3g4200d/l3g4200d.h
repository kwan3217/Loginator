#ifndef l3g4200d_h
#define l3g4200d_h

#include "spi_user.h"

class L3G4200D:public spi_user {
public:
  L3G4200D(HardSPI *Ls, int Lp0):spi_user(Ls,Lp0){};
  virtual uint8_t begin();
  uint8_t read();
  void read(int16_t& x, int16_t& y, int16_t& z, uint8_t& t, uint8_t& status);
};

#endif

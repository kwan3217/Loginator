#ifndef l3g4200d_h
#define l3g4200d_h

#include "spi_user.h"
#include "packet.h"
#include "float.h"

template<class P,class S>
class L3G4200D:public spi_user<S> {
public:
  L3G4200D(S& Ls, int Lp0):spi_user<S>(Ls,Lp0){};
  uint8_t begin(char sens=3, char odr=3, char bw=3);
  uint8_t whoami() {
    char buf[2];
    this->s.rx_block(this->p0,0x8F,buf,2);
    return buf[1];
  };
  void set_sens(char fs);
  void read(int16_t& x, int16_t& y, int16_t& z, uint8_t& t, uint8_t& status);
  bool fillConfig(P& ccsds);
};

#include "l3g4200d.inc"

#endif

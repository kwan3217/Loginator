#ifndef l3g4200d_h
#define l3g4200d_h

#include "spi_user.h"
#include "packet.h"
#include "float.h"

class L3G4200D:public spi_user {
private:
  static int i_hwDesc;
public:
  L3G4200D();
  L3G4200D(HardSPI& Ls, int Lp0):spi_user(Ls,Lp0){};
  virtual uint8_t begin(char sens=3, char odr=3, char bw=3);
  uint8_t whoami();
  void set_sens(char fs);
  void read(int16_t& x, int16_t& y, int16_t& z, uint8_t& t, uint8_t& status);
  bool fillConfig(Packet& ccsds);
};

#endif

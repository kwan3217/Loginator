#ifndef adxl345_h
#define adxl345_h

#include "spi_user.h"
#include "packet.h"

class ADXL345: public spi_user {
private:
  static int i_hwDesc;
public:
  ADXL345();
  ADXL345(HardSPI &Ls, int Lp0):spi_user(&Ls,Lp0){};
  bool fillConfig(Packet& ccsds);
  bool begin();
  uint8_t whoami();
  void read(int16_t& x, int16_t& y, int16_t& z);
};

#endif

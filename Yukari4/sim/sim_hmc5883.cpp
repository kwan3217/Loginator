#include "hmc5883.h"
#include "sim.h"

/*
// Sets the configuration and mode registers such that the part is continuously
// generating measurements. This function should be called at the beginning of 
// the program
static char reg[13];
void HMC5883::begin() {
  reg[0]=(3<<5 | 6<<2 | 0<<0); //MA - 0b11  = 8 samples average
                                //DO - 0b110 = 75Hz measurment rate
                                //MS - 0b00  = normal measurement mode
  reg[1]=1<<5;  //GN - 0b001 = +-1.3Ga (1090DN/Ga)
  reg[2]=0; //MD - 0b00  = continuous measurement mode
  reg[9]=0; //SR1 - 0b0 - Lock bit
            //SR0 - 0b0 - Data Ready bit
  reg[10]='H'; //First char of ID
  reg[11]='4'; //Second char of ID
  reg[12]='3'; //Third char of ID
}

// Read 1 byte from the BMP085 at 'address'
int8_t HMC5883::read(uint8_t address) {
  return reg[address];
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int16_t HMC5883::read16(uint8_t address) {
  return 0;
}

void HMC5883::read(int16_t& x, int16_t& y, int16_t& z) {

}

void HMC5883::whoami(char* id) {
  id[0]=read(10);
  id[1]=read(11);
  id[2]=read(12);
  id[3]=0;
}

bool HMC5883::fillConfig(Packet& ccsds) {
  #include "write_packet_hmccfg.INC"
  return true;
}


*/

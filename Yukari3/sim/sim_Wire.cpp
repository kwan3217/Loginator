#include "Wire.h"
#include "LPC214x.h"

extern "C" {
  #include <stdlib.h>
  #include <inttypes.h>
}

void TwoWire::begin(unsigned int freq) {

}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity) {
  return 0;
}

void TwoWire::beginTransmission(uint8_t address) {

}

uint8_t TwoWire::endTransmission(void) {
  return 0;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
void TwoWire::write(uint8_t data) {

}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::available(void) {
  return 0;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::read(void) {
  return 0;
}

int TwoWire::peek() {
  return 0;
}

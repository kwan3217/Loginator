//SPI interface
#include "gpio.h"

template<class T> inline
void HardSPI<T>::select_cs(int p0) {
  gpio_write(p0,0); //Lower the CS pin (active low)
}

template<class T> inline
void HardSPI<T>::deselect_cs(int p0) {
  gpio_write(p0,1); //Raise the CS pin (active low)
}

template<class T> inline
void HardSPI<T>::claim_cs(int p0) {
  set_pin(p0,0); //Set this to GPIO, so that we control it
  gpio_set_write(p0); //Set pin to output
  deselect_cs(p0);
}

template<class T> inline
void HardSPI<T>::release_cs(int p0) {
  set_pin(p0,0); //Set this to GPIO, so that we control it
  gpio_set_read(p0); //Set pin to input
}



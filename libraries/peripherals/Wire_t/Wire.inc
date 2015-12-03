#include "LPC214x.h"

extern "C" {
  #include <stdlib.h>
  #include <inttypes.h>
}

template<class T> inline
void TwoWire<T>::begin(unsigned int freq) {
  rxBufferIndex = 0;
  rxBufferLength = 0;

  txBufferIndex = 0;
  txBufferLength = 0;

  static_cast<T*>(this)->twi_init(freq);
}

template<class T> inline
uint8_t TwoWire<T>::requestFrom(uint8_t address, uint8_t quantity) {
  // clamp to buffer length
  if(quantity > BUFFER_LENGTH){
    quantity = BUFFER_LENGTH;
  }
  // perform blocking read into buffer
  uint8_t result = static_cast<T*>(this)->twi_readFrom(address, rxBuffer, quantity);
  // set rx buffer iterator vars
  rxBufferIndex = 0;
  rxBufferLength = result;

  return result;
}

template<class T> inline
void TwoWire<T>::beginTransmission(uint8_t address) {
  // indicate that we are transmitting
  transmitting = 1;
  // set address of targeted slave
  txAddress = address;
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
}

template<class T> inline
uint8_t TwoWire<T>::endTransmission(void) {
  // transmit buffer (blocking)
  int8_t ret = static_cast<T*>(this)->twi_writeTo(txAddress, txBuffer, txBufferLength, 1);
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
  // indicate that we are done transmitting
  transmitting = 0;
  return ret;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
template<class T> inline
void TwoWire<T>::write(uint8_t data) {
  if(transmitting){
  // in master transmitter mode
    // don't bother if buffer is full
    if(txBufferLength >= BUFFER_LENGTH){
      return;
    }
    // put byte in tx buffer
    txBuffer[txBufferIndex] = data;
    ++txBufferIndex;
    // update amount in buffer   
    txBufferLength = txBufferIndex;
  }
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
template<class T> inline
int TwoWire<T>::available(void) {
  return rxBufferLength - rxBufferIndex;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
template<class T> inline
int TwoWire<T>::read(void) {
  if(rxBufferIndex >= rxBufferLength) return -1;
  int value = rxBuffer[rxBufferIndex];
  ++rxBufferIndex;

  return value;
}

template<class T> inline
int TwoWire<T>::peek() {
  if(rxBufferIndex >= rxBufferLength) return -1;
  return rxBuffer[rxBufferIndex];
}

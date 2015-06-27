//SPI interface
#include <stdlib.h>
#include "LPC214x.h"
#include "gpio.h"
#include "Time.h"
#include "HardSPI.h"
#include "Serial.h"

HardSPI0 SPI;
HardSPI1 SPI1;

void HardSPI::select_cs(int p0) {

}

void HardSPI::deselect_cs(int p0) {

}

void HardSPI::claim_cs(int p0) {

}

void HardSPI::release_cs(int p0) {

}

//Setup the SPI0 port, including claiming the pins
void HardSPI0::begin(unsigned int freq, unsigned char CPOL, unsigned char CPHA) {

}

void HardSPI0::setfreq(unsigned int freq) {

}

//Release the SPI0 port, setting the used pins back to GPIO input
void HardSPI0::stop(void) {

}

//Send a string to the slave, throw away the returned data, block until finished
//input
//  txb - pointer to bytes to send
//  tx_count - number of bytes to send
void HardSPI0::tx_block(const char* txb, int count) {

}

void HardSPI0::send_byte(uint8_t tx) {

}

//Send a byte to the slave, receive a string, block until finished
//input
//  tx - byte to send
//  rxb - pointer to bytes to send
//  rx_count - number of bytes to receive, incuding dummy byte at beginning
//side effect
//  buffer pointed to by rxb will hold received data
void HardSPI0::rx_block(uint8_t tx, char* rxb, int count) {

}

uint8_t HardSPI0::rec_byte() {
  return 0;
}

//Setup the SPI1 port, including claiming the pins
void HardSPI1::begin(unsigned int freq, unsigned char CPOL, unsigned char CPHA) {
}

void HardSPI1::setfreq(unsigned int freq) {

}

//Release the SPI1 port, setting the used pins back to GPIO input
void HardSPI1::stop(void) {

}

//Send a string to the slave, throw away the returned data, block until finished
//input
//  txb - pointer to bytes to send
//  tx_count - number of bytes to send
void HardSPI1::tx_block(const char* txb, int count) {

}

void HardSPI1::send_byte(uint8_t tx) {

}

//Send a byte to the slave, receive a string, block until finished
//input
//  tx - byte to send
//  rxb - pointer to bytes to send
//  rx_count - number of bytes to receive, incuding dummy byte at beginning
//side effect
//  buffer pointed to by rxb will hold received data
void HardSPI1::rx_block(uint8_t tx, char* rxb, int count) {

}

uint8_t HardSPI1::rec_byte() {
  return 0;
}


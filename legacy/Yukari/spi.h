#ifndef spi_h
#define spi_h

//Setup the SPI1 port, including claiming the pins
//input
//  freq - Approximate bus frequency in Hz
//return
//  0 is success
//  some other number for failure
int spi1_setup(unsigned int freq, unsigned char CPOL, unsigned char CPHA);

//Release the SPI1 port, setting the used pins back to GPIO input
//return
//  0 is success
//  some other number for failure
int spi1_stop(void);

//Trade a string with the slave, block until finished
//input
//  txb - pointer to bytes to send
//  tx_count - number of bytes to send
//  rxb - pointer to receive buffer
//  rx_count - number of bytes to receive
//return
//  0 is success
//  some other number for failure
//note - first receive byte will almost always be garbage
int spi1_txrx_string_block(char* txb, int tx_count, char* rxb, int rx_count);

//Send a string to the slave, throw away the returned data, block until finished
//input
//  txb - pointer to bytes to send
//  tx_count - number of bytes to send
//return
//  0 is success
//  some other number for failure
int spi1_tx_string_block(char* txb, int tx_count);

//Send a byte to the slave, receive a string, block until finished
//input
//  tx - byte to send
//  rxb - pointer to bytes to send
//  rx_count - number of bytes to send
//return
//  0 is success
//  some other number for failure
//note - first receive byte will almost always be garbage
int spi1_rx_string_block(unsigned char tx, char* rxb, int rx_count);

#endif

#ifndef SERIAL_H_
#define SERIAL_H_

//int putc_serial0 (int ch);
//int putc_serial1 (int ch);
int tx_serial(int port, char* data, unsigned int len);
int tx_serialz(int port, char* data);
void tx_serial_block(int port, char* data, unsigned int len);
void tx_serialz_block(int port, char* data);
typedef void (*user_uart_rx_t)(int);
void setup_serial(int port, int setbaud, user_uart_rx_t Luser_uart_rx=0);
#endif

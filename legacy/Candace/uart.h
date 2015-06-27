#ifndef uart_h
#define uart_h

#include "circular.h"
#include "setup.h"

extern circular uartbuf[2];

void setup_uart(int port, int setbaud, int want_ints);
void startRecordUART(void);
int autobaud(int port);

#endif

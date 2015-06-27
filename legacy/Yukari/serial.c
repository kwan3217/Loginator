#include "LPC214x.h"
#include "target.h"
#include "serial.h"
#include "setup.h"
#include "armVIC.h"
#include "irq.h"
#include "main.h"
#include <string.h>

static int putc_serial0 (int ch) {
    while (!(U0LSR & 0x20));
    return (U0THR = ch);
}

static int putc_serial1 (int ch) {
    while (!(U1LSR & 0x20));
    return (U1THR = ch);
}

//== Transmitter management
#define QUEUE_DEPTH 10
char* tx_ptr[2][QUEUE_DEPTH];
unsigned int tx_len[2][QUEUE_DEPTH];
unsigned int tx_que[2];

#define FIFO_DEPTH 15

static void tx_serial_fill_fifo(unsigned int port) {
  int len=FIFO_DEPTH;
  if(len>tx_len[port][0]) len=tx_len[port][0];
  if(port==0) {
    for(int i=0;i<len;i++) U0THR=*(tx_ptr[0][0]+i);
  } else {
    for(int i=0;i<len;i++) U1THR=*(tx_ptr[1][0]+i);
  }
  tx_len[port][0]-=len;
  tx_ptr[port][0]+=len;
  if(tx_len[port][0]==0) {
    for(int i=0;i<QUEUE_DEPTH-2;i++) {
      tx_ptr[port][i]=tx_ptr[port][i+1];
      tx_len[port][i]=tx_len[port][i+1];
    }
    tx_ptr[port][QUEUE_DEPTH-1]=NULL;
    tx_len[port][QUEUE_DEPTH-1]=0;

  }
}

int tx_serial(int port, char* data, unsigned int len) {
  if(tx_ptr[port][QUEUE_DEPTH-1]!=NULL) return 0;
  int i=0;
  while(i<QUEUE_DEPTH-1 && tx_ptr[port][i]!=NULL)i++;
  tx_ptr[port][i]=data;
  tx_len[port][i]=len;
  tx_serial_fill_fifo(port);
  return 1;
}

int tx_serialz(int port, char* data) {
  if(tx_ptr[port][QUEUE_DEPTH-1]!=NULL) return 0;
  int i=0;
  while(i<QUEUE_DEPTH-1 && tx_ptr[port][i]!=NULL)i++;
  tx_ptr[port][i]=data;
  tx_len[port][i]=strlen(data);
  tx_serial_fill_fifo(port);
  return 1;
}

void tx_serial_block(int port, char* data, unsigned int len) {
  if(len==0) return;
  if(port==0) {
    for(int i=0;i<len;i++) putc_serial0(data[i]);
  } else {
    for(int i=0;i<len;i++) putc_serial1(data[i]);
  }    
}

void tx_serialz_block(int port, char* data) {
  int len=strlen(data);
  if(len==0) return;
  if(port==0) {
    for(int i=0;i<len;i++) putc_serial0(data[i]);
  } else {
    for(int i=0;i<len;i++) putc_serial1(data[i]);
  }    
}

//Array of pointers to void(void) functions
static void (*user_uart_rx[2])(int);

static void uart0_isr(void) {
  int isr_cause=U0IIR;
  if(0x02 & isr_cause) {//If it's a THR int...
    tx_serial_fill_fifo(0); //Send more data
  }
  if(0x04 & isr_cause) {
    if(user_uart_rx[0]) user_uart_rx[0](0);
  }
  
  //Acknowledge the VIC
  VICVectAddr = 0;
}

static void uart1_isr(void) {
  int isr_cause=U1IIR;
  if(0x02 & isr_cause) {//If it's a THR int...
    tx_serial_fill_fifo(1); //Send more data
  }
  if(0x04 & isr_cause) {//If it's an RXE int...
    if(user_uart_rx[1]) user_uart_rx[1](1);    //Call the user function to do something about it
  }
  //Acknowledge the VIC
  VICVectAddr = 0;
}

//== Port setup
void setup_serial(int port, int setbaud, void (*Luser_uart_rx)(int)) {
  user_uart_rx[port]=Luser_uart_rx;
  ULCR(port) = 0x83;   // 8 bits, no parity, 1 stop bit, DLAB = 1
  //DLAB - Divisor Latch Access bit. When set, a certain memory address
  //       maps to the divisor latches, which control the baud rate. When
  //       cleared, those same addresses correspond to the processor end 
  //       of the FIFOs. In other words, set the DLAB to change the baud
  //       rate, and clear it to use the FIFOs.

  int Denom=PCLK/setbaud;
  int UDL=Denom/16;
  
  UDLM(port)=(UDL >> 8) & 0xFF;
  UDLL(port)=(UDL >> 0) & 0xFF;
  
  UFCR(port) = 0xC7; //Enable and clear both FIFOs
  ULCR(port) = 0x03; //Turn of DLAB - FIFOs accessable
  
  enableIRQ();
  if(port==0) {
    install_irq(UART0_INT,uart0_isr);
    U0IER=0x03; //Rx ready, Tx empty, not line status, not autobaud ints
  } else {
    install_irq(UART1_INT,uart1_isr);
    U1IER=0x03; //Rx ready, Tx empty, not line status, not autobaud ints
  }	
}


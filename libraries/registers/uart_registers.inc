/* Universal Asynchronous Receiver Transmitter */
#if MCU == MCU_ARM7TDMI
#define UART0_BASE_ADDR 0xE000'C000
#define UART1_BASE_ADDR 0xE001'0000
#define N_UART 2

#define UART_BASE_ADDR(i) (i==0?UART0_BASE_ADDR:UART1_BASE_ADDR)

#endif //MCU_ARM7TDMI

#if MCU == MCU_CORTEXM4
#define UART0_BASE_ADDR 0x4000'C000
#define UART1_BASE_ADDR 0x4001'0000
#define UART2_BASE_ADDR 0x4008'8000
#define UART3_BASE_ADDR 0x4009'C000
#define UART4_BASE_ADDR 0x400A'4000
#define N_UART 5

#define UART_BASE_ADDR(i) (i==0?UART0_BASE_ADDR:(i==1?UART1_BASE_ADDR:(i==2?UART2_BASE_ADDR:(i==3?UART3_BASE_ADDR:UART4_BASE_ADDR))))

#endif


/** UART Receive Buffer Register. This register holds the next byte received by 
the UART. Reading it will make it ready to receive the next byte, filling it from
the Rx FIFO if it is enabled. Read only, has the same address as UTHR. Only 
available when the DLAB (bit 7 in ULCR) is 0, UDLL is visible at this address
when DLAB=1. */
ro1(uart,URBR,N_UART,UART_BASE_ADDR(i)+0x00);

/** UART Transmit Holding Register. This register holds the next byte to be
transmitted by the UART. Writing it will either start sending the byte
immediately, or put it in the Tx FIFO if that is enabled. Write only, has the 
same address as URBR */
wo1(uart,UTHR,N_UART,UART_BASE_ADDR(i)+0x00);

/** UART Divisor Latch LSB. This is used to control the baud rate, in
combination with UDLM. The baud rate controlled by the 16-bit value stored in 
UDLL+UDLM<<8, and fine tuned by UFDR. Only available when
DLAB=1, URBR is visible at this address when DLAB=0 */
rw1(uart,UDLL,N_UART,UART_BASE_ADDR(i)+0x00);

/** UART Divisor Latch MSB. This is used to control the baud rate, in
combination with UDLL. Only available when DLAB=1, UIER is visible at this
address when DLAB=0*/
rw1(uart,UDLM,N_UART,UART_BASE_ADDR(i)+0x04);

/** UART Interrupt Enable Register. 
*    * Bit 0 controls Receive Data Available and Character Receive Timeout
*        interrupts
*    * Bit 1 enables the threshold interrupt, which is identified in ULSR
*        bit 5
*    * Bit 2 enables the RX line status interrupts, which are identified in ULSR
*        bits 1-4
*
*Only available when DLAB=0, UDLM is visible at this address when DLAB=1*/
rw1(uart,UIER,N_UART,UART_BASE_ADDR(i)+0x04);

/** UART Interrupt ID Register. Reading this register clears the interrupt,
*  so this register must be read before the service routine finishes. Read-only,
*  shares address with UFCR.
*    * Bit 0 (active low) indicates an interrupt is pending
*    * Bits 1-3 indicate the source of the interrupt
*     + 011 (3) - Receive Line Status
*     + 010 (2) - Receive Data Available
*     + 110 (6) - Character Timeout Indicator
*     + 001 (1) - Threshold
*     + 000 (0) - Modem (only available on UART1, and only on LPC2144/6/8)
*    * Bits 6 and 7 mirror the FIFO enable bit in UFCR bit 0
*    * Bit 8 indicates successful end of auto-baud
*    * Bit 9 indicates auto-baud timeout
*/
ro1(uart,UIIR,N_UART,UART_BASE_ADDR(i)+0x08);

/** UART FIFO Control Register. Write-only, shares address with UIIR
*    * Bit 0 enables both FIFOs
*    * Bit 1 resets the Rx FIFO
*    * Bit 2 resets the Tx FIFO
*    * Bits 6-7 controlt the RX trigger level
*     + 00 (0) - 1 character
*     + 01 (1) - 4 characters
*     + 10 (2) - 8 characters
*     + 11 (3) - 14 characters
*/
wo1(uart,UFCR,N_UART,UART_BASE_ADDR(i)+0x08);

/** UART Line Control Register
*    * Bits 0-1 identify character length (=5+bits)
*    * Bit 2 controls stop bits (=1+bit)
*    * Bit 3 enables parity generation and checking
*    * Bits 4-5 select parity
*     + 00 (0) - Odd Parity - bit is set such that number of 1s transmitted is odd
*     + 01 (1) - Even Parity - bit is set such that number of 1s transmitted is even
*     + 10 (2) - Mark Parity - bit is always 1
*     + 11 (3) - Space Parity - bit is always 0
*    * Bit 6 controls break transmission
*    * Bit 7 is the divisor latch access bit. Setting to 0 makes URBR, UTHR, and
*      UIER visible. Setting to 1 makes UDLL and UDLM visisble
*/
rw1(uart,ULCR,N_UART,UART_BASE_ADDR(i)+0x0C);

/** UART Line Status Register. Read-only
*    * Bit 0 - Receiver Data Ready
*    * Bit 1 - Overrun Error, cleared on read
*    * Bit 2 - Parity Error, cleared on read
*    * Bit 3 - Framing Error, cleared on read
*    * Bit 4 - Break Interrupt, cleared on read
*    * Bit 5 - Trasmistter Holding Register Empty
*    * Bit 6 - Transmitter Empty
*    * Bit 7 - Error in Rx FIFO, cleared on read
*/
ro1(uart,ULSR,N_UART,UART_BASE_ADDR(i)+0x14);

/** UART Modem Control Register. Only available on UART1, and not available
* on LPC2141/2.
*    * Bit 0 - DTR control
*    * Bit 1 - RTS control
*    * Bit 4 - Loopback mode select. If set, the TX and RX will be internally 
*        connected and no activity will be visible on the external pins
*    * Bit 6 controls Auto-RTS
*    * Bit 7 controls Auto-CST
*/
rw1(uart,UMCR,N_UART,UART_BASE_ADDR(i)+0x10);

/** UART Modem Status Register. Only available on UART1, and not available
* on LPC2141/2.
*    * Bit 0 - Input CTS status changed, cleared on read
*    * Bit 1 - Input DSR status changed, cleared on read
*    * Bit 2 - Trailing edge RI, cleared on read
*    * Bit 3 - Input DCD status changed, cleared on read
*    * Bit 4 - Clear to Send state. Bits 4-7 are active high, while the pins
*                 themselves are active low.
*    * Bit 5 - Data Set Ready state
*    * Bit 6 - Ring Indicator state
*    * Bit 7 - Data Carrier Detect state
*/
ro1(uart,UMSR,N_UART,UART_BASE_ADDR(i)+0x18);

/** UART scratch register. Not used by UART, but holds its value and is read/write */
rw1(uart,USCR,N_UART,UART_BASE_ADDR(i)+0x1C);

/** UART Auto-baud control register */
rw1(uart,UACR,N_UART,UART_BASE_ADDR(i)+0x20);

/** UART IrDA control register. Only available on UART4 */
rw1(uart,UICR,N_UART,UART_BASE_ADDR(i)+0x20);

/** UART Fractional divider register. Used to fine-tune the baud rate. */
rw1(uart,UFDR,N_UART,UART_BASE_ADDR(i)+0x28);

/** UART Oversampling register. Only available on UART4 */
rw1(uart,UOSR,N_UART,UART_BASE_ADDR(i)+0x20);

/** UART Smart Card Interface control register. Only available on UART4 */
rw1(uart,USCICTRL,N_UART,UART_BASE_ADDR(i)+0x20);

/** UART Transmit Enable Register. Bit 7 controls whether the transmitter is enabled. */
rw1(uart,UTER,N_UART,UART_BASE_ADDR(i)+0x30);

#undef UART_BASE_ADDR
#undef N_UART
#undef UART0_BASE_ADDR
#undef UART1_BASE_ADDR
#if MCU == MCU_CORTEXM4
#undef UART2_BASE_ADDR
#undef UART3_BASE_ADDR
#undef UART4_BASE_ADDR
#endif



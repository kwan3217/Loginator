#include "registers.h"
#include "irq.h"
#include <inttypes.h>
#include <stddef.h> //for size_t

//Now we see why we like high-level languages for IRQ handling. It says treat
//the number as a pointer to a function and call it.
typedef void (*fvoid)(void);
void IRQHandler::IRQ_Wrapper() {
  ((fvoid)(uintptr_t)(VICVectAddr()))();
}

/**
  Default interrupt handler, called if no handler is installed for a particular interrupt.
  If the IRQ is not installed into the VIC, and interrupt occurs, the
  default interrupt VIC address will be used. This could happen in a race
  condition. For debugging, use this endless loop to trace back.
  For more details, see Philips appnote AN10414 */
void IRQHandler::DefaultVICHandler(void) {
  //Print 'E' forever on UART0 at whatever is its current settings
  while ( 1 ) {
    while (!(ULSR(0) & 0x20));
    UTHR(0) = 'E';
  }
}

/** Initialize the interrupt controller. Clear out all vector slots */
void IRQHandler::begin(void) {
  // initialize VIC
  VICIntEnClr() = 0xffffffff;
  VICVectAddr() = 0;
  VICIntSelect() = 0;

  // set all the vector and vector control register to 0 
  for (int i = 0; i < VIC_SIZE; i++ ) {
    VICVectAddrSlot(i) = 0;
    VICVectCntlSlot(i) = 0;
  }

  /* Install the default VIC handler here */
  //Weird cast below is for machines where pointer isn't 32 bit (like an x86_64 host)
  VICDefVectAddr() = (uint32_t)(size_t)&DefaultVICHandler;
  return;
}

/**
  Install an interrupt handler in the VIC. This finds an empty slot,
  installs the handler there, and associates it with the correct source channel.
\note  The VIC slots are associated with priority, lower number is earlier priority.
   This handler installs the handler in the lowest numbered slot available, so
   if you care about priority, then when installing multiple handlers, install them
   first priority first, and so on.

 \param IntNumber Interrupt source channel
 \param HandlerAddr interrupt handler address
 \return      true if handler installed, false if not (table full)
*/
bool IRQHandler::install(unsigned int IntNumber, irqHandler HandlerAddr ) {
  VICIntEnClr() = 1 << IntNumber;   //Disable Interrupt

  for (int i = 0; i < VIC_SIZE; i++ ) {
    // find first un-assigned VIC address for the handler

    if ( VICVectAddrSlot(i) == 0 ) {
      //Weird cast below is for machines where pointer isn't 32 bit (like an x86_64 host)
      VICVectAddrSlot(i) = (uint32_t)(size_t)HandlerAddr;    // set interrupt vector
      VICVectCntlSlot(i) = (IRQ_SLOT_EN | IntNumber);
      VICIntEnable() |= 1 << IntNumber;  // Enable Interrupt
      return true;
    }
  }

  return false;        // fatal error, can't find empty vector slot
}

/**
  Remove an interrupt handler in the VIC. This finds the slot associated with this
  source channel and clears it.,

 \param IntNumber Interrupt source channel
 \return      true if handler was uninstalled, false if not (no handler for this channel installed in the first place)

 \note You can install the same channel in multiple VIC slots. It's not a good idea, but possible. If you do so, this
       will only uninstall the one with the earliest priority (lowest numbered slot).
*/
bool IRQHandler::uninstall(unsigned int IntNumber ) {
  VICIntEnClr() = 1 << IntNumber;   /* Disable Interrupt */

  for (int i = 0; i < VIC_SIZE; i++ ) {
    //find first VIC address assigned to this channel
    if ( (VICVectCntlSlot(i) & 0x1f ) == IntNumber ) {
      VICVectAddrSlot(i) = 0;   // clear the VIC entry in the VIC table
      VICVectCntlSlot(i) = 0;   // disable SLOT_EN bit and mark slot as available
      return true;
    }
  }
  return false;        // fatal error, can't find interrupt number in vector slot
}



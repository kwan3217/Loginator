#include "Startup.h"
#include "registers.h"
#include "Time.h"
#include "irq.h"

/** This is the less assembly-dependent part of the startup, which can be run in emulator mode */
void reset_handler_core() {
  //Set up system peripherals. Don't run this until now because we might
  //set global variables like PCLK.
//  setup_pll(0,5); //Set up CCLK PLL to 5x crystal rate
  // Enabling MAM and setting number of clocks used for Flash memory fetch (4 cclks in this case)
  //MAMTIM=0x3; //VCOM?
//  MAMCR()=0x2;
//  MAMTIM()=0x4; //Original
  // Setting peripheral Clock (pclk) to System Clock (cclk)
//  VPBDIV()=0x1;

//  IRQHandler::begin(); //Can't call before ctors are run
}

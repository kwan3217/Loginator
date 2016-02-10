#include "Startup.h"

/** 
  \file Startup.cpp Second stage startup code, for any processor but not for host.
  \brief Startup code, run at reset.
 
This is tightly integrated with the linker script [lo|hi]mem_arm_eabi_cpp.ld and
depends on certain symbols there with the correct name and correct location.
A symbol is simply a name for an address. So, linker-defined symbols for the 
beginning and end of certain data segments are addresses. But,
the best match in C or C++ is to define them as arrays, which are always usable
as named pointer constants. Frequently we want to treat them as arrays, anyway.
Generally sections are delimited by start and end symbols, which can be
subtracted to get the size of the section. The start symbol is perfectly usable
as an array, and the end symbol, while never used directly, is the same array
type as the start symbol so that their pointers are compatible. All symbols
which start with an underscore here are created by the linker, and their names
are controlled by the linker script.

The variable _ctors_start actually points to a proper array, so we can actually 
use the mechanism as intended. It is an array of pointers to void functions.
Each pointer is the pointer to a short block of code, and appears to be a static
function which constructs all the global objects in the same compilation unit.
This code is free to call constructors and/or perform inline constructors
directly. As a static function, the pointee code uses BX lr to return
*/

extern int _bss_start[];    ///< Start of BSS data, to be zeroed out during startup
extern int _bss_end[];      ///< End of BSS data, one byte past the last byte to be zeroed out                                                              
extern int _bss_lib_start[];///< Start of BSS data for library routines
extern int _bss_lib_end[];  ///< End of BSS data for library routines
extern int _bdata[];        ///< Start of initialized data image in ROM 
extern int _data[];         ///< Start of initialized data in RAM
extern int _edata[];        ///< End of initialized data in RAM, one byte past last byte with a value. Note that this is in RAM, not ROM, so relative to _data
extern int _bdata_lib[];    ///< Start of initialized data image in ROM for library
extern int _data_lib[];     ///< Start of initialized data in RAM for library
extern int _edata_lib[];    ///< End of initialized data in RAM for library

typedef void (*fvoid)(void); ///< Pointer to a function with no parameters and no return value
extern fvoid _ctors_start[]; ///< Pointer to start of constructor block
extern fvoid _ctors_end[];   ///< Pointer to end of constructor block, one byte past the last pointer
extern fvoid _ctors_lib_start[]; ///< Pointer to start of constructor block for library
extern fvoid _ctors_lib_end[];   ///< Pointer to end of constructor block for library 

/** Runtime system init, called by reset handler. This code zeroes out BSS, 
copies initialized data, and calls the static constructor blocks. It also sets
up certain system peripherals, like the system clock PLL and the VIC. Finally
it calls setup() to run user setup code, then repeatedly calls loop() to run 
user main loop code. 

 -No cleanup code because function won't return 
 */
void __attribute__ ((noreturn)) init() {
  //Stack is available from this point on, but interrupts are still disabled

// Relocate .data sections (initialized variables)
  for(int i=0;i<(_edata-_data);i++)         _data    [i]=_bdata    [i];
  for(int i=0;i<(_edata_lib-_data_lib);i++) _data_lib[i]=_bdata_lib[i];
//Initialize .bss sections
  for(int i=0;i<(_bss_end-_bss_start);i++)         _bss_start    [i]=0;
  for(int i=0;i<(_bss_lib_end-_bss_lib_start);i++) _bss_lib_start[i]=0;

  //Global and static function variables are now available

  // call C++ constructors of global objects
  for(int i=0;i<_ctors_end-_ctors_start;i++) _ctors_start[i]();
  for(int i=0;i<_ctors_lib_end-_ctors_lib_start;i++) _ctors_lib_start[i]();

  reset_handler_core();

//We have finally broken the tyranny of main(). Directly call user's setup()
  setup();
//Directly call user's loop();
  for(;;) loop();
}

/**Default exception handler. This is named Undef_Handler, but several other
exceptions are mapped to this as well. */  
/**Handlers for various exceptions, defined as weak so that they may be replaced
by user code. These default handlers just go into infinite loops. Reset and
IRQ are handled by real (strong) functions, as they do the right thing and
it doesn't make sense for user code to replace them. We save a trivial amount
of space by re-using the code for Undef_Handler for all the exceptions. Because
these routines do not return, they don't care about proper exception returns,
but any real functions will need to be __attribute__((interrupt("FIQ"))) etc. */  
extern "C" void __attribute__ ((weak)) __attribute__ ((noreturn)) Undef_Handler(void) {for(;;);}
extern "C" void __attribute__ ((weak)) __attribute__ ((noreturn)) __attribute__((alias ("Undef_Handler"))) SWI_Handler(void);
extern "C" void __attribute__ ((weak)) __attribute__ ((noreturn)) __attribute__((alias ("Undef_Handler"))) PAbt_Handler(void);
extern "C" void __attribute__ ((weak)) __attribute__ ((noreturn)) __attribute__((alias ("Undef_Handler"))) DAbt_Handler(void);
extern "C" void __attribute__ ((weak)) __attribute__ ((noreturn)) __attribute__((alias ("Undef_Handler"))) FIQ_Handler(void);
extern "C" void __attribute__ ((weak)) __attribute__ ((noreturn)) __attribute__((alias ("Undef_Handler"))) __cxa_pure_virtual(void);


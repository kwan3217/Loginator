#include "LPC214x.h"
#include "irq.h"
#include "hardware_stack.h"
#include "Startup.h"

/** 
  \file Startup.cpp
  \brief Startup code, run at reset.
 
There are two reasons that Startup has to be in asm:

1. Registers CPSR and SP are not directly available to C/C++ code.
     So, we write a crumb of inline asm to access these registers,
     then write the rest in C/C++.
2. Exact control of the interrupt table is needed
     So, we use more inline asm, with calls to symbols defined in C/C++

Other complications: 

- Setting up the stack before the stack is available (use naked)
- IRQ wrapper proper return (use interrupt("IRQ"))
- Vector table can have any name, but must be linked in the right place

Modes as used by Loginator-type code:
- Normal code runs in system mode, so no restricted instructions. Fine, since there is no OS.
- IRQs run in IRQ mode with irqs disabled
- FIQ runs in FIQ mode with irqs and fiqs disabled, if we ever need/write an FIQ handler
- System starts in Supervisor mode but shifts to system mode before running main()

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

// Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs
static const int Mode_USR=0x10; ///< CPSR User mode bits. This is the unpriveleged mode. 
static const int Mode_FIQ=0x11; ///< CPSR Fast Interrupt mode bits. This mode is entered when a fast interrupt is triggered
static const int Mode_IRQ=0x12; ///< CPSR Interrupt mode bits. This mode is entered when a normal hardware interrupt is triggered
static const int Mode_SVC=0x13; ///< CPSR Supervisor mode bits. Supervisor mode is entered when a software interrupt is triggered                     
static const int Mode_ABT=0x17; ///< CPSR Abort mode bits. This mode is entered on a Prefecth (code) or Data abort exception
static const int Mode_UND=0x1B; ///< CPSR Undefined mode bits. This mode is entered on an Undefined Instruction exception
static const int Mode_SYS=0x1F; ///< CPSR System mode. This mode is the same as user mode, but priveleged.
static const int I_Bit=0x80;    ///< when CPSR I bit is set, IRQ is disabled
static const int F_Bit=0x40;    ///< when CPSR F bit is set, FIQ is disabled

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
/**Pointer to byte just above end of RAM. Put the stack here. The ABI in use 
specifies a "Full-Decrementing" stack, which means that the stack pointer points
at a value which has data (is "Full") and that you decrement the stack pointer
when you push. The "Full" part means that the stack is predecrement, so the 
first push will first decrement the stack pointer from _ram_end, which actually
points past the RAM, and isn't allowed to actually hold a stack value. The value
will then be written at the new location of the stack pointer, which is now in
RAM. */ 
extern int _ram_end[]; 
typedef void (*fvoid)(void); ///< Pointer to a function with no parameters and no return value
extern fvoid _ctors_start[]; ///< Pointer to start of constructor block
extern fvoid _ctors_end[];   ///< Pointer to end of constructor block, one byte past the last pointer
extern fvoid _ctors_lib_start[]; ///< Pointer to start of constructor block for library
extern fvoid _ctors_lib_end[];   ///< Pointer to end of constructor block for library 


//The docs say that a successful feed must consist of two writes with no
//intervening APB cycles. Use asm to make sure that it is done with two
//intervening instructions.
void feed(int channel) {
  asm("mov r0, %0\n\t"
      "mov r1,#0xAA\n\t"
      "mov r2,#0x55\n\t"
      "str r1,[r0]\n\t"
      "str r2,[r0]\n\t" : :"r"(&PLLFEED(channel)):"r0","r1","r2");
//  PLLFEED(channel)=0xAA;
//  PLLFEED(channel)=0x55;
}

/** Interrupt vector table. In an ARM processor, the interrupt table consists
of actual code, in this case an LDR instruction which loads PC with the value
specified by PC+24 bytes. So we have those instructions, then pointers to the 
actual code to run for each interrupt. The linker fills these in for us, based
on the symbols given, which must be mangled if pointing to C++ code. */ 
__attribute__ ((naked)) __attribute__ ((section(".vectors"))) void vectorg(void) {
  asm("ldr pc,[pc,#24]\n\t"
      "ldr pc,[pc,#24]\n\t"
      "ldr pc,[pc,#24]\n\t"
      "ldr pc,[pc,#24]\n\t"
      "ldr pc,[pc,#24]\n\t"
      ".word 0xb8a06f60\n\t" //NXP checksum, constant as long as the other 7 instructions in first 8 are constant
      "ldr pc,[pc,#24]\n\t"
      "ldr pc,[pc,#24]\n\t"
      ".word Reset_Handler\n\t"   //Startup code location
      ".word Undef_Handler\n\t"   //Undef
      ".word SWI_Handler\n\t"   //SWI
      ".word PAbt_Handler\n\t"   //PAbt
      ".word DAbt_Handler\n\t"   //DAbt
      ".word 0\n\t"               //Reserved (hole in vector table above)
      ".word _ZN10IRQHandler11IRQ_WrapperEv\n\t"     //IRQ (wrapper so that normal C/C++ functions can be installed in VIC)
      ".word FIQ_Handler"); //FIQ
}

//Static inline basically means "always inline" so this is the closest to a macro that we can get
/** Set the CPSR to the given value
@param val Value to set CPSR to. This controls which processor mode is in
effect, and whether hardware interrupts are enabled.

This is a static inline function, so hopefully it is optimized into a single
instruction and acts like an intrinsic. Observation of the disassembly shows
that this works in the case where we use it below, in Reset_Handler() .*/ 
static inline void __set_cpsr_c(int  val) {asm volatile (" msr  cpsr_c, %0" : : "r" (val));} 

/** Set the stack pointer (R13) to the given value
@param val Value to set the stack pointer to. 

This is a static inline function, so hopefully it is optimized into a single
instruction and acts like an intrinsic. Observation of the disassembly shows
that this works in the case where we use it below, in Reset_Handler() .*/ 
static inline void __set_sp    (int *val) {asm volatile (" mov  sp    , %0" : : "r" (val));}

/** Reset handler, called when processor resets. Actually called after NXP 
bootloader finishes and jumps to address 0. This code sets up the stacks,
puts us in system mode, zeroes out BSS, copies initialized data, and calls the
static constructor blocks. It also sets up certain system peripherals, like the
system clock PLL and the VIC. Finally it calls setup() to run user setup code,
then repeatedly calls loop() to run user main loop code. 

- No setup code because stack isn't available yet
 -No cleanup code because function won't return (what would it return to?)
 */
extern "C" void /*__attribute__ ((naked))*/ __attribute__ ((noreturn)) Reset_Handler() {
  //Set up stacks...
  register int* stack=_ram_end; //If this doesn't end up in a register, we are in trouble 
  //For each of the modes, switch to that mode with hardware interrupts disabled, 
  //set up the stack pointer (a shadowed register only visible in its own mode) and subtract
  //the size of the stack we are allocating to this mode. USR and SYS use the same shadowed
  //registers so when we set it up for SYS, we set it up for USR. Besides, we are never going
  //to actually get to USR mode. Set up SYS/USR stack last, so it is using all 
  //the space between the other stacks and the global variables. 
  //
  __set_cpsr_c(Mode_UND | I_Bit | F_Bit); __set_sp(stack); stack-= (UND_Stack_Size)/sizeof(int);
  __set_cpsr_c(Mode_ABT | I_Bit | F_Bit); __set_sp(stack); stack-= (ABT_Stack_Size)/sizeof(int);
  __set_cpsr_c(Mode_FIQ | I_Bit | F_Bit); __set_sp(stack); stack-= (FIQ_Stack_Size)/sizeof(int);
  __set_cpsr_c(Mode_IRQ | I_Bit | F_Bit); __set_sp(stack); stack-= (IRQ_Stack_Size)/sizeof(int);
  __set_cpsr_c(Mode_SVC | I_Bit | F_Bit); __set_sp(stack); stack-= (SVC_Stack_Size)/sizeof(int);
  __set_cpsr_c(Mode_SYS | I_Bit | F_Bit); __set_sp(stack);

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

  //Enable hardware interrupts only after VIC is set up to handle them. Note that
  //vectors STILL probably aren't set up right, since the details depend on which
  //peripherals are in use, and those depend on setup(). We will trust that code
  //to only activate peripherals and enable their interrupts after the vectors
  //are set right.
  __set_cpsr_c(Mode_SYS);

//We have finally broken the tyranny of main(). Directly call user's setup()
  setup();
//Directly call user's loop();
  for(;;) loop();
}

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
/**This fills the vtable slots of a class where the method is abstract. Why not
just zero? That would cause a spontaneous reset on an ARM processor, and "Thou
shalt not follow the NULL pointer, for chaos and madness await thee at its
end." But, if you call an abstract method, you get what you deserve.
Defined as weak so that if you want to write a function that beats the
programmer about the head when called, you can. */
extern "C" void __attribute__ ((weak)) __attribute__ ((noreturn)) __attribute__((alias ("Undef_Handler"))) __cxa_pure_virtual();

#include "LPC214x.h"
#include "Time.h"
#include "irq.h"
#include "hardware_stack.h"

/*
There are two reasons that Startup has to be in asm:

1. Registers CPSR and SP are not directly available to C code.
     So, we write a crumb of inline asm to access these registers,
     then write the rest in C.
2. Exact control of the interrupt table is needed
     So, we use more inline asm 

Other complications: Setting up the stack before the stack is available (use naked)
                     IRQ wrapper proper return (use interrupt("IRQ"))
                     Vector table can have any name, but must be in section 
		       called .vectors so that it will link to the right place

This is tightly integrated with the linker script [lo|hi]mem_arm_eabi_cpp.ld and
depends on certain symbols there with the correct name and correct location.
 */

// Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs
static const int Mode_USR=0x10;
static const int Mode_FIQ=0x11;
static const int Mode_IRQ=0x12;
static const int Mode_SVC=0x13;
static const int Mode_ABT=0x17;
static const int Mode_UND=0x1B;
static const int Mode_SYS=0x1F;
static const int I_Bit=0x80;    // when I bit is set, IRQ is disabled 
static const int F_Bit=0x40;    // when F bit is set, FIQ is disabled 

//Modes as used by Loginator-type code:
//Normal code runs in system mode, so no restricted instructions. Fine, since there is no OS.
//IRQs run in IRQ mode with irqs disabled
//FIQ runs in FIQ mode with irqs and fiqs disabled, if we ever need/write an FIQ handler
//System starts in Supervisor mode but shifts to system mode before running main()

//A symbol is simply a name for an address. So, all of these are addresses. But,
//the best match in C or C++ is to define them as arrays, which are always usable
//as named pointer constants. Frequently we want to treat them as arrays, anyway.
//Generally sections are delimited by start and end symbols, which can be subtracted
//to get the size of the section. The start symbol is perfectly usable as an
//array, and the end symbol, while never used directly, is the same array type as
//the start symbol so that their pointers are compatible.
//All of these symbols are created by the linker, and their names are controlled
//by the linker script.
extern int bss_start[];
extern int bss_end[];
extern int etext[];
extern int data[];
extern int edata[];
//ctors_start actually points to a proper array, so we can actually use 
//the mechanism as intended. It is an array of pointers to void functions.
typedef void (*fvoid)(void);
extern fvoid ctors_start[];
extern fvoid ctors_end[];

//Handlers for various exceptions, defined as weak so that they may be replaced
//by user code. These default handlers just go into infinite loops. Reset and
//IRQ are handled by real (strong) functions, as they do the right thing and
//it doesn't make sense for user code to replace them.  
void __attribute__ ((weak)) __attribute__ ((noreturn)) Undef_Handler(void) {for(;;);}
void __attribute__ ((weak)) __attribute__ ((noreturn)) SWI_Handler(void) {for(;;);}
void __attribute__ ((weak)) __attribute__ ((noreturn)) PAbt_Handler(void) {for(;;);}
void __attribute__ ((weak)) __attribute__ ((noreturn)) DAbt_Handler(void) {for(;;);}
void __attribute__ ((weak)) __attribute__ ((noreturn)) FIQ_Handler(void) {for(;;);}
/**This fills the vtable slots of a class where the method is abstract. Why not 
just zero? That would cause a spontaneous reset on an ARM processor, and "Thou
shalt not follow the NULL pointer, for chaos and madness await thee at its 
end." But, if you call an abstract method, you get what you deserve.
Defined as weak so that if you want to write a function that beats the 
programmer about the head when called, you can. */
extern "C" void __attribute__ ((weak)) __attribute__ ((noreturn)) __cxa_pure_virtual() { for(;;); }

//Sketch main routines. We actually put the symbol weakness to work here - we
//expect to replace these functions.
void __attribute__ ((weak)) setup(void) {}
void __attribute__ ((weak)) loop(void) {}

//No setup code because stack isn't available yet
//No cleanup code because function won't return (what would it return to?)
void __attribute__ ((naked))            Reset_Handler(void); 
//IRQ wrapper handles proper return from IRQ so that the actual handlers
//can be just void foo(void) without needing their own __attribute__
void __attribute__ ((interrupt("IRQ"))) IRQ_Wrapper(void);

void __attribute__ ((naked)) __attribute__ ((section(".vectors"))) vectorg(void) {
  asm("ldr pc,[pc,#24]\n\t"
      "ldr pc,[pc,#24]\n\t"
      "ldr pc,[pc,#24]\n\t"
      "ldr pc,[pc,#24]\n\t"
      "ldr pc,[pc,#24]\n\t"
      ".word 0xb8a06f60\n\t" //NXP checksum, constant as long as the other 7 instructions in first 8 are constant
      "ldr pc,[pc,#24]\n\t"
      "ldr pc,[pc,#24]\n\t"
      ".word _Z13Reset_Handlerv\n\t"   //Startup code location
      ".word _Z13Undef_Handlerv\n\t"   //Undef
      ".word _Z11SWI_Handlerv\n\t"   //SWI
      ".word _Z12PAbt_Handlerv\n\t"   //PAbt
      ".word _Z12DAbt_Handlerv\n\t"   //DAbt
      ".word 0\n\t"               //Reserved (hole in vector table above)
      ".word _Z11IRQ_Wrapperv\n\t"     //IRQ (wrapper so that normal C functions can be installed in VIC)
      ".word _Z11FIQ_Handlerv"); //FIQ
}

void setup_mam(void) {
  // Enabling MAM and setting number of clocks used for Flash memory fetch (4 cclks in this case)
  //MAMTIM=0x3; //VCOM?
  MAMCR=0x2;
  MAMTIM=0x4; //Original
}

#define __set_cpsr_c(val) asm volatile (" msr  cpsr_c, %0" : /* no outputs */ : "r" (val)  );	
#define __set_sp(val) asm volatile (" mov  sp, %0" : : "r" (val))
#define setModeStack(tag,iflags) \
  __set_cpsr_c(Mode_##tag | iflags); \
  __set_sp((int)(&(Stack_##tag[tag##_Stack_Size/sizeof(int)]))); \
  for(unsigned int i=0;i<(tag##_Stack_Size/sizeof(int));i++) Stack_##tag[i]=stackPattern

void Reset_Handler() {
  //Set up stacks...
  setModeStack(UND,I_Bit | F_Bit);  //...for Undefined Instruction mode
  setModeStack(ABT,I_Bit | F_Bit);  //...for Abort mode
  setModeStack(FIQ,I_Bit | F_Bit);  //...for FIQ mode
  setModeStack(IRQ,I_Bit | F_Bit);  //...for IRQ mode
  setModeStack(SVC,I_Bit | F_Bit);  //...for Supervisor mode
  setModeStack(USR,0);  //...for User and System mode

//Now stay in system mode for good                

// Relocate .data section (initialized variables)
  for(int i=0;i<(edata-data);i++) data[i]=etext[i];

//Initialize .bss section 
  for(int i=0;i<(bss_end-bss_start);i++) bss_start[i]=0;

  //Set up system peripherals
  setup_pll(0,5); //Set up CCLK PLL to 5x crystal rate
  setup_mam();
  setup_clock();

// call C++ constructors of global objects
//Each pointer is the pointer to a short block of code, and appears to be a static
//function which which constructs all the global objects in the same compilation unit.
//This code is free to call constructors and/or perform inline constructors directly.
//As a static function, the pointee code uses BX lr to return
  for(int i=0;i<ctors_end-ctors_start;i++) ctors_start[i]();

  IRQHandler::begin(); //Can't call before ctors are run

//We have finally broken the tyranny of main(). Directly call user's setup()
  setup();
//Directly call user's loop();
  for(;;) loop();
}

//Now we see why we like high-level languages for IRQ handling. It says treat
//the number as a pointer to a function and call it.
void __attribute__ ((interrupt("IRQ"))) IRQ_Wrapper() {
  ((fvoid)(VICVectAddr))(); 
}



#ifndef Startup_h
#define Startup_h

//Part of the reset which can be run on the host
void reset_handler_core();

//The part of the startup code which is processor-independent
void __attribute__ ((noreturn)) init();

/** Sketch setup routine, called once after the system has set itself up but before
loop() . This routine is typically used by user code to set up peripherals. */
void setup();
/** Sketch loop routine, called repeatedly after setup() . This typically performs
the main work of the program. */
void loop();

/**Handlers for various exceptions, defined as weak so that they may be replaced
by user code. These default handlers just go into infinite loops. Reset and
IRQ are handled by real (strong) functions, as they do the right thing and
it doesn't make sense for user code to replace them. We save a trivial amount
of space by re-using the code for Undef_Handler for all the exceptions. Because
these routines do not return, they don't care about proper exception returns,
but any real functions will need to be __attribute__((interrupt("FIQ"))) etc. */  
extern "C" void __attribute__ ((weak)) __attribute__ ((noreturn)) Undef_Handler(void);
extern "C" void __attribute__ ((weak)) __attribute__ ((noreturn)) SWI_Handler(void);
extern "C" void __attribute__ ((weak)) __attribute__ ((noreturn)) PAbt_Handler(void);
extern "C" void __attribute__ ((weak)) __attribute__ ((noreturn)) DAbt_Handler(void);
extern "C" void __attribute__ ((weak)) __attribute__ ((noreturn)) FIQ_Handler(void);
/**This fills the vtable slots of a class where the method is abstract. Why not
just zero? That would cause a spontaneous reset on an ARM processor, and "Thou
shalt not follow the NULL pointer, for chaos and madness await thee at its
end." But, if you call an abstract method, you get what you deserve.
Defined as weak so that if you want to write a function that beats the
programmer about the head when called, you can. */
extern "C" void __attribute__ ((weak)) __attribute__ ((noreturn)) __cxa_pure_virtual();

#endif

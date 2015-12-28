#ifndef Startup_h
#define Startup_h

void reset_handler_core();
/** Sketch setup routine, called once after the system has set itself up but before
loop() . This routine is typically used by user code to set up peripherals. */
void setup();
/** Sketch loop routine, called repeatedly after setup() . This typically performs
the main work of the program. */
void loop();

#endif

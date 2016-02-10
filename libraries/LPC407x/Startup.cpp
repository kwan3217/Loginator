#include "Startup.h"
#include "registers.h"
/** 
  \file Startup.cpp
  \brief Startup code for LPC407x/408x, run at reset. 

The Cortex-M4 has a quite different reset model than the ARM7TDMI. Instead of 
a vector table consisting of a series of actual instructions, the table consists
of addresses to jump to, similar to the Intel x86 interrupt vector table. The
first address of the table is the starting value for the stack pointer, the next
is the address to go to after reset, and the rest of the table is the address
to jump to after any particular exception or IRQ.

0x00 Starting SP value
0x04 reset handler start address
0x08 NMI handler
0x0C Hard Fault handler
0x10 Memory Management fault handler
0x14 Bus Fault handler
0x18 Usage Fault handler
...reserved...
0x2C SVCall
...reserved...
0x38 PendSV
0x3C SysTick
0x40+4n IRQn

This largely replaces the VIC in the ARM7TDMI. 

*/

/**Pointer to byte just above end of RAM. Put the stack here. The ABI in use 
specifies a "Full-Decrementing" stack, which means that the stack pointer points
at a value which has data (is "Full") and that you decrement the stack pointer
when you push. The "Full" part means that the stack is predecrement, so the 
first push will first decrement the stack pointer from _ram_end, which actually
points past the RAM, and isn't allowed to actually hold a stack value. The value
will then be written at the new location of the stack pointer, which is now in
RAM. */ 
extern int _ram_end[]; 

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
__attribute__ ((section(".vectors"))) const void* vectorg[] {
  &_ram_end,      //0x00 Starting SP value
  (void*)&init,          //0x04 reset handler start address
  nullptr,        //0x08 NMI handler
  nullptr,        //0x0C Hard Fault handler
  nullptr,        //0x10 Memory Management fault handler
  nullptr,        //0x14 Bus Fault handler
  nullptr,        //0x18 Usage Fault handler
  nullptr,nullptr,nullptr,nullptr, //0x1C-0x28, reserved
  nullptr,        //0x2C SVCall
  nullptr,nullptr, //0x30-0x34, reserved
  nullptr,        //0x38 PendSV
  nullptr,        //0x3C SysTick
  nullptr         //0x40+4n IRQn
};



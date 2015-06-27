#ifndef HARDWARE_STACK_H
#define HARDWARE_STACK_H

//Stack sizes are in bytes
const unsigned int UND_Stack_Size=0;  //No stack for you! You are just an infinite loop
const unsigned int SVC_Stack_Size=0;  //Likewise
const unsigned int ABT_Stack_Size=0;  //Likewise
const unsigned int FIQ_Stack_Size=128;
const unsigned int IRQ_Stack_Size=512;
const unsigned int USR_Stack_Size=0;

#endif

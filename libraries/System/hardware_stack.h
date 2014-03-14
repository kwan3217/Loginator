#ifndef HARDWARE_STACK_H
#define HARDWARE_STACK_H

//Stack sizes are in bytes
const unsigned int UND_Stack_Size=0;  //No stack for you! You are just an infinite loop
const unsigned int SVC_Stack_Size=0;  //Likewise
const unsigned int ABT_Stack_Size=0;  //Likewise
const unsigned int FIQ_Stack_Size=128;
const unsigned int IRQ_Stack_Size=512;
const unsigned int USR_Stack_Size=2048;

const int stackPattern=0x6E61774B; // Appears as "Kwan" in little-endian ascii

extern int Stack_UND[UND_Stack_Size/sizeof(int)];
extern int Stack_SVC[SVC_Stack_Size/sizeof(int)];
extern int Stack_ABT[ABT_Stack_Size/sizeof(int)];
extern int Stack_FIQ[FIQ_Stack_Size/sizeof(int)];
extern int Stack_IRQ[IRQ_Stack_Size/sizeof(int)];
extern int Stack_USR[USR_Stack_Size/sizeof(int)];

//Each one returns number of bytes on each stack which have never been touched
//rounded down to nearest multiple of 4. Negative if every byte has been touched
int checkFIQStack();
int checkIRQStack();
int checkUSRStack();

#endif

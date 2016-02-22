/* System Control Block(SCB) modules include Memory Accelerator Module,
Phase Locked Loop, VPB divider, Power Control, External Interrupt,
Reset, and Code Security/Debugging */

#if MCU == MCU_ARM7TDMI
#define SCB_BASE_ADDR 0xE01FC000
#endif //MCU_ARM7TDMI

#if MCU == MCU_CORTEXM4
#define SCB_BASE_ADDR 0x400FC000
#endif //MCU_CORTEXM4

rw0(scb,MAMCR   ,SCB_BASE_ADDR+0x000);
rw0(scb,MAMTIM  ,SCB_BASE_ADDR+0x004);
rw0(scb,MEMMAP  ,SCB_BASE_ADDR+0x040);

/* Phase Locked Loop (PLL) */
rw1(scb,PLLCON ,2,SCB_BASE_ADDR+0x080+0x20*i);
rw1(scb,PLLCFG ,2,SCB_BASE_ADDR+0x084+0x20*i);
ro1(scb,PLLSTAT,2,SCB_BASE_ADDR+0x088+0x20*i);
wo1(scb,PLLFEED,2,SCB_BASE_ADDR+0x08C+0x20*i);

/* Power Control */
rw0(scb,PCON    ,SCB_BASE_ADDR+0x0C0);
rw0(scb,PCONP   ,SCB_BASE_ADDR+0x0C4);

/* VPB Divider (APB Divider in 2015 version of user manual) */
rw0(scb,VPBDIV  ,SCB_BASE_ADDR+0x100);

/* External Interrupts */
rw0(scb,EXTINT  ,SCB_BASE_ADDR+0x140);
rw0(scb,INTWAKE ,SCB_BASE_ADDR+0x144);
rw0(scb,EXTMODE ,SCB_BASE_ADDR+0x148);
rw0(scb,EXTPOLAR,SCB_BASE_ADDR+0x14C);

/* Reset (RSID in 2015 version of user manual)*/
rw0(scb,RSIR    ,SCB_BASE_ADDR+0x180);

/* Code Security/Debugging */
rw0(scb,CSPR    ,SCB_BASE_ADDR+0x184);

/* System Controls and Status */
rw0(scb,SCS     ,SCB_BASE_ADDR+0x1A0);

#undef SCB_BASE_ADDR

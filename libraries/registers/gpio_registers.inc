/* Universal Asynchronous Receiver Transmitter */
#if MCU == MCU_ARM7TDMI
#define PINSEL_BASE_ADDR 0xE002C000
#define GPIO_BASE_ADDR 0xE0028000

/* General Purpose Input/Output (GPIO) */
rw1(gpio,IOPIN,2,GPIO_BASE_ADDR+0x00+i*0x10);
rw1(gpio,IOSET,2,GPIO_BASE_ADDR+0x04+i*0x10);
rw1(gpio,IODIR,2,GPIO_BASE_ADDR+0x08+i*0x10);
wo1(gpio,IOCLR,2,GPIO_BASE_ADDR+0x0C+i*0x10);

/* Pin Connect Block */
rw1(gpio,PINSEL,3,PINSEL_BASE_ADDR+(i==0?0x00:(i==1?0x04:0x14)));

#undef PINSEL_BASE_ADDR
#undef GPIO_BASE_ADDR

#endif //MCU_ARM7TDMI

#if MCU == MCU_CORTEXM4
#define GPIO_BASE_ADDR   0x2009'8000
#define PINSEL_BASE_ADDR 0x4002'C000

rw2(gpio,IOCON,6,32,PINSEL_BASE_ADDR+i*0x04+j*0x80);

rw1(gpio,IODIR ,6,GPIO_BASE_ADDR+i*0x20+0x00);
rw1(gpio,IOMASK,6,GPIO_BASE_ADDR+i*0x20+0x10);
rw1(gpio,IOPIN ,6,GPIO_BASE_ADDR+i*0x20+0x14);
rw1(gpio,IOSET ,6,GPIO_BASE_ADDR+i*0x20+0x18);
rw1(gpio,IOCLR ,6,GPIO_BASE_ADDR+i*0x20+0x1C);

#endif



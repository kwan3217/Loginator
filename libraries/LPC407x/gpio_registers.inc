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


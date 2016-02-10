/* D/A Converter */
#define DAC_BASE_ADDR 0xE006C000
rw0(dac,DACR,DAC_BASE_ADDR+0x00);

#undef DAC_BASE_ADDR

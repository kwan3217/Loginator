#define SPI0_BASE_ADDR 0xE0020000

rw0(spi,S0SPCR ,SPI0_BASE_ADDR+0x00);
ro0(spi,S0SPSR ,SPI0_BASE_ADDR+0x04);
rw0(spi,S0SPDR ,SPI0_BASE_ADDR+0x08);
rw0(spi,S0SPCCR,SPI0_BASE_ADDR+0x0C);
rw0(spi,S0SPINT,SPI0_BASE_ADDR+0x1C);

#undef SPI0_BASE_ADDR
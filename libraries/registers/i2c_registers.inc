#define I2C0_BASE_ADDR 0xE001C000
#define I2C1_BASE_ADDR 0xE005C000

rw1(i2c,I2CCONSET,2,I2C0_BASE_ADDR+(I2C1_BASE_ADDR-I2C0_BASE_ADDR)*i+0x00);
ro1(i2c,I2CSTAT  ,2,I2C0_BASE_ADDR+(I2C1_BASE_ADDR-I2C0_BASE_ADDR)*i+0x04);
rw1(i2c,I2CDAT   ,2,I2C0_BASE_ADDR+(I2C1_BASE_ADDR-I2C0_BASE_ADDR)*i+0x08);
rw1(i2c,I2CADR   ,2,I2C0_BASE_ADDR+(I2C1_BASE_ADDR-I2C0_BASE_ADDR)*i+0x0C);
rw1(i2c,I2CSCLH  ,2,I2C0_BASE_ADDR+(I2C1_BASE_ADDR-I2C0_BASE_ADDR)*i+0x10);
rw1(i2c,I2CSCLL  ,2,I2C0_BASE_ADDR+(I2C1_BASE_ADDR-I2C0_BASE_ADDR)*i+0x14);
wo1(i2c,I2CCONCLR,2,I2C0_BASE_ADDR+(I2C1_BASE_ADDR-I2C0_BASE_ADDR)*i+0x18);

#undef I2C0_BASE_ADDR
#undef I2C1_BASE_ADDR

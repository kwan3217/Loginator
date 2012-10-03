#ifndef setup_h
#define setup_h

//Obtained from reading high-frequency crystal markings on Logomatic v2
#define FOSC 12000000

void setup(void);
extern unsigned int CCLK;
extern unsigned int PCLK;
void set_pin(int pin, int mode);

#endif

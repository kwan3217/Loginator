#ifndef setup_h
#define setup_h

//Obtained from reading high-frequency crystal markings on Logomatic v2
const unsigned int FOSC=12000000;
const unsigned int designCCLK=FOSC*5; //Intended CCLK for constructors and things
                                      //set up before the PLL is running

void setup(void);
extern unsigned int CCLK;
extern unsigned int PCLK;
void set_pin(int pin, int mode);

#endif

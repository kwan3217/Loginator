#include "sim.h"
#include "Startup.h"


SimUart uart;
SimGpio gpio;
SimTimer timer;
SimRtc rtc;
SimPwm pwm;
SimAdc adc;
SimPeripherals peripherals(gpio,uart,timer,rtc,pwm,adc);

/* Since in this simulation, only blinklock calls delay, and blinklock only gets
   called at the end, we will just punch out when we get called here.*/
void delay(unsigned int ms) {
  exit(0);
}

int main(int argc, char** argv) {
  setvbuf(stdout, NULL, _IONBF, 0);
  reset_handler_core(); //Do the stuff that the embedded reset_handler would do

  setup(); //Run robot setup code
  //matrixTest does everything in setup() so we don't even run loop(), just exit
  for(;;) {
    loop();
  }
}

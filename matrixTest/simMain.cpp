#include "sim.h"
#include "Startup.h"


SimUart uart;
SimGpio gpio;
SimTimer timer;
SimRtc rtc;
SimPwm pwm;
SimAdc adc;
SimPeripherals peripherals(gpio,uart,timer,rtc,pwm,adc);

/* Simulate a delay by advancing the playbackState clock the right number of ticks */
void delay(unsigned int ms) {
  timer.advance(0,ms*60000);
  fprintf(stderr,"Delay %d ms, ttc now equals %u\n",ms,timer.read_TTC(0));
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

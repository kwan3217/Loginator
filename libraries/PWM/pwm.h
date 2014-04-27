#ifndef pwm_h
#define pwm_h

/** Set up and start the pulse width modulator.
@param prescale Number of PCLK ticks per PWM tick
@param period Number of PWM ticks in PWM cycle
*/
void setup_pwm(const unsigned int prescale, const unsigned int period);
void setup_pwm(const unsigned int period);
void servoWrite(const int channel, const signed char val);

#endif

#include "pwm.h"
#include "Serial.h"
#include "LPC214x.h"

void setup() {
  setup_pwm(16667); //60Hz
  servoWrite(4,-64);
  Serial.begin(38400);
}

void loop() {
  unsigned int pwmpr=PWMPR;
  unsigned int pwmpc=PWMPC;
  unsigned int pwmtcr=PWMTCR;
  unsigned int pwmtc=PWMTC;
  unsigned int pwmir=PWMIR;
  unsigned int pwmmr0=PWMMR(0);
  unsigned int pwmmr4=PWMMR(4);
  unsigned int pwmmcr=PWMMCR;
  unsigned int pwmler=PWMLER;
  unsigned int pwmpcr=PWMPCR;
  unsigned int io0dir=IODIR(0);
  unsigned int io0pin=IOPIN(0);
  Serial.println("---");
  Serial.print("PWMPR:  0x");
  Serial.println(pwmpr, HEX,8);
  Serial.print("PWMPC:  0x");
  Serial.println(pwmpc, HEX,8);
  Serial.print("PWMTCR: 0x");
  Serial.println(pwmtcr,HEX,8);
  Serial.print("PWMTC:  0x");
  Serial.println(pwmtc, HEX,8);
  Serial.print("PWMIR:  0x");
  Serial.println(pwmir, HEX,8);
  Serial.print("PWMMR0:  0x");
  Serial.println(pwmmr0, HEX,8);
  Serial.print("PWMMR4:  0x");
  Serial.println(pwmmr4, HEX,8);
  Serial.print("PWMMCR:  0x");
  Serial.println(pwmmcr, HEX,8);
  Serial.print("PWMLER:  0x");
  Serial.println(pwmler, HEX,8);
  Serial.print("PWMPCR:  0x");
  Serial.println(pwmpcr, HEX,8);
  Serial.print("IO0DIR:  0x");
  Serial.println(io0dir, HEX,8);
  Serial.print("IO0pin:  0x");
  Serial.println(io0pin, HEX,8);
}

#include "Time.h"
#include "gpio.h"
#include "LPCduino.h"
#include "Serial.h"

void setup() {
  Serial.begin(57600);
  Serial.println("Start Pinewood");
  Serial.println("TC         aft  fwd");
}

void loop() {
  int TC=TTC(0);
  int aft=analogRead(5);
  int fwd=analogRead(6);
  Serial.print(TC,DEC,10);
  Serial.print(" ");
  Serial.print(aft,DEC,4);
  Serial.print(" ");
  Serial.println(fwd,DEC,4);
//  delay(100);
}

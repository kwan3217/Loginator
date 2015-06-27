#include "gpio.h"
#include "Time.h"

void setup() {
}

void loop() {
  static int lightSwitch=0;
  set_light(0,lightSwitch);
  lightSwitch=1-lightSwitch;
  delay(75);
}



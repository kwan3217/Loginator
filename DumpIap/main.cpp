#include "Time.h"
#include "Serial.h"
#include "LPCduino.h"
#include "dump.h"

void setup() {
  Serial.begin(38400);
  setup_clock();
  IntelHex dump(Serial);
  dump.region((char*)0x7D000,0x7D000,12*1024,32);
}

void loop() {
}

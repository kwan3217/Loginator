#include "Time.h"
#include "Serial.h"
#include "LPCduino.h"
#include "dump.h"

int __attribute__((__target__("thumb"))) thumb_fn(int a) {
  return a*2;
}

void setup() {
  Serial.begin(38400);
  setup_clock();
  IntelHex dump(Serial);
  dump.region((char*)0x7D000,0x7D000,12*1024,thumb_fn(16));
}

void loop() {
}

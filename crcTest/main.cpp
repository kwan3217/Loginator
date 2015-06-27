#include "Time.h"
#include "Serial.h"
#include "crc.h"

const uint32_t crc=crc32("some-id");

void setup() {
  Serial.begin(4800);
  Serial.print("The CRC is:");
  Serial.print((unsigned int)crc,HEX,8);
  Serial.print((unsigned int)0x12345678,HEX,8);
}

void loop() {
}

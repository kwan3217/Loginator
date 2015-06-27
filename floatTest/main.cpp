#include "float.h"
#include "Serial.h"
#include "crc.h"

void setup() {
  Serial.begin(4800);
  Serial.println((fp)2.0,6);
  Serial.println(sint(450),6);
  Serial.println(sin(45.0*PI/180.0),6);
  for(int i=1;i<=100;i++) {
    Serial.print(i/10.0,6);
    Serial.print(" ");
    Serial.println(Q_rsqrt(fp(i)/10.0),6);
  }
  Serial.println((unsigned int)crc32("Some text"),HEX,8);
}

void loop() {
}

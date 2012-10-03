#include "LPC214x.h"
#include "system.h"
#include "main.h"

void setup() {
  pinMode(1,OUTPUT);
  pinMode(2,INPUT);
}

void loop() {
  for(int i=0;i<1024;i++) {
    analogWrite(6,i);
    delay(2);
//    if(i==512) delay(5000);
//    if(i==1023) delay(5000);
  }

  for(int i=0;i<1024;i++) {
    analogWrite(6,1023-i);
    delay(2);
  }
}

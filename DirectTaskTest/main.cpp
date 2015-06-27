#include "gpio.h"
#include "DirectTask.h"

void step1Task(void* stuff) {
  static int pinState=0;
  set_light(0,pinState);
  pinState=1-pinState;
  directTaskManager.reschedule(1,30,0,step1Task,0);
}

void step2Task(void* stuff) {
  static int pinState=0;
  set_light(1,pinState);
  pinState=1-pinState;
  directTaskManager.reschedule(2,70,0,step2Task,0);
}

void step3Task(void* stuff) {
  static int pinState=0;
  set_light(2,pinState);
  pinState=1-pinState;
  directTaskManager.reschedule(3,130,0,step3Task,0);
}

void setup() {
  directTaskManager.begin();
  directTaskManager.schedule(1, 30,0,step1Task,0);
  directTaskManager.schedule(2, 70,0,step2Task,0);
  directTaskManager.schedule(3,130,0,step3Task,0);
}

void loop() {

}



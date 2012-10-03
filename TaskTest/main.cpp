#include "LPCduino.h"
#include "Serial.h"
#include "Task.h"
#include "float.h"

void step1Task(void* stuff);
void step2Task(void* stuff);
void step3Task(void* stuff);

void step1Task(void* stuff) {
  Serial.write('1');
  static int pinState=0;
  digitalWrite(1,pinState);
  pinState=1-pinState;
  taskManager.reschedule(5,0,step2Task,NULL);
}

void step2Task(void* stuff) {
  Serial.write('2');
  static int pinState=0;
  digitalWrite(2,pinState);
  pinState=1-pinState;
  taskManager.reschedule(25,0,step3Task,NULL);
}

void step3Task(void* stuff) {
  Serial.write('3');
  static int pinState=0;
  digitalWrite(3,pinState);
  pinState=1-pinState;
  taskManager.reschedule(200,0,step1Task,NULL);
}

void testTask(void* stuff) {
  static int lightState=0;
  set_light(0,(lightState & 1)>0);
  lightState=(lightState+1)%8;
  taskManager.reschedule(500,0,testTask,NULL);
}

void setup() {
  Serial.begin(115200);
  pinMode(1,OUTPUT);
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  taskManager.begin();
  taskManager.schedule(500,0,testTask,NULL);
  taskManager.schedule(200,0,step1Task,NULL);
}

void loop() {

}



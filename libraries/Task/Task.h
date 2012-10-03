#ifndef TASK_H
#define TASK_H

#include "Heap.h"

//Heaps are ideally suited for priority queues, especially since this 
//implementation uses no pointers or dynamic memory management, unusual for 
//advanced data structures.

typedef void (*taskfunc)(void*);

class Task {
public:
  Task(taskfunc Lf,void* Lstuff):f(Lf),stuff(Lstuff) {};
  Task():f((taskfunc)0),stuff((void*)0) {};
  taskfunc f;
  void* stuff;
};

class TaskManager: public Heap<unsigned int, Task> {
private:
  static const int timer=0;
  static void handleTimerISR();
  void handle();
  int scheduleCore(unsigned int ticks, taskfunc f, void* stuff);
  unsigned int freeze_tc;
  virtual int compare(unsigned int a, unsigned int b);
public:
  TaskManager() {};
  void begin();
//input:
//  ticks - how many ticks from now to fire this task
//  f     - task function to run
//  stuff - "stuff" pointer, a void pointer which points to whatever
//          the task wants. Typically a pointer to an object, so that
//          the task can call that object's methods.
//Return:
//  0 if all is ok, some negative error code if not.
//    -1: Task time is too far
//    -2: Task time is too near (I don't think this can happen)
//    -3: Task list is full
  int schedule(unsigned int ticks, taskfunc f, void* stuff);
  int reschedule(unsigned int ticks, taskfunc f, void* stuff);
  int schedule(unsigned int ms, unsigned int ticks, taskfunc f, void* stuff);
  int reschedule(unsigned int ms, unsigned int ticks, taskfunc f, void* stuff);
};

extern TaskManager taskManager;
#endif

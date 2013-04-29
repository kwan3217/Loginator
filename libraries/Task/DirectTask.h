#ifndef DIRECTTASK_H
#define DIRECTTASK_H

/* Task manager with each task assigned to a particular timer match channel.
Since there are a limited number of match channels, this is less flexible than
the heap implementation, but should be easier to get working properly. This
uses timer0, set up as normal with match 0 used to wrap around appropriately, 
therefore unavailable for timers. Matches 1, 2, and 3 are used for tasks 1,
2, and 3 respectively. */

typedef void (*taskfunc)(void*);

class DirectTask {
public:
  DirectTask(taskfunc Lf,void* Lstuff):f(Lf),stuff(Lstuff) {};
  DirectTask():f((taskfunc)0),stuff((void*)0) {};
  taskfunc f;
  void* stuff;
};

class DirectTaskManager {
private:
  DirectTask taskList[4]; //Allocate one for match channel 0 even though we can't use it.
                 //If we ever need more tasks, we will attach this to timer 1
                 //and perhaps PWM.
  int timer;
  static void handleTimerISR();
  void handle();
  int scheduleCore(unsigned int ch, unsigned int ticks, taskfunc f, void* stuff, unsigned int base);
  int schedule(unsigned int ch, unsigned int ticks, taskfunc f, void* stuff);
  int reschedule(unsigned int ch, unsigned int ticks, taskfunc f, void* stuff);
public:
  DirectTaskManager(int Ltimer):timer(Ltimer) {};
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
  int schedule(unsigned int ch, unsigned int ms, unsigned int ticks, taskfunc f, void* stuff);
  int reschedule(unsigned int ch, unsigned int ms, unsigned int ticks, taskfunc f, void* stuff);
};

extern DirectTaskManager directTaskManager;
#endif

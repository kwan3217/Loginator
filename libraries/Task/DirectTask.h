#ifndef DIRECTTASK_H
#define DIRECTTASK_H

typedef void (*taskfunc)(void*);

class DirectTask {
public:
  DirectTask(taskfunc Lf,void* Lstuff):f(Lf),stuff(Lstuff) {};
  DirectTask():f((taskfunc)0),stuff((void*)0) {};
  taskfunc f;
  void* stuff;
};

/** Task manager with each task assigned to a particular timer match channel.
Since there are a limited number of match channels, this is less flexible than
the heap implementation, but should be easier to get working properly. Matches
1, 2, and 3 are used for tasks 1, 2, and 3 respectively. Task 0 is unavailable.
The task manager requires exclusive access to the interrupt for the timer it
uses. In a sense that's the whole reason for this task manager, multiplexing
the interrupt handler to several tasks. It also has some convenience routines
for scheduling and rescheduling tasks.

This code has some support for being attached to several channels, but
support is incomplete. Only timer 0 can be used at present. Timer 0 is also
used as a general-purpose high-precision timer, and channel 0 is used to
reset the timer on a periodic basis. That use of channel 0 must be set
up elsewhere, before this task manager. No particular reset frequency is
assumed, it is up to the other code with no restrictions from here.*/
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
/**
Schedule a task a certain amount of time in the future, IE after this call. This is done
by setting up a task with the current value of TTC0 as the reference.
@param  ch     which timer channel to use, may be 1, 2, or 3
@param  ms     how many milliseconds from now to fire this task
@param  ticks  how many additional ticks from now to fire this task.
@param  f     task function to run
@param  stuff  "stuff" pointer, a void pointer which points to whatever
          the task wants. Typically a pointer to an object, so that
          the task can call that object's methods.
@Return
  0 if all is ok, some negative error code if not.
    -1: Task time is too far
    -2: Task time is too near (I don't think this can happen)
    -3: Task list is full */
  int schedule(unsigned int ch, unsigned int ms, unsigned int ticks, taskfunc f, void* stuff);
/**
Reschedule a task a certain amount of time after it was last called. This is done by setting up
a task with the value of TMR(channel) as a reference. Intended to be called
within the task function which is currently active, to reschedule itself, usually on a periodic
basis.
@param  ch     which timer channel to use, may be 1, 2, or 3
@param  ms     how many milliseconds from the last time this task ran to fire this task
@param  ticks  how many additional ticks to fire this task.
@param  f      task function to run
@param  stuff  "stuff" pointer, a void pointer which points to whatever
          the task wants. Typically a pointer to an object, so that
          the task can call that object's methods.
@Return
  0 if all is ok, some negative error code if not.
    -1: Task time is too far
    -2: Task time is too near (I don't think this can happen)
    -3: Task list is full */
  int reschedule(unsigned int ch, unsigned int ms, unsigned int ticks, taskfunc f, void* stuff);
};

extern DirectTaskManager directTaskManager;
#endif

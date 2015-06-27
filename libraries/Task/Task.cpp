#include "Task.h"
#include "Time.h"
#include "LPC214x.h"
#include "irq.h"

TaskManager taskManager;

static const int TIR_MR0=(1<<0);
static const int TIR_MR1=(1<<1);
static const int TIR_MR2=(1<<2);
static const int TIR_MR3=(1<<3);
static const int TIR_CR0=(1<<4);
static const int TIR_CR1=(1<<5);
static const int TIR_CR2=(1<<6);
static const int TIR_CR3=(1<<7);

void TaskManager::begin() {
  //Monopolize the timer's match channel 1 and interrupt
  IRQHandler::install(IRQHandler::TIMER0+timer,handleTimerISR);
  TMCR(timer) |= (1 << 3);     // On MR1, int but no reset
}

void TaskManager::handleTimerISR() {
  taskManager.handle();
  //ACK the timer
  TIR(timer)=TIR_MR1;
  //ACK the VIC
  VICVectAddr()=0;
}

//Check if time x is in between limits a and b
//accounts for "around the corner" case where b is off
//the end of the second and therefore less than a
inline bool between(unsigned int a,unsigned int x,unsigned int b) {
  if(a<b) {
    //Normal case - a is less than b
    return (x>=a) & (x<b);
  } else {
    //Around the corner case
    return (x>=a) | (x<b);
  }
}

void TaskManager::handle() {
  if(empty()) return; //If no tasks are pending, return immediately
  //I saw a nasty bug where two tasks ended up scheduled within ~500
  //ticks of eachother, about 10 us. The first task took longer than
  //10us to execute, so by the time the second task was queued, it 
  //had already expired and was effectively pushed into the future
  //by exactly 1 second. Since the first task rescheduled itself
  //exactly twice per second, it always came first and the second
  //task never got to run.
  //To fix this, put the task caller into a loop. Always do the first
  //one, then keep doing tasks until either the list is empty or the
  //next task is in the future.
  unsigned int old_freeze_tc=freeze_tc;
  freeze_tc=TMR(timer,1);
  do {
    taskfunc f=peek().elt.f; 
    void* stuff=peek().elt.stuff;
    pop(); //Get rid of this task
    f(stuff); //Call the task
    //So that if we loop again and the next task reschedules, it does so off
    //its own scheduled time, not the first scheduled time.
    if(!empty()) TMR(timer,1)=peek().pty; 
    //Condition below: While there is a task, and it is between the time the last task int fired and now
  } while(!empty() && between(freeze_tc,peek().pty,TTC(timer)));
  freeze_tc=old_freeze_tc;
}

/**Schedule a task to fire a given number of ticks from when
schedule is called
\param ticks how many PCLK ticks from now to fire this task
\param f     task function to run
\param stuff "Stuff" pointer, a void pointer which points to whatever
          the task and the task caller agree to. Typically a pointer
          to an object, so that the task can call that object's methods.

\return 0 if all is ok, some negative error code if not.
    -1: Task time is too far
    -2: Task time is too near (I don't think this can happen)
    -3: Task list is full
*/
int TaskManager::schedule(unsigned int ticks, taskfunc f, void* stuff) {
  if(ticks>timerInterval) return -1;
  if(ticks+freeze_tc<freeze_tc) return -1;
  unsigned int old_freeze_tc=freeze_tc;
  freeze_tc=TTC(timer);
  int result=scheduleCore(TTC(timer)+ticks,f,stuff);
  freeze_tc=old_freeze_tc;
  return result;
}

//Schedule a task a at given number of milliseconds and ticks
int TaskManager::schedule(unsigned int ms, unsigned int ticks, taskfunc f, void* stuff) {
  return schedule(ms*(PCLK/1000)+ticks,f,stuff);
}

//Schedule a task to fire a given number of ticks from when the
//last task fired. Must be used inside of a task to accurately
//reschedule itself.
int TaskManager::reschedule(unsigned int ticks, taskfunc f, void* stuff) {
  if(ticks>timerInterval) return -1;
  if(ticks+freeze_tc<freeze_tc) return -1;
  unsigned int old_freeze_tc=freeze_tc;
  freeze_tc=TMR(timer,1);
  int result=scheduleCore(TMR(timer,1)+ticks,f,stuff);
  freeze_tc=old_freeze_tc;
  return result;
}

//Reschedule a task a given number of milliseconds and ticks
int TaskManager::reschedule(unsigned int ms, unsigned int ticks, taskfunc f, void* stuff) {
  return reschedule(ms*(PCLK/1000)+ticks,f,stuff);
}

int TaskManager::scheduleCore(unsigned int ticks, taskfunc f, void* stuff) {
  if(full()) return -3;
  if(ticks>timerInterval) ticks-=timerInterval;
  push(Task(f,stuff),ticks);
  //First unsigned int is first, whether new or old. Use it to set the MR, doesn't
  //matter if it is the same MR as before.
  TMR(timer,1)=peek().pty;
  //Really? That's it? Heaps rock!
  return 0;
}

int TaskManager::compare(unsigned int a, unsigned int b) {
  if(a<freeze_tc) a+=timerInterval;
  if(b<freeze_tc) b+=timerInterval;
  if(a<b) return -1;
  if(a==b) return 0;
  /*if(a>b)*/ return 1;
}



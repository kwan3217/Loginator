#include "DirectTask.h"
#include "Time.h"
#include "LPC214x.h"
#include "irq.h"
#include "gpio.h"
#ifdef DEBUG
#include "Serial.h"
#endif

DirectTaskManager directTaskManager(0);

static const int TIR_MR0=(1<<0);
static const int TIR_MR1=(1<<1);
static const int TIR_MR2=(1<<2);
static const int TIR_MR3=(1<<3);
#define TIR_MR(i) (1<<(i))
static const int TIR_CR0=(1<<4);
static const int TIR_CR1=(1<<5);
static const int TIR_CR2=(1<<6);
static const int TIR_CR3=(1<<7);
#define TIR_CR(i) (1<<((i)+4))

void DirectTaskManager::begin() {
  //Monopolize the timer's interrupt
  IRQHandler::install(IRQHandler::TIMER0+timer,handleTimerISR);
}

void DirectTaskManager::handleTimerISR() {
  directTaskManager.handle();
  //ACK the VIC
  VICVectAddr=0;
}

void DirectTaskManager::handle() {
//  flicker();
  unsigned int tir_in=TIR(timer);
  for(unsigned int i=1;i<4;i++) if(tir_in&TIR_MR(i)) {
    taskfunc f=taskList[i].f;
    //De-schedule the task
    taskList[i].f=0;
    if(f!=0) f(taskList[i].stuff);
  }
  TIR(timer)=tir_in;
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

int DirectTaskManager::scheduleCore(unsigned int channel, unsigned int ticks, taskfunc f, void* stuff, unsigned int base) {
  //Set up the task
#ifdef DEBUG
  Serial.print("scheduleCore(channel=");Serial.print(channel,DEC);
  Serial.print(",ticks: ");Serial.print(ticks,DEC);
  Serial.print(",f=0x");Serial.print((unsigned int)f,HEX,8);
  Serial.print(",stuff=0x");Serial.print((unsigned int)stuff,HEX,8);
  Serial.print(",base=");Serial.print(base,DEC);
  Serial.println(")");
#endif
  taskList[channel].f=f;
  taskList[channel].stuff=stuff;
  //set the match value. 
  TMR(timer,channel)=(base+ticks)%(TMR0(timer)+1);
  //Don't reset or stop the timer on match
  TMCR(timer)&=~(6<<(channel*3));
  //Interrupt on match
  TMCR(timer)|=(1<<(channel*3));
  return 0;
}

int DirectTaskManager::schedule(unsigned int channel, unsigned int ticks, taskfunc f, void* stuff) {
  return scheduleCore(channel,ticks,f,stuff,TTC(timer));
}

//Schedule a task a at given number of milliseconds and ticks
int DirectTaskManager::schedule(unsigned int channel, unsigned int ms, unsigned int ticks, taskfunc f, void* stuff) {
#ifdef DEBUG
  Serial.print("schedule(channel=");Serial.print(channel,DEC);
  Serial.print(",ms: ");Serial.print(ms,DEC);
  Serial.print(",ticks: ");Serial.print(ticks,DEC);
  Serial.print(",f=0x");Serial.print((unsigned int)f,HEX,8);
  Serial.print(",stuff=0x");Serial.print((unsigned int)stuff,HEX,8);
  Serial.println(")");
#endif
  return schedule(channel,ms*(PCLK/1000)+ticks,f,stuff);
}

int DirectTaskManager::reschedule(unsigned int channel, unsigned int ticks, taskfunc f, void* stuff) {
  return scheduleCore(channel,ticks,f,stuff,TMR(timer,channel));
}

//Reschedule a task a given number of milliseconds and ticks
int DirectTaskManager::reschedule(unsigned int channel, unsigned int ms, unsigned int ticks, taskfunc f, void* stuff) {
#ifdef DEBUG
  Serial.print("reschedule(channel=");Serial.print(channel,DEC);
  Serial.print(",ms: ");Serial.print(ticks,DEC);
  Serial.print(",ticks: ");Serial.print(ticks,DEC);
  Serial.print(",f=0x");Serial.print((unsigned int)f,HEX,8);
  Serial.print(",stuff=0x");Serial.print((unsigned int)stuff,HEX,8);
  Serial.println(")");
#endif
  return reschedule(channel,ms*(PCLK/1000)+ticks,f,stuff);
}



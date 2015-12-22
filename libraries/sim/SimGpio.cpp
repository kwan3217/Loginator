#include "sim.h"

void SimGpio::addListener(SimGpioListener& listener, std::vector<int> Lpins0, std::vector<int> Lpins1) {
  listeners.push_back(&listener);
  pins0.push_back(Lpins0);
  pins1.push_back(Lpins1);
}

bool SimGpio::listenerCares(int index, int port, int pin) {
  std::vector<int>* thisPins;
  if(port==0) {
    thisPins=&pins0[index]; 
  } else {
    thisPins=&pins1[index];
  }
  for(int i=0;i<thisPins->size();i++) if(thisPins->at(i)==pin) return true;
  return false;
}

void SimGpio::write_IOPIN(int port, uint32_t value) {
  auto oldValue=IOPIN[port]; //Grab the old value so we can tell if things change
  SimSubGpio::write_IOPIN(port,value); //Write the register now just in case the listener tries to check the pins
  for(int pin=0;pin<32;pin++) {
    if((value & (1<<pin))!=(oldValue & (1<<pin))) { //if the pin has changed value AND...
      //Give each listener a chance to hear the pin
      for(int i=0;i<listeners.size();i++) {
        if(listenerCares(i,port,pin)) {                  //...the listener cares, then
          //Shift the register by the number of pins and mask off the lowest bit
          listeners[i]->pinOut(port,pin,(value >> pin) & 1);
        }
      }
    }
  }
}

void SimGpio::write_IOSET(int port, uint32_t value) {
  auto oldValue=IOPIN[port]; //Grab the old value so we can tell if things change
  //Set all the bits that are specified, do it now in case the listener checks the pins
  IOPIN[port]=IOPIN[port] | value;
  for(int pin=0;pin<32;pin++) {
    if((   value & (1<<pin))> 0 && //If the pin is requested to be set AND...
       (oldValue & (1<<pin))==0) { //...the pin is currently cleared then
      //Give each listener a chance to hear the pin
      for(int i=0;i<listeners.size();i++) {
        if(listenerCares(i,port,pin)) { //the listener cares, then 
          listeners[i]->pinOut(port,pin,1); //Let the listener know that the pin is set
        }
      }
    }
  }
}

void SimGpio::write_IOCLR(int port, uint32_t value) {
  auto oldValue=IOPIN[port]; //Grab the old value so we can tell if things change
  //Clear all the bits that are specified, do it now in case the listener checks the pins
  IOPIN[port]=IOPIN[port] & (0xFFFF'FFFF ^ value);
  for(int pin=0;pin<32;pin++) {
    if((   value & (1<<pin))>0 && //If the pin is requested to be cleared AND...
       (oldValue & (1<<pin))>0) { //...the pin is currently set then
      //Give each listener a chance to hear the pin
      for(int i=0;i<listeners.size();i++) {
        if(listenerCares(i,port,pin)) { //the listener cares, then 
          listeners[i]->pinOut(port,pin,0); //Let the listener know that the pin is cleared
        }
      }
    }
  }
}

void SimGpio::write_IODIR(int port, uint32_t value) {
  auto oldValue=IOPIN[port];
  SimSubGpio::write_IODIR(port,value);
  for(int pin=0;pin<32;pin++) {
    if((oldValue & (1<<pin)) != (value & (1<<pin))) { //If the pin has changed direction
      auto pinDir=(value & (1<<pin))>0; //Will be true if the selected bit is 1
      //Give each listener a chance to hear the pin
      for(int i=0;i<listeners.size();i++) {
        if(listenerCares(i,port,pin)) { //the listener cares, then 
          listeners[i]->pinMode(port,pin,pinDir); //Let the listener know that the pin is cleared
        }
      }
    }
  }
}

uint32_t SimGpio::read_IOPIN(int port) {
  //For each pin
  for(int pin=0;pin<32;pin++) {
    if((IODIR[port] & (1<<pin))==0) {//If the given pin is input then
      //Give each listener a chance to drive the pin
      for(int i=0;i<listeners.size();i++) if(listenerCares(i,port,pin)) {
        //BitAND the value with a mask that has all but current pin set
        IOPIN[port]=(IOPIN[port] & (0xFFFF'FFFF ^ (1<<pin))) |
        //then BitOR it with the driven value shifted to the right position
                    (listeners[i]->pinIn(port,pin) << pin)   ;
      }
    }
  }
  return IOPIN[port];
}



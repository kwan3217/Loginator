#include "LPC214x.h"
#include "pktwrite.h"
#include "sdbuf.h"
#include "compass.h"
#include "gyro.h"
#include "acc.h"
#include "pinger.h"
#include "serial.h"
#include "main.h"

sensor Acc,Bfld,Gyro,Ping;
sensor* const sensorQuick[]={&Acc,&Gyro,&Ping};
sensor* const sensorSlow[]={&Bfld};
int slowCount;

void writeSensorGuts(sensor* this, circular* buf) {
  buf->dataComma=1;
  buf->dataDec=1;
  buf->dataDigits=6;
  fillPktInt(buf,this->TC/60);        
  buf->dataDigits=2;
  fillPktByte(buf,this->TC % 60);     
  buf->dataDigits=0;
  for(int i=0;i<this->n_ele;i++) fillPktShort(buf,this->dn[i]);
  for(int i=0;i<this->n_ele;i++) fillPktFP(buf,this->cal[i]);
}

void setupSensors(circular* buf) {
  setupAcc(buf);
  drainToSD(buf);
  setupCompass(buf);
  drainToSD(buf);
  setupGyro(buf);
  drainToSD(buf);
  setupPinger(buf);
  drainToSD(buf);
  slowCount=0;
}

int readAllSensors(circular* buf) {
  unsigned int TC=T0TC;
  int processSlow=0;
  for(int i=0;i<3;i++) {
    sensorQuick[i]->readSensor(sensorQuick[i],TC);
  }
  if(slowCount==10) {
    for(int i=0;i<1;i++) sensorSlow[i]->readSensor(sensorSlow[i],TC);
    processSlow=1;
    for(int i=0;i<1;i++) {
      sensorSlow[i]->calSensor(sensorSlow[i]);
      sensorSlow[i]->writeSensor(sensorSlow[i],buf);
      drainToSD(buf);
    }
    slowCount=0;
  } else {
    processSlow=0;
    slowCount++;
  }
  for(int i=0;i<3;i++) {
    sensorQuick[i]->calSensor(sensorQuick[i]);
    sensorQuick[i]->writeSensor(sensorQuick[i],buf);
    drainToSD(buf);
  }
  return processSlow;
}


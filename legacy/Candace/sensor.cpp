#include "LPC214x.h"
#include "circular.h"
#include "pktwrite.h"
#include "sdbuf.h"
#include "compass.h"
#include "gyro.h"
#include "acc.h"
#include "pinger.h"
#include "serial.h"
#include "main.h"

#define N_QUICK 3
#define N_SLOW 1
static sensor* const sensorQuick[N_QUICK]={&Acc,&Gyro,&Ping};
static sensor* const sensorSlow[N_SLOW]={&Bfld};
int slowCount;

void sensor::writeGuts(circular& buf) {
  buf.dataComma=1;
  buf.dataDec=1;
  buf.dataDigits=6;
  fillPktInt(buf,TC/60);
  buf.dataDigits=2;
  fillPktByte(buf,TC % 60);
  buf.dataDigits=0;
  for(int i=0;i<n_ele;i++) fillPktShort(buf,dn[i]);
  for(int i=0;i<n_ele;i++) fillPktFP(buf,cal[i]);
}

void setupSensors(circular& buf) {
  Acc.setup(buf);
  drainToSD(buf);
  Bfld.setup(buf);
  drainToSD(buf);
  Gyro.setup(buf);
  drainToSD(buf);
  Ping.setup(buf);
  drainToSD(buf);
  slowCount=0;
}

int readAllSensors(circular& buf) {
  unsigned int TC=T0TC;
  int processSlow=0;
  for(int i=0;i<N_QUICK;i++) {
    sensorQuick[i]->read(TC);
  }
  if(slowCount==10) {
    for(int i=0;i<1;i++) sensorSlow[i]->read(TC);
    processSlow=1;
    for(int i=0;i<N_SLOW;i++) {
      sensorSlow[i]->calibrate();
      sensorSlow[i]->write(buf);
      drainToSD(buf);
    }
    slowCount=0;
  } else {
    processSlow=0;
    slowCount++;
  }
  for(int i=0;i<N_QUICK;i++) {
    sensorQuick[i]->calibrate();
    sensorQuick[i]->write(buf);
    drainToSD(buf);
  }
  return processSlow;
}


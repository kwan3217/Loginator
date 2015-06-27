#include "script.h"
#include "LPC214x.h"
#include "compass.h"
#include "gyro.h"
#include "acc.h"
#include "IMU.h"
#include "sdbuf.h"
#include "pktwrite.h"
#include "main.h"
#include "pinger.h"
#include "sensor.h"
#include "control.h"
#include "motor.h"
#include <math.h>

static int avgGcount=0;
static int doAvgG=1;

//Average G accumulators
int naa;
static fp xaa=0,yaa=0,zaa=0;
int nba;
static fp xba=0,yba=0,zba=0;
int nga;
static fp xga=0,yga=0,zga=0;

circular sensorBuf;

int doneMotor=0;
int motorDir=0;
int motorCount=0;
int controlCount=0;
int motorMax=180;

static void scriptTimeToReadNormal(void) {
  int processSlow=readAllSensors(&sensorBuf);
  int result=46;
  if(Gyro.checkSensor(&Gyro)) result=imuUpdate(&Gyro);
  if(Acc.checkSensor(&Acc)) result=imuUpdate(&Acc);
  if(processSlow) {
    if(!doneMotor) {
      if(!motorDir) {
        if(motorCount>=motorMax) doneMotor=1; else motorCount+=5;
      }
    }
  //  if(result==0) result=imuUpdate(&Bfld);
        /*
    for(int i=0;i<13;i++) {
      for(int j=0;j<IMU_M;j++) {
        writeKalmanState(&spiBuf,i,j);
        drain(&spiBuf,&sdBuf);
        flushAsNeeded();
      }
    }*/

  }
  if(result>=0) { //If not, no sensors passed their sanity checks
    writeIMUstate(&sensorBuf,result);
    drainToSD(&sensorBuf);
    unsigned int TC=T0TC;
    controlCount++;
    if(controlCount>2000) controlCount=0;
    fp actual_pitch=b2i_comp(0,1,0,2); //sine of pitch is z component of
                                      //nose vector transformed from body
                                      //to inertial
    fp y=control(&yaw,controlCount<1000?1:-1,Gyro.cal[2],TC);
    fp p=control(&pitch,0,actual_pitch,TC);
    fillPktStart(&sensorBuf,PT_I2C);
    fillPktString(&sensorBuf,"Control");
    fillPktInt(&sensorBuf,TC/60);
    fillPktInt(&sensorBuf,TC%60);
    fillPktFP(&sensorBuf,Gyro.cal[2]);
    fillPktFP(&sensorBuf,y);
    fillPktFP(&sensorBuf,actual_pitch);
    fillPktFP(&sensorBuf,p);
    fillPktFinish(&sensorBuf);
    drainToSD(&sensorBuf);
    spinMotors(motorCount+y,motorCount-y,p);
  }
}

static void scriptTimeToReadAverageG(void) {
  int processSlow=readAllSensors(&sensorBuf);
  if(processSlow) {
    avgGcount++;
    if(avgGcount>30) {
      set_light(0,1);
      xba+=Bfld.cal[0];
      yba+=Bfld.cal[1];
      zba+=Bfld.cal[2];
      nba++;
    }
  }
  if(avgGcount>30) {
    xga+=Gyro.cal[0];
    yga+=Gyro.cal[1];
    zga+=Gyro.cal[2];
    nga++;

    xaa+=Acc.cal[0];
    yaa+=Acc.cal[1];
    zaa+=Acc.cal[2];
    naa++;
  }
  if(avgGcount>60) {
    doAvgG=0;
    set_light(0,0);

    Bix=xba/nba;
    Biy=yba/nba;
    Biz=zba/nba;
    xg_extra=xga/nga;
    yg_extra=yga/nga;
    zg_extra=zga/nga;
    xaa/=naa;
    yaa/=naa;
    zaa/=naa;
    extraAccScale=sqrt(xaa*xaa+yaa*yaa+zaa*zaa)/gz;
    //Calculate the original orientation
    int imuo=IMUOrient(xaa, yaa, zaa, //Point body       (acceleration measured in body)
                       0,   0,   1,   //Point reference  (+z axis, straight up)
                       Bix, Biy, Biz, //Toward body      (magnetic field measured in body)
                       0,   1,   0,&sensorBuf);  //Toward reference (+y axis, magnetic north)
    fillPktStart(&sensorBuf,PT_I2C);
    fillPktString(&sensorBuf,"AverageA");
    fillPktFP(&sensorBuf,xaa);
    fillPktFP(&sensorBuf,yaa);
    fillPktFP(&sensorBuf,zaa);
    fillPktFinish(&sensorBuf);
    drainToSD(&sensorBuf);
    fillPktStart(&sensorBuf,PT_I2C);
    fillPktString(&sensorBuf,"AverageB");
    fillPktFP(&sensorBuf,Bix);
    fillPktFP(&sensorBuf,Biy);
    fillPktFP(&sensorBuf,Biz);
    fillPktFinish(&sensorBuf);
    drainToSD(&sensorBuf);
    fillPktStart(&sensorBuf,PT_I2C);
    fillPktString(&sensorBuf,"AverageG");
    fillPktFP(&sensorBuf,xg_extra);
    fillPktFP(&sensorBuf,yg_extra);
    fillPktFP(&sensorBuf,zg_extra);
    fillPktFinish(&sensorBuf);
    drainToSD(&sensorBuf);
    fillPktStart(&sensorBuf,PT_I2C);
    fillPktString(&sensorBuf,"IMU Orient");
    fillPktInt(&sensorBuf,imuo);
    fillPktFP(&sensorBuf,k_IMUQ.xh.data[0]);
    fillPktFP(&sensorBuf,k_IMUQ.xh.data[1]);
    fillPktFP(&sensorBuf,k_IMUQ.xh.data[2]);
    fillPktFP(&sensorBuf,k_IMUQ.xh.data[3]);
    fillPktFinish(&sensorBuf);
    drainToSD(&sensorBuf);
  }
}

void scriptTimeToRead() {
  if(doAvgG) {
    scriptTimeToReadAverageG();
  } else {
    scriptTimeToReadNormal();
  }
}

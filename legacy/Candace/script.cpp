#include "script.h"
#include "LPC214x.h"
#include "compass.h"
#include "gyro.h"
#include "acc.h"
#include "imu.h"
#include "sdbuf.h"
#include "pktwrite.h"
#include "main.h"
#include "pinger.h"
#include "sensor.h"
#include "control.h"
#include "motor.h"
#include "loop.h"
#include <math.h>
#include "gps.h"

int avgGcount=0;
int doAvgG=1;

//Average G accumulators
int naa;
fp xaa=0,yaa=0,zaa=0;
int nba;
fp xba=0,yba=0,zba=0;
int nga;
fp xga=0,yga=0,zga=0;

circular sensorBuf;

int doneMotor=1;
int motorDir=0;
int motorCount=0;
int controlCount=0;
int motorMax=180;
int met0=-1;

const fp cdec=0.987459771; //Magnetic declination at Sparkfun 9.083deg (magnetic north is this far east of true north)
const fp sdec=0.157870834; //So if you have a certain magnetic heading, subtract this to get true heading

static void scriptTimeToReadNormal(void) {
  int processSlow=readAllSensors(sensorBuf);
  int result=46;
  if(Gyro.check()) result=imuUpdate(Gyro);
  //if(Acc.check()) result=imuUpdate(Acc);
  if(processSlow) {
    logFirmware();
    if(!doneMotor) {
      if(!motorDir) {
        spinMotors(motorCount,motorCount);
        if(motorCount>=motorMax) motorDir=1; else motorCount+=5;
      } else {
        spinMotors(motorCount,motorCount);
        if(motorCount<=0) doneMotor=1; else motorCount-=5;
      }
    }
  //  if(result==0) result=imuUpdate(Bfld);

  } 
  if(doneMotor && result<=0 && hasNewGPS) { //If not, no sensors passed their sanity checks
    hasNewGPS=0;
    if(met0<0 || met0>0xFFFFF) met0=met;
    writeIMUstate(sensorBuf,result);
    drainToSD(sensorBuf);
    unsigned int TC=T0TC;
    controlCount++;
    if(controlCount>2000) controlCount=0;
//    fp sy=Bfld.cal[0];
//    fp cy=Bfld.cal[1];
//    fp yl=1.0/sqrt(sy*sy+cy*cy);
    //The compass reads backwards! If the vehicle is pointed Y towards
    //+60deg, the compass reads cos(-60deg), sin(-60deg)=cos(60deg),-sin(60deg)
    //so flip the sy term to get the cos and sin of vehicle heading in reference
    //frame. Plus, the X and Y axes seem to be labeled backwards (Z is correct),
    //so reverse both. Net: reverse cy only
//    sy*=yl;
//    cy*=-yl;
    //Subtract off magnetic declination. Use 
    //cos(a+-b)=cos(a)*cos(b)-+sin(a)*sin(b)
    //sin(a+-b)=sin(a)*cos(b)+-cos(a)*sin(b)
    fp ce,se,cd,sd,ct,st;
    int spd;
    if(GPSspd>100) {
      ct=ccourse;
      st=scourse;
    } else {
      ct=1.0;
      st=0.0;
    }
    const int segLength=4;
    const int maxSpd=20;
    if((met-met0)<segLength*1) {
      cd=1;
      sd=0;
      spd=maxSpd;
    } else if((met-met0)<segLength*2) {
      cd=0;
      sd=1;
      spd=maxSpd;
    } else if((met-met0)<segLength*3) {
      cd=-1;
      sd=0;
      spd=maxSpd;
    } else if((met-met0)<segLength*4) {
      cd=0;
      sd=-1;
      spd=maxSpd;
    } else {
      cd=1;
      sd=0;
      spd=0;
    }
    //Course error is desired minus actual
    trigm(cd,ct,sd,st,&ce,&se);
    fp y=-yaw.control(0,se,TC);
    fillPktStart(sensorBuf,PT_I2C);
    fillPktString(sensorBuf,"Control");
    fillPktInt(sensorBuf,TC/60);
    fillPktInt(sensorBuf,TC%60);
    sensorBuf.dataDec=0;
    sensorBuf.dataDigits=8;
    fillPktInt(sensorBuf,met0);
    sensorBuf.dataDec=1;
    sensorBuf.dataDigits=0;
    fillPktInt(sensorBuf,met);
    fillPktInt(sensorBuf,met-met0);
    fillPktInt(sensorBuf,spd);
    fillPktFP(sensorBuf,y);
    fillPktInt(sensorBuf,GPSspd);
    fillPktInt(sensorBuf,GPScourse);
    fillPktFP(sensorBuf,ct);
    fillPktFP(sensorBuf,st);
    fillPktFP(sensorBuf,ce);
    fillPktFP(sensorBuf,se);
    fillPktFP(sensorBuf,cd);
    fillPktFP(sensorBuf,sd);
    fillPktFinish(sensorBuf);
    drainToSD(sensorBuf);
    spinMotors(spd,y);
  }
}

static void scriptTimeToReadAverageG(void) {
  int processSlow=readAllSensors(sensorBuf);
  if(processSlow) {
    logFirmware();
    avgGcount++;
    if(avgGcount>30) {
      set_light(2,1);
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
                       0,   1,   0,sensorBuf);  //Toward reference (+y axis, magnetic north)
    fillPktStart(sensorBuf,PT_I2C);
    fillPktString(sensorBuf,"AverageA");
    fillPktFP(sensorBuf,xaa);
    fillPktFP(sensorBuf,yaa);
    fillPktFP(sensorBuf,zaa);
    fillPktFinish(sensorBuf);
    drainToSD(sensorBuf);
    fillPktStart(sensorBuf,PT_I2C);
    fillPktString(sensorBuf,"AverageB");
    fillPktFP(sensorBuf,Bix);
    fillPktFP(sensorBuf,Biy);
    fillPktFP(sensorBuf,Biz);
    fillPktFinish(sensorBuf);
    drainToSD(sensorBuf);
    fillPktStart(sensorBuf,PT_I2C);
    fillPktString(sensorBuf,"AverageG");
    fillPktFP(sensorBuf,xg_extra);
    fillPktFP(sensorBuf,yg_extra);
    fillPktFP(sensorBuf,zg_extra);
    fillPktFinish(sensorBuf);
    drainToSD(sensorBuf);
    fillPktStart(sensorBuf,PT_I2C);
    fillPktString(sensorBuf,"IMU Orient");
    fillPktInt(sensorBuf,imuo);
    fillPktFP(sensorBuf,k_IMUQ.xh.data[0]);
    fillPktFP(sensorBuf,k_IMUQ.xh.data[1]);
    fillPktFP(sensorBuf,k_IMUQ.xh.data[2]);
    fillPktFP(sensorBuf,k_IMUQ.xh.data[3]);
    fillPktFinish(sensorBuf);
    drainToSD(sensorBuf);
  }
}

void scriptTimeToRead() {
  if(doAvgG) {
    set_light(2,1);
    scriptTimeToReadAverageG();
  } else {
    set_light(2,0);
    scriptTimeToReadNormal();
  }
}

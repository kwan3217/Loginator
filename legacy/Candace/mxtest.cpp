#include "float.h"
#include "matrix.h"
#include "IMU.h"
#include <stdio.h>
#include <math.h>
#include "hoststub.h"

circular buf;

fp noseData[3]={-1,0,0};
matrix nose={3,1,noseData};

void main() {
  initIMU();
  unsigned int TC=0;
  fp actual_pitch=b2i_comp(&nose,2); //sine of pitch is z component of
                                    //nose vector transformed from body
                                    //to inertial
  printf("%f",actual_pitch);
/*
  fp y=control(&yaw,0,0,TC);
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
  spinMotors(0,0,p);
*/
}

#ifndef sensor_h
#define sensor_h

#include "float.h"
#include "circular.h"
#include "kalman.h"

struct s_sensor;

//Physically read the sensor from its own hardware to this.dn[]
typedef void (*readSensorF)(struct s_sensor* this, unsigned int TC);
//Convert dn to cal
typedef void (*calSensorF)(struct s_sensor* this);
//Write a packet containing this sensors' data
typedef void (*writeSensorF)(struct s_sensor* this, circular* buf);
//Check reasonableness of this sensor
typedef int (*checkSensorF)(struct s_sensor* this);

typedef struct s_sensor {
  int n_ele,n_k,g_ofs;
  readSensorF readSensor;
  calSensorF calSensor;
  writeSensorF writeSensor;
  checkSensorF checkSensor;
  unsigned int TC;
  //On sensors which are naturally vector, dn[0]..dn[2] are x,y,z.
  //On sensors with a temperature, dn[3] is t
  short dn[4];
  //Calibrated units, always in SI, except for temperature which is degC
  fp cal[4];
  fp calLast[4];
  fp R[4];
  kalman_state* k;
  gfunc const *g;
  Ffunc const *H;
} sensor;

#define setupSensorFunc(s,a) \
s.readSensor=read##a; \
s.calSensor=cal##a; \
s.checkSensor=check##a; \
s.writeSensor=write##a

void writeSensorGuts(sensor* this, circular* buf);
void setupSensors(circular* buf);
//Returns 1 if slow sensors were read, 0 otherwise
int readAllSensors(circular* buf);

extern sensor Acc,Gyro,Bfld,Ping;

#endif
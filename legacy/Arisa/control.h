#ifndef control_h
#define control_h

#include "kalman.h"

typedef struct {
  kalMatrices(3,pid);
  kalman_state k_pid;
  kalMatrices(2,cmd);
  kalman_state k_cmd;
  fp Kp,Ki,Kd;
  int result1,result2;
} control_state;

fp control(control_state* this, fp command, fp actual, unsigned long TC);
int initControl(control_state* this, fp Kp, fp Ki, fp Kd);
int initControls(void);
void writeControl(control_state* this, char* name, circular *buf);

//Probably will end up static
int F_vel(matrix* F, matrix* x);
int Phi_vel(matrix* Phi,matrix* x);
fp g_vel(matrix* x);
int H_vel(matrix* H,matrix* x);
int F_pid(matrix* F, matrix* x);
int Phi_pid(matrix* Phi,matrix* x);
fp g_pid(matrix* x);
int H_pid(matrix* H,matrix* x);

extern control_state yaw,pitch;

#endif

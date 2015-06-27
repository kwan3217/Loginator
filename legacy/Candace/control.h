#ifndef control_h
#define control_h

#include "kalman.h"

class control_state {
public:
  fp Kp,Ki,Kd;
  kalMatrices(2,cmd);
  kalman_state k_cmd;
  kalMatrices(3,pid);
  kalman_state k_pid;
  int result1,result2;
  fp control(fp command, fp actual, unsigned long TC);
  control_state(fp Kp, fp Ki, fp Kd);
  void writeControl(char* name, circular& buf);
};

//Probably will end up static
int F_vel(matrix& F, const matrix& x);
int Phi_vel(matrix& Phi, const matrix& x);
fp g_vel(const matrix& x);
int H_vel(matrix& H,const matrix& x);
int F_pid(matrix& F, const matrix& x);
int Phi_pid(matrix& Phi, const matrix& x);
fp g_pid(const matrix& x);
int H_pid(matrix& H, const matrix& x);

extern control_state yaw,pitch;

#endif

#include "control.h"
#include "setup.h"
#include "pktwrite.h"

control_state yaw,pitch;

fp control(control_state* this, fp command, fp actual, unsigned long TC) {
  fp dt=ekf_update_time(&this->k_cmd,TC);
  fp P=command-actual;
  return this->Kp*P;

  this->result1=ekf_step(&this->k_cmd,dt,command,0.0001,g_vel,H_vel);
  this->result2=ekf_step(&this->k_pid,dt,actual-this->k_pid.xh.data[0],0.0001,g_pid,H_pid);
}

void writeControl(control_state* this, char* name, circular *buf) {
  fillPktStart(buf,PT_I2C);
  fillPktString(buf,"Cmd");
  fillPktString(buf,name);
  fillPktInt(buf,this->result1);
  fillPktInt(buf,this->k_cmd.lastUpdate/60);
  fillPktInt(buf,this->k_cmd.lastUpdate%60);
  for(int i=0;i<this->k_cmd.xh.row;i++) fillPktFP(buf,this->k_cmd.xh.data[i]);
  fillPktFinish(buf);
  fillPktStart(buf,PT_I2C);
  fillPktString(buf,"Pid");
  fillPktString(buf,name);
  fillPktInt(buf,this->result2);
  fillPktInt(buf,this->k_cmd.lastUpdate/60);
  fillPktInt(buf,this->k_cmd.lastUpdate%60);
  for(int i=0;i<this->k_cmd.xh.row;i++) fillPktFP(buf,this->k_cmd.xh.data[i]);
  fillPktFinish(buf);
}

int initControl(control_state* this, fp Kp, fp Ki, fp Kd) {
  int result=kalInit2(this->k_cmd,2,1,this->,cmd,F_vel,Phi_vel);
  if(result<0) return result*100-1;
  result=kalInit2(this->k_pid,3,1,this->,pid,F_pid,Phi_pid);
  if(result<0) return result*100-2;
  this->Kp=Kp;
  this->Ki=Ki;
  this->Kd=Kd;
  return 0;
}

int initControls() {
  int result=initControl(&yaw,-20,0,0);
  if(result<0) return result*100-1;
  result=initControl(&pitch,-1000,0,0);
  if(result<0) return result*100-2;
  return 0;
}

int F_vel(matrix* F, matrix* x) {
  F->data[0]=x->data[1];
  F->data[1]=0;
  return 0;

}

int Phi_vel(matrix* Phi,matrix* x) {
  for(int i=0;i<4;i++) Phi->data[i]=0;
  px(Phi,0,1,1);
  return 0;
}

fp g_vel(matrix* x) {
  return x->data[0];
}

int H_vel(matrix* H,matrix* x) {
  H->data[0]=1;H->data[1]=0;
  return 0;
}

int F_pid(matrix* F, matrix* x) {
  F->data[0]=x->data[2];
  F->data[1]=x->data[0];
  F->data[2]=0;
  return 0;

}

int Phi_pid(matrix* Phi,matrix* x) {
  for(int i=0;i<9;i++) Phi->data[i]=0;
  px(Phi,0,2,1);
  px(Phi,1,0,1);
  return 0;
}

fp g_pid(matrix* x) {
  return x->data[0];
}

int H_pid(matrix* H,matrix* x) {
  H->data[0]=1;H->data[1]=0;H->data[2]=0;
  return 0;
}




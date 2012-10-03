#include "control.h"
#include "setup.h"
#include "pktwrite.h"

control_state yaw(-500,0,0),pitch(-1000,0,0);

fp control_state::control(fp command, fp actual, unsigned long TC) {
  fp dt=k_cmd.update_time(TC);
  fp P=command-actual;
  return Kp*P;

  result1=k_cmd.step(dt,command,0.0001,g_vel,H_vel);
  result2=k_pid.step(dt,actual-k_pid.xh.data[0],0.0001,g_pid,H_pid);
}

void control_state::writeControl(char* name, circular& buf) {
  fillPktStart(buf,PT_I2C);
  fillPktString(buf,"Cmd");
  fillPktString(buf,name);
  fillPktInt(buf,result1);
  fillPktInt(buf,k_cmd.lastUpdate/60);
  fillPktInt(buf,k_cmd.lastUpdate%60);
  for(int i=0;i<k_cmd.xh.rows;i++) fillPktFP(buf,k_cmd.xh.data[i]);
  fillPktFinish(buf);
  fillPktStart(buf,PT_I2C);
  fillPktString(buf,"Pid");
  fillPktString(buf,name);
  fillPktInt(buf,result2);
  fillPktInt(buf,k_cmd.lastUpdate/60);
  fillPktInt(buf,k_cmd.lastUpdate%60);
  for(int i=0;i<k_cmd.xh.rows;i++) fillPktFP(buf,k_cmd.xh.data[i]);
  fillPktFinish(buf);
}

control_state::control_state(fp LKp, fp LKi, fp LKd):
  Kp(LKp),Ki(LKi),Kd(LKd),k_cmd kalInit(2,1,cmd,F_vel,Phi_vel), k_pid kalInit(3,1,pid,F_pid,Phi_pid)
{
}

int F_vel(matrix& F, const matrix& x) {
  F.data[0]=x.data[1];
  F.data[1]=0;
  return 0;
}

int Phi_vel(matrix& Phi, const matrix& x) {
  for(int i=0;i<4;i++) Phi.data[i]=0;
  Phi(0,1)=1;
  return 0;
}

fp g_vel(const matrix& x) {
  return x.data[0];
}

int H_vel(matrix& H,const matrix& x) {
  H.data[0]=1;H.data[1]=0;
  return 0;
}

int F_pid(matrix& F, const matrix& x) {
  F.data[0]=x.data[2];
  F.data[1]=x.data[0];
  F.data[2]=0;
  return 0;

}

int Phi_pid(matrix& Phi, const matrix& x) {
  for(int i=0;i<9;i++) Phi.data[i]=0;
  Phi(0,2)=1;
  Phi(1,0)=1;
  return 0;
}

fp g_pid(matrix& x) {
  return x.data[0];
}

int H_pid(matrix& H, const matrix& x) {
  H.data[0]=1;H.data[1]=0;H.data[2]=0;
  return 0;
}




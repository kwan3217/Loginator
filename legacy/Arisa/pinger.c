#include "pinger.h"
#include "i2c.h"
#include "pktwrite.h"
#include "main.h"
#include "adc.h"
#include "IMU.h"

const fp logoVCC=3.3;     //Voltage provided by the Logomatic
const fp pingVCC=3.3;     //Voltage received by the pinger

void readPinger(sensor* this, unsigned int TC) {
  this->TC=TC;

  convertBothADC(4,4);
  this->dn[0]=readoutADC0();
  this->dn[1]=readoutADC1();
}

void calPinger(sensor* this) {
  this->cal[0]=(this->dn[0]*logoVCC/1023);     //x now in volts
  this->cal[0]/=(pingVCC/512);                 //x now in inches
  this->cal[0]*=0.0254;                        //x now in meters
  this->cal[1]=this->dn[1]*logoVCC/1024*2.26;  //y in volts
}

int checkPinger(sensor* this) {
  return 1;
}

void writePinger(sensor* this, circular* buf) {
  //Write the results in a packet
  fillPktStart(buf,PT_I2C);
  fillPktString(buf,"PING"); //Transmit ID
  writeSensorGuts(this,buf);
  fillPktFinish(buf);
}

void setupPinger(circular *buf) {
  setupSensorFunc(Ping,Pinger);
  Ping.n_ele=2;
  Ping.n_k=1;
  Ping.g_ofs=11; 
  Ping.R[0]=0.0254*5*0.0254*5;//Gotta measure some real noise
  Ping.R[1]=Ping.R[0];
  Ping.R[2]=Ping.R[0];
  Ping.k=&k_IMUR;
  Ping.g=g_IMUR;
  Ping.H=H_IMUR;
}


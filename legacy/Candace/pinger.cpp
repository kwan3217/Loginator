#include "pinger.h"
#include "i2c_int.h"
#include "pktwrite.h"
#include "main.h"
#include "adc.h"
#include "imu.h"

const fp logoVCC=3.3;     //Voltage provided by the Logomatic
const fp pingVCC=3.3;     //Voltage received by the pinger
ping Ping;

void ping::read(unsigned int LTC) {
  TC=LTC;

  convertBothADC(4,4);
  dn[0]=readoutADC0();
  dn[1]=readoutADC1();
}

void ping::calibrate() {
  cal[0]=(dn[0]*logoVCC/1023);     //x now in volts
  cal[0]/=(pingVCC/512);                 //x now in inches
  cal[0]*=0.0254;                        //x now in meters
  cal[1]=dn[1]*logoVCC/1024*2.26;  //y in volts
}

void ping::write(circular& buf) {
  //Write the results in a packet
  fillPktStart(buf,PT_I2C);
  fillPktString(buf,"PING"); //Transmit ID
  sensor::writeGuts(buf);
  fillPktFinish(buf);
}

ping::ping():sensor(2,1,11,&k_IMUR,g_IMUR,H_IMUR) {
  R[0]=0.0254*5*0.0254*5;//Gotta measure some real noise;
  R[1]=R[0];
  R[2]=R[0];
}



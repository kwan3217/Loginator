#include "compass.h"
#include "i2c_int.h"
#include "pktwrite.h"
#include "nmeawrite.h"
#include "conparse.h"
#include "main.h"
#include "sensor.h"
#include "imu.h"

bfld Bfld;

char compassID[3];
char compassReading[6];

#define COMPASS_ADDR 0x1E

void bfld::read(unsigned int LTC) {
  TC=LTC;
  //Read the compass
  i2c0.tx_string(COMPASS_ADDR,"\x02\x01",2);
  i2c0.txrx_string(COMPASS_ADDR,"\x0A",1,compassID,3);
  i2c0.txrx_string(COMPASS_ADDR,"\x03",1,compassReading,6);
  
  //Set it back to single shot mode
  i2c0.tx_string(COMPASS_ADDR,"\x02\x01",2);
  dn[0]=compassReading[0]<<8 | compassReading[1];
  dn[1]=compassReading[2]<<8 | compassReading[3];
  dn[2]=compassReading[4]<<8 | compassReading[5];
}


void bfld::calibrate() {
  cal[0]=dn[0]+  3.5;
  cal[1]=dn[1]- 78.5;
  cal[2]=dn[2]+102.5;
}

void bfld::write(circular& buf) {
  //Write the results in a packet
  fillPktStart(buf,PT_I2C);
  buf.dataDec=0;
  fillPktByte(buf,COMPASS_ADDR); //Transmit ID, the device we think we are talking to
  buf.dataDigits=2;
  fillPktChar(buf,compassID[0]); //Receive ID, which proves that we are in fact talking to it
  buf.dataComma=0;
  fillPktChar(buf,compassID[1]);
  fillPktChar(buf,compassID[2]);
  writeGuts(buf);
  fillPktFinish(buf);
}

bfld::bfld():sensor(4,3,3,&k_IMUQ,g_IMUQ,H_IMUQ) {
  R[0]=20.0;
  R[1]=R[0];
  R[2]=R[0];
#ifdef HOST
  compassID[0]='K';
  compassID[1]='4';
  compassID[2]='3';
#endif
}

void bfld::setup(circular& buf) {
#ifndef HOST
  //Set it to single-shot mode
  char gyroBuf2[16];
  i2c0.tx_string(COMPASS_ADDR,"\x02\x01",2);
  //Read back all registers
  fillPktStart(buf,PT_I2C);
  fillPktString(buf,"HMC");
  buf.dataDec=0;
  buf.dataDigits=2;
  fillPktByte(buf,0);
  for(int j=0;j<13;j++) {
    gyroBuf2[0]=j;
    i2c0.txrx_string(COMPASS_ADDR,gyroBuf2+ 0,1,gyroBuf2+1+j,1);
  }
  fillPktBlock(buf,gyroBuf2+1,13);
  fillPktFinish(buf);
#endif
}


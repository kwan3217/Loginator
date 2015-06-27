#include "compass.h"
#include "i2c.h"
#include "pktwrite.h"
#include "nmeawrite.h"
#include "conparse.h"
#include "main.h"
#include "sensor.h"
#include "IMU.h"

char compassID[3];
char compassReading[6];

#define COMPASS_ADDR 0x1E

void readCompass(sensor* this, unsigned int TC) {
  this->TC=TC;
  //Read the compass
  i2c_tx_string(COMPASS_ADDR,"\x02\x01",2);
  i2c_txrx_string(COMPASS_ADDR,"\x0A",1,compassID,3);
  i2c_txrx_string(COMPASS_ADDR,"\x03",1,compassReading,6);
  
  //Set it back to single shot mode
  i2c_tx_string(COMPASS_ADDR,"\x02\x01",2);
  this->dn[0]=compassReading[0]<<8 | compassReading[1];
  this->dn[1]=compassReading[2]<<8 | compassReading[3];
  this->dn[2]=compassReading[4]<<8 | compassReading[5];
}


void calCompass(sensor* this) {
  this->cal[0]=this->dn[0]+  3.5;
  this->cal[1]=this->dn[1]- 78.5;
  this->cal[2]=this->dn[2]+102.5;
}

int checkCompass(sensor* this) {
  return 1;
}

void writeCompass(sensor* this, circular* buf) {
  //Write the results in a packet
  fillPktStart(buf,PT_I2C);
  buf->dataDec=0;
  fillPktByte(buf,COMPASS_ADDR); //Transmit ID, the device we think we are talking to
  buf->dataDigits=2;
  fillPktChar(buf,compassID[0]); //Receive ID, which proves that we are in fact talking to it
  buf->dataComma=0;
  fillPktChar(buf,compassID[1]);
  fillPktChar(buf,compassID[2]);
  writeSensorGuts(this,buf);
  fillPktFinish(buf);
}

void setupCompass(circular *buf) {
#ifdef HOST
  compassID[0]='K';
  compassID[1]='4';
  compassID[2]='3';
#else
  setupSensorFunc(Bfld,Compass);
  Bfld.n_ele=4;
  Bfld.n_k=3;
  Bfld.g_ofs=3;
  Bfld.R[0]=20;
  Bfld.R[1]=Bfld.R[0];
  Bfld.R[2]=Bfld.R[0];
  Bfld.k=&k_IMUQ;
  Bfld.g=g_IMUQ;
  Bfld.H=H_IMUQ;
  //Set it to single-shot mode
  char gyroBuf2[2];
  i2c_tx_string(COMPASS_ADDR,"\x02\x01",2);
  //Read back all registers
  fillPktStart(buf,PT_I2C);
  fillPktString(buf,"HMC");
  buf->dataDec=0;
  buf->dataDigits=2;
  fillPktByte(buf,0);
  for(int j=0;j<13;j++) {
    buf->dataComma=(j==0);
    gyroBuf2[0]=j;
    i2c_txrx_string(COMPASS_ADDR,gyroBuf2+ 0,1,gyroBuf2+1,1);
    fillPktByte(buf,gyroBuf2[1]);
  }
  fillPktFinish(buf);
#endif
}


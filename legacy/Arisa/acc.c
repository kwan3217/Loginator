#include "acc.h"
#include "LPC214x.h"
#include "spi.h"
#include "pktwrite.h"
#include "nmeawrite.h"
#include "conparse.h"
#include "tictoc.h"
#include "sensor.h"
#include "IMU.h"

//From FloorSixAxis2_acc.ods
const fp accZo[]={-120.0339033103,-445.0162170131,696.1329715606};
const fp accScale[]={0.00090722,0.00087936,0.00088756};
//inverse of transpose of matrix in FloorSixAxis2_acc.ods
static const fp sens2box_c00= 0.9997124483,sens2box_c01= 0.0009596353,sens2box_c02=0.0278753066;
static const fp sens2box_c10=-0.0097715809,sens2box_c11= 0.9998209736,sens2box_c12=0.0222347846;
static const fp sens2box_c20=-0.020107788 ,sens2box_c21=-0.0112778706,sens2box_c22=0.9998730285;
fp extraAccScale=1.0;

               //   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14
char accBuf[81]="\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x35\x00\x00\x0D\x00";

//Ack any pre-existing ints
void ackAcc() {
  spi1_rx_string_block((1<<7)|0x0D,accBuf,2);
  accBuf[0]=(0<<7)|0x0D;
  accBuf[1]|=0x40; //Set the reset_int bit to clear existing ints
  spi1_tx_string_block(accBuf,2);
}

//Pass in TC so you can get an exact cycle-accurate timestamp from the
//timer capture channel
void readAcc(sensor* this, unsigned int TC) {
  this->TC=TC;
  //Read the version and sensors
  spi1_rx_string_block((1<<7)|0x00,accBuf,10);
  //Read the gain setting
  spi1_rx_string_block((1<<7)|0x35,accBuf+10,2);
  this->dn[0]=accBuf[ 4]<<8 | accBuf[ 3]; //5 - Acc X (regs 0x03 MSB and 0x02 LSB)
  this->dn[1]=accBuf[ 6]<<8 | accBuf[ 5]; //6 - Acc Y (regs 0x05 MSB and 0x04 LSB)
  this->dn[2]=accBuf[ 8]<<8 | accBuf[ 7]; //7 - Acc Z (regs 0x07 MSB and 0x06 LSB)
  this->dn[3]=accBuf[9];                  //8 - temperature (reg 0x08)
}

void calAcc(sensor* this) {
  fp x=(this->dn[0]-accZo[0])*accScale[0];
  fp y=(this->dn[1]-accZo[1])*accScale[1];
  fp z=(this->dn[2]-accZo[2])*accScale[2];
  this->cal[0]=(sens2box_c00*x+sens2box_c01*y+sens2box_c02*z)*extraAccScale;
  this->cal[1]=(sens2box_c10*x+sens2box_c11*y+sens2box_c12*z)*extraAccScale;
  this->cal[2]=(sens2box_c20*x+sens2box_c21*y+sens2box_c22*z)*extraAccScale;
  this->cal[3]=this->dn[3];
}

int checkAcc(sensor* this) {
  return 1;
}

//Returns 1 if anything written, 0 otherwise
void writeAcc(sensor* this, circular* buf) {
  //Write the results in a packet
  fillPktStart(buf,PT_I2C);
  buf->dataDec=0;
  buf->dataDigits=2;
  fillPktString(buf,"SPI");        //1 - Accelerometer address
  fillPktByte(buf,accBuf[1]);      //2 - Accelerometer ID (is 0314 for my part)
  buf->dataComma=0;
  fillPktByte(buf,accBuf[2]);      //2 - Accelerometer ID (is 0314 for my part)
  buf->dataComma=1;
  fillPktByte(buf,accBuf[11]);      //3 - Gain setting (among others, register 0x35)
  writeSensorGuts(this,buf);
  fillFinishNMEA(buf);
}

void setupAcc(circular *buf) {
  setupSensorFunc(Acc,Acc);
  Acc.n_ele=4;
  Acc.n_k=3;
  Acc.g_ofs=6;
  Acc.R[0]=1;Acc.R[1]=1;Acc.R[2]=1; //Gotta measure some real noise
  Acc.k=&k_IMUR;
  Acc.g=g_IMUR;
  Acc.H=H_IMUR;
  char range=GSense;
  spi1_setup(10000000, 1, 1);
  //Unlock gain setting registers
  spi1_rx_string_block((1<<7)|0x0D,accBuf,2);
  accBuf[0]=(0<<7)|0x0D;
  accBuf[1]|=(1 << 4);
  spi1_tx_string_block(accBuf,2);
  //Set range
  spi1_rx_string_block((1<<7)|0x35,accBuf,2);
  accBuf[0]=(0<<7)|0x35;
  accBuf[1]=(accBuf[1] & 0xF0) | (range << 1) | (1<<0); //Also turn on sample skip
  spi1_tx_string_block(accBuf,2);
  //Set bandwidth
  spi1_rx_string_block((1<<7)|0x20,accBuf,2);
  accBuf[0]=(0<<7)|0x20;
  accBuf[1]=(accBW<<4)|(accBuf[1] & 0x0F);
  spi1_tx_string_block(accBuf,2);
  //Set interrupts
  spi1_rx_string_block((1<<7)|0x21,accBuf,2);
  accBuf[0]=(0<<7)|0x21;
  accBuf[1]=0x00; //Turn off new_data_int, adv_int off (no slope, tap, low-g, etc)
  spi1_tx_string_block(accBuf,2);
  ackAcc();
  //Re-lock gain setting registers
  spi1_rx_string_block((1<<7)|0x0D,accBuf,2);
  accBuf[0]=(0<<7)|0x0D;
  accBuf[1]&=~(1 << 4);
  spi1_tx_string_block(accBuf,2);

  //Read the whole register set
  spi1_rx_string_block((1<<7)|0x00,accBuf,0x81);
  //Write the results in a packet
  for(int i=0;i<8;i++) {
    fillPktStart(buf,PT_I2C);
    fillPktString(buf,"BMA");        //1 - Accelerometer address
    buf->dataDec=0;
    buf->dataDigits=2;
    fillPktByte(buf,i*16);
    fillPktByte(buf,accBuf[i*16+1]);
    buf->dataComma=0;
    for(int j=1;j<16;j++) fillPktByte(buf,accBuf[i*16+j+1]);
    fillFinishNMEA(buf);
  }
}

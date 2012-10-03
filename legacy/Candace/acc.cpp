#include "acc.h"
#include "LPC214x.h"
#include "spi.h"
#include "pktwrite.h"
#include "nmeawrite.h"
#include "conparse.h"
#include "tictoc.h"
#include "sensor.h"
#include "imu.h"

acc Acc;

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
void acc::read(unsigned int LTC) {
  TC=LTC;
  //Read the version and sensors
  spi1_rx_string_block((1<<7)|0x00,accBuf,10);
  //Read the gain setting
  spi1_rx_string_block((1<<7)|0x35,accBuf+10,2);
  dn[0]=accBuf[ 4]<<8 | accBuf[ 3]; //5 - Acc X (regs 0x03 MSB and 0x02 LSB)
  dn[1]=accBuf[ 6]<<8 | accBuf[ 5]; //6 - Acc Y (regs 0x05 MSB and 0x04 LSB)
  dn[2]=accBuf[ 8]<<8 | accBuf[ 7]; //7 - Acc Z (regs 0x07 MSB and 0x06 LSB)
  dn[3]=accBuf[9];                  //8 - temperature (reg 0x08)
}

void acc::calibrate() {
  fp x=(dn[0]-accZo[0])*accScale[0];
  fp y=(dn[1]-accZo[1])*accScale[1];
  fp z=(dn[2]-accZo[2])*accScale[2];
  cal[0]=(sens2box_c00*x+sens2box_c01*y+sens2box_c02*z)*extraAccScale;
  cal[1]=(sens2box_c10*x+sens2box_c11*y+sens2box_c12*z)*extraAccScale;
  cal[2]=(sens2box_c20*x+sens2box_c21*y+sens2box_c22*z)*extraAccScale;
  cal[3]=dn[3];
}

//Returns 1 if anything written, 0 otherwise
void acc::write(circular& buf) {
  //Write the results in a packet
  fillPktStart(buf,PT_I2C);
  fillPktString(buf,"SPI");        //1 - Accelerometer address
  fillPktBlock(buf,accBuf+1,2);
  buf.dataDec=0;
  buf.dataDigits=2;
  fillPktByte(buf,accBuf[11]);      //3 - Gain setting (among others, register 0x35)
  writeGuts(buf);
  fillPktFinish(buf);
}

acc::acc():sensor(4,3,6,&k_IMUQ,g_IMUQ,H_IMUQ) {
  R[0]=1;
  R[1]=R[0];
  R[2]=R[0];
}

void acc::setup(circular& buf) {
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
    buf.dataDec=0;
    buf.dataDigits=2;
    fillPktByte(buf,i*16);
    fillPktBlock(buf,accBuf+i*16+1,16);
    fillPktFinish(buf);
  }
}

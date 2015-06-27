#include "gyro.h"
#include "i2c_int.h"
#include "pktwrite.h"
#include "nmeawrite.h"
#include "conparse.h"
#include "sensor.h"
#include "main.h"
#include <math.h>
#include "imu.h"
#ifdef HOST
#include <stdio.h>
#endif

gyro Gyro;

//Polynomials of gyro offset in DN with respect to temperature in DN plus. Subtract
//This from the raw measurement to get the zeroed offset DN measurement
//fp xzo=xDN-poly(tDN+25000,xofs);
const fp xofs[]={7.3788764e2,-1.0979095e-4,-2.4414819e-7};
const fp yofs[]={1.7459222e1,-1.0612476e-3};
const fp zofs[]={2.8077884e1,-1.4576032e-4};
//Inverse of the transpose of the matrix from FloorSixAxis2_gcal.ods
//static const fp sens2box_c00= 0.99924297,sens2box_c01=-0.00119116,sens2box_c02= 0.04009371;
//static const fp sens2box_c10=-0.00074813,sens2box_c11= 1.00061794,sens2box_c12= 0.01229537;
//static const fp sens2box_c20=-0.04120100,sens2box_c21= 0.00258682,sens2box_c22= 0.99928599;

static const fp sens2box_c00= 1,sens2box_c01= 0,sens2box_c02= 0;
static const fp sens2box_c10= 0,sens2box_c11= 1,sens2box_c12= 0;
static const fp sens2box_c20= 0,sens2box_c21= 0,sens2box_c22= 1;

fp xg_extra=0,yg_extra=0,zg_extra=0;

//Scale factor. Multiply xzo by This to convert DN to rad/s
#define nominal_scale 8.2362683e2
const fp scale[]={  1.0/(nominal_scale*0.9849),
                    1.0/(nominal_scale*1.0107),
                    1.0/(nominal_scale*0.9993)};

               
char gyroBuf[]=   "\x00\x00\x1B\x00\x00\x00\x00\x00\x00\x00\x00\x04\xD2";
#define GYRO_ADDR 0x68
unsigned int lastGyroTC;

void gyro::read(unsigned int LTC) {
  TC=LTC;
  //Read the Whoami register
  i2c0.txrx_string(GYRO_ADDR,gyroBuf+ 0,1,gyroBuf+1,1);
  //Read the settings
  i2c0.txrx_string(GYRO_ADDR,gyroBuf+ 2,1,gyroBuf+3,8);
  //two bytes written, 9 bytes read, 11 total
  dn[3]=gyroBuf[ 3]<<8 | gyroBuf[ 4]; //3 - Temperature (reg 0x1B MSB and 0x1C LSB)
  dn[0]=gyroBuf[ 5]<<8 | gyroBuf[ 6];//4 - Gyro X axis (reg 0x1D MSB and 0x1E LSB)
  dn[1]=gyroBuf[ 7]<<8 | gyroBuf[ 8];//5 - Gyro Y axis (reg 0x1F MSB and 0x20 LSB)
  dn[2]=gyroBuf[ 9]<<8 | gyroBuf[10];//6 - Gyro Z axis (reg 0x21 MSB and 0x22 LSB)
}

void gyro::calibrate() {
  fp tgDNp=dn[3]+25000;
#ifdef HOST
  //printf("%d %17.8e %17.8e %17.8e\n",tgDN+25000,poly(tgDNp,(fp*)xofs,2),poly(tgDNp,(fp*)yofs,1),-poly(tgDNp,(fp*)zofs,1));
#endif
  cal[3]=(dn[3]+13200.0)/280.0+35.0;
  fp x,y,z;
  x=(dn[0]-poly(tgDNp,xofs,2))*scale[0]-xg_extra;
  y=(dn[1]-poly(tgDNp,yofs,1))*scale[1]-yg_extra;
  z=(dn[2]-poly(tgDNp,zofs,1))*scale[2]-zg_extra;
  //Rotate to the box frame
  cal[0]=sens2box_c00*x+sens2box_c01*y+sens2box_c02*z;
  cal[1]=sens2box_c10*x+sens2box_c11*y+sens2box_c12*z;
  cal[2]=sens2box_c20*x+sens2box_c21*y+sens2box_c22*z;
}
  
void gyro::write(circular& buf) {
  //Write the results in a packet
  fillPktStart(buf,PT_I2C);
  buf.dataDec=0;
  buf.dataDigits=2;
  fillPktByte(buf,GYRO_ADDR);  //1 - Output address
  fillPktByte(buf,gyroBuf[1]); //2 - ID - same as output address (reg 0)
  writeGuts(buf);
  fillPktFinish(buf);
}

int gyro::check() {
  //Check if they are too different from the last reading
  for(int i=0;i<3;i++) if(fabs(calLast[i]-cal[i])>0.5) return 0;
  //They have to all be reasonable before we remember any of them
  for(int i=0;i<3;i++) calLast[i]=cal[i];
  return 1;
}

gyro::gyro():sensor(4,3,0,&k_IMUQ,g_IMUQ,H_IMUQ) {
  R[0]=5e-4;
  R[1]=R[0];
  R[2]=R[0];
#ifdef HOST
  compassID[0]='K';
  compassID[1]='4';
  compassID[2]='3';
#endif
}

void gyro::setup(circular& buf) {
  //Set gain and bandwidth to normal and 188Hz
  i2c0.tx_string(GYRO_ADDR,"\x16\x19",2);
  //Set clock mode to use X gyro
  i2c0.tx_string(GYRO_ADDR,"\x3E\x01",2);
  //Set interrupt mode to
  //7 - ACTL           - 0 active high
  //6 - OPEN           - 0 TTL push-pull
  //5 - LATCH_INT_EN   - 1 Latch until read back
  //4 - INT_ANYRD_2CLR - 1 Read any register to clear interrupt
  //3 -                  0
  //2 - ITG_RDY_EN     - 0 Don't interrupt on ITG ready
  //1 -                  0
  //0 - RAW_RDY_EN     - 1 Interrupt on data ready
  i2c0.tx_string(GYRO_ADDR,"\x17\x31",2);
  //Set gyro period in ms
  char gyroBuf2[17];
  gyroBuf2[0]=0x15;
  gyroBuf2[1]=gyroPeriod;
  i2c0.tx_string(GYRO_ADDR,gyroBuf2,2);
  //Read back all registers
  for(int i=0;i<8;i++) {
    fillPktStart(buf,PT_I2C);
    fillPktString(buf,"ITG");        //1 - Accelerometer address
    buf.dataDec=0;
    buf.dataDigits=2;
    fillPktByte(buf,i*16);
    for(int j=0;j<16;j++) {
      gyroBuf2[0]=i*16+j;
      i2c0.txrx_string(GYRO_ADDR,gyroBuf2+ 0,1,gyroBuf2+j+1,1);
    }
    fillPktBlock(buf,gyroBuf2+1,16);
    fillPktFinish(buf);
  }
}


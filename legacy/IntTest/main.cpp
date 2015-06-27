#include "LPC214x.h"
#include "system.h"
#include "main.h"
#include "i2c.h"
#include "uart.h"
#include <string.h>

I2C i2c_3v3;
#define GYRO_ADDR 0x69
char gyroBuf[16];

void setup() {
  pinMode(13,OUTPUT);
  pinMode(14,OUTPUT);
  pinMode(1,INPUT);

  setupUart(1,9600,(void*)0);
  delay(1000);

  set_pin(28,2); //Capture input on  AD3 (P0.28, CAP0.2)
  T0TCR=0; //Stop the timer
  T0TC=0;  //Set the timer count to 0
  T0PR=0;  //Prescale set to 0, so timer ticks on every PCLK
  T0PC=0;  //Prescale count set to 0
  T0MR0=PCLK; //Reset timer every 1 second
  T0MCR=0x00; //No reset or interrupt on any matches
  T0CCR=0x00;//(0x01 << (2*3)); //Capture on rising edge only, no interrupt
  T0TCR=1; //Start the timer
/*

  i2c_setup(&i2c_3v3,6,7,100000);

  //Set gain and bandwidth to normal and 188Hz
  // 7:5 -            000
  // 4:3 - FS_SEL -   11 Full Scale=2000deg/s (normal)
  // 2:0 - DLPF_CFG - 001 188Hz low pass, 1kHz sample
  i2c_tx_string(&i2c_3v3,GYRO_ADDR,"\x16\x19",2);
  //Set clock mode to use X gyro
  i2c_tx_string(&i2c_3v3,GYRO_ADDR,"\x3E\x01",2);
  //Set interrupt mode to
  //7 - ACTL           - 0 active high
  //6 - OPEN           - 0 TTL push-pull
  //5 - LATCH_INT_EN   - 1 Latch until read back
  //4 - INT_ANYRD_2CLR - 1 Read any register to clear interrupt
  //3 -                  0
  //2 - ITG_RDY_EN     - 0 Don't interrupt on ITG ready
  //1 -                  0
  //0 - RAW_RDY_EN     - 1 Interrupt on data ready
  i2c_tx_string(&i2c_3v3,GYRO_ADDR,"\x17\x31",2);
  //Set gyro period in ms
  i2c_tx_string(&i2c_3v3,GYRO_ADDR,"\x15\x0A",2);

  //Read the Whoami register
  i2c_txrx_string(&i2c_3v3,GYRO_ADDR,"\x00",1,gyroBuf,1);
  //Read the settings
  i2c_txrx_string(&i2c_3v3,GYRO_ADDR,"\x1B",1,gyroBuf+1,8);
 */
}

unsigned int cap=0;

char buf[]="---,---,___,___,";


void loop() {
  digitalWrite(13,HIGH);
//  cap=T0TC;
  delay(100);
  //Read the Whoami register
//  i2c_txrx_string(&i2c_3v3,GYRO_ADDR,"\x00",1,gyroBuf,1);
  //Read the settings
//  i2c_txrx_string(&i2c_3v3,GYRO_ADDR,"\x1B",1,gyroBuf+1,8);
  digitalWrite(14,HIGH);
  delay(100);
  //two bytes written, 9 bytes read, 11 total
  /*
  unsigned int CR2=cap/60;
  int p=15;
  do {
    buf[p]=CR2%10+0x30;
    CR2/=10;
    p--;
  } while(CR2>0);
  */
//  tx_serial_block(1, buf,16);
  
  digitalWrite(13,LOW);
  digitalWrite(14,LOW);
}

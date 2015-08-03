#include "LPCduino.h"
#include "pwm.h"
//Pin 13 is STAT0 (red light)
//Pin 12 is STAT1 (green light)

//LPCduino numbers:
// 0 - P0.13, BATLVL
// 1 - P0.30, Loginator AD1/IC (Compass Interrupt)
// 2 - P0.29, Loginator AD2/IG (Gyro    Interrupt)
// 3 - P0.28, Loginator AD3/IA (Acc     Interrupt)
// 4 - P0.25, Loginator AD4/CSG (Chip Select Gyro)
// 5 - P0.22, Loginator AD5
// 6 - P0.21, Loginator AD6, PWM5
// 7 - P0.10, Loginator AD7
// 8 - P0.12, Loginator AD8/ADREF (2.5V reference)
// 9 - P0.20, Loginator CS1/CSA (Chip Select Acc)
//10 - P0.19, Loginator MOSI1
//11 - P0.18, Loginator MISO1
//12 - P0.17, Loginator SCK1
//13 - P0.02, Loginator SCL0, Logomatic STAT0
//14 - P0.11, Loginator SCL1, Logomatic STAT1
//15 - P0.08, Loginator TX1, PWMR, PWM4
//16 - P0.09, Loginator RX1,       PWM6

void pinMode(int pin, int mode) {

}

void digitalWrite(int pin, int level) {

}

int digitalRead(int pin) {
  return 0;
}


int analogRead(int pin) {
  return 0;
}

void analogWrite(int pin, int val) {

}

void setServoPeriod(const unsigned int usecPeriod) {

}

void switchServo(const int pin, const bool on) {

}

void servoWrite(const int pin, const signed char val) {

}




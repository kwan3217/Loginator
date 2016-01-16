#include "Time.h"
#include "LPCduino.h"
#include "Serial.h"
#include "kalmanEigen.h"
#include <random>

extern "C" {
void __attribute__ ((weak)) _exit(int status) {};
void __attribute__ ((weak)) _sbrk() {};
void __attribute__ ((weak)) _kill() {};
void __attribute__ ((weak)) _getpid() {};
void __attribute__ ((weak)) _write() {};
void __attribute__ ((weak)) _close() {};
void __attribute__ ((weak)) _fstat() {};
void __attribute__ ((weak)) _isatty() {};
void __attribute__ ((weak)) _lseek() {};
void __attribute__ ((weak)) _read() {};
}

class KalmanVelocity: public Kalman<2,1,KalmanVelocity> {

};

class KalmanConstant: public Kalman<1,1,KalmanConstant> {

};

uint32_t ticks;
fp t;
const fp dt=0.01;
KalmanConstant kc;
const fp sig_z=0.1;
std::default_random_engine generator;
std::normal_distribution<fp> distribution(0.0,sig_z);
Eigen::Matrix<fp,1,1> z;

void setup() {
  pinMode(13,OUTPUT);
  Serial.begin(4800);
  Serial.println("Begin");
  kc.A << 1;
  kc.H << 1;
  kc.Q << 0;
  kc.R << sig_z*sig_z;
  kc.P << 1;
  kc.xh << 0;
}

void loop() {
  z << 0.5+distribution(generator);
  kc.step(dt,z);
  Serial.print("xh[0]: ");
  Serial.println(kc.xh(0),6);
}

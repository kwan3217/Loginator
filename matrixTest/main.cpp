#include "Time.h"
#include "LPCduino.h"
#include "Serial.h"
#include "kalmanEigen.h"

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

void setup() {
  pinMode(13,OUTPUT);
  Serial.begin(4800);
  Serial.println("Begin");
  KalmanVelocity kv;
  fp dt=0.1;
  fp sig_z=0.1;
  fp sig_v=0.1;
  kv.A << 1, dt,
		  0,  1;
  kv.H << 1,  0;
  kv.Q << dt*dt*dt*dt/4, dt*dt*dt   /2,
		  dt*dt*dt   /2, dt*dt        ;
  kv.Q*=sig_v;
  kv.R << sig_z*sig_z;
  kv.P << 1, 0,
		  0, 1;
  kv.xh << 0.5,
		   0.0;
  Eigen::Matrix<fp,1,1> z;
  z << 1.0;
  kv.step(dt,z);
  Serial.print("xh[0]: ");
  Serial.println(kv.xh(0));
  Serial.print("xh[1]: ");
  Serial.println(kv.xh(1));
}

void loop() {

}

#include "Time.h"
#include "LPCduino.h"
#include "Serial.h"
#include "kalmanEigen.h"
#include <random>

//These are a "scarlet letter". If you need these to compile, it means that Eigen
//is trying to print a message, and your code could (almost certainly will) fail.
#ifdef SCARLET_LETTER
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
#endif

uint32_t ticks;
fp t;
const fp tps=100;
const fp dt=1.0/tps;
LinearKalman<2,1,fp> kc;
const fp sig_z=0.1;
const fp sig_v=0.1;
std::default_random_engine generator;
std::normal_distribution<fp> distribution(0.0,sig_z);
Eigen::Matrix<fp,1,1> z;

void setup() {
  pinMode(13,OUTPUT);
  Serial.begin(4800);
  Serial.println("Begin");
  kc.A << 1, dt,
		  0,  1;
  kc.H << 1,  0;
  kc.Q << dt*dt*dt*dt/4,dt*dt*dt   /2,
		  dt*dt*dt   /2,dt*dt        ;
  kc.R << sig_z*sig_z;
  kc.P << 1,0,
		  0,1;
  kc.xh << 0,
		   0;
}

fp flare(int tick) {
  const fp flareStartTime=5;
  fp t=fp(tick)/tps;
  if(t<flareStartTime) return 20;
  const fp flarePeakDt=1; //Number of seconds from start to peak
  const fp flareA=log(2)/flarePeakDt;
  return 20+20*4*(exp(-flareA*(t-flareStartTime))-exp(-2*flareA*(t-flareStartTime)));
}

void loop() {
  fp True=flare(ticks);
  z << True+distribution(generator);
  kc.step(z,dt);
  Serial.print("t: ");
  Serial.print(fp(ticks)/tps,6);
  Serial.print("true: ");
  Serial.print(True,6);
  Serial.print("meas: ");
  Serial.print(z(0),6);
  Serial.print("  xh[0]: ");
  Serial.print(kc.xh(0),6);
  Serial.print("  xh[1]: ");
  Serial.println(kc.xh(1),6);
  ticks++;
  if(ticks==1000) blinklock(1000);
}

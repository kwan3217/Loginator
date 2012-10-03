#ifndef sensor_h
#define sensor_h

#include "float.h"
#include "circular.h"
#include "kalman.h"

class sensor {
public:
  sensor(int Ln_ele,int Ln_k,int Lg_ofs, kalman_state* Lk, const gfunc* Lg, const Ffunc* LH)
          :n_ele(Ln_ele),n_k(Ln_k),g_ofs(Lg_ofs),k(Lk),g(Lg),H(LH) {}
  virtual void setup(circular& buf) {}
  virtual void read(unsigned int TC)=0;
  virtual void calibrate()=0;
  virtual int check() {return 1;}
  virtual void write(circular& buf)=0;
protected:
  void writeGuts(circular& buf);
public:
  const int n_ele,n_k,g_ofs;
  kalman_state* const k;
  //Pointer to array of gfunc, which are function pointers
  const gfunc *g;
  //Pointer to array of Ffunc, which are function pointers
  const Ffunc *H;

  unsigned int TC;
  //On sensors which are naturally vector, dn[0]..dn[2] are x,y,z.
  //On sensors with a temperature, dn[3] is t
  short dn[4];
  //Calibrated units, always in SI, except for temperature which is degC
  fp cal[4];
  fp R[4];
};

void setupSensors(circular& buf);
//Returns 1 if slow sensors were read, 0 otherwise
int readAllSensors(circular& buf);

#endif

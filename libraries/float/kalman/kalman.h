#ifndef KALMAN_H
#define KALMAN_H

//Code to run the Extended Kalman Filter. This particular implementation is
//restricted to filters using a scalar measurement at each measurement update.
//In practice, it is better this way and no real restriction. If you have a
//vector measurement, treat it as a series of scalar measurements with zero time
//in between them. The only restriction is that the measurement covariance must
//be diagonal, that is no correlation presumed between the elements of the noise
//vector, which is what we usually do anyway.

//With this restriction, the matrix inverse involved in calculating the Kalman
//gain is turned into a scalar divide.

#include "matrix.h"
#ifdef PACKET
#include "circular.h"
#endif
//Function pointer to a function that given a state matrix calculates a matrix.
//All of F, Phi, H fit this definition. The result is the first parameter and
//the state matrix (mx1 column vector) is the second parameter.
//Calculates F=Ffunc(x)
typedef int(*Ffunc)(matrix& F,const matrix& x);

//Function pointer to a function that given a state matrix calculates a scalar.
//Calculates gfunc(x)
typedef fp(*gfunc)(const matrix& x);

//Yay, we get to abuse the preprocessor!
#define kalMatrices(m,tag) \
fp Adata##tag[m*m]; \
fp P1data##tag[m*m]; \
fp Hdata##tag[m];     \
fp HPdata##tag[m];     \
fp Kdata##tag[m];       \
fp xhdata##tag[m];   \
fp Pdata##tag[m*m]; \
fp Qdata##tag[m*m]; \
fp Fdtdata##tag[m];  \
fp KHPdata##tag[m*m]; \
fp Xhdata##tag[m]      \

#define kalInit(m,n,tag,fF,fPhi) \
(m, n, xhdata##tag, Pdata##tag, Qdata##tag, \
       Adata##tag, P1data##tag, Hdata##tag, HPdata##tag, \
       Kdata##tag, Fdtdata##tag, KHPdata##tag, Xhdata##tag, fF, fPhi)
#define kalInit2(m,n,pre,tag,fF,fPhi) \
(m, n, pre xhdata##tag, pre Pdata##tag, pre Qdata##tag, \
       pre Adata##tag, pre P1data##tag, pre Hdata##tag, pre HPdata##tag, \
       pre Kdata##tag, pre Fdtdata##tag, pre KHPdata##tag, pre Xhdata##tag, fF, fPhi)
class kalman_state {
public:

  int m,n;
  matrix xh,P,Q;
  matrix A,P1,H,HP,K,Fdt,KHP,Xh;
  fp Gamma,Z,g,dt;
  unsigned int lastUpdate;
  Ffunc fF;
  Ffunc fPhi;

  //xhdata  - pointer to 1D array of m   floats
  //P1data  - pointer to 1D array of m*m floats
  //Adata   - pointer to 1D array of m*m floats
  //P1data  - pointer to 1D array of m*m floats
  //Hdata   - pointer to 1D array of m   floats
  //HPdata  - pointer to 1D array of m   floats
  //Kdata   - pointer to 1D array of m   floats
  //Fdtdata - pointer to 1D array of m   floats
  //KHPdata - pointer to 1D array of m*m floats
  //XHdata  - pointer to 1D array of m   floats
  //fF -  Physics function. Given x, calculates state derivative dx/dt
  //fg -  Measurement function. Given x, calculates measurement y
  //fPhi- Physics matrix. Given x, calculates jacobian of physics function with
  //      respect to the state vector at the given value of the state vector
  //fH  - Measurement matrix. Given x, calculates the jacobian of the measurement
  //      function with respect to the state vector at the given value of the
  //      state vector.
  kalman_state(int Lm, int Ln, fp* xhdata, fp* Pdata, fp* Qdata,
              fp* Adata, fp* P1data, fp* Hdata, fp* HPdata, fp* Kdata,
              fp* Fdtdata, fp* KHPdata, fp* Xhdata, Ffunc LfF, Ffunc LfPhi);
  //Take a step using a scalar observation
  //z  - scalar measurement
  //dt_major - time step size
  //R   - Measurement covariance. Scalar since the measurement is scalar
  int step(fp dt, fp z, fp R, gfunc fg, Ffunc fH);
#ifdef PACKET
  void write(circular& buf, int which1, int which2);
#endif
  fp update_time(unsigned int TC);

  virtual int intxA_euler1();
};

#endif

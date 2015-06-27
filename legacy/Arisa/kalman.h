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
#include "circular.h"
//Function pointer to a function that given a state matrix calculates a matrix.
//All of F, Phi, H fit this definition. The result is the first parameter and
//the state matrix (mx1 column vector) is the second parameter.
//Calculates F=Ffunc(x)
typedef int(*Ffunc)(matrix* F,matrix* x);

//Function pointer to a function that given a state matrix calculates a scalar.
//Calculates gfunc(x)
typedef fp(*gfunc)(matrix* x);

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

#define kalInit(k,m,n,tag,fF,fPhi) \
ekf_setup(&k, m, n, xhdata##tag, Pdata##tag, Qdata##tag, \
                 Adata##tag, P1data##tag, Hdata##tag, HPdata##tag, \
                 Kdata##tag, Fdtdata##tag, KHPdata##tag, Xhdata##tag, fF, fPhi)
#define kalInit2(k,m,n,pre,tag,fF,fPhi) \
ekf_setup(&k, m, n, pre xhdata##tag, pre Pdata##tag, pre Qdata##tag, \
                 pre Adata##tag, pre P1data##tag, pre Hdata##tag, pre HPdata##tag, \
                 pre Kdata##tag, pre Fdtdata##tag, pre KHPdata##tag, pre Xhdata##tag, fF, fPhi)
typedef struct {
  int m,n;
  matrix xh,P,Q;
  matrix A,P1,H,HP,K,Fdt,KHP,Xh;
  fp Gamma,Z,g,dt;
  unsigned int lastUpdate;
  Ffunc fF;
  Ffunc fPhi;
} kalman_state;

//Function pointer to a numerical integrator which does the time update of both
//xh and A.
//xh -       Previous state vector xh_im1 on input, updated state estimate
//           xh_im1 on output
//A  -       On output, state transition matrix
//dt -       Amount of time to integrate over
//F -        pointer to Physics function, used to advance the state vector estimate
//Phi -      pointer to Physics matrix function, used to advance the state transistion matrix
typedef int(*intxAfunc)(kalman_state* k, matrix* xh, fp dt, Ffunc F, Ffunc Phi);

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
int ekf_setup(kalman_state* k, int m, int n, fp* xhdata, fp* Pdata, fp* Qdata,
              fp* Adata, fp* P1data,
              fp* Hdata, fp* HPdata, fp* Kdata,
              fp* Fdtdata, fp* KHPdata, fp* Xhdata,
              Ffunc fF, Ffunc fPhi);

//Take a step using a scalar observation
//z  - scalar measurement
//dt_major - time step size
//R   - Measurement covariance. Scalar since the measurement is scalar
int ekf_step(kalman_state* k, fp dt, fp z, fp R, gfunc fg, Ffunc fH);
void writeKalmanState(kalman_state*k, circular* buf, int which1, int which2);
fp ekf_update_time(kalman_state* k, unsigned int TC);

//Was static
int intxA_euler1(kalman_state* k, fp dt);
#endif

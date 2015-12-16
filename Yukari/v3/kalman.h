#ifndef kalman_h
#define kalman_h

/** Kalman filter with scalar measurement.
@param x Estimate of state vector
@param P Estimate covariance
@param A State transition matrix
@param Q Process noise covariance
@param H Measurement matrix
@param R Measurement (co)variance
*/
#include "matrix.h"
template<int n>
void kalman_scalar(fp (&x)[n][1], fp (&P)[n][n], const fp (&A)[n][n], const fp (&Q)[n][n], const fp (&H)[1][n], const fp (&R)[1][1], const fp &z) {
  auto const m=1;
  //Time update of measurement
  fp xm[n][1];
  mm(A,x,xm);
  //Time update of covariance
  //P=[A][P_i-1][A]^T+[Q]
  {
    fp AP[n][n]; //scratch
    mm(A,P,AP); //AP=A##P
    mmt(AP,A,P); //P=AP##A^T
    mpm(P,Q); //P is now [P^-]
  }

  //Calculate Kalman gain
  //S: P(2x2)#H^T(2x1)=S(2x1)
  fp K[n][m];
  fp (&S)[n][m]=K; //Just to make things easier to read, hopefully optimization gets rid of this completely
  mmt(P,H,S);

  //gamma: H(1x2)#S(2x1)=Gamma'(1x1)
  fp Gamma[m][m];
  //Gamma'(1x1)+R(1x1)=Gamma(1x1)
  mm(H,S,Gamma);
  Gamma[0][0]+=R;

  //K=S # Gamma^-1, but Gamma is scalar, so K=S/Gamma. Note that S is a reference to K, so K contains the values of S at this point
  ms(K,1.0/Gamma[0][0]);

  //Measurement residual
  fp Hx[m][1];
  mm(H,xm,Hx);
  fp y[m][1];
  y[0][0]=z;
  mmm(y,Hx);

  //Measurement update of estimate
  //x=xm+[K]y

}

#endif

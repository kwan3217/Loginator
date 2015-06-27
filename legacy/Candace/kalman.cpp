#include "kalman.h"
#include "setup.h"

//Perform numerical integration
int kalman_state::intxA_euler1() {
  //1a - xh=xh+F(xh)*dt;
  assertmx(fF(Fdt,xh),-1); //Fdt=F(xh);
  Fdt*=dt;             //Fdt=F(xh)*dt;
  xh+=Fdt;             //xh=xh+F(xh)*dt
  //1b - A=A+Phi(xh)*A*dt;
  //In this single-step integrator, A will always be [1] at this point.
  //So we do it as A=Phi*dt+diag(1)
  assertmx(fPhi(A,xh),-2);   //A=Phi(xh);
  A*=(dt);                  //A*=dt;
  for(int i=0;i<A.rows;i++) A(i,i)+=1;
  return 0;
}

kalman_state::kalman_state(int Lm, int Ln, fp* xhdata, fp* Pdata, fp* Qdata,
              fp* Adata, fp* P1data, fp* Hdata, fp* HPdata, fp* Kdata,
              fp* Fdtdata, fp* KHPdata, fp* Xhdata, Ffunc LfF, Ffunc LfPhi):
    m(Lm),n(Ln),
    xh (m,1,xhdata),
    P  (m,m,Pdata),
    Q  (m,m,Qdata),
    A  (m,m,Adata),
    P1 (m,m,P1data),
    H  (1,m,Hdata),
    HP (1,m,HPdata),
    K  (m,1,Kdata),
    Fdt(m,1,Fdtdata),
    KHP(m,m,KHPdata),
    Xh (m,1,Xhdata),
    fF(LfF),fPhi(LfPhi) {
  }

fp kalman_state::update_time(unsigned int TC) {
  fp dtt=(((fp)(TC)-((fp)(lastUpdate))))/((fp)(CCLK));
  if(dtt<0) dtt+=1;
  lastUpdate=TC;
  return dtt;
}

int kalman_state::step(fp dt_i, fp z, fp R, gfunc fg, Ffunc fH) {
  dt=dt_i;
  //xh starts as xh_im1
  //P starts as P_im1
  if(dt>0) {
    //Do the time update by numerical integration
    //Equation 1A and 1B.
//    eye(&k->A); //Not needed for optimized single step as above
    assertmx(intxA_euler1(),-1);
    //xh is now xh_im
    //Equation 2. P=A*P*A'+Q;
    assertmx(P1.mx(A,P),-2);
    assertmx(P.mxt(P1,A),-3);
    assertmx(P+=Q,-4);
    //
  }
  //Otherwise, no time update and xh_im=xh_im1 and P_im=P_im1
  //1c - Xh_im=<0>. We will just "optimize out" any references to Xh_im
  //3A - Linearized observation matrix [H]=H(x);
  assertmx(fH(H,xh),-5);
  //3B - Kalman gain K=P*H'/(H*P*H'+R);
  //H is 1xm, P is mxm so H*P is 1xm. H' is mx1 so H*P*H' is 1x1, or as we like
  //to say, scalar. R is also scalar so this works out and (H*P*H'+R) is scalar
  //and its inverse is just a conventional reciprocal. P is mxm and H' is mx1,
  //so K is mx1, a column vector.
  assertmx(HP.mx(H,P),-6);
  assertmx(Gamma=matrix::mx1t(HP,H),-7);
  Gamma+=R;
  //Since P is symmetric, (HP)'=P'H'=PH' so we can use HP to calculate K
  assertmx(K.mts(HP,1.0/Gamma),-8);
  //4a - Measurement deviation Z_i=z_i-g(xh_im)
  g=fg(xh);
  Z=z-g;
  //4b - Measurement update of state deviation Xh_i=Xh_im+K*(Z_i-H_i*Xh_im)
  //Since as we said above, Xh_im is <0>, this reduces to Xh_i=K*Z;
  //We could do it in place K=K*Z, since we are done with K after this step, if
  //step 5 is done first, but we do this so we can keep everything around for
  //debugging purposes. We have memory, and assign is a quick O(m) function.
  Xh=K;
  Xh*=Z;
  //4c - Measurement update of reference state
  assertmx(xh+=Xh,-11);
  //5 - Measurement update of estimate covariance. 
  //P_i=(1-KH)P=P_im-K*H*P_im. K*(H*P) is mx1 by
  //1xm for mxm. We calculated HP above.
  assertmx(KHP.mx(K,HP),-9);
  assertmx(P-=(KHP),-10);   //Subtract KHP from P

  return 0;
}

//Comment this part out if you are not using my packet writing library
#include "pktwrite.h"
void kalman_state::write(circular& buf,int which1,int which2) {
  switch(which1) {
    case 0:
      A.fillPktMx(buf,"A");
      break;
    case 1:
      P1.fillPktMx(buf,"P1");
      break;
    case 2:
      H.fillPktMx(buf,"H");
      break;
    case 3:
      HP.fillPktMx(buf,"HP");
      break;
    case 4:
      K.fillPktMx(buf,"K");
      break;
    case 5:
      Fdt.fillPktMx(buf,"Fdt");
      break;
    case 6:
      KHP.fillPktMx(buf,"KHP");
      break;
    case 7:
      Xh.fillPktMx(buf,"Xh");
      break;
    case 8:
      xh.fillPktMx(buf,"xh");
      break;
    case 9:
      P.fillPktMx(buf,"P");
      break;
    case 10:
      Q.fillPktMx(buf,"Q");
      break;
    case 11:
      if(which2>0) return;
      fillPktStart(buf,PT_I2C);
      fillPktString(buf,"Gamma_g_Z_dt");
      fillPktFP(buf,Gamma);
      fillPktFP(buf,g);
      fillPktFP(buf,Z);
      fillPktFP(buf,dt);
      fillPktFinish(buf);
  }
}

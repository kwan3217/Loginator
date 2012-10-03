#include "kalman.h"
#include "setup.h"

//Perform numerical integration
int intxA_euler1(kalman_state* k, fp dt) {
  //1a - xh=xh+F(xh)*dt;
  assertmx(k->fF(&k->Fdt,&k->xh),-1); //Fdt=F(xh);
  mxs(&k->Fdt,dt);             //Fdt=F(xh)*dt;
  ma(&k->xh,&k->Fdt);               //xh=xh+F(xh)*dt
  //1b - A=A+Phi(xh)*A*dt;
  //In this single-step integrator, A will always be [1] at this point.
  //So we do it as A=Phi*dt+diag(1)
  assertmx(k->fPhi(&k->A,&k->xh),-2);   //A=Phi(xh);
  mxs(&k->A,dt);                  //A*=dt;
  for(int i=0;i<k->A.row;i++) px(&k->A,i,i,gx(&k->A,i,i)+1);
  return 0;
}

#define mxinit(base,m,n) \
k->base.row=m; \
k->base.col=n; \
k->base.data=base##data

#define mxinit2(base,m,n) \
if(Adata==base##data) return -1; \
k->base.row=m; \
k->base.col=n; \
k->base.data=base##data


//Set up the scratch space for the filter
//m - number of elements in state vector
//Adata   - pointer to 1D array of m*m floats
//P1data  - pointer to 1D array of m*m floats
//Hdata   - pointer to 1D array of m   floats
//HPdata  - pointer to 1D array of m   floats
//Kdata   - pointer to 1D array of m   floats
//Fdtdata - pointer to 1D array of m   floats
//Phidata - pointer to 1D array of m*m floats
//KHPdata - pointer to 1D array of m*m floats
//Xhdata -  pointer to 1D array of m floats
int ekf_setup(kalman_state* k, int m, int n, fp* xhdata, fp* Pdata, fp* Qdata,
              fp* Adata, fp* P1data,
              fp* Hdata, fp* HPdata, fp* Kdata, 
              fp* Fdtdata, fp* KHPdata, fp* Xhdata,
              Ffunc fF, Ffunc fPhi) {
  mxinit(A,m,m);
  mxinit2(xh,m,1);
  mxinit2(P,m,m);
  mxinit2(Q,m,m);
  mxinit2(P1,m,m);
  mxinit2(H,1,m);
  mxinit2(HP,1,m);
  mxinit2(K,m,1);
  mxinit2(Fdt,m,1);
  mxinit2(KHP,m,m);
  mxinit2(Xh,m,1);
  k->fF=fF;
  k->fPhi=fPhi;
  k->m=m;
  k->n=n;
  return 0;
}

fp ekf_update_time(kalman_state* k, unsigned int TC) {
  fp dt=(((fp)(TC)-((fp)(k->lastUpdate))))/((fp)(CCLK));
  if(dt<0) dt+=1;
  k->lastUpdate=TC;
  return dt;
}

int ekf_step(kalman_state* k, fp dt_i, fp z, fp R, gfunc fg, Ffunc fH) {
  k->dt=dt_i;
  //xh starts as xh_im1
  //P starts as P_im1
  if(k->dt>0) {
    //Do the time update by numerical integration
    //Equation 1A and 1B.
//    eye(&k->A); //Not needed for optimized single step as above
    assertmx(intxA_euler1(k,k->dt),-1);
    //xh is now xh_im
    //Equation 2. P=A*P*A'+Q;
    assertmx(mx(&k->P1,&k->A,&k->P),-2);
    assertmx(mxt(&k->P,&k->P1,&k->A),-3);
    assertmx(ma(&k->P,&k->Q),-4);
    //
  }
  //Otherwise, no time update and xh_im=xh_im1 and P_im=P_im1
  //1c - Xh_im=<0>. We will just "optimize out" any references to Xh_im
  //3A - Linearized observation matrix [H]=H(x);
  assertmx(fH(&k->H,&k->xh),-5);
  //3B - Kalman gain K=P*H'/(H*P*H'+R);
  //H is 1xm, P is mxm so H*P is 1xm. H' is mx1 so H*P*H' is 1x1, or as we like
  //to say, scalar. R is also scalar so this works out and (H*P*H'+R) is scalar
  //and its inverse is just a conventional reciprocal. P is mxm and H' is mx1,
  //so K is mx1, a column vector.
  assertmx(mx(&k->HP,&k->H,&k->P),-6);
  assertmx(mx1t(&k->Gamma,&k->HP,&k->H),-7);
  k->Gamma+=R;
  assertmx(mxt(&k->K,&k->P,&k->H),-8);
  mxs(&k->K,1.0/k->Gamma);
  //4a - Measurement deviation Z_i=z_i-g(xh_im)
  k->g=fg(&k->xh);
  k->Z=z-k->g;
  //4b - Measurement update of state deviation Xh_i=Xh_im+K*(Z_i-H_i*Xh_im)
  //Since as we said above, Xh_im is <0>, this reduces to Xh_i=K*Z;
  //We could do it in place K=K*Z, since we are done with K after this step, if
  //step 5 is done first, but we do this so we can keep everything around for
  //debugging purposes. We have memory, and assign is a quick O(m) function.
  assign(&k->Xh,&k->K);  
  mxs(&k->Xh,k->Z); 
  //4c - Measurement update of reference state
  assertmx(ma(&k->xh,&k->Xh),-11); //K holds Xh_i;
  //5 - Measurement update of estimate covariance. 
  //P_i=(1-KH)P=P_im-K*H*P_im. K*(H*P) is mx1 by
  //1xm for mxm. We calculated HP above.
  assertmx(mx(&k->KHP,&k->K,&k->HP),-9); 
  assertmx(ms(&k->P,&k->KHP),-10);   //Subtract KHP from P

  return 0;
}

//Comment this part out if you are not using my packet writing library
#include "pktwrite.h"
void writeKalmanState(kalman_state *k, circular *buf,int which1,int which2) {
  switch(which1) {
    case 0:
      fillPktMx(buf,"A",&k->A);
      break;
    case 1:
      fillPktMx(buf,"P1",&k->P1);
      break;
    case 2:
      fillPktMx(buf,"H",&k->H);
      break;
    case 3:
      fillPktMx(buf,"HP",&k->HP);
      break;
    case 4:
      fillPktMx(buf,"K",&k->K);
      break;
    case 5:
      fillPktMx(buf,"Fdt",&k->Fdt);
      break;
    case 6:
      fillPktMx(buf,"KHP",&k->KHP);
      break;
    case 7:
      fillPktMx(buf,"Xh",&k->Xh);
      break;
    case 8:
      fillPktMx(buf,"xh",&k->xh);
      break;
    case 9:
      fillPktMx(buf,"P",&k->P);
      break;
    case 10:
      fillPktMx(buf,"Q",&k->Q);
      break;
    case 11:
      if(which2>0) return;
      fillPktStart(buf,PT_I2C);
      fillPktString(buf,"Gamma_g_Z_dt");
      fillPktFP(buf,k->Gamma);
      fillPktFP(buf,k->g);
      fillPktFP(buf,k->Z);
      fillPktFP(buf,k->dt);
      fillPktFinish(buf);
  }
}

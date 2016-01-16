/*
//Perform numerical integration of state vector estimate xh and
//state transformation matrix A. This implements a single-step Euler method.
template<int n_state, int n_meas, class T, class Scalar>
void KalmanEuler<n_state,n_meas,T,Scalar>::fA() {
  //1a - xh=xh+F(xh)*dt;
  (static_cast<T*>(this))->fF(); //Fdt=F(xh);
  Fdt*=dt;    //Fdt=F(xh)*dt;
  xh+=Fdt;    //xh=xh+F(xh)*dt
  //1b - A=A+Phi(xh)*A*dt;
  //In this single-step integrator, A will always be [1] at this point.
  //So we do it as A=Phi*dt+diag(1)
  (static_cast<T*>(this))->fPhi();   //A=Phi(xh), calculate Phi from xh
  A*=dt;                  //A*=dt;
  for(int i=0;i<n_state;i++) A(i,i)+=1;
}
*/
template<int n_state, int n_meas, class T, class Scalar>
void Kalman<n_state,n_meas,T,Scalar>::step(Scalar dt_i, Eigen::Matrix<Scalar,n_meas,1>& z) {
  dt=dt_i;
  (static_cast<T*>(this))->fH(); //calculate H from xh
  (static_cast<T*>(this))->fg(); //calcualte zh from xh
  //xh starts as xh_{i-1}
  //P starts as P_{i-1}
  if(dt>0) {
	(static_cast<T*>(this))->fA(); //calculate A
    //Equation 1A and 1B.
    //xh is now xh_im
    //Equation 2. P_n+1=A*P_n*A'+Q;
    AP=A*P;
    P=AP*A.transpose(); //P_n+1=A*P_n*A'
    P+=Q;
  }

  //This is the part where we take each element of the observation in turn.
  for(int j=0;j<n_meas;j++) {
    //3B - Kalman gain K=P*H'/(H*P*H'+R);
	//S=P*H'
    S=P*H.row(j).transpose();
	//Gamma=H*P*H'+R
    //     =H*S   +R
    Gamma=H.row(j)*S;
    Gamma+=R(j);

    //K=S*Gamma^-1, or since Gamma is scalar, K=S/Gamma;
    K=S/(1.0/Gamma);
    //4a - Measurement deviation y_i=z_i-xh_i.
    y=z(j)-zh(j);
    //4b - Measurement update of state deviation Xh_i=Xh_im+K*(Z_i-H_i*Xh_im)
    //Since as we said above, Xh_im is <0>, this reduces to Xh_i=K*Z;
    //We could do it in place K=K*Z, since we are done with K after this step, if
    //step 5 is done first, but we do this so we can keep everything around for
    //debugging purposes. We have memory, and assign is a quick O(m) function.
    //4c - Measurement update of reference state
    xh+=K*y;
    //5 - Measurement update of estimate covariance.
    //P_i=(1-KH)P=P_im-K*H*P_im. K*(H*P) is mx1 by
    //1xm for mxm. We calculated HP above.
    KST=K*S.transpose();
    P-=KST;   //Subtract KHP from P
  }
}


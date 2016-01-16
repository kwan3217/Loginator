#ifndef KALMAN_H
#define KALMAN_H

#include "Eigen/Core"

/**Code to run the Extended Kalman Filter. This particular implementation is
restricted to filters using a vector measurement with uncorrelated uncertainty
(R is diagonal). This is implemented by considering each component of the
measurement in turn, with no time update in between. With this restriction,
the matrix inverse involved in calculating the Kalman gain is turned into a
scalar divide. We call this element-by-element operation the scalar aspect.

This is implemented according to the notation on the Kwan Hypertext Library
Kalman filter page, https://omoikane.kwansystems.org/wiki/index.php/Kalman_filter

By itself, this implements a linear Kalman filter. The A, H, Q, and R matrices
have to be initialized outside of this routine.

 @tparam n_state Number of elements in state vector (I got tired of asking myself if n or m was correct here, thus the longer names)
 @tparam n_meas  Number of elements in measurement vector
 @tparam T CRTP derived class. We are constrained to use CRTP by Eigen
           and the fact that we are using fixed-sized matrices.
 @tparam Scalar scalar type to use for state, measurement, and time. Probably should
           be either float or double (or an alias to one of these)
*/
template<int n_state, int n_meas, class T, class Scalar=fp>
class Kalman {
private:
  //Matrices only used internally
  /** Factor of A*P*A^T, used in time update of P */
  Eigen::Matrix<Scalar,n_state,n_state> AP;
  /** Physics vector times delta-T, used in time update of xh */
  Eigen::Matrix<Scalar,n_state,1> Fdt;
  /** Measurement estimate */
  Eigen::Matrix<Scalar,n_meas,1> zh;
  /** H_slice*P, factor of Gamma, uses only one row of H in scalar aspect, so n_meas-element row vector in scalar aspect.*/
  Eigen::Matrix<Scalar,1,n_meas> HP;
  /** Measurement residual, scalar in scalar aspect */
  Scalar y;
  /** Residual uncertainty in this element of the measurement. Scalar in scalar aspect. */
  Scalar Gamma;
  /** Cross-covariance matrix. Don't ask what it means indpendently, but it is
      a factor of K. n_state rows by n_meas columns, so n_state column vector
      in scalar aspect.
   */
  Eigen::Matrix<Scalar,n_state,1> S;
  /** Measurement gain matrix (Kalman gain). Multiply K by the measurement
      to get a state correction, compatible by addition with the state vector.
      In general, this is a n_state row by n_meas column matrix.
      In scalar aspect, this works out to be a n_state column vector*/
  Eigen::Matrix<Scalar,n_state,1> K;
  /** Decrease in estimate covariance due to this measurement. n_state x n_state matrix so as to be compatible by subtraction with P. */
  Eigen::Matrix<Scalar,n_state,n_state> KST;
  Scalar dt;
public:
  /** State vector estimate, n_state column vector */
  Eigen::Matrix<Scalar,n_state,1> xh;
  /** State estimate covariance, n_state x n_state */
  Eigen::Matrix<Scalar,n_state,n_state> P;
  /** State transition matrix, n x n */
  Eigen::Matrix<Scalar,n_state,n_state> A;
  /** Process noise covariance matrix, n x n */
  Eigen::Matrix<Scalar,n_state,n_state> Q;
  /** Measurement matrix, n_meas row by n_state column matrix. Each row can be used as an
      independent H matrix, which when multiplied by the state estimate produces
      a measurement estimate for the corresponding measurement vector element */
  Eigen::Matrix<Scalar,n_meas,n_state> H;
  /** Measurement noise covariance, n_meas x n_meas. Must be diagonal, to fit
      the model of using each measurement element in turn. The current implementation wastes memory on the off-diagonal values*/
  Eigen::Matrix<Scalar,n_meas,n_meas> R;

  /** State transition matrix function. Calculates state transition matrix A.
      Uses CRTP. Doesn't do anything in linear filter, nonlinear filters will
      probably implement a matrix numerical integrator here */
  /*virtual*/ void fA() {};
  /** Measurement matrix. Given x, calculates the jacobian of the measurement
      function with respect to the state vector at the given value of the
      state vector. Doesn't do anything in linear filter, the external code will
      have to set H directly. Implemented in derived class.
  /*virtual*/ void fH() {};
  /** Measurement function. Given x, calculates measurement estimate zh.
      Implemented in derived class. Linear filter uses H matrix to calculate
      measurement value */
  /*virtual*/ void fg() {zh=H*xh;};

  //fPhi-
  //fH  -
  //Take a step using a scalar observation
  //z  - scalar measurement
  //dt_major - time step size
  //R   - Measurement covariance. Scalar since the measurement is scalar
  void step(Scalar dt, Eigen::Matrix<Scalar,n_meas,1>& z);
};
//virtual void fH()=0;   //

#include "kalmanEigen.inc"

#endif

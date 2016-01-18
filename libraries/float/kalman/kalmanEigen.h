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
Kalman filter page, https://omoikane.kwansystems.org/wiki/index.php/Kalman_filter .

The Kalman filter is an algorithm which uses a model of how a physical system
works, combined with a series of observations of the system, to produce a series
of estimates of the state of the system. The filter works in the presence of
measurement and model uncertainty, and in fact is designed to give the best
possible estimate of the state, as well as an estimate of the uncertainty of the
estimate.

The simplest form of the filter is the linear filter. In this case, the physics
of the system are all perfectly described with matrices, as follows:

$$\vec{x}_i=\M{A}\vec{x}_{i-1}+\vec{w}_{i}$$
$$\vec{z}_i=\M{H}\vec{x}_{i}+\vec{v}_{i}$$

where

*  * $i$ is the measurement index
*  * $\vec{x}_i$ is the (unknown) actual state of the system at measurement $i$. This is an $n_{state}$-dimensional vector, or one with $n_{state}$ components.
*  * $\M{A}$ is the state transition matrix. Given a previous state, the state transition matrix is used to transform it to the current state. Since it operates on an $n$-dimensional vector and returns an $n$-dimensional vector (the number of components in the state never changes) it must be an $n\times n$ square matrix. This matrix might change between measurements, but the filter presumes that it is constant.
*  * $\vec{w}_i$ is the (unknown) process noise, an $n_{state}$-dimensional Gaussian random vector with a zero mean and covariance matrix $\M{Q}$. This is uncertainty in the process itself, unrelated to measurement. It implies that the next state is not perfectly determined by the previous state. Since the process noise is compatible with addition to the state vector, it must be a vector with $n$ components, and its covariance must be a matrix with $n\times n$ components.
*  * $\vec{z}_i$ is the measurement vector for measurement $i$. This is an $m$-dimensional vector, where $m$ may be greater than, less than, or equal to $n$.
*  * $\M{H}$ is the observation matrix. Given a state, the observation matrix is used to transform it to an observation vector. Since it operates on an $n$-dimensional vector and returns an $m$-dimensional vector (the number of components of the observation may be different than the number of components of the state) it must be an $m \times n$ matrix. This matrix might change between measurements, but the filter presumes that it is constant.
*  * $\vec{v}_i$ is the (unknown) measurement noise, an $m$-dimensional Gaussian random vector with a zero mean and covariance matrix $\M{R}$. Since the measurement noise is compatible with addition to the measurement vector, it must be a vector with $m$ components, and its covariance must be a matrix with $m \times m$ components.

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
  /** Measurement matrix. Given x, calculates the Jacobian of the measurement
      function with respect to the state vector at the given value of the
      state vector. Doesn't do anything in linear filter, the external code will
      have to set H directly. Implemented in derived class.
  /*virtual*/ void fH() {};
  /** Measurement function. Given x, calculates measurement estimate zh.
      Implemented in derived class. Linear filter uses H matrix to calculate
      measurement value */
  /*virtual*/ void fg() {zh=H*xh;};

  /*virtual*/ void timeUpdate(Scalar dt_i);
  /*virtual*/ void stateUpdate() {xh=A*xh;};
  void measUpdate(Eigen::Matrix<Scalar,n_meas,1>& z);
  void step(Eigen::Matrix<Scalar,n_meas,1>& z, Scalar dt_i=0) {if(dt_i>0) static_cast<T*>(this)->timeUpdate(dt_i); measUpdate(z);};
};

template<int n_state, int n_meas, typename Scalar=fp>
class LinearKalman:public Kalman<n_state, n_meas, LinearKalman<n_state,n_meas,Scalar>,Scalar> {};

#include "kalmanEigen.inc"

#endif

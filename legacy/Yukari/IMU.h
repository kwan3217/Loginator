#ifndef IMU_H
#define IMU_H

#include "kalman.h"
#include "circular.h"
#include "sensor.h"
#define IMUQ_M 7
#define IMUR_M 9
#define IMUQ_N 6
#define IMUR_N 9
extern fp Bix,Biy,Biz;
extern kalman_state k_IMUQ,k_IMUR;
extern const gfunc g_IMUQ[];
extern const Ffunc H_IMUQ[];
extern const gfunc g_IMUR[];
extern const Ffunc H_IMUR[];
extern const fp gz;

fp i2b_comp(fp Ix, fp Iy, fp Iz, int comp);
fp b2i_comp(fp Bx, fp By, fp Bz, int comp);

int initIMU(void);
int imuUpdate(sensor* s);
void writeIMUstate(circular* buf, int result);
int F_IMUQ(matrix* F, matrix* x);
int Phi_IMUQ(matrix* Phi, matrix* x);
int F_IMUR(matrix* F, matrix* x);
int Phi_IMUR(matrix* Phi, matrix* x);
extern const gfunc g_IMU[];
extern const Ffunc H_IMU[];
void quatNormalize(void);

//Given:
//point  body      vector <pbx,pby,pbz>,
//point  reference vector <prx,pry,prz>,
//toward body      vector <tbx,tby,tpz>,
//toward reference vector <tbx,tby,tpz>,
//change the orientation part of the rotation
//state vector to one which aligns the point vectors
//and aligns the toward vectors as closely as possible
int IMUOrient(fp pbx, fp pby, fp pbz,
              fp prx, fp pry, fp prz,
              fp tbx, fp tby, fp tbz,
              fp trx, fp try, fp trz, circular* buf);

fp g0_IMUQ(matrix* x);
fp g1_IMUQ(matrix* x);
fp g2_IMUQ(matrix* x);
fp g3_IMUQ(matrix* x);
fp g4_IMUQ(matrix* x);
fp g5_IMUQ(matrix* x);
fp g0_IMUR(matrix* x);
fp g1_IMUR(matrix* x);
fp g2_IMUR(matrix* x);
fp g3_IMUR(matrix* x);
fp g4_IMUR(matrix* x);
fp g5_IMUR(matrix* x);
fp g6_IMUR(matrix* x);
fp g7_IMUR(matrix* x);
fp g8_IMUR(matrix* x);
int H0_IMUQ(matrix* H,matrix* x);
int H1_IMUQ(matrix* H,matrix* x);
int H2_IMUQ(matrix* H,matrix* x);
int H3_IMUQ(matrix* H,matrix* x);
int H4_IMUQ(matrix* H,matrix* x);
int H5_IMUQ(matrix* H,matrix* x);
int H0_IMUR(matrix* H,matrix* x);
int H1_IMUR(matrix* H,matrix* x);
int H2_IMUR(matrix* H,matrix* x);
int H3_IMUR(matrix* H,matrix* x);
int H4_IMUR(matrix* H,matrix* x);
int H5_IMUR(matrix* H,matrix* x);
int H6_IMUR(matrix* H,matrix* x);
int H7_IMUR(matrix* H,matrix* x);
int H8_IMUR(matrix* H,matrix* x);

#endif

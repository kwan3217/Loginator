#include "IMU.h"
#include "kalman.h"
#include "gyro.h"
#include "compass.h"
#include "setup.h"
#include "pktwrite.h"
#include <math.h>
#include "sensor.h"
#include "serial.h"
#include "main.h"

fp Bix,Biy,Biz;
const fp gz=9.798422; //Gravity in greater Denver, CO in m/s^2. Includes 1600m
                      //above geoid and 1600m of ground between geoid and parking
                      //lot.

kalMatrices(IMUQ_M,imuq);
kalMatrices(IMUR_M,imur);
kalman_state k_IMUQ,k_IMUR;

#define quat_guts(a)   \
  fp w2=w*w; \
  fp x2=x*x; \
  fp y2=y*y; \
  fp z2=z*z; \
  fp result; \
  fp xy=x*y; \
  fp wz=w*z; \
  fp xz=x*z; \
  fp wy=w*y; \
  fp yz=y*z; \
  fp wx=w*x; \
  fp Mx,My,Mz; \
  if(comp==0) { \
    Mx=(w2+x2-y2-z2); \
    My=2*(xy+wz); \
    Mz=2*(xz-wy); \
  } else if(comp==1) { \
    Mx=2*(xy-wz); \
    My=(w2+x2-y2-z2); \
    Mz=2*(yz+wx); \
  } else { \
    Mx=2*(xz+wy); \
    My=2*(yz-wx); \
    Mz=(w2+x2-y2-z2); \
  } \
  result=Mx*a##x+My*a##y+Mz*a##z; \

void quatNormalize(void) {
  fp elen=sqrt(k_IMUQ.xh.data[0]*k_IMUQ.xh.data[0]+
               k_IMUQ.xh.data[1]*k_IMUQ.xh.data[1]+
               k_IMUQ.xh.data[2]*k_IMUQ.xh.data[2]+
               k_IMUQ.xh.data[3]*k_IMUQ.xh.data[3]);
  k_IMUQ.xh.data[0]/=elen;
  k_IMUQ.xh.data[1]/=elen;
  k_IMUQ.xh.data[2]/=elen;
  k_IMUQ.xh.data[3]/=elen;
}

//Sometimes you only need one component of a transformed vector
fp i2b_comp(fp Ix, fp Iy, fp Iz, int comp) {
  fp w=k_IMUQ.xh.data[0];
  fp x=k_IMUQ.xh.data[1];
  fp y=k_IMUQ.xh.data[2];
  fp z=k_IMUQ.xh.data[3];
  quat_guts(I);
  return result;
}

extern circular sensorBuf;

//Same as above, but with quaternion conjugate
fp b2i_comp(fp Bx, fp By, fp Bz, int comp) {
  fp w=k_IMUQ.xh.data[0];
  fp x=-k_IMUQ.xh.data[1];
  fp y=-k_IMUQ.xh.data[2];
  fp z=-k_IMUQ.xh.data[3];

  quat_guts(B);
  return result;
}

int initIMU() {
  int result=kalInit(k_IMUQ,IMUQ_M,IMUQ_N,imuq,F_IMUQ,Phi_IMUQ);
  if(result<0) return result*100-1;
  result=kalInit(k_IMUR,IMUR_M,IMUR_N,imur,F_IMUR,Phi_IMUR);
  if(result<0) return result*100-2;
  k_IMUQ.xh.data[0]=1;
  for(int i=0;i<IMUQ_M;i++) px(&k_IMUQ.P,i,i,1.0);
  for(int i=4;i<IMUQ_M;i++) px(&k_IMUQ.Q,i,i,1);
  for(int i=0;i<IMUR_M;i++) px(&k_IMUR.P,i,i,1.0);
  for(int i=6;i<IMUR_M;i++) px(&k_IMUR.Q,i,i,1);
  return 0;
}

int imuUpdate(sensor* s) {
  fp dt=ekf_update_time(s->k,s->TC);
  for(int i=0;i<s->n_k;i++) {
    if(s->g_ofs+i>s->k->n) return -i*2-1;
    assertmx(ekf_step(s->k, i==0?dt:0, s->cal[i], s->R[i], s->g[s->g_ofs+i], s->H[s->g_ofs+i]),-i*2-2);
  }
  quatNormalize();
  return 0;
}

void writeIMUstate(circular *buf,int result) {
  fillPktStart(buf,PT_I2C);
  fillPktString(buf,"IMU");
  fillPktInt(buf,result);
  fillPktInt(buf,k_IMUQ.lastUpdate/60);
  fillPktInt(buf,k_IMUQ.lastUpdate%60);
  for(int i=0;i<k_IMUQ.xh.row;i++) fillPktFP(buf,k_IMUQ.xh.data[i]);
  for(int i=0;i<k_IMUR.xh.row;i++) fillPktFP(buf,k_IMUR.xh.data[i]);
  fillPktFinish(buf);
}

int IMUOrient(fp pbx, fp pby, fp pbz,
              fp prx, fp pry, fp prz,
              fp tbx, fp tby, fp tbz,
              fp trx, fp try, fp trz, circular* buf) {
  //Set things up in matrix form
  fp pbdata[3], prdata[3], tbdata[3], trdata[3],
     nbdata[3], nrdata[3], sbdata[3], srdata[3];
  fp Ab2tdata[9], At2rdata[9], Ab2rdata[9];
  matrix pb={3,1,pbdata},
         pr={3,1,prdata},
         tb={3,1,tbdata},
         tr={3,1,trdata},
         nb={3,1,nbdata},
         nr={3,1,nrdata},
         sb={3,1,sbdata},
         sr={3,1,srdata},
         Ab2t={3,3,Ab2tdata},
         At2r={3,3,At2rdata},
         Ab2r={3,3,Ab2rdata};
  //Copy the input data into the vectors
  pbdata[0]=pbx;pbdata[1]=pby;pbdata[2]=pbz;
  prdata[0]=prx;prdata[1]=pry;prdata[2]=prz;
  tbdata[0]=tbx;tbdata[1]=tby;tbdata[2]=tbz;
  trdata[0]=trx;trdata[1]=try;trdata[2]=trz;
  //Make sure they are normalized
  mxs(&pb,1.0/vlength(&pb));
  mxs(&pr,1.0/vlength(&pr));
  mxs(&tb,1.0/vlength(&tb));
  mxs(&tr,1.0/vlength(&tr));

  //Do the cross products
  assertmx(vcross(&nb,&tb,&pb),-6);
  mxs(&nb,1.0/vlength(&nb));
  assertmx(vcross(&nr,&tr,&pr),-7);
  mxs(&nr,1.0/vlength(&nr));
  assertmx(vcross(&sb,&pb,&nb),-8);
  assertmx(vcross(&sr,&pr,&nr),-9);
  for(int i=0;i<3;i++) {
    //Load up Ab2t with row vectors
    px(&Ab2t,0,i,nb.data[i]);
    px(&Ab2t,1,i,sb.data[i]);
    px(&Ab2t,2,i,pb.data[i]);

    //Load up At2r with column vectors
    px(&At2r,i,0,nr.data[i]);
    px(&At2r,i,1,sr.data[i]);
    px(&At2r,i,2,pr.data[i]);
  }
  assertmx(mx(&Ab2r,&At2r,&Ab2t),-10);

  assertmx(mq(&k_IMUQ.xh,&Ab2t),-11);
  //Jeppesen's law of signs. We don't need to conjugate (Good night, everyone)
  //here for whatever reason
//  k_IMUQ.xh.data[1]=-k_IMUQ.xh.data[1];
//  k_IMUQ.xh.data[2]=-k_IMUQ.xh.data[2];
//  k_IMUQ.xh.data[3]=-k_IMUQ.xh.data[3];

  fillPktMx(buf,"IMUO pb", &pb);
  fillPktMx(buf,"IMUO pr", &pr);
  fillPktMx(buf,"IMUO tb", &tb);
  fillPktMx(buf,"IMUO tr", &tr);
  fillPktMx(buf,"IMUO nb", &nb);
  fillPktMx(buf,"IMUO nr", &nr);
  fillPktMx(buf,"IMUO sb", &sb);
  fillPktMx(buf,"IMUO sr", &sr);
  fillPktMx(buf,"IMUO At2r", &At2r);
  fillPktMx(buf,"IMUO Ab2t", &Ab2t);
  fillPktMx(buf,"IMUO Ab2r", &Ab2r);
  fillPktMx(buf,"IMUO xh", &k_IMUQ.xh);

  return 0;
}

int F_IMUQ(matrix* F, matrix* x) {
#ifdef CHECK_COMPAT
  if(F->row!=IMUQ_M) return -1;
  if(F->col!=1) return -2;
  if(x->row!=IMUQ_M) return -3;
  if(x->col!=1) return -4;
#endif
  fp ewh = x->data[ 0]*0.5;
  fp exh = x->data[ 1]*0.5;
  fp eyh = x->data[ 2]*0.5;
  fp ezh = x->data[ 3]*0.5;
  fp wx  = x->data[ 4];
  fp wy  = x->data[ 5];
  fp wz  = x->data[ 6];
  F->data[ 0] = -exh*wx-eyh*wy-ezh*wz;
  F->data[ 1] =  ewh*wx+eyh*wz-ezh*wy;
  F->data[ 2] =  ewh*wy-exh*wz+ezh*wx;
  F->data[ 3] =  ewh*wz+exh*wy-eyh*wx;
  F->data[ 4] = 0.0;
  F->data[ 5] = 0.0;
  F->data[ 6] = 0.0;
  return 0;
}

int F_IMUR(matrix* F, matrix* x) {
#ifdef CHECK_COMPAT
  if(F->row!=IMUR_M) return -1;
  if(F->col!=1) return -2;
  if(x->row!=IMUR_M) return -3;
  if(x->col!=1) return -4;
#endif
  fp vx  = x->data[3];
  fp vy  = x->data[4];
  fp vz  = x->data[5];
  fp aix = x->data[6];
  fp aiy = x->data[7];
  fp aiz = x->data[8];
  F->data[0] = vx;
  F->data[1] = vy;
  F->data[2] = vz;
  F->data[3] = aix;
  F->data[4] = aiy;
  F->data[5] = aiz;
  F->data[6] = 0.0;
  F->data[7] = 0.0;
  F->data[8] = 0.0;
  return 0;
}

int Phi_IMUQ(matrix* Phi,matrix* x) {
#ifdef CHECK_COMPAT
  if(Phi->row!=IMUQ_M) return -1;
  if(Phi->col!=IMUQ_M) return -2;
  if(x->row!=IMUQ_M) return -3;
  if(x->col!=1) return -4;
#endif
  fp ewh = x->data[0]*0.5;
  fp exh = x->data[1]*0.5;
  fp eyh = x->data[2]*0.5;
  fp ezh = x->data[3]*0.5;
  fp wxh = x->data[4]*0.5;
  fp wyh = x->data[5]*0.5;
  fp wzh = x->data[6]*0.5;
  int row=0,col=0;
      //0                      1                        2                        3                        4                        5                        6                                            8                     9                    10                    11                    12                    13                    14                    15
/* 0*/  px(Phi,row,col++,  0);px(Phi, row,col++, -wxh);px(Phi, row,col++, -wyh);px(Phi, row,col++, -wzh);px(Phi, row,col++, -exh);px(Phi, row,col++, -eyh);px(Phi, row,col++, -ezh);col=0;row++;
/* 1*/  px(Phi,row,col++,wxh);px(Phi, row,col++,    0);px(Phi, row,col++,  wzh);px(Phi, row,col++, -wyh);px(Phi, row,col++,  ewh);px(Phi, row,col++, -ezh);px(Phi, row,col++,  eyh);col=0;row++;
/* 2*/  px(Phi,row,col++,wyh);px(Phi, row,col++, -wzh);px(Phi, row,col++,    0);px(Phi, row,col++,  wxh);px(Phi, row,col++,  ezh);px(Phi, row,col++,  ewh);px(Phi, row,col++, -exh);col=0;row++;
/* 3*/  px(Phi,row,col++,wzh);px(Phi, row,col++,  wyh);px(Phi, row,col++, -wxh);px(Phi, row,col++,    0);px(Phi, row,col++, -eyh);px(Phi, row,col++,  exh);px(Phi, row,col++,  ewh);col=0;row++;
  return 0;
}

int Phi_IMUR(matrix* Phi,matrix* x) {
#ifdef CHECK_COMPAT
  if(Phi->row!=IMUR_M) return -1;
  if(Phi->col!=IMUR_M) return -2;
  if(x->row!=IMUR_M) return -3;
  if(x->col!=1) return -4;
#endif
  for(int i=0;i<6;i++) {
    px(Phi,i,i+3,1);
  }
  return 0;
}

fp g0_IMUQ(matrix* x) {
#ifdef CHECK_COMPAT
  if(x->row!=IMUQ_M) return -1;
  if(x->col!=1) return -2;
#endif
  fp wx  = x->data[ 4];
  return wx;
}

fp g1_IMUQ(matrix* x) {
#ifdef CHECK_COMPAT
  if(x->row!=IMUQ_M) return -1;
  if(x->col!=1) return -1;
#endif
  fp wy = x->data[5];
  return wy;
}

fp g2_IMUQ(matrix* x) {
#ifdef CHECK_COMPAT
  if(x->row!=IMUQ_M) return -1;
  if(x->col!=1) return -2;
#endif
  fp wz = x->data[6];
  return wz;
}

fp g3_IMUQ(matrix* x) {
#ifdef CHECK_COMPAT
  if(x->row!=IMUQ_M) return -1;
  if(x->col!=1) return -2;
#endif
  fp ew  = x->data[ 0];
  fp ex  = x->data[ 1];
  fp ey  = x->data[ 2];
  fp ez  = x->data[ 3];
  fp c00=(ew*ew+ex*ex-ey*ey-ez*ez);fp c01=2*(ew*ez + ex*ey);        fp c02=2*(ex*ez-ew*ey);
  return Bix*c00+Biy*c01+Biz*c02;
}

fp g4_IMUQ(matrix* x) {
#ifdef CHECK_COMPAT
  if(x->row!=IMUQ_M) return -1;
  if(x->col!=1) return -2;
#endif
  fp ew  = x->data[ 0];
  fp ex  = x->data[ 1];
  fp ey  = x->data[ 2];
  fp ez  = x->data[ 3];
  fp c10=2*(ex*ey-ew*ez);          fp c11=(ew*ew-ex*ex+ey*ey-ez*ez);fp c12=2*(ew*ex+ey*ez);
  return Bix*c10+Biy*c11+Biz*c12;
}

fp g5_IMUQ(matrix* x) {
  //G_IMU
  //    G = G_IMU(IN1,BIX,BIY,BIZ,M,N)

  //    This function was generated by the Symbolic Math Toolbox version 5.5.
  //    04-Apr-2011 19:11:28
  // Translated to C 06-Apr-2011 by Chris Jeppesen
#ifdef CHECK_COMPAT
  if(x->row!=IMUQ_M) return -1;
  if(x->col!=1) return -2;
#endif
  fp ew  = x->data[ 0];
  fp ex  = x->data[ 1];
  fp ey  = x->data[ 2];
  fp ez  = x->data[ 3];
  fp c20=2*(ew*ey+ex*ez);          fp c21=2*(ey*ez-ew*ex);          fp c22=(ew*ew-ex*ex-ey*ey+ez*ez);
  return Bix*c20+Biy*c21+Biz*c22;
}

fp g0_IMUR(matrix* x) {
#ifdef CHECK_COMPAT
  if(x->row!=IMUR_M) return -1;
  if(x->col!=1) return -2;
#endif
  fp rx  = x->data[0];
  return rx;
}

fp g1_IMUR(matrix* x) {
#ifdef CHECK_COMPAT
  if(x->row!=IMUR_M) return -1;
  if(x->col!=1) return -1;
#endif
  fp ry  = x->data[1];
  return ry;
}

fp g2_IMUR(matrix* x) {
#ifdef CHECK_COMPAT
  if(x->row!=IMUR_M) return -1;
  if(x->col!=1) return -2;
#endif
  fp rz  = x->data[2];
  return rz;
}

fp g3_IMUR(matrix* x) {
#ifdef CHECK_COMPAT
  if(x->row!=IMUR_M) return -1;
  if(x->col!=1) return -2;
#endif
  fp vx  = x->data[3];
  return vx;
}

fp g4_IMUR(matrix* x) {
#ifdef CHECK_COMPAT
  if(x->row!=IMUR_M) return -1;
  if(x->col!=1) return -1;
#endif
  fp vy  = x->data[4];
  return vy;
}

fp g5_IMUR(matrix* x) {
#ifdef CHECK_COMPAT
  if(x->row!=IMUR_M) return -1;
  if(x->col!=1) return -2;
#endif
  fp vz  = x->data[5];
  return vz;
}

fp g6_IMUR(matrix* x) {
#ifdef CHECK_COMPAT
  if(x->row!=IMUR_M) return -1;
  if(x->col!=1) return -2;
#endif
  fp ew  = k_IMUQ.xh.data[ 0];
  fp ex  = k_IMUQ.xh.data[ 1];
  fp ey  = k_IMUQ.xh.data[ 2];
  fp ez  = k_IMUQ.xh.data[ 3];
  fp aix = x->data[6];
  fp aiy = x->data[7];
  fp aiz = x->data[8];
  fp c00=(ew*ew+ex*ex-ey*ey-ez*ez);
  fp c10=2*(ex*ey-ew*ez);
  fp c20=2*(ew*ey+ex*ez);
  return aix*c00+aiy*c10+(aiz+gz)*c20;
}

fp g7_IMUR(matrix* x) {
#ifdef CHECK_COMPAT
  if(x->row!=IMUR_M) return -1;
  if(x->col!=1) return -2;
#endif
  fp ew  = k_IMUQ.xh.data[ 0];
  fp ex  = k_IMUQ.xh.data[ 1];
  fp ey  = k_IMUQ.xh.data[ 2];
  fp ez  = k_IMUQ.xh.data[ 3];
  fp aix = x->data[6];
  fp aiy = x->data[7];
  fp aiz = x->data[8];
  fp c01=2*(ew*ez + ex*ey);
  fp c11=(ew*ew-ex*ex+ey*ey-ez*ez);
  fp c21=2*(ey*ez-ew*ex);
  return aix*c01+aiy*c11+(aiz+gz)*c21;
}

fp g8_IMUR(matrix* x) {
#ifdef CHECK_COMPAT
  if(x->row!=IMUR_M) return -1;
  if(x->col!=1) return -2;
#endif
  fp ew  = k_IMUQ.xh.data[ 0];
  fp ex  = k_IMUQ.xh.data[ 1];
  fp ey  = k_IMUQ.xh.data[ 2];
  fp ez  = k_IMUQ.xh.data[ 3];
  fp aix = x->data[6];
  fp aiy = x->data[7];
  fp aiz = x->data[8];
  fp c02=2*(ex*ez-ew*ey);
  fp c12=2*(ew*ex+ey*ez);
  fp c22=(ew*ew-ex*ex-ey*ey+ez*ez);
  return aix*c02+aiy*c12+(aiz+gz)*c22;
}

int H0_IMUQ(matrix* H,matrix* x) {
#ifdef CHECK_COMPAT
  if(H->row!=1) return -1;
  if(H->col!=IMUQ_M) return -2;
  if(x->row!=IMUQ_M) return -3;
  if(x->col!=1) return -4;
#endif
  /* 0*/  H->data[0]=0.0;	H->data[1]=    0.0;	H->data[2]=    0.0;	H->data[3]=    0.0;	H->data[4]=    1.0;	H->data[5]=    0.0;    	H->data[6]=    0.0;
  return 0;
}

int H1_IMUQ(matrix* H,matrix* x) {
#ifdef CHECK_COMPAT
  if(H->row!=1) return -1;
  if(H->col!=IMUQ_M) return -2;
  if(x->row!=IMUQ_M) return -3;
  if(x->col!=1) return -4;
#endif
  /* 1*/  H->data[0]=    0.0;	H->data[1]=    0.0;	H->data[2]=    0.0;	H->data[3]=    0.0;	H->data[4]=    0.0;	H->data[5]=    1.0;	H->data[6]=    0.0;
  return 0;
}

int H2_IMUQ(matrix* H,matrix* x) {
#ifdef CHECK_COMPAT
  if(H->row!=1) return -1;
  if(H->col!=IMUQ_M) return -2;
  if(x->row!=IMUQ_M) return -3;
  if(x->col!=1) return -4;
#endif
  /* 2*/  H->data[0]=    0.0;	H->data[1]=    0.0;	H->data[2]=    0.0;	H->data[3]=    0.0;	H->data[4]=    0.0;	H->data[5]=    0.0;	H->data[6]=    1.0;
  return 0;
}

int H3_IMUQ(matrix* H,matrix* x) {
#ifdef CHECK_COMPAT
  if(H->row!=1) return -1;
  if(H->col!=IMUQ_M) return -2;
  if(x->row!=IMUQ_M) return -3;
  if(x->col!=1) return -4;
#endif
  fp ew = x->data[ 0];
  fp ex = x->data[ 1];
  fp ey = x->data[ 2];
  fp ez = x->data[ 3];
  fp t564 = Biy*ew*2.0;
  fp t565 = Biz*ex*2.0;
  fp t577 = Bix*ez*2.0;
  fp t566 = t564+t565-t577;
  fp t567 = Biy*ex*2.0;
  fp t568 = Bix*ex*2.0;
  fp t569 = Biy*ey*2.0;
  fp t570 = Biz*ez*2.0;
  fp t571 = t568+t569+t570;
  fp t572 = Bix*ew*2.0;
  fp t573 = Biy*ez*2.0;
  /* 3*/  H->data[0]=    t572+t573-Biz*ey*2.0;	H->data[1]=    t571;	H->data[2]=    t567-Biz*ew*2.0-Bix*ey*2.0;	H->data[3]=    t566;	H->data[4]=    0.0;	H->data[5]=    0.0;	H->data[6]=    0.0;
  return 0;
}

int H4_IMUQ(matrix* H,matrix* x) {
#ifdef CHECK_COMPAT
  if(H->row!=1) return -1;
  if(H->col!=IMUQ_M) return -2;
  if(x->row!=IMUQ_M) return -3;
  if(x->col!=1) return -4;
#endif
  fp ew = x->data[ 0];
  fp ex = x->data[ 1];
  fp ey = x->data[ 2];
  fp ez = x->data[ 3];
  fp t564 = Biy*ew*2.0;
  fp t565 = Biz*ex*2.0;
  fp t577 = Bix*ez*2.0;
  fp t566 = t564+t565-t577;
  fp t567 = Biy*ex*2.0;
  fp t568 = Bix*ex*2.0;
  fp t569 = Biy*ey*2.0;
  fp t570 = Biz*ez*2.0;
  fp t571 = t568+t569+t570;
  fp t572 = Bix*ew*2.0;
  fp t573 = Biy*ez*2.0;
  fp t574 = Biz*ew*2.0;
  fp t575 = Bix*ey*2.0;
  fp t576 = -t567+t574+t575;
  fp t578 = Biz*ey*2.0;
  /* 4*/  H->data[0]=    t566;	H->data[1]=    t576;	H->data[2]=    t571;	H->data[3]=    -t572-t573+t578;	H->data[4]=    0.0;	H->data[5]=    0.0;	H->data[6]=    0.0;
  return 0;
}

int H5_IMUQ(matrix* H,matrix* x) {
#ifdef CHECK_COMPAT
  if(H->row!=1) return -1;
  if(H->col!=IMUQ_M) return -2;
  if(x->row!=IMUQ_M) return -3;
  if(x->col!=1) return -4;
#endif
  fp ew = x->data[ 0];
  fp ex = x->data[ 1];
  fp ey = x->data[ 2];
  fp ez = x->data[ 3];
  fp t564 = Biy*ew*2.0;
  fp t565 = Biz*ex*2.0;
  fp t577 = Bix*ez*2.0;
  fp t567 = Biy*ex*2.0;
  fp t568 = Bix*ex*2.0;
  fp t569 = Biy*ey*2.0;
  fp t570 = Biz*ez*2.0;
  fp t571 = t568+t569+t570;
  fp t572 = Bix*ew*2.0;
  fp t573 = Biy*ez*2.0;
  fp t574 = Biz*ew*2.0;
  fp t575 = Bix*ey*2.0;
  fp t576 = -t567+t574+t575;
  fp t578 = Biz*ey*2.0;
  /* 5*/  H->data[0]=    t576;	H->data[1]=    -t564-t565+t577;	H->data[2]=    t572+t573-t578;	H->data[3]=    t571;	H->data[4]=    0.0;	H->data[5]=    0.0;	H->data[6]=    0.0;
  return 0;
}

#define Hn_IMUR(j) \
int H##j##_IMUR(matrix* H,matrix* x) { \
  for(int i=0;i<9;i++) H->data[i]=(i==j)?1.0:0.0; \
  return 0; \
}

Hn_IMUR(0)
Hn_IMUR(1)
Hn_IMUR(2)
Hn_IMUR(3)
Hn_IMUR(4)
Hn_IMUR(5)

int H6_IMUR(matrix* H,matrix* x) {
#ifdef CHECK_COMPAT
  if(H->row!=1) return -1;
  if(H->col!=IMUR_M) return -2;
  if(x->row!=IMUR_M) return -3;
  if(x->col!=1) return -4;
#endif
  fp ew  = k_IMUQ.xh.data[ 0];
  fp ex  = k_IMUQ.xh.data[ 1];
  fp ey  = k_IMUQ.xh.data[ 2];
  fp ez  = k_IMUQ.xh.data[ 3];
  for(int i=0;i<6;i++) H->data[i]=0.0;
  H->data[6]=ew*ew+ex*ex-ey*ey-ez*ez;
  H->data[7]=2*(ex*ey-ew*ez);
  H->data[8]=2*(ew*ey+ex*ez);
  return 0;
}

int H7_IMUR(matrix* H,matrix* x) {
#ifdef CHECK_COMPAT
  if(H->row!=1) return -1;
  if(H->col!=IMUR_M) return -2;
  if(x->row!=IMUR_M) return -3;
  if(x->col!=1) return -4;
#endif
  fp ew  = k_IMUQ.xh.data[ 0];
  fp ex  = k_IMUQ.xh.data[ 1];
  fp ey  = k_IMUQ.xh.data[ 2];
  fp ez  = k_IMUQ.xh.data[ 3];
  for(int i=0;i<6;i++) H->data[i]=0.0;
  H->data[6]=2*(ew*ez+ex*ey);
  H->data[7]=ew*ew-ex*ex+ey*ey-ez*ez;
  H->data[8]=2*(ey*ez-ew*ex);
  return 0;
}

int H8_IMUR(matrix* H,matrix* x) {
#ifdef CHECK_COMPAT
  if(H->row!=1) return -1;
  if(H->col!=IMUR_M) return -2;
  if(x->row!=IMUR_M) return -3;
  if(x->col!=1) return -4;
#endif
  fp ew  = k_IMUQ.xh.data[ 0];
  fp ex  = k_IMUQ.xh.data[ 1];
  fp ey  = k_IMUQ.xh.data[ 2];
  fp ez  = k_IMUQ.xh.data[ 3];
  for(int i=0;i<6;i++) H->data[i]=0.0;
  H->data[6]=2*(ex*ez-ew*ey);
  H->data[7]=2*(ew*ex-ey*ez);
  H->data[8]=ew*ew-ex*ex-ey*ey+ez*ez;
  return 0;
}

const gfunc g_IMUQ[]={g0_IMUQ,g1_IMUQ,g2_IMUQ,g3_IMUQ,g4_IMUQ,g5_IMUQ};
const Ffunc H_IMUQ[]={H0_IMUQ,H1_IMUQ,H2_IMUQ,H3_IMUQ,H4_IMUQ,H5_IMUQ};
const gfunc g_IMUR[]={g0_IMUR,g1_IMUR,g2_IMUR,g3_IMUR,g4_IMUR,g5_IMUR,g6_IMUR,g7_IMUR,g8_IMUR};
const Ffunc H_IMUR[]={H0_IMUR,H1_IMUR,H2_IMUR,H3_IMUR,H4_IMUR,H5_IMUR,H6_IMUR,H7_IMUR,H8_IMUR};

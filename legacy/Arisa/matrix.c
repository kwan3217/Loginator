#include "matrix.h"
#include "sdbuf.h"
#include <math.h>

//Performs C=A*B. Can't be in place.
int mx(matrix* c, matrix* a, matrix* b) {
  //check dimension constraints
#ifdef CHECK_COMPAT
  if(a==c) return -1; //Can't do this in place
  if(b==c) return -1;
  if(a->col!=b->row) return -1; //Check factor matrix compatibility
  if(c->row!=a->row) return -2; //Check product matrix size
  if(c->col!=b->col) return -3;
#endif
  for(int i=0;i<c->row;i++) {
    for(int j=0;j<c->col;j++) {
      int cidx=idx(c,i,j);
      c->data[cidx]=0;
      for(int k=0;k<a->col;k++) {
        int aidx=idx(a,i,k);
        int bidx=idx(b,k,j);
        c->data[cidx]+=a->data[aidx]*b->data[bidx];
      }
    }
  }
  return 0;
}

//Performs C=A*B'. Can't be in place.
int mxt(matrix* c, matrix* a, matrix* bt) {
  //check dimension constraints
#ifdef CHECK_COMPAT
  if(a==c) return -1; //Can't do this in place
  if(bt==c) return -1;
  if(a->col!=bt->col) return -1; //Check factor matrix compatibility
  if(c->row!=a->row)  return -2; //Check product matrix size
  if(c->col!=bt->row) return -3;
#endif
  for(int i=0;i<c->row;i++) {
    for(int j=0;j<c->col;j++) {
      int cidx=idx(c,i,j);
      c->data[cidx]=0;
      for(int k=0;k<a->col;k++) {
        int aidx=idx(a,i,k);
        int bidx=idx(bt,j,k);
        c->data[cidx]+=a->data[aidx]*bt->data[bidx];
      }
    }
  }
  return 0;
}

//Performs c=A*B for the special case of a 1xm by mx1 multiplication, which does
//occur in the Kalman filter.
int mx1(fp* c, matrix* a, matrix* b) {
#ifdef CHECK_COMPAT
  if(a->row!=1) return -1;
  if(b->col!=1) return -2;
#endif
  *c=0;
  for(int i=0;i<a->col;i++) {
    *c+=a->data[i]*b->data[i];
  }
  return 0;
}

//Performs c=A*B' for the special case of a 1xm by 1xm multiplication, which does
//occur in the Kalman filter.
int mx1t(fp* c, matrix* a, matrix* bt) {
#ifdef CHECK_COMPAT
  if(a->row!=1) return -1;
  if(bt->row!=1) return -2;
#endif
  *c=0;
  for(int i=0;i<a->col;i++) {
    *c+=a->data[i]*bt->data[i];
  }
  return 0;
}

//Performs A=b*A, Multiply a matrix by a scalar in-place. This one cannot fail
void mxs(matrix* a, fp b) {
  for(int i=0;i<a->col*a->row;i++) {
    a->data[i]*=b;
  }
}

//Performs A=A+B in place.
int ma(matrix* a, matrix* b) {
  //check dimension constraints
#ifdef CHECK_COMPAT
  if(a->row!=b->row) return -1; //Check term matrix compatibility
  if(a->col!=b->col) return -2;
#endif
  for(int i=0;i<a->row*a->col;i++) {
    a->data[i]+=b->data[i];
  }
  return 0;
}

//Performs A=A-B in place.
int ms(matrix* a, matrix* b) {
  //check dimension constraints
#ifdef CHECK_COMPAT
  if(a->row!=b->row) return -1; //Check term matrix compatibility
  if(a->col!=b->col) return -2;
#endif
  for(int i=0;i<a->row*a->col;i++) {
    a->data[i]-=b->data[i];
  }
  return 0;
}

//Performs vc=va x vb. Can't be in place.
int vcross(matrix* vc, matrix* va, matrix* vb) {
  //check dimension constraints
#ifdef CHECK_COMPAT
  if(va==vc) return -1; //Can't do this in place
  if(vb==vc) return -2;
  if(va->row!=3 || va->col!=1) return -3;
  if(vb->row!=3 || vb->col!=1) return -4;
  if(vc->row!=3 || vc->col!=1) return -5;
#endif
  vc->data[0]=va->data[1]*vb->data[2]-va->data[2]*vb->data[1];
  vc->data[1]=va->data[2]*vb->data[0]-va->data[0]*vb->data[2];
  vc->data[2]=va->data[0]*vb->data[1]-va->data[1]*vb->data[0];
  return 0;
}

//Transforms a matrix which does a vector up like <vt>=[M]<vf> into a quaternion
//which does up a vector like <vt>=<e~><vf><e>
int mq(matrix* q, matrix* m) {
#ifdef CHECK_COMPAT
  if(q==m) return -1; //Doesn't even make sense to do in place
  if(m->row!=3 || m->col!=3) return -2; //Need a 3x3 matrix.
  if(q->row<4)               return -3; //Just check that the vector is at least
                                        //long enough
#endif
  //We go with 1-based indexes, since it matches the documentation
  fp c11=gx(m,0,0),c12=gx(m,0,1),c13=gx(m,0,2);
  fp c21=gx(m,1,0),c22=gx(m,1,1),c23=gx(m,1,2);
  fp c31=gx(m,2,0),c32=gx(m,2,1),c33=gx(m,2,2);
  fp qw,qx,qy,qz;
  if((qw=sqrt(c11+c22+c33+1)/2.0)>0.5) {
    qx=(c23-c32)/(4*qw);
    qy=(c31-c13)/(4*qw);
    qz=(c12-c21)/(4*qw);
  } else if((qx=sqrt(c11-c22-c33+1)/2.0)>0.5) {
    qw=(c23-c32)/(4*qx);
    qy=(c12+c21)/(4*qx);
    qz=(c13+c31)/(4*qx);
  } else if((qy=sqrt(-c11+c22-c33+1)/2.0)>0.5) {
    qw=(c31-c13)/(4*qy);
    qx=(c12+c21)/(4*qy);
    qz=(c23+c32)/(4*qy);
  } else {qz=sqrt(-c11-c22+c33+1)/2.0;
    qw=(c12-c21)/(4*qz);
    qx=(c13+c31)/(4*qz);
    qy=(c23+c32)/(4*qz);
  }
  q->data[0]=qw;
  q->data[1]=qx;
  q->data[2]=qy;
  q->data[3]=qz;
  return 0;
}

#include "pktwrite.h"

void fillPktRow(circular* buf, matrix* M, int row) {
  for(int col=0;col<M->col;col++) {
    fillPktFP(buf,gx(M,row,col));
  }
}

void fillPktVec(circular* buf, matrix* M) {
  for(int ele=0;ele<M->row;ele++) {
    fillPktFP(buf,M->data[ele]);
  }
}

void fillPktMx(circular* buf, char* name, matrix* M) {
  for(int row=0;row<M->row;row++) {
    fillPktStart(buf,PT_I2C);
    fillPktString(buf,name);
    fillPktInt(buf,row);
    for(int col=0;col<M->col;col++) {
      fillPktFP(buf,gx(M,row,col));
    }
    fillPktFinish(buf);
#ifndef HOST
    drainToSD(buf);
#endif
  }
}


#include "matrix.h"

#ifdef CHECK_COMPAT
int mxerrnum=0;
#endif

//Performs this=A*B. Can't be in place.
void matrix::mx(matrix& a, matrix& b) {
  //check dimension constraints
#ifdef CHECK_COMPAT
  if(&a==this) {mxerrnum=-1;return;} //Can't do this in place
  if(&b==this) {mxerrnum=-2;return;}
  if(a.cols!=b.rows) {mxerrnum=-3;return;} //Check factor matrix compatibility
  if(  rows!=a.rows) {mxerrnum=-4;return;} //Check product matrix size
  if(  cols!=b.cols) {mxerrnum=-5;return;}
  mxerrnum=0;
#endif
  for(int i=0;i<rows;i++) {
    for(int j=0;j<cols;j++) {
      int cidx=idx(i,j);
      data[cidx]=0;
      for(int k=0;k<a.cols;k++) {
        data[cidx]+=a(i,k)*b(k,j);
      }
    }
  }
}

//Performs this=A*B'. Can't be in place.
void matrix::mxt(matrix& a, matrix& bt) {
  //check dimension constraints
#ifdef CHECK_COMPAT
  if(&a==this) {mxerrnum=-1;return;} //Can't do this in place
  if(&bt==this) {mxerrnum=-2;return;}
  if(a.cols!=bt.cols) {mxerrnum=-3;return;} //Check factor matrix compatibility
  if(  rows!=a.rows)  {mxerrnum=-4;return;} //Check product matrix size
  if(  cols!=bt.rows) {mxerrnum=-5;return;}
  mxerrnum=0;
#endif
  for(int i=0;i<rows;i++) {
    for(int j=0;j<cols;j++) {
      int cidx=idx(i,j);
      data[cidx]=0;
      for(int k=0;k<a.cols;k++) {
        data[cidx]+=a(i,k)*bt(j,k);
      }
    }
  }
}

//Performs this=A'*b. Can't be in place.
void matrix::mts(matrix& at, fp b) {
  //check dimension constraints
#ifdef CHECK_COMPAT
  if(&at==this) {mxerrnum=-1;return;} //Can't do this in place
  if(rows!=at.cols) {mxerrnum=-2;return;} //Check factor matrix compatibility
  if(cols!=at.rows) {mxerrnum=-3;return;}
  mxerrnum=0;
#endif
  for(int i=0;i<rows;i++) {
    for(int j=0;j<cols;j++) {
      data[idx(i,j)]=at(j,i)*b;
    }
  }
}

//Performs c=A*B for the special case of a 1xm by mx1 multiplication, which does
//occur in the Kalman filter.
fp matrix::mx1(matrix& a, matrix& b) {
#ifdef CHECK_COMPAT
  if(a.rows!=1) {mxerrnum=-1;return 0;};
  if(b.cols!=1) {mxerrnum=-2;return 0;};
  mxerrnum=0;
#endif
  fp c=0;
  for(int i=0;i<a.cols;i++) {
    c+=a.data[i]*b.data[i];
  }
  return c;
}

//Performs c=A*B' for the special case of a 1xm by 1xm multiplication, which does
//occur in the Kalman filter.
fp matrix::mx1t(matrix& a, matrix& bt) {
#ifdef CHECK_COMPAT
  if(a.rows!=1) {mxerrnum=-1;return 0;}
  if(bt.rows!=1) {mxerrnum=-2;return 0;}
  mxerrnum=0;
#endif
  fp c=0;
  for(int i=0;i<a.cols;i++) {
    c+=a.data[i]*bt.data[i];
  }
  return c;
}

//Performs this*=b, Multiply a matrix by a scalar in-place. This one cannot fail
matrix& matrix::operator*=(fp b) {
  for(int i=0;i<cols*rows;i++) {
    data[i]*=b;
  }
  return *this;
}

//Performs this+=B in place.
matrix& matrix::operator+=(matrix& b) {
  //check dimension constraints
#ifdef CHECK_COMPAT
  if(b.rows!=rows) {mxerrnum=-1;return *this;}
  if(b.cols!=cols) {mxerrnum=-2;return *this;}
  mxerrnum=0;
#endif
  for(int i=0;i<rows*cols;i++) {
    data[i]+=b.data[i];
  }
  return *this;
}

//Performs this-=B in place.
matrix& matrix::operator-=(matrix& b) {
  //check dimension constraints
#ifdef CHECK_COMPAT
  if(b.rows!=rows) {mxerrnum=-1;return *this;}
  if(b.cols!=cols) {mxerrnum=-2;return *this;}
  mxerrnum=0;
#endif
  for(int i=0;i<rows*cols;i++) {
    data[i]-=b.data[i];
  }
  return *this;
}

//Performs vc=va x vb. Can't be in place.
void matrix::vcross(matrix& va, matrix& vb) {
  //check dimension constraints
#ifdef CHECK_COMPAT
  if(&va==this) {mxerrnum=-1;return;} //Can't do this in place
  if(&vb==this) {mxerrnum=-2;return;}
  if(va.rows*va.cols!=3) {mxerrnum=-3;return;}
  if(vb.rows*vb.cols!=3) {mxerrnum=-4;return;}
  if(   rows*   cols!=3) {mxerrnum=-5;return;}
  mxerrnum=0;
#endif
  data[0]=va.data[1]*vb.data[2]-va.data[2]*vb.data[1];
  data[1]=va.data[2]*vb.data[0]-va.data[0]*vb.data[2];
  data[2]=va.data[0]*vb.data[1]-va.data[1]*vb.data[0];
}

//Transforms a matrix which does a vector up like <vt>=[M]<vf> into a quaternion
//which does up a vector like <vt>=<this~><vf><this>
void matrix::mq(matrix& m, int ofs) {
#ifdef CHECK_COMPAT
  if(this==&m) {mxerrnum=-1;return;} //Doesn't even make sense to do in place
  if(m.rows!=3 || m.cols!=3) {mxerrnum=-2;return;} //Need a 3x3 matrix.
  if(rows*cols<4+ofs)        {mxerrnum=-3;return;} //Just check that the vector
                                                  //is at least long enough
  mxerrnum=0;
#endif
  //We go with 1-based indexes, since it matches the documentation
  fp c11=m(0,0),c12=m(0,1),c13=m(0,2);
  fp c21=m(1,0),c22=m(1,1),c23=m(1,2);
  fp c31=m(2,0),c32=m(2,1),c33=m(2,2);
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
  data[0+ofs]=qw;
  data[1+ofs]=qx;
  data[2+ofs]=qy;
  data[3+ofs]=qz;
}

#ifdef PACKET
#include "pktwrite.h"

void matrix::fillPktRow(circular& buf, int row) {
  for(int col=0;col<cols;col++) {
    fillPktFP(buf,(*this)(row,col));
  }
}

void matrix::fillPktVec(circular& buf) {
  for(int ele=0;ele<rows*cols;ele++) {
    fillPktFP(buf,data[ele]);
  }
}

void matrix::fillPktMx(circular& buf, const char* name) {
  for(int row=0;row<rows;row++) {
    fillPktStart(buf,PT_I2C);
    fillPktString(buf,name);
    fillPktInt(buf,row);
    for(int col=0;col<cols;col++) {
      fillPktFP(buf,(*this)(row,col));
    }
    fillPktFinish(buf);
  }
}

#endif


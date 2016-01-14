#ifndef MATRIX_H
#define MATRIX_H

#define CHECK_COMPAT
#define CHECK_ASSERT

#include "float.h"
#ifdef PACKET
#include "Circular.h"
#endif

#include <math.h>

extern int mxerrnum;

#ifdef CHECK_ASSERT
#define assertmx(x,y) {x; if(mxerrnum<0) {mxerrnum=mxerrnum*100+y;return mxerrnum;}}
#else
#define assertmx(x,y) x
#endif

//Row index is first, then column. A row is contiguous in memory. I think this
//makes it "Row major" but who cares. Both indices start at 0.
class matrix {
public:
  const int rows,cols;
  fp* data;  //This is a pointer for the same reason strings are char*, variable length
  matrix(int Lrows, int Lcols, fp* Ldata):rows(Lrows),cols(Lcols),data(Ldata) {}
  int idx(int row, int col) {return row*cols+col;}
  fp& operator() (unsigned row, unsigned col) {return data[idx(row,col)];}
  //Effectively this=a. Copy elements from matrix a to matrix b
  matrix& operator=(const matrix& a) {
    #ifdef CHECK_COMPAT
      if(&a==this) {mxerrnum=-1;return *this;} //Can't do this in place
      if(rows!=a.rows) {mxerrnum=-2;return *this;} //Check matrix compatibility
      if(cols!=a.cols) {mxerrnum=-3;return *this;}
      mxerrnum=0;
    #endif
    for(int i=0;i<a.rows*a.cols;i++) data[i]=a.data[i];
    return *this;
  }

  //Performs this=va x vb. Can't be in place.
  void vcross(matrix& va, matrix& vb);
  //Performs va . vb.
  static fp  vdot(matrix& a, matrix& b) {
    #ifdef CHECK_COMPAT
      if((b.rows*b.cols)!=(a.rows*a.cols)) {mxerrnum=-1;return 0;} //Check matrix compatibility
      mxerrnum=0;
    #endif
    fp c=0.0;
    for(int i=0;i<a.rows*a.cols;i++)c+=a.data[i]*b.data[i];
    return c;
  }
  fp vlensq() {return vdot(*this,*this);}
  fp vlength() {return sqrt(vlensq());}
  void normalize() {*this*=(1.0/sqrt(vlength()));};


  //Transforms a matrix which does a vector up like <vt>=[M]<vf> into a quaternion
  //which does up a vector like <vt>=<this~><vf><this>
  void mq(matrix& m,int ofs=0);
  //Transforms a quaternion which does a vector up like <vt>=<this~><vf><this> into a quaternion
  //which does up a vector like <vt>=[M]<vf>
  void qm(matrix& m,int ofs=0);
  //this=A*B
  void mx(matrix& a, matrix& b);
  //this=A*B'
  void mxt(matrix& a, matrix& bt);
  //this=A'*b
  void mts(matrix& at, fp b);
  //c=A*B, A is 1xm and B is mx1
  static fp mx1(matrix& a, matrix& b);
  //c=A*B', A is 1xm and B is 1xm
  static fp mx1t(matrix& a, matrix& bt);
  //this*=b
  matrix& operator*=(fp b);
  //this+=B
  matrix& operator+=(matrix& b);
  //this-=B
  matrix& operator-=(matrix& b);
#ifdef PACKET
  void fillPktRow(circular& buf, int row);
  void fillPktVec(circular& buf);
  void fillPktMx(circular& buf, const char* name);
#endif
};

#endif

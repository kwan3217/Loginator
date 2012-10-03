#ifndef MATRIX_H
#define MATRIX_H

#define CHECK_COMPAT
#define CHECK_ASSERT

#include "float.h"
#include "circular.h"

#ifdef CHECK_ASSERT
#define assertmx(x,y) {int result=(x); if(result<0) return result*100+y;}
#else
#define assertmx(x,y) x
#endif

//Row index is first, then column. A row is contiguous in memory. I think this
//makes it "Row major" but who cares. Both indices start at 0.
typedef struct matrix {
  int row,col;
  fp* data;
} matrix;

//Convert a 2D index into a 1D index
#define idx(a,row,kol) ((row)*(a)->col+(kol))

//Get an element
#define gx(a,row,kol)  ((a)->data[(row)*(a)->col+(kol)])

//Convert a 2D index into a 1D index
#define px(a,row,kol,val) ((a)->data[(row)*(a)->col+(kol)])=(val)

//Effectively b=a. Copy elements from matrix a to matrix b
static inline int assign(matrix* b, matrix* a) {
#ifdef CHECK_COMPAT
  if(a->row!=b->row) return -1;
  if(a->col!=b->col) return -1;
#endif
  for(int i=0;i<a->row*a->col;i++) b->data[i]=a->data[i];
  return 0; 
}

#define vlength(a) (sqrt((a)->data[0]*(a)->data[0]+(a)->data[1]*(a)->data[1]+(a)->data[2]*(a)->data[2]))
#define qlength(a) (sqrt((a)->data[0]*(a)->data[0]+(a)->data[1]*(a)->data[1]+(a)->data[2]*(a)->data[2]+(a)->data[3]*(a)->data[3]))

//Performs vc=va x vb. Can't be in place.
int vcross(matrix* vc, matrix* va, matrix* vb);

//Transforms a matrix which does a vector up like <vt>=[M]<vf> into a quaternion
//which does up a vector like <vt>=<q~><vf><q>
int mq(matrix* q, matrix* m);

//C=A*B
int mx(matrix* c, matrix* a, matrix* b);

//C=A*B'
int mxt(matrix* c, matrix* a, matrix* bt);

//c=A*B, A is 1xm and B is mx1
int mx1(fp* c, matrix* a, matrix* b);

//c=A*B', A is 1xm and B is 1xm
int mx1t(fp* c, matrix* a, matrix* bt);

//A=A*b
void mxs(matrix* a, fp b);

//A=A+B
int ma(matrix* a, matrix*b);

//A=A-B
int ms(matrix* a, matrix*b);

void fillPktRow(circular* buf, matrix* M, int row);
void fillPktVec(circular* buf, matrix* M);
void fillPktMx(circular* buf, char* name, matrix* M); 


#endif

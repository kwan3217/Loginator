#ifndef matrix_h
#define matrix_h

#include "float.h"

/** \file matrix.h

In this file, matrices are two-dimensional arrays.
    For display and convention purposes, these dimensions are thought of as
    "rows" and "columns". All the elements in a row are displayed on the 
    same vertical position and form a horizontal row of numbers (hence it is
    called a "row"), and likewise all elements in a column are in the same
    horizontal position and form a vertical column of numbers.

    Following the usual math convention (documented in the Library), the
    row index is first, followed by the column index. Since this is C++,
    rows and columns are numbered from zero. Since C/C++ thinks of 2D arrays
    as 1D arrays of 1D arrays, and since the first index is the one allowed
    to be arbitrary (empty), it follows that our matrices are arrays of
    rows, each of which is an array of cells, and it just happens that
    columns consist of the corresponding elements of several rows.

    So, a matrix which is printed as:

        [ 1 2 3 4 ]
        [ 5 6 7 8 ]

    is held in a fp[2][4]. In this example, the cell on row 1 (bottom row,
    remember 0-based indexing), column 2 has a value of 7 and is indexed as
    [1][2]. The array can be initialized and worked with as follows:

        fp a[2][4]={{1,2,3,4},
                    {5,6,7,8}};
        if(a[1][2]==7) ... //This test should evaluate to true

    Due to the way C++ works with multidimensional arrays, you can easily slice
    out rows -- If you have matrix a, a[1] is a 1D array representing the bottom
    row. It isn't as easy to slice out columns. This doesn't come up a lot.
    Of course it is easy to slice out cells, just use both indexes.

    An unfortunate concept in conventional matrix algebra is the separation of
    row vectors and column vectors. We would like to treat a vector as a 1D array,
    but in order to be treated like a matrix it must be promoted to 2D, and it can
    be done as either a 1-row by n column matrix (where n is the number of elements
    in the vector) or as a n-row by 1-column matrix. The first case is called a
    row matrix, while the second is a column matrix. As it turns out, due to the way
    that C/C++ stores 2D arrays, there is no space overhead in treating vectors
    as 2D. Likewise, the array index arithmetic would involve multiplying one of the
    array indices by constant 1, or adding constant 0, which will get optimized
    out. Therefore there is no time overhead either. As a consequence, all vectors
    will be stored as 2D arrays that just happen to be 1 element long in one or
    the other dimension.
*/

/** Perform conventional matrix multiplication. Given a matrix A
    with m rows and n columns, and a matrix B with n rows and p
    columns, calculate the matrix product which has m rows and
    p columns, and store it in AB. In order for this multiplication
    to be defined, the number of columns in A must match the number
    of rows in B. Each cell of the result is the dot product of the
    row vector from A for this cell's row and the column vector
    from B for this cell's column. This routine requires a preallocated
    result matrix.

    Algorithm
    For each row i in the first matrix
      For each column j in the second matrix
        Zero out AB[i][j]
        for each element k in the row and column vectors
          Accumulate into AB[i][j] the product of element k from the row vector i of A and the column vector j of B
        next k
      next j
    next i

    Note: The template mechanism will catch incompatible matrices
    at compile-time. No run-time check is needed or used.

*/
template<int m,int n,int p>
inline void mm(fp (&a)[m][n], fp (&b)[n][p], fp (&ab)[m][p]) {
  for(int i=0;i<m;i++) {
    for(int j=0;j<p;j++) {
      ab[i][j]=0;
      for(int k=0;k<n;k++) {
        ab[i][j]+=a[i][k]*b[k][j];
      }
    }
  }
}

/** Special case transpose matrix multiplication. Given a matrix A
    with m rows and n columns, and a matrix B with p rows and n
    columns, calculate the matrix product A##transpose(B)=AB^T
    which has m rows and p columns, and store it in ATB. Note that you
    pass matrix B, not B^T. The transpose is never stored separately,
    and is effectively created on-the-fly by switching array indexes.

    Algorithm
    For each row i in the first matrix
      For each ROW j in the second matrix (equal to column j in the transpose of that matrix)
        Zero out AB^T[i][j]
        for each element k in the row and ROW vectors
          accumulate into AB^T[i][j] the product of element k from the row vector i of A and the ROW vector j of B
        next k
      next j
    next i
*/
template<int m,int n,int p>
inline void mmt(fp (&a)[m][n], fp (&b)[p][n], fp (&abt)[m][p]) {
  for(int i=0;i<m;i++) {
    for(int j=0;j<p;j++) {
      abt[i][j]=0;
      for(int k=0;k<n;k++) {
        abt[i][j]+=a[i][k]*b[j][k];
      }
    }
  }
}

/** Special case transpose matrix multiplication. Given a matrix A
    with n rows and m columns, and a matrix B with n rows and p
    columns, calculate the matrix product transpose(A)##B=A^TB
    which has m rows and p columns, and store it in ATB. Note that you
    pass matrix A, not A^T. The transpose is never stored separately,
    and is effectively created on-the-fly by switching array indexes.

    Algorithm
    For each COLUMN i in the first matrix (equal to ROW i in the transpose of that matrix)
      For each column j in the second matrix
        Zero out AB^T[i][j]
        for each element k in the COLUMN and column vectors
          accumulate into AB^T[i][j] the product of element k from the COLUMN vector i of A and the column vector j of B
        next k
      next j
    next i
*/
template<int m,int n,int p>
inline void mtm(fp (&a)[n][m], fp (&b)[n][p], fp (&atb)[m][p]) {
  for(int i=0;i<m;i++) {
    for(int j=0;j<p;j++) {
      atb[i][j]=0;
      for(int k=0;k<n;k++) {
        atb[i][j]+=a[k][i]*b[k][j];
      }
    }
  }
}

/** Add matrix B to matrix A. Equivalent to a+=b */
template<int m,int n>
inline void mpm(fp (&a)[m][n], fp (&b)[m][n]) {
  for(int i=0;i<m;i++) {
    for(int j=0;j<n;j++) {
      a[i][j]+=b[i][j];
    }
  }
}

/** Subtract matrix B from matrix A. Equivalent to a-=b */
template<int m,int n>
inline void mmm(fp (&a)[m][n], fp (&b)[m][n]) {
  for(int i=0;i<m;i++) {
    for(int j=0;j<n;j++) {
      a[i][j]-=b[i][j];
    }
  }
}

/** Multiply matrix A by a scalar. Equivalent to a*=s */
template<int m,int n>
inline void ms(fp (&a)[m][n], const fp &s) {
  for(int i=0;i<m;i++) {
    for(int j=0;j<n;j++) {
      a[i][j]*=s;
    }
  }
}

#endif

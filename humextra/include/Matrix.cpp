//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Feb 24 03:30:03 PST 1999
// Last Modified: Fri May 28 15:40:39 PDT 1999
// Last Modified: Wed Jul 31 17:27:18 PDT 2002 (added getRow() and getColumn())
// Last Modified: Wed Jul 31 17:27:18 PDT 2002 (added setRow() and setColumn())
// Filename:      .../sig/maint/code/net/Matrix/Matrix.cpp
// Syntax:        C++ 
// $Smake:        g++ -O3 -g -c %f && rm %b.o
//

#ifndef _MATRIX_CPP_INCLUDED
#define _MATRIX_CPP_INCLUDED

#include "Matrix.h"

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif



//////////////////////////////
//
// Matrix<type>::Matrix --
//

template <class type>
Matrix<type>::Matrix (void) { 
   transposeQ = 0;
   dim1 = 0;
   dim2 = 0;
   storage = NULL;
}


template <class type>
Matrix<type>::Matrix(const Matrix<type>& aMatrix) { 
   transposeQ = 0;
   if (aMatrix.transposeQ) {
      dim1 = aMatrix.dim2;   
      dim2 = aMatrix.dim1;   
   } else {
      dim1 = aMatrix.dim1;   
      dim2 = aMatrix.dim2;   
   }

   int size = dim1 * dim2;
   storage = new type[size];

   for (int r=0; r<size; r++) {
      for (int c=0; c<dim2; c++) {
         cell(r, c) = aMatrix.cell(r, c);
      }  
   }
}


template <class type>
Matrix<type>::Matrix (int rowCount, int columnCount) { 
   if (rowCount < 1 || columnCount < 1) {
      cout << "Error: invalid maxtrix dimensions: " << rowCount 
           << ", " << columnCount << endl;
      exit(1);
   }
   transposeQ = 0;
   dim1 = rowCount;
   dim2 = columnCount;
   storage = new type[dim1 * dim2];
}


template <class type>
Matrix<type>::Matrix (int rowCount, int columnCount, type& thing) { 
   if (rowCount < 1 || columnCount < 1) {
      cout << "Error: invalid maxtrix dimensions: " << rowCount 
           << ", " << columnCount << endl;
      exit(1);
   }
   transposeQ = 0;
   dim1 = rowCount;
   dim2 = columnCount;
   int aSize = dim1 * dim2;
   storage = new type[aSize];
   for (int i=0; i<aSize; i++) {
      storage[i] = thing;
   }
}



template <class type>
Matrix<type>::Matrix (int columnCount) { 
   if (columnCount < 1) {
      cout << "Error: invalid maxtrix dimensions: " << columnCount << endl;
      exit(1);
   }
   transposeQ = 0;
   dim1 = 1;
   dim2 = columnCount;
   storage = new type[dim1 * dim2];
}


template <class type>
Matrix<type>::Matrix (type* data, int rowCount, int columnCount) { 
   if (rowCount < 1 || columnCount < 1) {
      cout << "Error: invalid maxtrix dimension: " << rowCount 
           << ", " << columnCount << endl;
      exit(1);
   }
   transposeQ = 0;
   dim1 = rowCount;
   dim2 = columnCount;
   int size = dim1 * dim2;
   storage = new type[size];
   for (int i=0; i<size; i++) {
      storage[i] = data[i];
   }
}



//////////////////////////////
//
// Matrix<type>::~Matrix --
//

template <class type>
Matrix<type>::~Matrix() {
   if (storage != NULL) {
      delete [] storage;
   }
   storage = NULL;
}



//////////////////////////////
//
// Matrix<type>::add -- add element by element
//

template<class type>
Matrix<type>& Matrix<type>::add(type scalar) {
   for (int r=0; r<getRowCount(); r++) {
      for (int c=0; c<getRowCount(); c++) {
         cell(r, c) += scalar;
      }
   }
  
   return *this;
}


template<class type>
Matrix<type>& Matrix<type>::add(const Matrix<type>& aMatrix) {
   for (int r=0; r<getRowCount(); r++) {
      for (int c=0; c<getRowCount(); c++) {
         cell(r, c) += aMatrix.cell(r, c);
      }
   }
  
   return *this;
}



//////////////////////////////
//
// Matrix<type>::cell -- returns the specified matrix cell (row, column).
//

template <class type>
type& Matrix<type>::cell(int row, int column) {
   checkdim(row, column);

   if (transposeQ) {
      return storage[column * dim2 + row];
   } else {
      return storage[row * dim2 + column];
   }

}


template <class type>
type Matrix<type>::cell(int row, int column) const {
   checkdim(row, column);
   if (transposeQ) {
      return storage[column * dim2 + row];
   } else {
      return storage[row * dim2 + column];
   }
}



//////////////////////////////
//
// Matrix<type>::getRow --
//

template <class type>
void Matrix<type>::getRow(Array<type>& anArray, int index) {
   int length = getColumnCount();
   anArray.setSize(length);
   
   int i;
   for (i=0; i<length; i++) {
      anArray[i] = cell(index, i);
   }
}



//////////////////////////////
//
// Matrix<type>::getRow --
//

template <class type>
void Matrix<type>::getColumn(Array<type>& anArray, int index) {
   int length = getRowCount();
   anArray.setSize(length);
   
   int i;
   for (i=0; i<length; i++) {
      anArray[i] = cell(i, index);
   }
}



//////////////////////////////
//
// Matrix<type>::setRow --
//

template <class type>
void Matrix<type>::setRow(int index, Array<type>& anArray) {
   int length = getColumnCount();
   if (anArray.getSize() < length) {
      length = anArray.getSize();
   }

   int i;
   for (i=0; i<length; i++) {
      cell(index, i) = anArray[i];
   }
}



//////////////////////////////
//
// Matrix<type>::setColumn --
//

template <class type>
void Matrix<type>::setColumn(int index, Array<type>& anArray) {
   int length = getRowCount();
   if (anArray.getSize() < length) {
      length = anArray.getSize();
   }

   int i;
   for (i=0; i<length; i++) {
      cell(i, index) = anArray[i];
   }
}



//////////////////////////////
//
// Matrix<type>::getColumnCount --
//

template <class type>
int Matrix<type>::getColumnCount (void) const { 
   if (transposeQ) {
      return dim1;
   } else {
      return dim2;
   }
}



//////////////////////////////
//
// Matrix<type>::getRowCount --
//

template <class type>
int Matrix<type>::getRowCount(void) const { 
   if (transposeQ) {
      return dim2;
   } else {
      return dim1;
   }
}



//////////////////////////////
//
// Matrix<type>::getSize --
//

template<class type>
int Matrix<type>::getSize(void) const {
   return dim1 * dim2;
}



//////////////////////////////
//
// Matrix<type>::multiply -- multiply element by element
//

template<class type>
Matrix<type>& Matrix<type>::multiply(type scalar) {
   for (int r=0; r<getRowCount(); r++) {
      for (int c=0; c<getColumnCount(); c++) {
         cell(r, c) *= scalar;
      }
   }
  
   return *this;
}



//////////////////////////////
//
// Matrix<type>::operator[] --
//

template<class type>
type& Matrix<type>::operator[](int index) { 
   if (dim1 == 1) {
      if (index < dim2) {
         return storage[index];
      } else {
         cerr << "Error: invalid matrix access, returning 0" << endl;
         exit(1);
      }
   } else if (dim2 == 1) {
      if (index < dim1) {
         return storage[index];
      } else {
         cerr << "Error: invalid matrix access, returning 0" << endl;
         exit(1);
      }
   } else {
      if (index < dim1 * dim2) {
         return storage[index];
      } else {
         cerr << "Error: invalid matrix access, returning 0" << endl;
         exit(1);
      }
   }
}



//////////////////////////////
//
// Matrix<type>::operator= --
//

template<class type>
Matrix<type>& Matrix<type>::operator=(const Matrix& aMatrix) { 
   if (this == &aMatrix) {
      return *this;
   }

   if (dim1 * dim2 == aMatrix.dim1 * aMatrix.dim2) {
      // don't change the storage size
   } else {
      delete [] storage;
      storage = new type[aMatrix.dim1 * aMatrix.dim2];
   }

   dim1 = aMatrix.getRowCount();
   dim2 = aMatrix.getColumnCount();
   transposeQ = 0;

   int size = dim1 * dim2;

   for (int r=0; r<dim1; r++) {
      for (int c=0; c<dim2; c++) {
         cell(r, c) = aMatrix.cell(r, c);
      }
   }

   return *this;
}



//////////////////////////////
//
// Matrix<type>::operator+= --
//

template<class type>
Matrix<type>& Matrix<type>::operator+=(const Matrix& aMatrix) { 
   if (dim1 * dim2 != aMatrix.dim1 * aMatrix.dim2) {
      cout << "Error in operator += for Matrices" << endl;
   }

   int rows = getRowCount();
   int cols = getColumnCount();

   for (int r=0; r<rows; r++) {
      for (int c=0; c<cols; c++) {
         cell(r, c) = aMatrix.cell(r, c);
      }
   }

   return *this;
}



//////////////////////////////
//
// Matrix<type>::setSize -- very destructive to previous contents
//

template<class type>
void Matrix<type>::setSize(int row, int column) {
   if (storage != NULL) {
      delete [] storage;
   }
   storage = new type[row * column];

   dim1 = row;
   dim2 = column;
   transposeQ = 0;
}



//////////////////////////////
//
// Matrix<type>::transpose --
//

template<class type>
void Matrix<type>::transpose(void) { 
   transposeQ = !transposeQ;
}



//////////////////////////////
//
// Matrix<type>::zero --
//

template<class type>
void Matrix<type>::zero(void){ 
   int size = dim1 * dim2;
   for (int i=0; i<size; i++) {
      storage[i] = (type)0;
   }
}


///////////////////////////////////////////////////////////////////////////
//
// static functions
//

//////////////////////////////
//
// Matrix<type>::multiply --
//

template<class type>
Matrix<type>& Matrix<type>::multiply(Matrix& output, const Matrix& one, 
      Matrix<type>& two) {
   if (one.getColumnCount() != two.getRowCount()) {
      cerr << "Error: sizes of input matrices not condusive"
           << " for multiplication." << endl;
      exit(1);
   }
   if (output.getRowCount() != one.getRowCount()) {
      cerr << "Error: Row length of output matrix is not equal to input ." 
           << endl;
      exit(1);
   }
   if (output.getColumnCount() != two.getColumnCount()) {
      cerr << "Error: Column length of output matrix is not equal to input." 
           << endl;
      exit(1);
   }

   output.zero();
   two.transpose();
   for (int r=0; r<output.getRowCount(); r++) {
      for (int c=0; c<output.getColumnCount(); c++) {
         for (int m=0; m<one.getColumnCount(); m++) {
            output.cell(r, c) += one.cell(r, m) * two.cell(c, m);
         }
      }
   }
   two.transpose();

   return output;
}

template<class type>
Matrix<type>& Matrix<type>::multiply(Matrix& output, Matrix& one, 
      type aScalar) {
   for (int i=0; i<one.getSize(); i++) {
      output[i] = one[i] * aScalar;
   }
   
   return output;
}



//////////////////////////////
//
// Matrix<type>::add -- add two matrices together
//

template<class type>
Matrix<type>& Matrix<type>::add(Matrix& output, const Matrix& one, 
      const Matrix& two) {

   if (one.getRowCount() != two.getRowCount()) {
      cerr << "Error: Row length of matrices are not equal for adding." << endl;
      exit(1);
   }
   if (one.getColumnCount() != two.getColumnCount()) {
      cerr << "Error: Column length of matrices are not equal for adding." 
           << endl;
      exit(1);
   }
   if (one.getRowCount() != output.getRowCount()) {
      cerr << "Error: Row length of output matrix is not equal to input ." 
           << endl;
      exit(1);
   }
   if (output.getColumnCount() != two.getColumnCount()) {
      cerr << "Error: Column length of output matrix is not equal to input." 
           << endl;
      exit(1);
   }

   for (int r=0; r<output.getRowCount(); r++) {
      for (int c=0; c<output.getColumnCount(); c++) {
         output.cell(r, c) = one.cell(r, c) + two.cell(r, c);
      }
   }

   return output;
}



//////////////////////////////
//
// Matrix<type>::transpose --
//

template<class type>
void Matrix<type>::transpose(Matrix& output, const Matrix& one) {
   cerr << "Matrix<type>::tranpose note implemented" << endl;
}



//////////////////////////////
//
// Matrix::setAll --
//

template<class type>
void Matrix<type>::setAll(type& thing) {
   int rows = getRowCount();
   int cols = getColumnCount();
   int i, j;
   for (i=0; i<rows; i++) {
      for (j=0; j<cols; j++) {
         cell(i, j) = thing;
      }
   }
}



///////////////////////////////////////////////////////////////////////////
//
// private functions
//

//////////////////////////////
//
// Matrix<type>::checkdim -- checks that the dimensions are valid or not
//

template<class type>
void Matrix<type>::checkdim(int row, int column) const {
   if (transposeQ) {
      if (row >= 0 && row < dim2 && column >= 0 && column < dim1) {
         return;
      }
   } else {
      if (row >= 0 && row < dim1 && column >= 0 && column < dim2) {
         return;
      }
   }

   cerr << "Error: dimensions of matrix are: " << 
      getRowCount() << ", " << getColumnCount() << " but you accessed: "
      << row << ", " << column << endl;
   exit(1);
}


///////////////////////////////////////////////////////////////////////////
//
// external functions
//

template<class type>
ostream& operator<<(ostream& out, const Matrix<type>& aMatrix) {
   for (int r=0; r<aMatrix.getRowCount(); r++) {
      for (int c=0; c<aMatrix.getColumnCount(); c++) {
         out << aMatrix.cell(r, c) << '\t';
      }
      if (aMatrix.getRowCount() > 1) {
         out << '\n';
      }
   }

   return out;
}



#endif  /* _MATRIX_CPP_INCLUDED */

// md5sum: f8233c33179e27f9964058a999432b32 Matrix.cpp [20050403]

//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep  8 17:36:45 PDT 2010 Added operator<< for char
// Last Modified: Mon Dec 10 01:28:14 PST 2012 Added operator=(Array, char*)
// Filename:      ...sig/maint/code/base/Array/Array-typed.cpp
// Web Address:   http://sig.sapp.org/src/sigBase/Array-typed.cpp
// Syntax:        C++ 
//
// Description:   These are functions related to the Array template
//                class, but which are associated with a fixed type.
//

#include "Array.h"

#ifndef OLDCPP
   #include <ostream>
   using namespace std;
#else
   #include <ostream.h>
#endif



/////////////////////////////
//
// operator<< -- special function to print Array<char> as a string
//      of characters.
//

ostream& operator<<(ostream& out, Array<char>& astring) {
   int i;
   int maxx = astring.getSize() - 1;
   const char* ptr = astring.getBase();
   for (i=0; i<maxx; i++) {
      out << ptr[0];
      ptr++;
   }
   char lastval = astring[maxx];
   if (lastval != '\0') {
      out << lastval;
   }
   return out;
}



/////////////////////////////
//
// operator<< -- special function to print Array<int> as a list
//      of integers separated by spaces.  You have to give the newline
//      if you want a line break after the list is printed.
//

ostream& operator<<(ostream& out, Array<int>& alist) {
   int i;
   for (i=0; i<alist.getSize(); i++) {
      out << alist[i];
      if (i < alist.getSize()-1) {
         out << ' ';
      }
   }
   return out;
}



/////////////////////////////
//
// operator<< -- special function to print Array<double> as a list
//      of integers separated by spaces.  You have to give the newline
//      if you want a line break after the list is printed.
//

ostream& operator<<(ostream& out, Array<double>& alist) {
   int i;
   for (i=0; i<alist.getSize(); i++) {
      out << alist[i];
      if (i < alist.getSize()-1) {
         out << ' ';
      }
   }
   return out;
}



/////////////////////////////
//
// operator<< -- special function to print Array<float> as a list
//      of integers separated by spaces.  You have to give the newline
//      if you want a line break after the list is printed.
//

ostream& operator<<(ostream& out, Array<float>& alist) {
   int i;
   for (i=0; i<alist.getSize(); i++) {
      out << alist[i];
      if (i < alist.getSize()-1) {
         out << ' ';
      }
   }
   return out;
}



/////////////////////////////
//
// operator= == Definition for Array<char> = const char*
//

template<>
Array<char>& Array<char>::operator=(const char* string) {
   Array<char>& object = *this;
   if (string == NULL) {
      object.setSize(1);
      object[0] = '\0';
      return object;
   }
   object.setSize(strlen(string)+1);
   strcpy(object.getBase(), string);
   return object;
}


// md5sum: 22ff238f1ccb1e4cb517a1e9259227bb Array_typed.cpp [20100511]

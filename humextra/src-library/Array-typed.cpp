//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep  8 17:36:45 PDT 2010 (added operator<< for char)
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

// md5sum: 22ff238f1ccb1e4cb517a1e9259227bb Array_typed.cpp [20100511]

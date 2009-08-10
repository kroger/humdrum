//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jul  9 14:25:29 PDT 1998
// Last Modified: Thu Jul  9 14:25:29 PDT 1998
// Last Modified: Wed Jun 24 15:25:10 PDT 2009 (updated for GCC 4.4)
// Filename:      ...sig/src/sigInfo/Identify.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/Identify.cpp
// Syntax:        C++ 
//
// Description:   Helping class to identify various string formats.
//

#include "Identify.h"

#include <string.h>
#include <ctype.h>

#ifndef OLDCPP
   using namespace std;
#endif



//////////////////////////////
//
// Identify::humdrumNullField --
//

int Identify::humdrumNullField(const char* aField) {
   if (strcmp(aField, ".") == 0) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// Identify::kernTimeSignature --
//

int Identify::kernTimeSignature(const char* kernString) {
   // meter tandem interpretation must start with a '*'
   if (kernString[0] != '*') {
      return 0;
   }

   // next character after '*' must be a number:
   if (!isdigit(kernString[1])) {
      return 0;
   }

   // there must be a '/' in the string:
   const char* slash = strchr(kernString, '/');
   if (slash == NULL) {
      return 0;
   }

   // The next character after a slash must be a number
   if (!isdigit(slash[1])) {
      return 0;
   }
  
   // got this far, so assume that input is a time signature   

   return 1;
}



// md5sum: 10ca2402db078436f0cc70329e4815db Identify.cpp [20050403]

//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jun 10 22:56:56 PDT 1998
// Last Modified: Fri Oct 13 15:04:45 PDT 2000 (changed name to EnumerationEI)
// Last Modified: Sat Oct 14 19:12:37 PDT 2000 (extracted .cpp file)
// Last Modified: Sun Mar 24 12:10:00 PST 2002 (small changes for visual c++)
// Filename:      ...sig/src/sigInfo/EnumerationEI.cpp
// Web Address:   http://sig.sapp.org/src/sigInfo/EnumerationEI.cpp
// Syntax:        C++ 
//
// Description:   Enumeration class for Humdrum Exclusive Interpretations.
//

#include "EnumerationEI.h"

#ifndef OLDCPP
   using namespace std;
#endif

// static declarations
int EnumerationEI::nextenumeration = 1000;


///////////////////////////////
//
// EnumerationEI::EnumerationEI --
//

EnumerationEI::EnumerationEI(void) : Enumeration() {
   add(E_UNKNOWN_EXINT, E_UNKNOWN_EXINT_NAME,  ENUM_FIXED_ALLOC);

   // standard types of exclusive interpretations
   add(E_KERN_EXINT   , E_KERN_EXINT_NAME   ,  ENUM_FIXED_ALLOC);

   // museinfo pre defined exclusive interpretations
   add(E_QUAL_EXINT   , E_QUAL_EXINT_NAME    , ENUM_FIXED_ALLOC);
}



///////////////////////////////
//
// EnumerationEI::add --
//

int EnumerationEI::add(const char* aString) { 
   add(nextenumeration++, aString);
   return nextenumeration-1; 
}

void EnumerationEI::add(int aValue, const char* aString, int allocType) { 
   Enumeration::add(aValue, aString, allocType); 
}



// md5sum: ff8a4d9c9c5eef33726b3ecd3e1b4f56 EnumerationEI.cpp [20050403]

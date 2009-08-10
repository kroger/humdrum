//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jun 10 22:56:56 PDT 1998
// Last Modified: Wed Jun 10 22:57:02 PDT 1998
// Last Modified: Fri Oct 13 15:04:45 PDT 2000 (changed name to EnumerationEI)
// Last Modified: Sat Oct 14 19:16:34 PDT 2000 (extracted EnumerationEI.cpp)
// Filename:      ...sig/include/sigInfo/EnumerationEI.h
// Web Address:   http://sig.sapp.org/include/sigInfo/EnumerationEI.h
// Syntax:        C++ 
//
// Description:   Enumeration database for Humdrum exclusive interpretations.
//

#ifndef _ENUMERATIONEI_H_INCLUDED
#define _ENUMERATIONEI_H_INCLUDED


#include "Enumeration.h"
#include "Enum_exInterp.h"


class EnumerationEI : public Enumeration {
   public:
            EnumerationEI        (void);

     int    add                  (const char* aString);
     void   add                  (int aValue, const char* aString, 
                                    int allocType = ENUM_TRANSIENT_ALLOC);

   private:
      static int nextenumeration;

};


#endif  /* _ENUMERATIONEI_H_INCLUDED */


// md5sum: edc4d6bdcbaaf7add1bc052f030bb6d9 EnumerationEI.h [20001204]

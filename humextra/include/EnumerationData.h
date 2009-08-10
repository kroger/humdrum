//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jun  8 20:19:26 PDT 1998
// Last Modified: Mon Jun  8 21:35:18 PDT 1998
// Filename:      ...sig/include/sigInfo/EnumerationData.h
// Web Address:   http://sig.sapp.org/include/sigInfo/EnumerationData.h
// Syntax:        C++ 
//
// Description:
//

#ifndef _ENUMERATIONDATA_H_INCLUDED
#define _ENUMERATIONDATA_H_INCLUDED

#include "Enum_basic.h"

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif


#define ENUM_FIXED_ALLOC        0
#define ENUM_TRANSIENT_ALLOC    1


class EnumerationData {
   public:
                  EnumerationData         (void);
                  EnumerationData         (int aValue, const char* aName, int
                                             allocType = ENUM_TRANSIENT_ALLOC);
                  EnumerationData         (const EnumerationData& aDatum);
                 ~EnumerationData         ();
 
      int         allocatedQ              (void) const;
      int         compare                 (int aValue) const;
      int         compare                 (const char* aName) const;
      int         compare                 (const EnumerationData& aDatum) const;
      int         compareByValue          (const EnumerationData& aDatum) const;
      int         compareByName           (const EnumerationData& aDatum) const;
      const char* getName                 (void) const;
      int         getValue                (void) const;
      EnumerationData& operator=          (const EnumerationData& aDatum);
      int         operator==              (const EnumerationData& aDatum);
      int         operator<               (const EnumerationData& aDatum);
      int         operator>               (const EnumerationData& aDatum);
      void        setRelation             (int aValue, const char* aName, int
                                             allocType = ENUM_TRANSIENT_ALLOC);
      void        setName                 (const char* aName, int allocType = 
                                             ENUM_TRANSIENT_ALLOC);
      void        setValue                (int aValue);

   protected:
      int value;
      char* name;
      int allocationType;

};

ostream& operator<<(ostream& out, const EnumerationData& aDatum);
ostream& operator<<(ostream& out, const EnumerationData* aDatum);


#endif  /* _ENUMERATIONDATA_H_INCLUDED */



// md5sum: 1536e4203ad26b3fb8f77db340e73b2a EnumerationData.h [20050403]

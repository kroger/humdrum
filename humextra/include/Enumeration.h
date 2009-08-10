//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jun  8 20:19:26 PDT 1998
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Filename:      ...sig/include/sigInfo/Enumeration.h
// Web Address:   http://sig.sapp.org/include/sigInfo/Enumeration.h
// Syntax:        C++ 
//
// Description:   Messy yet functional way of handling enumerations
//                and their string equivalents.
//

#ifndef _ENUMERATION_H_INCLUDED
#define _ENUMERATION_H_INCLUDED


#include "EnumerationData.h"
#include "SigCollection.h"

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif


class Enumeration {
   public:
                  Enumeration            (void);
                  Enumeration            (const Enumeration& anEnumeration);
                 ~Enumeration            ();

      void        add                    (int aValue, const char* aString, int 
                                            allocType = ENUM_TRANSIENT_ALLOC);
      void        add                    (const EnumerationData& aDatum);
      int         associate              (const char* aName, int
                                            allocType = ENUM_TRANSIENT_ALLOC);
      const char* getAssociation         (int aValue);
      int         getAssociation         (const char* aName);
      const char* getName                (int aValue);
      int         getValue               (const char* aName);
      int         memberQ                (int aValue);
      int         memberQ                (const char* aName);
      int         memberQ                (const EnumerationData& aDatum);
      int         getFreeValue           (void);
      void        print                  (ostream& out);
      void        printByValue           (ostream& out);
      void        printByName            (ostream& out);
      void        setNullName            (const char* aName, int 
                                            allocType = ENUM_TRANSIENT_ALLOC);

      
   protected:
      SigCollection<EnumerationData>  associations;
      SigCollection<EnumerationData*> sortByValue;
      SigCollection<EnumerationData*> sortByName;
      int sortQ;

   // protected functions:
      void        checksort              (void);
      void        sort                   (void);

      static int data_compare(const void* a, const void* b);
      static int data_compare_by_value(const void* a, const void* b);
      static int data_compare_by_name(const void* a, const void* b);
      static int data_compare_by_value_only(const void* a,const void* b);
      static int data_compare_by_name_only(const void* a, const void* b);

};


ostream& operator<<(ostream& out, const EnumerationData& aDatum);
ostream& operator<<(ostream& out, const EnumerationData* aDatum);


#endif  /* _ENUMERATIONDATA_H_INCLUDED */



// md5sum: f086b120b617714be0601517805247ae Enumeration.h [20001204]

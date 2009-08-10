//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 11:51:01 PDT 1998
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Filename:      ...sig/include/sigInfo/MuseRecordBasic.cpp
// Web Address:   http://sig.sapp.org/include/sigInfo/MuseRecordBasic.h
// Syntax:        C++ 
//
// Description:   basic data manipulations for lines in a Musedata file.
//


#ifndef _MUSERECORDBASIC_H_INCLUDED
#define _MUSERECORDBASIC_H_INCLUDED

#include "Enum_muserec.h"

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

class MuseRecordBasic {
   public:
                        MuseRecordBasic    (void);
                        MuseRecordBasic    (const char* aLine);
                        MuseRecordBasic    (const MuseRecordBasic& aRecord);
                       ~MuseRecordBasic    ();
  
      void              extract            (char* output, int start, int stop);
      char&             getColumn          (int index);
      int               getLength          (void) const;
      const char*       getLine            (void) const; 
      int               getType            (void) const;
      MuseRecordBasic&  operator=          (const MuseRecordBasic& aRecord);
      MuseRecordBasic&  operator=          (const MuseRecordBasic* aRecord);
      MuseRecordBasic&  operator=          (const char* aRecord);
      char&             operator[]         (int index);
      void              setLine            (const char* aString); 
      void              setType            (int aType);
      void              shrink             (void);

   protected:
      int               type;              // category of MuseRecordBasic record
      char*             recordString;      // record string
      int               length;            // length of string

};
   

ostream& operator<<(ostream& out, MuseRecordBasic& aRecord);


#endif  /* _MUSERECORDBASIC_H_INCLUDED */



// md5sum: b2da1554b44dd3a66577a7203e62ab6a MuseRecordBasic.h [20050403]

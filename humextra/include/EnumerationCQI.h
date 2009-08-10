//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jun 12 22:55:05 PDT 1998
// Last Modified: Fri Jun 12 22:55:09 PDT 1998
// Filename:      ...sig/include/EnumerationCQI.h
// Web Address:   http://sig.sapp.org/include/sigInfo/EnumerationCQI.h
// Syntax:        C++ 
//
// Description:   Enumeration database for chord inversions.
//

#ifndef _ENUMERATIONCQI_H_INCLUDED
#define _ENUMERATIONCQI_H_INCLUDED


#include "Enumeration.h"
#include "Enum_chordQuality.h"


class EnumerationCQI : public Enumeration {
   public:
      EnumerationCQI(void) : Enumeration() {
         setNullName("X", ENUM_FIXED_ALLOC);
         add(E_inv_root   ,   E_inv_name_root   ,   ENUM_FIXED_ALLOC);
         add(E_inv_first  ,   E_inv_name_first  ,   ENUM_FIXED_ALLOC);
         add(E_inv_second ,   E_inv_name_second ,   ENUM_FIXED_ALLOC);
         add(E_inv_third  ,   E_inv_name_third  ,   ENUM_FIXED_ALLOC);
         add(E_inv_fourth ,   E_inv_name_fourth ,   ENUM_FIXED_ALLOC);
         add(E_inv_fifth  ,   E_inv_name_fifth  ,   ENUM_FIXED_ALLOC);
         add(E_inv_sixth  ,   E_inv_name_sixth  ,   ENUM_FIXED_ALLOC);
      }

};


#endif  /* _ENUMERATIONCQI_H_INCLUDED */



// md5sum: 509183044f1587dd807ad3b428a53600 EnumerationCQI.h [20001204]

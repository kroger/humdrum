//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun  9 22:18:28 PDT 1998
// Last Modified: Fri Jun 12 22:53:00 PDT 1998
// Filename:      ...sig/include/sigInfo/EnumerationCQT.h
// Web Address:   http://sig.sapp.org/include/sigInfo/EnumerationCQT.h
// Syntax:        C++ 
//
// Description:   Enumeration database for chord qualities.
// 

#ifndef _ENUMERATIONCQT_H_INCLUDED
#define _ENUMERATIONCQT_H_INCLUDED


#include "Enumeration.h"
#include "Enum_chordQuality.h"


class EnumerationCQT : public Enumeration {
   public:
      EnumerationCQT(void) : Enumeration() {
         setNullName("X", ENUM_FIXED_ALLOC);

      // set one: basic chord qualities
         add(E_chord_note     ,   E_chord_name_note     ,   ENUM_FIXED_ALLOC);
         add(E_chord_incmin   ,   E_chord_name_incmin   ,   ENUM_FIXED_ALLOC);
         add(E_chord_incmaj   ,   E_chord_name_incmaj   ,   ENUM_FIXED_ALLOC);
         add(E_chord_dim      ,   E_chord_name_dim      ,   ENUM_FIXED_ALLOC);
         add(E_chord_min      ,   E_chord_name_min      ,   ENUM_FIXED_ALLOC);
         add(E_chord_maj      ,   E_chord_name_maj      ,   ENUM_FIXED_ALLOC);
         add(E_chord_aug      ,   E_chord_name_aug      ,   ENUM_FIXED_ALLOC);
         add(E_chord_minminx5 ,   E_chord_name_minminx5 ,   ENUM_FIXED_ALLOC);
         add(E_chord_domsevx5 ,   E_chord_name_domsevx5 ,   ENUM_FIXED_ALLOC);
         add(E_chord_fullydim ,   E_chord_name_fullydim ,   ENUM_FIXED_ALLOC);
         add(E_chord_halfdim  ,   E_chord_name_halfdim  ,   ENUM_FIXED_ALLOC);
         add(E_chord_minmin   ,   E_chord_name_minmin   ,   ENUM_FIXED_ALLOC);
         add(E_chord_minmaj   ,   E_chord_name_minmaj   ,   ENUM_FIXED_ALLOC);
         add(E_chord_domsev   ,   E_chord_name_domsev   ,   ENUM_FIXED_ALLOC);
         add(E_chord_majmaj   ,   E_chord_name_majmaj   ,   ENUM_FIXED_ALLOC);
         add(E_chord_frensix  ,   E_chord_name_frensix  ,   ENUM_FIXED_ALLOC);
         add(E_chord_germsix  ,   E_chord_name_germsix  ,   ENUM_FIXED_ALLOC);
         add(E_chord_italsix  ,   E_chord_name_italsix  ,   ENUM_FIXED_ALLOC);

      // set two: functional harmony qualities
         add(E_chord_neopol   ,   E_chord_name_neopol   ,   ENUM_FIXED_ALLOC);
         add(E_chord_secdom   ,   E_chord_name_secdom   ,   ENUM_FIXED_ALLOC);
         add(E_chord_secdomsev,   E_chord_name_secdomsev,   ENUM_FIXED_ALLOC);
         add(E_chord_secsev   ,   E_chord_name_secsev   ,   ENUM_FIXED_ALLOC);
         add(E_chord_secsevo  ,   E_chord_name_secsevo  ,   ENUM_FIXED_ALLOC);
         add(E_chord_secsevc  ,   E_chord_name_secsevc  ,   ENUM_FIXED_ALLOC);
        
      }

};


#endif  /* _ENUMERATIONCQT_H_INCLUDED */



// md5sum: 805fb4212cebafcb5d855075ba620f84 EnumerationCQT.h [20001204]

//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jul  8 14:09:13 PDT 1998
// Last Modified: Wed Jul  8 14:09:17 PDT 1998
// Filename:      ...sig/include/sigInfo/EnumerationInterval.h
// Web Address:   http://sig.sapp.org/include/sigInfo/EnumerationInterval.h
// Syntax:        C++ 
//
// Description:   Enumeration database for base-40 intervals.
//

#ifndef _ENUMERATIONINTERVAL_H_INCLUDED
#define _ENUMERATIONINTERVAL_H_INCLUDED


#include "Enumeration.h"
#include "Enum_chordQuality.h"
#include "Enum_base40.h"


class EnumerationInterval : public Enumeration {
   public:
      EnumerationInterval(void) : Enumeration() {
         setNullName("Error", ENUM_FIXED_ALLOC);
         add(E_base40_rest,   E_root_name_rest    ,   ENUM_FIXED_ALLOC);

         add(E_base40_dim1  ,   E_interval_name_dim1     ,   ENUM_FIXED_ALLOC);
         add(E_base40_per1  ,   E_interval_name_per1     ,   ENUM_FIXED_ALLOC);
         add(E_base40_aug1  ,   E_interval_name_aug1     ,   ENUM_FIXED_ALLOC);

         add(E_base40_dim2  ,   E_interval_name_dim2     ,   ENUM_FIXED_ALLOC);
         add(E_base40_min2  ,   E_interval_name_min2     ,   ENUM_FIXED_ALLOC);
         add(E_base40_maj2  ,   E_interval_name_maj2     ,   ENUM_FIXED_ALLOC);
         add(E_base40_aug2  ,   E_interval_name_aug2     ,   ENUM_FIXED_ALLOC);

         add(E_base40_dim3  ,   E_interval_name_dim3     ,   ENUM_FIXED_ALLOC);
         add(E_base40_min3  ,   E_interval_name_min3     ,   ENUM_FIXED_ALLOC);
         add(E_base40_maj3  ,   E_interval_name_maj3     ,   ENUM_FIXED_ALLOC);
         add(E_base40_aug3  ,   E_interval_name_aug3     ,   ENUM_FIXED_ALLOC);

         add(E_base40_dim4  ,   E_interval_name_dim4     ,   ENUM_FIXED_ALLOC);
         add(E_base40_per4  ,   E_interval_name_per4     ,   ENUM_FIXED_ALLOC);
         add(E_base40_aug4  ,   E_interval_name_aug4     ,   ENUM_FIXED_ALLOC);

         add(E_base40_dim5  ,   E_interval_name_dim5     ,   ENUM_FIXED_ALLOC);
         add(E_base40_per5  ,   E_interval_name_per5     ,   ENUM_FIXED_ALLOC);
         add(E_base40_aug5  ,   E_interval_name_aug5     ,   ENUM_FIXED_ALLOC);

         add(E_base40_dim6  ,   E_interval_name_dim6     ,   ENUM_FIXED_ALLOC);
         add(E_base40_min6  ,   E_interval_name_min6     ,   ENUM_FIXED_ALLOC);
         add(E_base40_maj6  ,   E_interval_name_maj6     ,   ENUM_FIXED_ALLOC);
         add(E_base40_aug6  ,   E_interval_name_aug6     ,   ENUM_FIXED_ALLOC);

         add(E_base40_dim7  ,   E_interval_name_dim7     ,   ENUM_FIXED_ALLOC);
         add(E_base40_min7  ,   E_interval_name_min7     ,   ENUM_FIXED_ALLOC);
         add(E_base40_maj7  ,   E_interval_name_maj7     ,   ENUM_FIXED_ALLOC);
         add(E_base40_aug7  ,   E_interval_name_aug7     ,   ENUM_FIXED_ALLOC);

         add(E_base40_dim8  ,   E_interval_name_dim8     ,   ENUM_FIXED_ALLOC);
         add(E_base40_per8  ,   E_interval_name_per8     ,   ENUM_FIXED_ALLOC);
         add(E_base40_aug8  ,   E_interval_name_aug8     ,   ENUM_FIXED_ALLOC);


         add(E_base40_dim2+40,   E_interval_name_dim2_8va , ENUM_FIXED_ALLOC);
         add(E_base40_min2+40,   E_interval_name_min2_8va , ENUM_FIXED_ALLOC);
         add(E_base40_maj2+40,   E_interval_name_maj2_8va , ENUM_FIXED_ALLOC);
         add(E_base40_aug2+40,   E_interval_name_aug2_8va , ENUM_FIXED_ALLOC);

         add(E_base40_dim3+40,   E_interval_name_dim3_8va , ENUM_FIXED_ALLOC);
         add(E_base40_min3+40,   E_interval_name_min3_8va , ENUM_FIXED_ALLOC);
         add(E_base40_maj3+40,   E_interval_name_maj3_8va , ENUM_FIXED_ALLOC);
         add(E_base40_aug3+40,   E_interval_name_aug3_8va , ENUM_FIXED_ALLOC);

         add(E_base40_dim4+40,   E_interval_name_dim4_8va , ENUM_FIXED_ALLOC);
         add(E_base40_per4+40,   E_interval_name_per4_8va , ENUM_FIXED_ALLOC);
         add(E_base40_aug4+40,   E_interval_name_aug4_8va , ENUM_FIXED_ALLOC);

         add(E_base40_dim5+40,   E_interval_name_dim5_8va , ENUM_FIXED_ALLOC);
         add(E_base40_per5+40,   E_interval_name_per5_8va , ENUM_FIXED_ALLOC);
         add(E_base40_aug5+40,   E_interval_name_aug5_8va , ENUM_FIXED_ALLOC);

         add(E_base40_dim6+40,   E_interval_name_dim6_8va , ENUM_FIXED_ALLOC);
         add(E_base40_min6+40,   E_interval_name_min6_8va , ENUM_FIXED_ALLOC);
         add(E_base40_maj6+40,   E_interval_name_maj6_8va , ENUM_FIXED_ALLOC);
         add(E_base40_aug6+40,   E_interval_name_aug6_8va , ENUM_FIXED_ALLOC);

         add(E_base40_dim7+40,   E_interval_name_dim7_8va , ENUM_FIXED_ALLOC);
         add(E_base40_min7+40,   E_interval_name_min7_8va , ENUM_FIXED_ALLOC);
         add(E_base40_maj7+40,   E_interval_name_maj7_8va , ENUM_FIXED_ALLOC);
         add(E_base40_aug7+40,   E_interval_name_aug7_8va , ENUM_FIXED_ALLOC);

         add(E_base40_dim8+40,   E_interval_name_dim8_8va , ENUM_FIXED_ALLOC);
         add(E_base40_per8+40,   E_interval_name_per8_8va , ENUM_FIXED_ALLOC);
         add(E_base40_aug8+40,   E_interval_name_aug8_8va , ENUM_FIXED_ALLOC);


         add(E_base40_dim2+80,   E_interval_name_dim2_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_min2+80,   E_interval_name_min2_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_maj2+80,   E_interval_name_maj2_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_aug2+80,   E_interval_name_aug2_15ma, ENUM_FIXED_ALLOC);

         add(E_base40_dim3+80,   E_interval_name_dim3_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_min3+80,   E_interval_name_min3_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_maj3+80,   E_interval_name_maj3_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_aug3+80,   E_interval_name_aug3_15ma, ENUM_FIXED_ALLOC);

         add(E_base40_dim4+80,   E_interval_name_dim4_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_per4+80,   E_interval_name_per4_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_aug4+80,   E_interval_name_aug4_15ma, ENUM_FIXED_ALLOC);

         add(E_base40_dim5+80,   E_interval_name_dim5_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_per5+80,   E_interval_name_per5_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_aug5+80,   E_interval_name_aug5_15ma, ENUM_FIXED_ALLOC);

         add(E_base40_dim6+80,   E_interval_name_dim6_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_min6+80,   E_interval_name_min6_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_maj6+80,   E_interval_name_maj6_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_aug6+80,   E_interval_name_aug6_15ma, ENUM_FIXED_ALLOC);

         add(E_base40_dim7+80,   E_interval_name_dim7_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_min7+80,   E_interval_name_min7_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_maj7+80,   E_interval_name_maj7_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_aug7+80,   E_interval_name_aug7_15ma, ENUM_FIXED_ALLOC);

         add(E_base40_dim8+80,   E_interval_name_dim8_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_per8+80,   E_interval_name_per8_15ma, ENUM_FIXED_ALLOC);
         add(E_base40_aug8+80,   E_interval_name_aug8_15ma, ENUM_FIXED_ALLOC);

      }
};


#endif  /* _ENUMERATIONINTERVAL_H_INCLUDED */



// md5sum: f5cd4b18f18fec7089ba1b57add993c1 EnumerationInterval.h [20001204]

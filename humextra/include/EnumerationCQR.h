//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jun 12 23:08:15 PDT 1998
// Last Modified: Fri Jun 12 23:08:20 PDT 1998
// Filename:      sig/include/sigInfo/EnumerationCQR.h
// Web Address:   http://sig.sapp.org/include/sigInfo/EnumerationCQR.h
// Syntax:        C++ 
//
// Description:   Enumeration database for chord roots.
//

#ifndef _ENUMERATIONCQR_H_INCLUDED
#define _ENUMERATIONCQR_H_INCLUDED


#include "Enumeration.h"
#include "Enum_chordQuality.h"


class EnumerationCQR : public Enumeration {
   public:
      EnumerationCQR(void) : Enumeration() {
         setNullName("X", ENUM_FIXED_ALLOC);
         add(E_root_rest ,   E_root_name_rest ,   ENUM_FIXED_ALLOC);
         add(E_root_cff  ,   E_root_name_cff  ,   ENUM_FIXED_ALLOC);
         add(E_root_cf   ,   E_root_name_cf   ,   ENUM_FIXED_ALLOC);
         add(E_root_c    ,   E_root_name_c    ,   ENUM_FIXED_ALLOC);
         add(E_root_cs   ,   E_root_name_cs   ,   ENUM_FIXED_ALLOC);
         add(E_root_css  ,   E_root_name_css  ,   ENUM_FIXED_ALLOC);
         add(E_root_dff  ,   E_root_name_dff  ,   ENUM_FIXED_ALLOC);
         add(E_root_df   ,   E_root_name_df   ,   ENUM_FIXED_ALLOC);
         add(E_root_d    ,   E_root_name_d    ,   ENUM_FIXED_ALLOC);
         add(E_root_ds   ,   E_root_name_ds   ,   ENUM_FIXED_ALLOC);
         add(E_root_dss  ,   E_root_name_dss  ,   ENUM_FIXED_ALLOC);
         add(E_root_eff  ,   E_root_name_eff  ,   ENUM_FIXED_ALLOC);
         add(E_root_ef   ,   E_root_name_ef   ,   ENUM_FIXED_ALLOC);
         add(E_root_e    ,   E_root_name_e    ,   ENUM_FIXED_ALLOC);
         add(E_root_es   ,   E_root_name_es   ,   ENUM_FIXED_ALLOC);
         add(E_root_ess  ,   E_root_name_ess  ,   ENUM_FIXED_ALLOC);
         add(E_root_fff  ,   E_root_name_fff  ,   ENUM_FIXED_ALLOC);
         add(E_root_ff   ,   E_root_name_ff   ,   ENUM_FIXED_ALLOC);
         add(E_root_f    ,   E_root_name_f    ,   ENUM_FIXED_ALLOC);
         add(E_root_fs   ,   E_root_name_fs   ,   ENUM_FIXED_ALLOC);
         add(E_root_fss  ,   E_root_name_fss  ,   ENUM_FIXED_ALLOC);
         add(E_root_gff  ,   E_root_name_gff  ,   ENUM_FIXED_ALLOC);
         add(E_root_gf   ,   E_root_name_gf   ,   ENUM_FIXED_ALLOC);
         add(E_root_g    ,   E_root_name_g    ,   ENUM_FIXED_ALLOC);
         add(E_root_gs   ,   E_root_name_gs   ,   ENUM_FIXED_ALLOC);
         add(E_root_gss  ,   E_root_name_gss  ,   ENUM_FIXED_ALLOC);
         add(E_root_aff  ,   E_root_name_aff  ,   ENUM_FIXED_ALLOC);
         add(E_root_af   ,   E_root_name_af   ,   ENUM_FIXED_ALLOC);
         add(E_root_a    ,   E_root_name_a    ,   ENUM_FIXED_ALLOC);
         add(E_root_as   ,   E_root_name_as   ,   ENUM_FIXED_ALLOC);
         add(E_root_ass  ,   E_root_name_ass  ,   ENUM_FIXED_ALLOC);
         add(E_root_bff  ,   E_root_name_bff  ,   ENUM_FIXED_ALLOC);
         add(E_root_bf   ,   E_root_name_bf   ,   ENUM_FIXED_ALLOC);
         add(E_root_b    ,   E_root_name_b    ,   ENUM_FIXED_ALLOC);
         add(E_root_bs   ,   E_root_name_bs   ,   ENUM_FIXED_ALLOC);
         add(E_root_bss  ,   E_root_name_bss  ,   ENUM_FIXED_ALLOC);
      }
};


#endif  /* _ENUMERATIONCQR_H_INCLUDED */



// md5sum: 70cdf6278d954b42d53b4f2bd991dd84 EnumerationCQR.h [20050403]

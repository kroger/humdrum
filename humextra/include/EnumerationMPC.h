//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jul  1 10:56:02 PDT 1998
// Last Modified: Wed Jul  1 11:08:00 PDT 1998
// Filename:      ...sig/include/sigInfo/EnumerationMPC.h
// Web Address:   http://sig.sapp.org/include/sigInfo/EnumerationMPC.h
// Syntax:        C++ 
//

#ifndef _ENUMERATIONMPC_H_INCLUDED
#define _ENUMERATIONMPC_H_INCLUDED


#include "Enumeration.h"
#include "Enum_musepitch.h"


class EnumerationMPC : public Enumeration {
   public:
      EnumerationMPC(void) : Enumeration() {
         setNullName("X", ENUM_FIXED_ALLOC);
         add(E_muse_rest ,   E_muse_name_rest ,   ENUM_FIXED_ALLOC);
         add(E_muse_cff  ,   E_muse_name_cff  ,   ENUM_FIXED_ALLOC);
         add(E_muse_cf   ,   E_muse_name_cf   ,   ENUM_FIXED_ALLOC);
         add(E_muse_c    ,   E_muse_name_c    ,   ENUM_FIXED_ALLOC);
         add(E_muse_cs   ,   E_muse_name_cs   ,   ENUM_FIXED_ALLOC);
         add(E_muse_css  ,   E_muse_name_css  ,   ENUM_FIXED_ALLOC);
         add(E_muse_dff  ,   E_muse_name_dff  ,   ENUM_FIXED_ALLOC);
         add(E_muse_df   ,   E_muse_name_df   ,   ENUM_FIXED_ALLOC);
         add(E_muse_d    ,   E_muse_name_d    ,   ENUM_FIXED_ALLOC);
         add(E_muse_ds   ,   E_muse_name_ds   ,   ENUM_FIXED_ALLOC);
         add(E_muse_dss  ,   E_muse_name_dss  ,   ENUM_FIXED_ALLOC);
         add(E_muse_eff  ,   E_muse_name_eff  ,   ENUM_FIXED_ALLOC);
         add(E_muse_ef   ,   E_muse_name_ef   ,   ENUM_FIXED_ALLOC);
         add(E_muse_e    ,   E_muse_name_e    ,   ENUM_FIXED_ALLOC);
         add(E_muse_es   ,   E_muse_name_es   ,   ENUM_FIXED_ALLOC);
         add(E_muse_ess  ,   E_muse_name_ess  ,   ENUM_FIXED_ALLOC);
         add(E_muse_fff  ,   E_muse_name_fff  ,   ENUM_FIXED_ALLOC);
         add(E_muse_ff   ,   E_muse_name_ff   ,   ENUM_FIXED_ALLOC);
         add(E_muse_f    ,   E_muse_name_f    ,   ENUM_FIXED_ALLOC);
         add(E_muse_fs   ,   E_muse_name_fs   ,   ENUM_FIXED_ALLOC);
         add(E_muse_fss  ,   E_muse_name_fss  ,   ENUM_FIXED_ALLOC);
         add(E_muse_gff  ,   E_muse_name_gff  ,   ENUM_FIXED_ALLOC);
         add(E_muse_gf   ,   E_muse_name_gf   ,   ENUM_FIXED_ALLOC);
         add(E_muse_g    ,   E_muse_name_g    ,   ENUM_FIXED_ALLOC);
         add(E_muse_gs   ,   E_muse_name_gs   ,   ENUM_FIXED_ALLOC);
         add(E_muse_gss  ,   E_muse_name_gss  ,   ENUM_FIXED_ALLOC);
         add(E_muse_aff  ,   E_muse_name_aff  ,   ENUM_FIXED_ALLOC);
         add(E_muse_af   ,   E_muse_name_af   ,   ENUM_FIXED_ALLOC);
         add(E_muse_a    ,   E_muse_name_a    ,   ENUM_FIXED_ALLOC);
         add(E_muse_as   ,   E_muse_name_as   ,   ENUM_FIXED_ALLOC);
         add(E_muse_ass  ,   E_muse_name_ass  ,   ENUM_FIXED_ALLOC);
         add(E_muse_bff  ,   E_muse_name_bff  ,   ENUM_FIXED_ALLOC);
         add(E_muse_bf   ,   E_muse_name_bf   ,   ENUM_FIXED_ALLOC);
         add(E_muse_b    ,   E_muse_name_b    ,   ENUM_FIXED_ALLOC);
         add(E_muse_bs   ,   E_muse_name_bs   ,   ENUM_FIXED_ALLOC);
         add(E_muse_bss  ,   E_muse_name_bss  ,   ENUM_FIXED_ALLOC);
      }
};


#endif  /* _ENUMERATIONMPC_H_INCLUDED */



// md5sum: 160e833f2ab14b8b608f308da7375617 EnumerationMPC.h [20050403]

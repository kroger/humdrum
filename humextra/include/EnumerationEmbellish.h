//
// Copyright 2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun 29 21:33:17 PDT 2000
// Last Modified: Thu Jun 29 21:33:20 PDT 2000
// Filename:      ...sig/include/sigInfo/EnumerationEmbellish.h
// Web Address:   http://sig.sapp.org/include/sigInfo/EnumerationEmbellish.h
// Syntax:        C++; museinfo
//
// Description:   Enumeration database for Humdrum emblishment types.
//

#ifndef _ENUMERATIONEMBELLISH_H_INCLUDED
#define _ENUMERATIONEMBELLISH_H_INCLUDED


#include "Enumeration.h"
#include "Enum_embel.h"

class EnumerationEmbellish : public Enumeration {
   public:
      EnumerationEmbellish(void) : Enumeration() {
         setNullName("Error", ENUM_FIXED_ALLOC);
         add(E_embel_aln    ,   E_embel_aln_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_ant    ,   E_embel_ant_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_app    ,   E_embel_app_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_apt    ,   E_embel_apt_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_arp    ,   E_embel_arp_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_arp7   ,   E_embel_arp7_name   ,   ENUM_FIXED_ALLOC);
         add(E_embel_aun    ,   E_embel_aun_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_chg    ,   E_embel_chg_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_cln    ,   E_embel_cln_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_ct     ,   E_embel_ct_name     ,   ENUM_FIXED_ALLOC);
         add(E_embel_ct7    ,   E_embel_ct7_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_cun    ,   E_embel_cun_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_cup    ,   E_embel_cup_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_et     ,   E_embel_et_name     ,   ENUM_FIXED_ALLOC);
         add(E_embel_ln     ,   E_embel_ln_name     ,   ENUM_FIXED_ALLOC);
         add(E_embel_ped    ,   E_embel_ped_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_rep    ,   E_embel_rep_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_ret    ,   E_embel_ret_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_23ret  ,   E_embel_23ret_name  ,   ENUM_FIXED_ALLOC);
         add(E_embel_78ret  ,   E_embel_78ret_name  ,   ENUM_FIXED_ALLOC);
         add(E_embel_sus    ,   E_embel_sus_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_43sus  ,   E_embel_43sus_name  ,   ENUM_FIXED_ALLOC);
         add(E_embel_98sus  ,   E_embel_98sus_name  ,   ENUM_FIXED_ALLOC);
         add(E_embel_76sus  ,   E_embel_76sus_name  ,   ENUM_FIXED_ALLOC);
         add(E_embel_un     ,   E_embel_un_name     ,   ENUM_FIXED_ALLOC);
         add(E_embel_un7    ,   E_embel_un7_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_upt    ,   E_embel_upt_name    ,   ENUM_FIXED_ALLOC);
         add(E_embel_upt7   ,   E_embel_upt7_name   ,   ENUM_FIXED_ALLOC);
      }
};


#endif  /* _ENUMERATIONEMBELLISH_H_INCLUDED */



// md5sum: fcbb17251e7c1907f07bced126fb7f3c EnumerationEmbellish.h [20001204]

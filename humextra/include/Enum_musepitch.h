//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jul  1 11:00:08 PDT 1998
// Last Modified: Wed Jul  1 11:00:11 PDT 1998
// Filename:      ...sig/include/sigInfo/Enum_musepitch.h
// Web Address:   http://sig.sapp.org/include/sigInfo/Enum_musepitch.h
// Syntax:        C++ 
//
// Description:   Enumeration defines for MuseData musical pitches.
//

#ifndef _ENUM_MUSE_PITCH_H_INCLUDED
#define _ENUM_MUSE_PITCH_H_INCLUDED

#include "Enum_basic.h"


///////////////////////////////////////////////////////////////////////////
//
// Chord Types
//

#define E_chord_unknown        (000)       /* example:   C D E F F# G B      */


///////////////////////////////////////////////////////////////////////////////
//
// Chord muses
//

#define E_muse_rest  (-9999)

#define E_muse_cff   (0)
#define E_muse_cf    (1)
#define E_muse_c     (2)
#define E_muse_cs    (3)
#define E_muse_css   (4)

#define E_muse_dff   (6)
#define E_muse_df    (7)
#define E_muse_d     (8)
#define E_muse_ds    (9)
#define E_muse_dss   (10)

#define E_muse_eff   (12)
#define E_muse_ef    (13)
#define E_muse_e     (14)
#define E_muse_es    (15)
#define E_muse_ess   (16)

#define E_muse_fff   (17)
#define E_muse_ff    (18)
#define E_muse_f     (19)
#define E_muse_fs    (20)
#define E_muse_fss   (21)

#define E_muse_gff   (23)
#define E_muse_gf    (24)
#define E_muse_g     (25)
#define E_muse_gs    (26)
#define E_muse_gss   (27)

#define E_muse_aff   (29)
#define E_muse_af    (30)
#define E_muse_a     (31)
#define E_muse_as    (32)
#define E_muse_ass   (33)

#define E_muse_bff   (35)
#define E_muse_bf    (36)
#define E_muse_b     (37)
#define E_muse_bs    (38)
#define E_muse_bss   (39)


// names for pitch class muses:

#define E_muse_name_rest    "r"
#define E_muse_name_cff     "Cff"
#define E_muse_name_cf      "Cf"
#define E_muse_name_c       "C"
#define E_muse_name_cs      "C#"
#define E_muse_name_css     "C##"
#define E_muse_name_dff     "Dff"
#define E_muse_name_df      "Df"
#define E_muse_name_d       "D"
#define E_muse_name_ds      "D#"
#define E_muse_name_dss     "D##"
#define E_muse_name_eff     "Eff"
#define E_muse_name_ef      "Ef"
#define E_muse_name_e       "E"
#define E_muse_name_es      "E#"
#define E_muse_name_ess     "E##"
#define E_muse_name_fff     "Fff"
#define E_muse_name_ff      "Ff"
#define E_muse_name_f       "F"
#define E_muse_name_fs      "F#"
#define E_muse_name_fss     "F##"
#define E_muse_name_gff     "Gff"
#define E_muse_name_gf      "Gf"
#define E_muse_name_g       "G"
#define E_muse_name_gs      "G#"
#define E_muse_name_gss     "G##"
#define E_muse_name_aff     "Aff"
#define E_muse_name_af      "Af"
#define E_muse_name_a       "A"
#define E_muse_name_as      "A#"
#define E_muse_name_ass     "A##"
#define E_muse_name_bff     "Bff"
#define E_muse_name_bf      "Bf"
#define E_muse_name_b       "B"
#define E_muse_name_bs      "B#"
#define E_muse_name_bss     "B##"


#endif  /* _ENUM_MUSE_PITCH_H_INCLUDED */



// md5sum: cbe055d560693b72ab12c6ded6f14bb9 Enum_musepitch.h [20050403]

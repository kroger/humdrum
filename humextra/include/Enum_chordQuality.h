//
// Copyright 1997-1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu May 22 21:38:55 PDT 1997
// Last Modified: Tue Jun  9 22:56:33 PDT 1998
// Filename:      ...sig/include/sigInfo/Enum_chordQuality.h
// Web Address:   http://sig.sapp.org/include/sigInfo/Enum_chordQuality.h
// Syntax:        C++ 
//
//
// Description:   Enumeration defined for the ChordQuality class.
//

#ifndef _ENUM_CHORDQUALITY_H_INCLUDED
#define _ENUM_CHORDQUALITY_H_INCLUDED

#include "Enum_basic.h"


///////////////////////////////////////////////////////////////////////////
//
// Chord Types
//

#define E_chord_unknown        (000)       /* example:   C D E F F# G B      */

//////////////////////////////
//
// basic chord quality set #1
//

#define E_chord_rest           (100)       /* example:                       */
#define E_chord_note           (101)       /* example:   C                   */

// dyads
#define E_chord_incmaj         (102)       /* example:   C E                 */
#define E_chord_incmin         (103)       /* example:   C Eb                */
 
// triads
#define E_chord_dim            (104)       /* example:   C Eb Gb             */
#define E_chord_min            (105)       /* example:   C Eb G              */
#define E_chord_maj            (106)       /* example:   C E  G              */
#define E_chord_aug            (107)       /* example:   C E  G#             */
#define E_chord_minminx5       (108)       /* example:   C Eb    Bb          */
#define E_chord_domsevx5       (109)       /* example:   C E     Bb          */

// tetrads
#define E_chord_fullydim       (110)       /* example:   C Eb Gb Bbb         */
#define E_chord_halfdim        (111)       /* example:   C Eb Gb Bb          */
#define E_chord_minmin         (112)       /* example:   C Eb G  Bb          */
#define E_chord_minmaj         (113)       /* example:   C Eb G  B           */
#define E_chord_domsev         (114)       /* example:   C E  G  Bb          */
#define E_chord_majmaj         (115)       /* example:   C E  G  B           */

// special chords
#define E_chord_frensix        (116)       /* example:   Ab C D  F#          */
#define E_chord_germsix        (117)       /* example:   Ab C Eb F#          */
#define E_chord_italsix        (118)       /* example:   Ab C    F#          */


// names of chord quality types for **qual data for the basic set

#define E_chord_name_rest      "r"
#define E_chord_name_unknown   "X"
#define E_chord_name_note      "note"
#define E_chord_name_incmaj    "incmaj"
#define E_chord_name_incmin    "incmin"
#define E_chord_name_dim       "dim"
#define E_chord_name_min       "min"
#define E_chord_name_maj       "maj"
#define E_chord_name_aug       "aug"
#define E_chord_name_minminx5  "minminx5"
#define E_chord_name_domsevx5  "domsevx5"
#define E_chord_name_fullydim  "fullydim"
#define E_chord_name_halfdim   "halfdim"
#define E_chord_name_minmin    "minmin"
#define E_chord_name_minmaj    "minmaj"
#define E_chord_name_domsev    "domsev"
#define E_chord_name_majmaj    "majmaj"
#define E_chord_name_frensix   "french"
#define E_chord_name_germsix   "german"
#define E_chord_name_italsix   "italian"



///////////////////////////////
//
// functional Harmony set #2
//

#define E_chord_neopol         (201)      /* example:   Db F  Ab             */
#define E_chord_secdom         (202)      /* example:   D  F# A              */
#define E_chord_secdomsev      (203)      /* example:   D  F# A  C           */
#define E_chord_secsev         (204)      /* example:   F# A  C              */
#define E_chord_secsevo        (205)      /* example:   F# A  C  Eb          */
#define E_chord_secsevc        (206)      /* example:   F# A  C  E           */


// names of chord quality types for **qual data for the basic set

#define E_chord_name_neopol    "neopol"
#define E_chord_name_secdom    "secdom"
#define E_chord_name_secdomsev "secdomsev"
#define E_chord_name_secsev    "secsev"
#define E_chord_name_secsevo   "secsevo"
#define E_chord_name_secsevc   "secsevc"



////////////////////////////
//
// Other chord quality sets
//

   // ...


///////////////////////////////////////////////////////////////////////////////
//
// Chord Inversions
//

#define E_inv_root    (0)  
#define E_inv_first   (1)  
#define E_inv_second  (2)  
#define E_inv_third   (3)  
#define E_inv_fourth  (4)  
#define E_inv_fifth   (5)  
#define E_inv_sixth   (6)  


#define E_inv_name_root   "0"
#define E_inv_name_first  "1"
#define E_inv_name_second "2"
#define E_inv_name_third  "3"
#define E_inv_name_fourth "4"
#define E_inv_name_fifth  "5"
#define E_inv_name_sixth  "6"



///////////////////////////////////////////////////////////////////////////////
//
// Chord Roots
//

#define E_root_rest  (-9999)

#define E_root_cff   (0)
#define E_root_cf    (1)
#define E_root_c     (2)
#define E_root_cs    (3)
#define E_root_css   (4)

#define E_root_dff   (6)
#define E_root_df    (7)
#define E_root_d     (8)
#define E_root_ds    (9)
#define E_root_dss   (10)

#define E_root_eff   (12)
#define E_root_ef    (13)
#define E_root_e     (14)
#define E_root_es    (15)
#define E_root_ess   (16)

#define E_root_fff   (17)
#define E_root_ff    (18)
#define E_root_f     (19)
#define E_root_fs    (20)
#define E_root_fss   (21)

#define E_root_gff   (23)
#define E_root_gf    (24)
#define E_root_g     (25)
#define E_root_gs    (26)
#define E_root_gss   (27)

#define E_root_aff   (29)
#define E_root_af    (30)
#define E_root_a     (31)
#define E_root_as    (32)
#define E_root_ass   (33)

#define E_root_bff   (35)
#define E_root_bf    (36)
#define E_root_b     (37)
#define E_root_bs    (38)
#define E_root_bss   (39)


// names for pitch class roots:

#define E_root_name_rest    "R"
#define E_root_name_cff     "C--"
#define E_root_name_cf      "C-"
#define E_root_name_c       "C"
#define E_root_name_cs      "C#"
#define E_root_name_css     "C##"
#define E_root_name_dff     "D--"
#define E_root_name_df      "D-"
#define E_root_name_d       "D"
#define E_root_name_ds      "D#"
#define E_root_name_dss     "D##"
#define E_root_name_eff     "E--"
#define E_root_name_ef      "E-"
#define E_root_name_e       "E"
#define E_root_name_es      "E#"
#define E_root_name_ess     "E##"
#define E_root_name_fff     "F--"
#define E_root_name_ff      "F-"
#define E_root_name_f       "F"
#define E_root_name_fs      "F#"
#define E_root_name_fss     "F##"
#define E_root_name_gff     "G--"
#define E_root_name_gf      "G-"
#define E_root_name_g       "G"
#define E_root_name_gs      "G#"
#define E_root_name_gss     "G##"
#define E_root_name_aff     "A--"
#define E_root_name_af      "A-"
#define E_root_name_a       "A"
#define E_root_name_as      "A#"
#define E_root_name_ass     "A##"
#define E_root_name_bff     "B--"
#define E_root_name_bf      "B-"
#define E_root_name_b       "B"
#define E_root_name_bs      "B#"
#define E_root_name_bss     "B##"


#endif  /* _ENUM_CHORDQUALITY_H_INCLUDED */



// md5sum: c8a453342d39bd06d2d7e088eadd5303 Enum_chordQuality.h [20001204]

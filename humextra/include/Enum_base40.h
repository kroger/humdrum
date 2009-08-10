//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May 18 16:36:55 PDT 1998
// Last Modified: Tue Jul  7 23:02:54 PDT 1998
// Filename:      ...sig/include/sigInfo/Enum_base40.h
// Web Address:   http://sig.sapp.org/include/sigInfo/Enum_base40.h
// Syntax:        C++ 
//
// Description:   Base-40 pitch and interval repersentation defines.
//

#ifndef _ENUM_BASEFORTY_H_INCLUDED
#define _ENUM_BASEFORTY_H_INCLUDED

#include "Enum_basic.h"

#define BASE40_NOTE(PITCHCLASS, OCTAVE)  ((OCTAVE) * 40 + (PITCHCLASS))
#define BASE40_PITCHCLASS(NOTE) ((NOTE) % 40)

#define E_base40_rest        (-9999)


// base 40 interval sizes

#define E_base40_dim1        (-1)
#define E_base40_per1        (0)
#define E_base40_aug1        (1)

#define E_base40_dim2        (4)
#define E_base40_min2        (5)
#define E_base40_maj2        (6)
#define E_base40_aug2        (7)

#define E_base40_dim3        (10)
#define E_base40_min3        (11)
#define E_base40_maj3        (12)
#define E_base40_aug3        (13)

#define E_base40_dim4        (16)
#define E_base40_per4        (17)
#define E_base40_aug4        (18)

#define E_base40_dim5        (22)
#define E_base40_per5        (23)
#define E_base40_aug5        (24)

#define E_base40_dim6        (27)
#define E_base40_min6        (28)
#define E_base40_maj6        (29)
#define E_base40_aug6        (30)

#define E_base40_dim7        (33)
#define E_base40_min7        (34)
#define E_base40_maj7        (35)
#define E_base40_aug7        (36)

#define E_base40_dim8        (39)
#define E_base40_per8        (40)
#define E_base40_aug8        (41)


// interval names


#define E_interval_name_dim1     "dim1"
#define E_interval_name_per1     "per1"
#define E_interval_name_aug1     "aug1"

#define E_interval_name_dim2     "dim2"
#define E_interval_name_min2     "min2"
#define E_interval_name_maj2     "maj2"
#define E_interval_name_aug2     "aug2"

#define E_interval_name_dim3     "dim3"
#define E_interval_name_min3     "min3"
#define E_interval_name_maj3     "maj3"
#define E_interval_name_aug3     "aug3"

#define E_interval_name_dim4     "dim4"
#define E_interval_name_per4     "per4"
#define E_interval_name_aug4     "aug4"

#define E_interval_name_dim5     "dim5"
#define E_interval_name_per5     "per5"
#define E_interval_name_aug5     "aug5"

#define E_interval_name_dim6     "dim6"
#define E_interval_name_min6     "min6"
#define E_interval_name_maj6     "maj6"
#define E_interval_name_aug6     "aug6"

#define E_interval_name_dim7     "dim7"
#define E_interval_name_min7     "min7"
#define E_interval_name_maj7     "maj7"
#define E_interval_name_aug7     "aug7"

#define E_interval_name_dim8     "dim8"
#define E_interval_name_per8     "per8"
#define E_interval_name_aug8     "aug8"




#define E_interval_name_dim2_8va     "dim2+8va"
#define E_interval_name_min2_8va     "min2+8va"
#define E_interval_name_maj2_8va     "maj2+8va"
#define E_interval_name_aug2_8va     "aug2+8va"

#define E_interval_name_dim3_8va     "dim3+8va"
#define E_interval_name_min3_8va     "min3+8va"
#define E_interval_name_maj3_8va     "maj3+8va"
#define E_interval_name_aug3_8va     "aug3+8va"

#define E_interval_name_dim4_8va     "dim4+8va"
#define E_interval_name_per4_8va     "per4+8va"
#define E_interval_name_aug4_8va     "aug4+8va"

#define E_interval_name_dim5_8va     "dim5+8va"
#define E_interval_name_per5_8va     "per5+8va"
#define E_interval_name_aug5_8va     "aug5+8va"

#define E_interval_name_dim6_8va     "dim6+8va"
#define E_interval_name_min6_8va     "min6+8va"
#define E_interval_name_maj6_8va     "maj6+8va"
#define E_interval_name_aug6_8va     "aug6+8va"

#define E_interval_name_dim7_8va     "dim7+8va"
#define E_interval_name_min7_8va     "min7+8va"
#define E_interval_name_maj7_8va     "maj7+8va"
#define E_interval_name_aug7_8va     "aug7+8va"

#define E_interval_name_dim8_8va     "dim8+8va"
#define E_interval_name_per8_8va     "per8+8va"
#define E_interval_name_aug8_8va     "aug8+8va"



#define E_interval_name_dim2_15ma     "dim2+15ma"
#define E_interval_name_min2_15ma     "min2+15ma"
#define E_interval_name_maj2_15ma     "maj2+15ma"
#define E_interval_name_aug2_15ma     "aug2+15ma"

#define E_interval_name_dim3_15ma     "dim3+15ma"
#define E_interval_name_min3_15ma     "min3+15ma"
#define E_interval_name_maj3_15ma     "maj3+15ma"
#define E_interval_name_aug3_15ma     "aug3+15ma"

#define E_interval_name_dim4_15ma     "dim4+15ma"
#define E_interval_name_per4_15ma     "per4+15ma"
#define E_interval_name_aug4_15ma     "aug4+15ma"

#define E_interval_name_dim5_15ma     "dim5+15ma"
#define E_interval_name_per5_15ma     "per5+15ma"
#define E_interval_name_aug5_15ma     "aug5+15ma"

#define E_interval_name_dim6_15ma     "dim6+15ma"
#define E_interval_name_min6_15ma     "min6+15ma"
#define E_interval_name_maj6_15ma     "maj6+15ma"
#define E_interval_name_aug6_15ma     "aug6+15ma"

#define E_interval_name_dim7_15ma     "dim7+15ma"
#define E_interval_name_min7_15ma     "min7+15ma"
#define E_interval_name_maj7_15ma     "maj7+15ma"
#define E_interval_name_aug7_15ma     "aug7+15ma"

#define E_interval_name_dim8_15ma     "dim8+15ma"
#define E_interval_name_per8_15ma     "per8+15ma"
#define E_interval_name_aug8_15ma     "aug8+15ma"


// interval short names by line of fifth value ("C" = 0)

#define FIFTH_n15 "D4"    /* Fbb */
#define FIFTH_n14 "D1"    /* Cbb */
#define FIFTH_n13 "D5"    /* Gbb */
#define FIFTH_n12 "d2"    /* Dbb */
#define FIFTH_n11 "d6"    /* Abb */
#define FIFTH_n10 "d3"    /* Ebb */
#define FIFTH_n9  "d7"    /* Bbb */
#define FIFTH_n8  "d4"    /* Fb  */
#define FIFTH_n7  "d1"    /* Cb  */
#define FIFTH_n6  "d5"    /* Gb  */
#define FIFTH_n5  "m2"    /* Db  */
#define FIFTH_n4  "m6"    /* Ab  */
#define FIFTH_n3  "m3"    /* Eb  */
#define FIFTH_n2  "m7"    /* Bb  */
#define FIFTH_n1  "p4"    /* F   */
#define FIFTH_0   "p1"    /* C   */
#define FIFTH_p1  "p5"    /* G   */
#define FIFTH_p2  "M2"    /* D   */
#define FIFTH_p3  "M6"    /* A   */
#define FIFTH_p4  "M3"    /* E   */
#define FIFTH_p5  "M7"    /* B   */
#define FIFTH_p6  "a4"    /* F#  */
#define FIFTH_07  "a1"    /* C#  */
#define FIFTH_p8  "a5"    /* G#  */
#define FIFTH_p9  "a2"    /* D#  */
#define FIFTH_p10 "a6"    /* A#  */
#define FIFTH_p11 "a3"    /* E#  */
#define FIFTH_p12 "a7"    /* B#  */
#define FIFTH_p13 "A4"    /* Fx  */
#define FIFTH_014 "A1"    /* Cx  */
#define FIFTH_p15 "A5"    /* Gx  */
#define FIFTH_p16 "A2"    /* Dx  */
#define FIFTH_p17 "A6"    /* Ax  */
#define FIFTH_p18 "A3"    /* Ex  */
#define FIFTH_p19 "A7"    /* Bx  */


#endif  /* _ENUM_BASEFORTY_H_INCLUDED */



// md5sum: 971dca1a2392ae84cdd21ca0b1bea5d5 Enum_base40.h [20001204]

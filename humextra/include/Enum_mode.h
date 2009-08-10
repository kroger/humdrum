//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jun 28 09:45:14 PDT 1998
// Last Modified: Sun Jun 28 09:45:18 PDT 1998
// Filename:      ...sig/include/sigInfo/Enum_mode.h
// Web Address:   http://sig.sapp.org/include/sigInfo/Enum_mode.h
// Syntax:        C++ 
//
// Description:   Enumeration defines for various muscial scales.
//

#ifndef _ENUM_MODE_H_INCLUDED
#define _ENUM_MODE_H_INCLUDED

#include "Enum_basic.h"


#define E_mode_major           (101)       /* example:   C D E F G A B       */
#define E_mode_minor           (102)       /* example:   C D E- F G A- B-    */
#define E_mode_harm_minor      (103)       /* example:   C D E- F G A- B     */

#define E_mode_ionian          (201)       /* example:   C D E F G A B       */
#define E_mode_dorian          (202)       /* example:   D E F G A B C       */
#define E_mode_phrygian        (203)       /* example:   E F G A B C D       */
#define E_mode_lydian          (204)       /* example:   F G A B C D E       */
#define E_mode_mixolydian      (205)       /* example:   G A B C D E F       */
#define E_mode_aeolian         (206)       /* example:   A B C D E F G       */
#define E_mode_locrian         (207)       /* example:   B C D E F G A       */

#define E_mode_octatonic       (301)       /* example:   C C# D# E F# G A B- */

// names of modes quality types for **qual data for the basic set

#define E_mode_name_major        "major"
#define E_mode_name_minor        "minor"
#define E_mode_name_harm_minor   "harm-minor"

#define E_mode_name_ionian       "ionian"
#define E_mode_name_dorian       "dorian"
#define E_mode_name_phrygian     "phrygian"
#define E_mode_name_lydian       "lydian"
#define E_mode_name_mixolydian   "mixolydian"
#define E_mode_name_aeolian      "aeolian"
#define E_mode_name_locrian      "locrian"

#define E_mode_name_octatonic     "octatonic"


#endif  /* _ENUM_MODE_H_INCLUDED */



// md5sum: 33c8bc999a25242f1982d7da2b6fb2f6 Enum_mode.h [20050403]

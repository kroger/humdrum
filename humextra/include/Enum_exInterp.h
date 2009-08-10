//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat May 23 20:05:41 PDT 1998
// Last Modified: Wed Jun 10 22:42:18 PDT 1998
// Last Modified: Wed Oct 18 12:58:50 PDT 2000
// Filename:      ...sig/include/sigInfo/Enum_exInterp.h
// Web Address:   http://sig.sapp.org/include/sigInfo/Enum_exInterp.h
// Syntax:        C++ 
//
// Description:   Default enumeration defined for the Humdrum
//                Exclusive Enumeration class.
//

#ifndef _ENUM_EXINTERP_H_INCLUDED
#define _ENUM_EXINTERP_H_INCLUDED

#include "Enum_basic.h"

#define E_UNKNOWN_EXINT       (0000)      

#define E_UNKNOWN_EXINT_NAME     "**unknown"

// standard humdrum exclusive interpretations
#define E_KERN_EXINT          (1)        /*     **kern       */
#define E_KERN_EXINT_NAME     "**kern"


// Museinfo exclusive interpretations.
#define E_QUAL_EXINT          (999)      /*    **qual        */
#define E_QUAL_EXINT_NAME     "**qual"


#endif  /* _ENUM_EXINTERP_H_INCLUDED */



// md5sum: d30e98b0e9d739d06ced0777999e95ea Enum_exInterp.h [20050403]

//
// Copyright 1998-1999 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat May 23 21:13:15 PDT 1998
// Last Modified: Mon May 31 16:18:36 PDT 1999
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Filename:      ...sig/include/sigInfo/humdrum.h
// Web Address:   http://sig.sapp.org/include/sigInfo/humdrum.h
// Syntax:        C++ 
//
// Description:   Includes all header files necessary for humdrum classes.
//

#ifndef _HUMDRUM_H_INCLUDED
#define _HUMDRUM_H_INCLUDED

// headers:
   #include "HumdrumEnumerations.h"
   #include "museinfoVersion.h"

// basic classes:
   #include "HumdrumRecord.h"
   #include "HumdrumFile.h"
   #include "humdrumfileextras.h"
   #include "HumdrumFileBasic.h"
   #include "EnumerationData.h"
   #include "Enumeration.h"
   #include "Convert.h"
   #include "ChordQuality.h"
   #include "Identify.h"
   #include "HumdrumInstrument.h"
   #include "IntervalWeight.h"
   #include "RootSpectrum.h"
   #include "Maxwell.h"

// support classes borrowed from sig++
   #include "SigCollection.h"
   #include "Array.h"
   #include "Options.h"
   #include "PixelColor.h"
   #include "EnvelopeString.h"

// have to include for some template bug:
//   #include "Options.cpph"
// Works in this release and later: 
// gcc version egcs-2.91.66 19990314/Linux (egcs-1.1.2 release)

#endif  /* _HUMDRUM_H_INCLUDED */



// md5sum: 877f01173a4ef5b626b40b35061cb1e5 humdrum.h [20050403]

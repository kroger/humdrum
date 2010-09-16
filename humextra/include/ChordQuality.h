//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jun  7 16:24:06 PDT 1998
// Last Modified: Sun Jun  7 16:24:06 PDT 1998
// Last Modified: Thu May 14 21:53:43 PDT 2009 (modified output display)
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
//
// Filename:      ...sig/include/sigInfo/ChordQuality.cpp
// Web Address:   http://sig.sapp.org/include/sigInfo/ChordQuality.cpp
// Syntax:        C++ 
//
// Description:   Data class for storing the description of a chord.
//                The ChordQuality class has three properties:
//                   1) Root (the root note of the chord)
//                   2) Inversion (the inversion number of the chord)
//                   3) The chord type (E.g., major, minor, etc.)
//

#ifndef _CHORDQUALITY_H_INCLUDED
#define _CHORDQUALITY_H_INCLUDED

#include "SigCollection.h"
#include "Array.h"

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif


class ChordQuality {
   public:
                         ChordQuality     (void);
                         ChordQuality     (const ChordQuality& aChordQuality);
                         ChordQuality     (int aType, int anInversion, 
                                             int aRoot);
                        ~ChordQuality     ();

      ChordQuality&      operator=        (ChordQuality& aQuality);
      int                getBassNote      (void) const;
      static const char* getDisplay       (void);
      int                getInversion     (void) const;
      const char*        getInversionName (void) const;
      SigCollection<int> getNotesInChord  (void) const;
      void               getNotesInChord  (SigCollection<int>& notes) const;
      ostream&           printPitchClasses(ostream& out);
      void               setPitchClasses  (Array<int>& newnotes);
      void               setPitchClasses  (SigCollection<int>& newnotes);
      int                getRoot          (void) const;
      const char*        getRootName      (void) const;
      int                getType          (void) const;
      const char*        getTypeName      (void) const;
      void               makeString       (char* space, int pcsQ = 0);
      void               print            (const char* aDisplayString, 
                                             ostream& out = cout) const;
      void               print            (ostream& out = cout) const;
      static void        setDisplay       (const char* aDisplayFormat);
      void               setInversion     (int anInversion);
      void               setInversion     (const char* anInversionName);
      void               setQuality       (const char* aQuality);
      void               setRoot          (int aRoot);
      void               setRoot          (const char* aRootName);
      void               setType          (int aType);
      void               setType          (const char* aTypeName);

   private:
      SigCollection<int> chordNotes;      // pitch classes for unknown sonority
      int                chordType;       // chord type enumeration
      int                chordInversion;  // inversion of the chord
      int                chordRoot;       // root pitch class of the chord
      static char*       displayString;   // format to display quality data
};

ostream& operator<<(ostream& out, const ChordQuality& aQuality);


#endif  /* _CHORDQUALITY_H_INCLUDED */



// md5sum: de11f173a424d185a9480033af18ac4e ChordQuality.h [20050403]

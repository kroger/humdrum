//
// Copyright 1998 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 11:51:01 PDT 1998
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Last Modified: Thu Jun 10 00:11:08 PDT 2010 (added functions)
// Filename:      ...sig/include/sigInfo/MuseRecordBasic.cpp
// Web Address:   http://sig.sapp.org/include/sigInfo/MuseRecordBasic.h
// Syntax:        C++ 
//
// Description:   basic data manipulations for lines in a Musedata file.
//


#ifndef _MUSERECORDBASIC_H_INCLUDED
#define _MUSERECORDBASIC_H_INCLUDED

#include "Enum_muserec.h"
#include "RationalNumber.h"
#include "Array.h"

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

class MuseRecordBasic {
   public:
                        MuseRecordBasic    (void);
                        MuseRecordBasic    (const char* aLine, int index = -1);
                        MuseRecordBasic    (MuseRecordBasic& aRecord);
                       ~MuseRecordBasic    ();
  
      void              extract            (char* output, int start, int stop);
      char&             getColumn          (int index);
      int               getLength          (void) const;
      const char*       getLine            (void); 
      int               getLineIndex       (void) { return lineindex; }
      void              setLineIndex       (int index);
      int               getLineNumber      (void) { return lineindex+1; }
      int               getType            (void) const;
      MuseRecordBasic&  operator=          (MuseRecordBasic& aRecord);
      MuseRecordBasic&  operator=          (MuseRecordBasic* aRecord);
      MuseRecordBasic&  operator=          (char* aRecord);
      char&             operator[]         (int index);
      void              setLine            (const char* aString); 
      void              setType            (int aType);
      void              shrink             (void);
      void              insertString       (int index, const char* string);
      void              setString          (Array<char>& astring);

      // mark-up accessor functions:

      void              setAbsBeat         (RationalNumber value);
      void              setAbsBeat         (int topval, int botval = 1);
      double            getAbsBeat         (void);
      RationalNumber    getAbsBeatR        (void);

      void              setLineDuration    (RationalNumber value);
      void              setLineDuration    (int topval, int botval = 1);
      double            getLineDuration    (void);
      RationalNumber    getLineDurationR   (void);

      void              setNoteDuration    (RationalNumber value);
      void              setNoteDuration    (int topval, int botval = 1);
      double            getNoteDuration    (void);
      RationalNumber    getNoteDurationR   (void);

      void              setMarkupPitch     (int aPitch);
      int               getMarkupPitch     (void);


      // tied note functions:
      int               isTied             (void);
      int               getLastTiedNoteLineIndex(void);
      int               getNextTiedNoteLineIndex(void);
      void              setLastTiedNoteLineIndex(int index);
      void              setNextTiedNoteLineIndex(int index);

   protected:
      Array<char>       recordString;      // actual characters on line

      // mark-up data for the line:
      int               lineindex;         // index into origina file
      int               type;              // category of MuseRecordBasic record
      RationalNumber    absbeat;           // dur in quarter notes from start
      RationalNumber    lineduration;      // duration of line 
      RationalNumber    noteduration;      // duration of note

      int               b40pitch;          // base 40 pitch
      int               nexttiednote;      // line number of next note tied to
                                           // this one (-1 if no tied note)
      int               lasttiednote;      // line number of previous note tied
                                           // to this one (-1 if no tied note)

};
   

ostream& operator<<(ostream& out, MuseRecordBasic& aRecord);


#endif  /* _MUSERECORDBASIC_H_INCLUDED */



// md5sum: b2da1554b44dd3a66577a7203e62ab6a MuseRecordBasic.h [20050403]

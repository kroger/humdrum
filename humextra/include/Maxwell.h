//
// Copyright 2003 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jan  1 16:19:42 PST 2003 (extracted from HumdrumFile.h)
// Creation Date: Wed Jan  1 16:38:30 PST 2003
// Filename:      ...sig/include/sigInfo/Maxwell.h
// Web Address:   http://sig.sapp.org/include/sigInfo/Maxwell.h
// Syntax:        C++ 
//
// Description:   Harmony analysis functions based on Maxwell's dissertation.
//

#ifndef _MAXWELL_H_INCLUDED
#define _MAXWELL_H_INCLUDED

#include "HumdrumFile.h"
#include "EnumerationInterval.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

//
// Maxwell::analyzeVerticalDissonance defines
//

#define CONSONANT_VERTICAL   0
#define DISSONANT_VERTICAL   1
#define UNDEFINED_VERTICAL  -1

//
// Maxwell::analyzeTertian defines
//

#define TERTIAN_YES       0
#define TERTIAN_NO        1
#define TERTIAN_UNKNOWN  -1

//
// Maxwell::analyzeAccent defines
//

#define ACCENT_YES       1
#define ACCENT_NO        0
#define ACCENT_UNKNOWN  -1

//
// Maxwell::analyzeTertianDissonanceLevel defines
//

#define TERTIAN_UNKNOWN  -1
#define TERTIAN_0         0   /* i.e., all rests or single notes */
#define TERTIAN_1         1
#define TERTIAN_1_5       1.5 /* dominant sevenths (=2) */
#define TERTIAN_2         2
#define TERTIAN_2_5       2.5 /* incomplete minor minor sevenths (=2) */
#define TERTIAN_3         3
#define TERTIAN_4         4
#define TERTIAN_5         5

//
// HumdrumFile::analyzeMaxwellDissonantInContext defines
//

#define DISSIC_UNDEFINED -1
#define DISSIC_YES        0
#define DISSIC_NO         1

//
// Maxwell::analyzeDissonantNotes defines
//

typedef Array<int> ArrayInt;

#define NOTEDISSONANT_UNKNOWN  -1
#define NOTEDISSONANT_NO        0
#define NOTEDISSONANT_YES       1

//
// Maxwell::analyzeSonorityRelations defines
//

#define CHORD_UNKNOWN 'X'
#define CHORD_CHORD   'c'
#define CHORD_PREVSUB 'p'
#define CHORD_NEXTSUB 'n'
#define CHORD_PASSING 'g'
#define CHORD_SUSPEND 's'
#define CHORD_SUBORD  'u'


class Maxwell {
   public:
            Maxwell                       (void);
           ~Maxwell                       ();

      static void  analyzeVerticalDissonance(HumdrumFile& score, 
                                             Array<int>& vertdis);
      static void  analyzeTertian           (HumdrumFile& score,
                                             Array<int>& tertian);
      static void  analyzeAccent            (HumdrumFile& score,
                                             Array<int>& accent, 
                                             int flag = AFLAG_COMPOUND_METER);
      static void  analyzeTertianDissonanceLevel (HumdrumFile& score,
                                             Array<double>& terdis);
      static void  analyzeDissonantInContext(HumdrumFile& score,
                                             Array<int>& dissic,
                                             Array<int>& vertdis, 
                                             Array<int>& tertian, 
                                             Array<double>& terdis, 
                                             Array<int>& accent,
                                             int flag = AFLAG_COMPOUND_METER);
      static void  analyzeDissonantInContext(HumdrumFile& score,
                                             Array<int>& dissic, 
                                             int flag = AFLAG_COMPOUND_METER);
      static void  analyzeDissonantNotes   (HumdrumFile& score, 
                                             Array<ArrayInt>& notediss);
      static void  analyzeDissonantNotes   (HumdrumFile& score,
                                             Array<ArrayInt>& notediss,
                                             Array<int>& vertdis, 
                                             Array<int>& tertian, 
                                             Array<double>& terdis, 
                                             Array<int>& accent, 
                                             Array<int>& dissic);
      static void  analyzeSonorityRelations (HumdrumFile& score, 
                                             Array<int>&sonrel,
                                             Array<int>& vertdis, 
                                             Array<int>& tertian, 
                                             Array<double>& terdis, 
                                             Array<int>& accent, 
                                             Array<int>& dissic,
                                             Array<double>& beatdur, 
                                             Array<ChordQuality>& cq, 
                                             int flag = AFLAG_COMPOUND_METER);
      static void  analyzeSonorityRelations (HumdrumFile& score,
                                             Array<int>&sonrel, 
                                             int flag = AFLAG_COMPOUND_METER);
           
   private:

      // for use with analyzeTertian
      static void  rotateNotes         (Array<int>& notes);
 
      // for use with analyzeDissonantNotes
      static int   measureNoteDissonance(HumdrumFile& score, int line, int note, 
                                       Array<int>& vertdis, Array<int>& accent, 
                                       Array<int>& dissic);

      // for use with analyzeSonorityRelations
      static int   measureChordFunction1(HumdrumFile& score, int line, 
                                       Array<int>& vertdis, Array<int>& tertian, 
                                       Array<double>& terdis, Array<int>& accent,
                                       Array<int>& dissic, Array<double>& beatdur,
                                       Array<ChordQuality>& cq);
      static int   measureChordFunction2(HumdrumFile& score, int line, 
                                       Array<int>& vertdis, Array<int>& tertian, 
                                       Array<double>& terdis, Array<int>& accent, 
                                       Array<int>& dissic, 
                                       Array<ChordQuality>& cq);

};

#endif /* _MAXWELL_H_INCLUDED */



// md5sum: 907896b19322bc5058f12cf37385d37d Maxwell.h [20050403]

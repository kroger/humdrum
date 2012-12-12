//
// Copyright 1998-2010 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jun  8 00:38:46 PDT 1998
// Last Modified: Tue Jun 23 14:00:23 PDT 1998
// Last Modified: Fri May  5 12:17:08 PDT 2000 (added kernToMidiNoteNumber())
// Last Modified: Wed Nov 29 12:28:00 PST 2000 (added base40ToMidiNoteNumber())
// Last Modified: Wed Jan  2 12:19:07 PST 2002 (added kotoToDuration)
// Last Modified: Wed Dec  1 01:36:29 PST 2004 (added base40ToIntervalAbbr())
// Last Modified: Sun Jun  4 21:04:50 PDT 2006 (added base40ToPerfViz())
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Last Modified: Wed Nov 18 16:40:33 PST 2009 (added base40/trans converts)
// Last Modified: Sat May 22 11:02:12 PDT 2010 (added RationalNumber)
// Last Modified: Sun Dec 26 04:54:46 PST 2010 (added kernClefToBaseline)
// Last Modified: Sat Jan 22 17:13:36 PST 2011 (added kernToDurationNoDots)
// Filename:      ...sig/include/sigInfo/Convert.h
// Web Address:   http://sig.sapp.org/include/sigInfo/Convert.h
// Syntax:        C++ 
//
// Description:   This class contains static function that can be used
//                to convert from one data representation to another.
//


#ifndef _CONVERT_H_INCLUDED
#define _CONVERT_H_INCLUDED

#include "HumdrumEnumerations.h"
#include "ChordQuality.h"
#include "SigCollection.h"
#include "RationalNumber.h"


class Convert {
   public: 
 
   // enumeration databases

      static EnumerationEI        exint;
      static EnumerationCQT       chordType;
      static EnumerationCQI       chordInversion;
      static EnumerationCQR       kernPitchClass;
      static EnumerationMPC       musePitchClass;
      static EnumerationInterval  intervalNames;

   // conversions dealing with humdrum data


   // conversions dealing with **kern data

      static int       kernToMidiNoteNumber      (const char* aKernString);
      static char*     durationToKernRhythm      (char* output, double input, 
                                                   int timebase = 1);
      static char*     durationRToKernRhythm     (char* output, 
                                                  RationalNumber input, 
                                                  int timebase = 1);
      static double    kernToDuration            (const char* aKernString);
      static RationalNumber kernToDurationR      (const char* aKernString);
      static double    kernToDurationNoDots      (const char* aKernString);
      static RationalNumber kernToDurationNoDotsR (const char* aKernString);
      static double    kernTimeSignatureTop      (const char* aKernString);
      static double    kernTimeSignatureBottomToDuration   
                                                 (const char* aKernString);
      static int       kernToDiatonicPitch       (const char* buffer);
      static int       kernToDiatonicPitchClass  (const char* buffer);
      static int       kernToDiatonicPitchClassNumeric(const char* buffer);
      static int       kernToDiatonicAlteration  (const char* buffer);
      static int       kernClefToBaseline        (const char* buffer);
      static char*     musePitchToKernPitch      (char* kernOutput, const
                                                    char* museInput);
      static char*     museClefToKernClef        (char* kernOutput, 
                                                    int museInput);
      static int       kernKeyToNumber           (const char* aKernString);
      static const char* keyNumberToKern         (int number);

   // conversions dealing with **qual data

      static ChordQuality chordQualityStringToValue (const char* aString);
      static int       chordQualityToBaseNote    (const ChordQuality& aQuality);
      static void      chordQualityToNoteSet     (SigCollection<int>& noteSet, 
                                                  const ChordQuality& aQuality);
      static int       chordQualityToInversion   (const char* aQuality);
      static int       chordQualityToRoot        (const char* aQuality);
      static int       chordQualityToType        (const char* aQuality);
      static void      noteSetToChordQuality     (ChordQuality& cq, 
                                                  const SigCollection<int>& aSet);

   // conversions dealing with base 40 system of notation

      static char*     base40ToKern               (char* output, int aPitch);
      static char*     base40ToKernTranspose      (char* output, int transpose,
                                                     int keysignature);
      static int       base40ToMidiNoteNumber     (int base40value);
      static int       base40ToAccidental         (int base40value);
      static int       base40IntervalToLineOfFifths(int base40interval);
      static int       base40IntervalToDiatonic   (int base40interval);
      static int       kernToBase40               (const char* kernfield);
      static int       kernToBase40Class          (const char* kernfield);
      static int       kernNoteToBase40           (const char* name);
      static SigCollection<int> keyToScaleDegrees (int aKey, int aMode);
      static int       museToBase40               (const char* pitchString);
      static int       base40ToScoreVPos          (int pitch, int clef);
      static char*     base40ToMuse               (int base40, char* buffer);
      static int       base40ToDiatonic           (int pitch);
      static char*     base40ToIntervalAbbr       (char* output, 
                                                   int base40value);
      static char*     base40ToIntervalAbbr2      (char* output, 
                                                   int base40value);
      static char*     base40ToPerfViz            (char* output, 
                                                   int base40value);
      static char*     base40ToTrans              (char* buffer, int base40);
      static int       transToBase40              (const char* buffer);

   // conversions dealing with MIDI base-12 system 
   
      static char*     base12ToKern               (char* output, int aPitch);
      static char*     base12ToPitch              (char* output, int aPitch);
      static int       base12ToBase40             (int aPitch);
      static int       base7ToBase12              (int aPitch, int alter = 0);
      static int       base7ToBase40              (int aPitch, int alter = 0);

   // conversions from frequency in hertz to MIDI note number
      static int       freq2midi                  (double freq, 
                                                   double a440 = 440.0);

   // conversions dealing with **koto data
      static double    kotoToDuration             (const char* aKotoString);
      static RationalNumber kotoToDurationR       (const char* aKotoString);

   // convsions related to serial interval descriptions
      static const char* base12ToTnSetName        (Array<int>& base12);
      static void      base12ToTnNormalForm       (Array<int>& tnorm, 
                                                   Array<int>& base12);
      static void      base12ToNormalForm         (Array<int>& nform, 
                                                   Array<int>& base12);
      static void      base40ToIntervalVector     (Array<int>& iv,  
                                                      Array<int>& base40);
      static void      base12ToIntervalVector     (Array<int>& iv,  
                                                      Array<int>& base12);

   protected:
      // findBestNormalRotation used with bse12ToNormalForm
      static int     findBestNormalRotation     (Array<int>& input, int asize, 
                                                 Array<int>& choices);

      static int     calculateInversion         (int aType, int bassNote, 
                                                    int root);
      static int     checkChord                 (const SigCollection<int>& aSet);
      static int     intcompare                 (const void* a, const void* b);
      static void    rotatechord                (SigCollection<int>& aChord);


};


#endif /* _CONVERT_H_INCLUDED */



// md5sum: 77d7b4adfff9e35c6cbd857dfa03ad7a Convert.h [20050403]

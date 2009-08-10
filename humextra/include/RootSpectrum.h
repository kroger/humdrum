//
// Copyright 2002 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Dec 24 18:20:23 PST 2002
// Last Modified: Mon Feb 10 17:08:20 PST 2003 (added voice-leading)
// Filename:      ...sig/include/sigInfo/RootSpectrum.h
// Web Address:   http://sig.sapp.org/include/sigInfo/RootSpectrum.h
// Syntax:        C++ 
//
// Description:   Store and calculate chord root calculations.
//

#ifndef _ROOTSPECTRUM_H_INCLUDED
#define _ROOTSPECTRUM_H_INCLUDED

#include "Array.h"
#include "IntervalWeight.h"
#include "HumdrumFile.h"

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif

#define DISPLAY_DIATONIC   0
#define DISPLAY_CHROMATIC  1
#define DISPLAY_BEST       2


class RootSpectrum {
   public: 
                  RootSpectrum   (void);
                 ~RootSpectrum   ();

      int         bestIndex        (void);
      double      getRawScore      (int index);
      double      getInvScore      (int index);
      double      getNormInvScore  (int index);
      int         getSize          (void);
      double&     operator[]       (int index);
      void        getBestOrdering  (Array<int>& bestorder);

      // rhythm parameters
      void        rhythmOn            (void);
      void        rhythmOff           (void);
      void        durationOn          (void);
      void        durationOff         (void);
      void        metricLevelOn       (void);
      void        metricLevelOff      (void);
      void        durationLinear      (void);
      void        durationLog         (void);
      void        meterLinear         (void);
      void        meterLog            (void);
      void        rhythmLinear        (void);
      void        rhythmLog           (void);
      void        setMeterBias        (double aBias);
      void        setDurationBias     (double aBias);
      double      getMeterBias        (void);
      double      getDurationBias     (void);

      void        setDuration         (int aState);
      void        setMetricLevel      (int aState);
      double      getDurationWeight   (void); 
      void        setDurationWeight   (double aWeight); 
      double      getMetricLevelWeight(void); 
      void        setMetricLevelWeight(double aWeight); 
      void        setRhythmWeights    (double durWeight, double levWeight); 

      double      getPower            (void);
      void        setPower            (double aPower);

      // melodic parameters
      void        melodyOn            (void);  
      void        melodyOff           (void);  
      void        setMelodyScaleFactor(double aFactor);
      double      getMelodyScaleFactor(void);
      void        setResolutionFactor (double aFactor);
      double      getResolutionFactor (void);


      // display functions
      ostream&    printMMA         (const char* label, ostream& = cout, 
                                    int functionsQ = 1, int normQ = 0);
      ostream&    printXfig        (ostream& out = cout, 
                                    const char* title = "Root Spectrum",
                                    int type = DISPLAY_DIATONIC);
      ostream&    printPlot        (ostream& out = cout,
                                    int type = DISPLAY_DIATONIC);
      ostream&    printHumdrum     (ostream& out = cout, 
                                    int type = DISPLAY_DIATONIC,
                                    int invQ = 1, int normQ = 0);

      // functions for the calculation of root scores
      int         calculate        (IntervalWeight& distances, 
                                    HumdrumFile& infile, int startline, 
                                    int stopline, int debugQ = 0);
   private:
      Array <double> values;        // scores for each root.
      double         power;         // for scaling inverse scores
      int            durationQ;     // for using duration information
      int            levelQ;        // for using metric level information
      int            melodyQ;       // for estimating non-harmonic tones
      double         durationWeight;// for using duration scaling
      double         levelWeight;   // for using metric level scaling
      double         melodyScaleFactor;  // for use with non-harmonic tones
      int            durlinear;     // for doing linear/log duration scaling
      int            metlinear;     // for doing linear/log metric scaling
      double         durationBias;  // for duration log offset
      double         meterBias;     // for metric log offset
      double         nonresolutionscaling; // for non-harmonic penalty

      static int     minfloat       (const void* a, const void* b);
      double         durationscaling(double duration);
      double         metricscaling  (double level);
      double         getMelodicScaling(int root, int note, 
                                    Array<int>& lastpitches, 
                                    Array<int>& nextpitches, 
                                    IntervalWeight& distances, 
                                    double absbeat, 
                                    double chordstartbeat,  
                                    double chordendbeat, 
                                    double duration);
      int            pedaltone     (int root, int note, double absbeat, 
                                    double chordstartbeat, 
                                    double chordendbeat, double duration);
      int            nonchordtoneQ  (int root, int note);
      int            nonchordtoneQno7th(int root, int note);
};


#endif /* _ROOTSPECTRUM_H_INCLUDED */



// md5sum: caf08905ff85ebb05fbdf2c0f078367b RootSpectrum.h [20001204]

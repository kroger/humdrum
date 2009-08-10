//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jan 16 03:35:40 PST 1999
// Last Modified: Sat Jan 16 03:35:48 PST 1999
// Filename:      ...sig/maint/code/control/Idler/Idler.h
// Web Address:   http://www-ccrma.stanford.edu/~craig/improv/include/Idler.h
// Syntax:        C++
//
// Description:   This class is used to sleep in an event loop for
//                a given amount of time.  There are two types of
//                sleeping that this class will do (1) fixed-duration
//                sleep, where the sleep time is fixed, and (2) a
//                variable-sleep time, where the period between
//                sleep times is fixed (useful for variable duration
//                event loop iterations.  The class is useful in Unix
//                MIDI event loops to allow multiprocessing.
//

#ifndef _IDLER_H_INCLUDED
#define _IDLER_H_INCLUDED

#include "SigTimer.h"


#define SLEEP_MODE_SOFT 0
#define SLEEP_MODE_HARD 1

class Idler {
   public:
                  Idler          (void);
                  Idler          (double aPeriod, 
                                       int aSleepType = SLEEP_MODE_SOFT);
                 ~Idler          ();

      int         has_saturated  (void) const;
      double      getPeriod      (void) const;
      int         getSaturation  (void) const;
      static void millisleep     (double aTime);
      void        reset          (void);
      void        setHardSleep   (double aPeriod = -1);
      void        setPeriod      (double aPeriod);
      void        setSoftSleep   (double aPeriod = -1);
      int         sleep          (void);


   protected:
      SigTimer    timer;         // for HardSleep time adjustment
      int         saturation;    // count of the time saturations
      int         lastSaturated; // for return value of sleep function
      double      sleepPeriod;   // sleep period in milliseconds
      int         sleepMode;     // sleep hard or soft
      double      adjustTimer;   // for hard sleep period determination
      double      lastAdjust;    // for hard sleep period determination
      double      lastTime;      // for hard sleep period determination
      double      currTime;      // for hard sleep period determination
};


#endif  /* _IDLER_H_INCLUDED */



// md5sum: 06187a3bccda585a000481500f965152 Idler.h [20020518]

//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jan 16 03:46:03 PST 1999
// Last Modified: Sat Jan 16 06:33:47 PST 1999
// Filename:      ...sig/maint/code/control/Idler/Idler.h
// Web Address:   http://www-ccrma.stanford.edu/~craig/improv/src/Idler.cpp
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

#include "Idler.h"

#ifndef VISUAL
   #include <unistd.h>
#endif


//////////////////////////////
//
// Idler::Idler --
//

Idler::Idler(void) {
   sleepPeriod = 1.0;  // default of one millisecond sleep period
   sleepMode = SLEEP_MODE_SOFT;
   saturation = -1;
}

Idler::Idler(double aPeriod, int aSleepType) {
   if (aPeriod >= 0.0) {
      sleepPeriod = aPeriod;
   }
   if (aSleepType == SLEEP_MODE_HARD) {
      sleepMode = SLEEP_MODE_HARD;
   } else {
      sleepMode = SLEEP_MODE_SOFT;
   }
   saturation = -1;
}



//////////////////////////////
//
// Idler::~Idler --
//

Idler::~Idler() { 
   // do nothing
}



//////////////////////////////
//
// Idler::has_saturated --
//

int Idler::has_saturated(void) const {
   if (saturation < 0) {
      return saturation;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// Idler::getPeriod --
//

double Idler::getPeriod(void) const {
   return sleepPeriod;
}



//////////////////////////////
//
// Idler::getSaturation --
//

int Idler::getSaturation(void) const {
   if (saturation < 0) {
      return saturation;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// Idler::millisleep -- sleep for specified number of milliseconds
//

void Idler::millisleep(double aTime) {
   #ifndef VISUAL
      // Linux treats any usleep time less than 10 milliseconds
      // as 10 milliseconds due to the way the kernel behaves.
      // For now, I will turn off sleeping if the sleep time is
      // supposed to be less than 1 millisecond.  Note that if
      // you ask for 2 milliseconds of sleep with this function
      // you will be getting about 10 milliseconds of sleep.
      if (aTime >= 1.0) {
         usleep((int)(aTime * 1000.0));
      }
   #else 
      // in Windows 95 you can only do millisecond values of time for now.
      // I can't find a function in Visual C++ to do microseconds, 
      // But here are two ways the help menu mentions (that don't work):
      // NdisMSleep((ulong)(aTime * 1000.0));               // Windows CE
      // Thread.sleep((ulong)aTime, (ulong)(1000.0 * (aTime - (ulong)aTime));  
                                                            // JAVA 
      Sleep((unsigned long)aTime);
   #endif
}



//////////////////////////////
//
// Idler::reset --
//

void Idler::reset(void) {
   saturation = -1;
}



//////////////////////////////
//
// Idler::setHardSleep --
//	default value: aPeriod = -1
//

void Idler::setHardSleep(double aPeriod) {
   setPeriod(aPeriod);
   sleepMode = SLEEP_MODE_HARD;
   reset();
}



//////////////////////////////
//
// Idler::setPeriod --
//

void Idler::setPeriod(double aPeriod) {
   if (aPeriod >= 0) {
      sleepPeriod = aPeriod;
   }
}



//////////////////////////////
//
// Idler::setSoftSleep --
//	default value: aPeriod = -1
//

void Idler::setSoftSleep(double aPeriod) {
   setPeriod(aPeriod);
   sleepMode = SLEEP_MODE_SOFT;
   reset();
}



//////////////////////////////
//
// Idler::sleep -- sleep for sleeptime if SoftSleep.  Otherwise,
//     if HardSleep then try to predict the correct sleep time
//     to return at the correct time again.  Returns false if
//     a time saturation occurred in the last sleep call.
//     Soft sleeping does not generate any saturation.
//

int Idler::sleep(void) {
   if (sleepMode == SLEEP_MODE_SOFT) {
      millisleep(sleepPeriod);
   } else if (saturation != -1) {
      currTime = timer.getTime();
      adjustTimer = (currTime - lastTime) - sleepPeriod;
      if (adjustTimer > 0) {
         saturation++;
         adjustTimer = 0;
         return 0;
      }
      millisleep(sleepPeriod + adjustTimer);
   } else {
      saturation = 0;
      lastTime = timer.getTime();
      millisleep(sleepPeriod);
   }

   if (saturation != lastSaturated) {
      lastSaturated = saturation;
      return 0;
   } else {
      return 1;
   }
}



// md5sum: 1513dc2ad940943d2f40c03a10592f22 Idler.cpp [20050403]

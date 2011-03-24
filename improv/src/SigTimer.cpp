//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Thanks to:     Erik Neuenschwander <erikn@leland.stanford.edu>
//                   for finding Win95 assembly code for Pentium clock cycles.
//                Ozgur Izmirli <ozgur@ccrma.stanford.edu> 
//                   for concept of periodic timer.
// Creation Date: Mon Oct 13 11:34:57 GMT-0800 1997
// Last Modified: Tue Feb 10 21:05:19 GMT-0800 1998
// Last Modified: Sat Sep 19 15:56:48 PDT 1998
// Last Modified: Mon Feb 22 04:44:25 PST 1999
// Last Modified: Sun Nov 28 12:39:39 PST 1999 (added adjustPeriod())
// Last Modofied: Sun Nov 20 01:19:24 PST 2005 (new cpu speed measurement)
// Last Modofied: Tue Jun  9 14:17:28 PDT 2009 (added Apple OSX capability)
// Filename:      .../sig/code/control/SigTimer/SigTimer.cpp
// Web Address:   http://improv.sapp.org/src/SigTimer.cpp
// Syntax:        C++ 
//
// Description:   This class can only be used on Motorola Pentinum 75 Mhz
//                chips or better because the timing information is
//                extracted from the clock cycle count from a register
//                on the CPU itself.  This class will estimate the 
//                speed of the computer, but it would be better if there
//                was a way of finding out the speed from some operating
//                system function.  Apple Macintosh OS X has a reasonable
//                interface to this timer, with a resolution of a nanosecond.
//                (See the measureCpuSpeed() function for OSX below)
//                This class is used primarily for timing of MIDI input 
//                and output at a millisecond resolution, so any resulotion
//                of less than a millisecond would do.
//
//                Using the timing methods in this class with variable
//                speed CPUs (such as in certain laptops which want to
//                save power by adjusting the speed of the CPU based
//                on CPU load) can be a bit problemmatic, since it is 
//                mostly assumed that the CPU speed remains constant.
//                If you want the class to work in those situations, 
//                the cpuspeed variable would have to be measured
//                occasionally, which it is currently measured only
//                once on the first creations of a SigTimer object.
//
// OSX Notes:     http://developer.apple.com/qa/qa2004/qa1398.html
//

#include "SigTimer.h"
#include <stdlib.h>

// define OSXTIMER below if you want nanosecond timer in OSX.
// This might be better method of timing if the computer can change
// CPU speeds, but that case has not been tested.
//#define OSXTIMER 
#ifdef OSXTIMER
   #include <mach/mach_time.h>
#endif

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

// get Sleep or usleep function definition for measureCpu function:

#ifdef VISUAL
   #define WIN32_LEAN_AND_MEAN
   #include <windows.h>             
   #undef WIN32_LEAN_AND_MEAN
#else
   #include <unistd.h>
   #include <time.h>
   #include <sys/timeb.h>
#endif

// declare static variables
int64bits SigTimer::globalOffset = 0;
int64bits SigTimer::cpuSpeed     = 0;      // in cycles per second


//////////////////////////////
//
// SigTimer::SigTimer -- Initialization of SigTimer objects
//

SigTimer::SigTimer(void) {
   if (globalOffset == 0) {             // coordinate offset between timers
      globalOffset = clockCycles();
   }
   if (cpuSpeed <= 0) {                 // initialize CPU speed value
      cpuSpeed = measureCpuSpeed(1);
      if (cpuSpeed <= 0) {
         cpuSpeed = 1000000000;
      }
   }
      
   offset = globalOffset;               // initialize the start time of timer
   ticksPerSecond = 1000;               // default of 1000 ticks per second
   period = 1000.0;                     // default period of once per second
}


SigTimer::SigTimer(int aSpeed) {
   if (globalOffset == 0) {
      globalOffset = clockCycles();
   }
   cpuSpeed = aSpeed;

   offset = globalOffset;
   ticksPerSecond = 1000;
   period = 1000.0;                     // default period of once per second
}


SigTimer::SigTimer(SigTimer& aTimer) {
   offset = aTimer.offset;
   ticksPerSecond = aTimer.ticksPerSecond;
   period = aTimer.period;
}



//////////////////////////////
//
// SigTimer::~SigTimer -- destructor for a SigTimer object.
//

SigTimer::~SigTimer() {
   // do nothing
}



//////////////////////////////
//
// SigTimer::adjustPeriod -- adjust the period of the timer.
//

void SigTimer::adjustPeriod(double periodDelta) {
   offset -= (int64bits)(getCpuSpeed() * getPeriod() * periodDelta / 
         getTicksPerSecond());
}



//////////////////////////////
//
// SigTimer::clockCycles -- returns the number of clock cycles since last reboot
//	   HARDWARE DEPENDENT -- currently for Pentiums only.
//     static function.
//

int64bits SigTimer::clockCycles() {

#ifdef OSXTIMER
   int64bits output = mach_absolute_time();
#elif defined(VISUAL)
   int64bits output;
   unsigned long high_end, low_end;
   __asm {
      __asm _emit 0x0f __asm _emit 0x31
      mov high_end, edx
      mov low_end, eax
   }
   output = high_end;
   output = output << 32;
   output += low_end;
#else /* for Linux or probably any unix in Intel CPUs */
   int64bits output;

   // For Pentiums, you can get the number of clock cycles elapsed 
   // since the last reboot with the following assembler code:
   __asm__ volatile (".byte 0x0f, 0x31" : "=A" (output));
#endif

   return output;
}



//////////////////////////////
//
// SigTimer::expired -- returns the integral number of periods
//    That have passed since the last update or reset.
//    See getPeriodCount which returns a floating point
//    count of the period position.
//

int SigTimer::expired(void) const {
   return (int)(getTime()/period);
}



//////////////////////////////
//
// SigTimer::getCpuSpeed -- returns the CPU speed of the computer.
//   (static function)
//

int64bits SigTimer::getCpuSpeed(void) {
   return cpuSpeed;
}



//////////////////////////////
//
// SigTimer::getPeriod -- returns the timing period of the timer,
//    if the timer is being used as a periodic timer.
//

double SigTimer::getPeriod(void) const {
   return period;
}



//////////////////////////////
//
// SigTimer::getPeriodCount -- returns the current period count
//    of the timer as a double.  Similar in behavior to expired
//    function but lists the current fraction of a period.
//

double SigTimer::getPeriodCount(void) const {
   return (double)getTime()/period;
}



//////////////////////////////
//
// SigTimer::getTempo -- returns the current tempo in terms
//     of beats (ticks) per minute.
//

double SigTimer::getTempo(void) const {
   return getTicksPerSecond() * 60.0 / getPeriod();
}



//////////////////////////////
//
// SigTimer::getTicksPerSecond -- return the number of ticks per
//     second.
//

int SigTimer::getTicksPerSecond(void) const {
   return ticksPerSecond;
}



//////////////////////////////
//
// SigTimer::getTime -- returns time in milliseconds by default.  
// 	time can be altered to be in some other unit of time
//	by using the setTicksPerSecond function.
//	(Default is 1000 ticks per second.)
//

int SigTimer::getTime(void) const {
   return (int)((clockCycles()-offset)/getFactor());
}
   


//////////////////////////////
//
// SigTimer::getTimeInSeconds 
//

double SigTimer::getTimeInSeconds(void) const {
   return ((clockCycles()-offset)/(double)cpuSpeed);
}
   


//////////////////////////////
//
// SigTimer::getTimeInTicks 
//

int SigTimer::getTimeInTicks(void) const {
   return (int)((clockCycles()-offset)/getFactor());
}
   


//////////////////////////////
//
// SigTimer::measureCpuSpeed -- returns the number of clock cycles in 
//	one second.  Accuracy to about +/- 5%.
//     default value: quantize = 0.
//

/*  Old code for measuring cpu speed.  Keep for historical comparison
 *  with new version which works better.
 */

/*
int SigTimer::measureCpuSpeed(int quantize) {
   int64bits a, b;
   a = clockCycles();
   #ifdef VISUAL
      Sleep(1000/4);
   #else
      usleep(1000000/4);
   #endif
   b = clockCycles();
   
   int output = (int)(4*(b-a-140100-450000));

   if (quantize) {
      // quantize to a known computer CPU speed
      if (output < 78000000) {
         output = 75000000;
      } else if (output < 95000000) {
         output = 90000000;
      } else if (output < 110000000) {
         output = 100000000;
      } else if (output < 140000000) {
         output = 133000000;
      } else if (output < 155000000) {
         output = 150000000;
      } else if (output < 180000000) {
         output = 166000000;
      } else if (output < 215000000) {
         output = 200000000;
      } else if (output < 250000000) {
         output = 233000000;
      } else if (output < 280000000) {
         output = 266000000;
      } else if (output < 325000000) {
         output = 300000000;
      } else if (output < 375000000) {
         output = 350000000;
      } else if (output < 425000000) {
         output = 400000000;
      } else {
         output = output;
      }
   } // end if quantize

   return output;
}

*/


/* New measureCpuSpeed function which is accurate to within 1%, and
 * usually a little better.
 */


//////////////////////////////
//
// measureCpuSpeed -- 
//

#ifdef OSXTIMER

// in OS X, there is a timer called mach_absolute_time
// which in theory is a clock cycle counter.  However, it
// is actually a nanosecond timer in implementation.  If
// A computer is slower than 1 GHz, the increments to the
// mach_absolute_time() will be quantized to a value larger
// than 1.  If the computer is faster than 1 GHz, the
// mach_absolute_time() function will round the clock cycles
// to the nearest nanosecond with no subdivisions of the
// nanosecond.  In any case, this class is not really about
// determining the speed of the CPU, but rather in giving
// a fine-granulation timer.  So for Macintosh OS X, spoof
// the CPU speed based on the time units of the clock cycle
// which are extracted from data provied by mach_timebase_info().
// For the most part, this class will declare any OS X computer
// to be running at 1 GHz, but this really means that the clock
// which the class is using is running at 1 GHz, not necessarily
// the CPU bus speed.
//
// OS X on Intel CPU can access the intel clock cycle counter
// with the same code as is used in Linux, so this version of
// measureCpuSpeed is only used if OSXTIMER is defined; otherwise,
// the same method as used in Linux and Windows will be used which
// is the second definition of measureCpuSpeed below.
//

int64bits SigTimer::measureCpuSpeed(int quantize) {
   mach_timebase_info_data_t tb;
   mach_timebase_info(&tb);
   return (int64bits)((double)tb.numer/tb.denom * 1e9 + 0.5);
}

#else /* measureCpuSpeed for Windows, Linux, and OSX on Intel CPUs */

int64bits SigTimer::measureCpuSpeed(int quantize) {
   // quantize input variable is now ignored
   int64bits cycles1, cycles2, t1, t2;

   double measures[40];
 
   int i;
   double sum = 0.0;
   for (i=0; i<40; i++) {
      getClockBoundary(cycles1, t1);
      getClockBoundary(cycles2, t2);
      measures[i] = (double)(cycles2 - cycles1) / ((t2 - t1) / 1000.0);
      sum += measures[i];
   }

   double average = sum / 40.0;

   // cout << "Sample measurement = " << measures[0] << endl;

   int count = 0;
   double newsum = 0.0;
   for (i=0; i<40; i++) {
      if ((measures[i] < average + average * 0.10) &&
           (measures[i] > average - average * 0.10)) {
         count++;
         newsum += measures[i];
      }
   }

   double newaverage;

   if (count > 5) {
      newaverage = newsum / count;
   } else {
      newaverage = average;
   }

   int newcount = 0;
   double newnewsum = 0.0;
   for (i=0; i<40; i++) {
      if ((measures[i] < newaverage + newaverage * 0.05) &&
           (measures[i] > newaverage - newaverage * 0.05)) {
         newcount++;
         newnewsum += measures[i];
      }
   }

   if (newcount > 5) {
      return int64bits(newnewsum / newcount + 0.5);
   } else {
      return int64bits(newaverage+0.5);
   }
}

#endif  /* measureCpuSpeed */



//////////////////////////////
//
// SigTimer::reset -- set the timer to 0.
//

void SigTimer::reset(void) {
   offset = clockCycles();
}



//////////////////////////////
//
// SigTimer::setCpuSpeed -- You can also vary the CPU speed here to cause the 
//	getTime function to output different units of time, but the 
//	setTicksPerSecond is a more appropriate place to do such a thing. 
//

void SigTimer::setCpuSpeed(int64bits aSpeed) {
   if (aSpeed <= 0) {
      cerr << "Error: Cannot set the cpu speed to be negative: " 
           << (double) aSpeed << endl;
      exit(1);
   }
   cpuSpeed = aSpeed;
}



//////////////////////////////
//
// SigTimer::setPeriod -- sets the period length of the timer.
//    input value cannot be less than 1.0.
//

void SigTimer::setPeriod(double aPeriod) {
   if (aPeriod < 1.0) {
      cerr << "Error: period too small: " << aPeriod << endl;
      exit(1);
   }
   period = aPeriod;
}



//////////////////////////////
//
// SigTimer::setPeriodCount -- adjusts the offset time according
//     to the current period to match the specified period count.
//

void SigTimer::setPeriodCount(double aCount) {
   offset = (int64bits)(clockCycles() - aCount * getPeriod() *
         getCpuSpeed() / getTicksPerSecond());
}



//////////////////////////////
//
// SigTimer::setTempo -- sets the period length in terms of 
//	beats per minute.
//

void SigTimer::setTempo(double beatsPerMinute) {
   if (beatsPerMinute < 1.0) {
      cerr << "Error: tempo is too slow: " << beatsPerMinute << endl;
      exit(1);
   }
   double count = getPeriodCount();
   period = getTicksPerSecond() * 60.0 / beatsPerMinute;
   setPeriodCount(count);
}



//////////////////////////////
//
// SigTimer::setTicksPerSecond 
//

void SigTimer::setTicksPerSecond(int aTickRate) {
   if (aTickRate <= 0) {
      cerr << "Error: Cannot set the tick rate to be negative: " 
           << aTickRate << endl;
      exit(1);
   }
   ticksPerSecond = aTickRate;
}



//////////////////////////////
//
// SigTimer::start 
//

void SigTimer::start(void) {
   reset();
}



//////////////////////////////
//
// SigTimer::sync 
//

void SigTimer::sync(SigTimer& aTimer) {
   offset = aTimer.offset;
}



//////////////////////////////
//
// SigTimer::update -- set the timer start to the next period.
//

void SigTimer::update(void) {
   if (getTime() >= getPeriod()) {
      offset += (int64bits)(getPeriod() * getFactor());
   }
}

// update(int) will do nothing if the periodCount is greater than
// than the expired period count.

void SigTimer::update(int periodCount) {
   if (periodCount > expired()) {
      return;
   } else {
      offset += (int64bits)(getPeriod() * getFactor() * periodCount);
   }
}


///////////////////////////////////////////////////////////////////////////
//
// protected functions:
//


//////////////////////////////
//
// SigTimer::getFactor -- 
//

double SigTimer::getFactor(void) const {
   return (double)((double)getCpuSpeed()/(double)getTicksPerSecond());
}



//////////////////////////////
//
// SigTimer::getClockBoundary -- Used by measureCpuSpeed to 
//   get a better estimate of the actual CPU speed.
//

/* This code is probably no longer needed and should be deleted.

   #ifdef VISUAL
     #include <time.h>
     #include <sys/timeb.h>
   #endif

void getClockBoundary(int64bits& cycles, int64bits& millisec) {
   time_t seconds;
   int    ms, startms;
   struct timeb tstruct;
 
   ftime(&tstruct);
   startms = tstruct.millitm;

   if (startms > 950) {
      millisleep(100);
      ftime(&tstruct);
      startms = tstruct.millitm;
   }

   ms = startms;

   while (ms == startms) {
      ftime(&tstruct);
      ms = tstruct.millitm;
   }

   cycles = SigTimer::clockCycles();

   time(&seconds);
   millisec = (int64bits)seconds * 1000 + ms;

}

*/

#ifdef VISUAL
   #include <time.h>
   #include <sys/timeb.h>
#endif

void SigTimer::getClockBoundary(int64bits& cycles, int64bits& millisec) {
   time_t seconds;
   int    ms, startms;
   struct timeb tstruct;
 
   ftime(&tstruct);
   startms = tstruct.millitm;

   if (startms > 950) {
      millisleep(100);
      ftime(&tstruct);
      startms = tstruct.millitm;
   }

   ms = startms;

   while (ms == startms) {
      ftime(&tstruct);
      ms = tstruct.millitm;
   }

   cycles = SigTimer::clockCycles();

   time(&seconds);
   millisec = (int64bits)seconds * 1000 + ms;
}



///////////////////////////////////////////////////////////////////////////
//
// Miscellaneous global timing functions are located here (used in the
//  Improv environments):
//

void millisleep(int milliseconds) {
   #ifdef VISUAL
      Sleep(milliseconds);
   #else
      usleep( milliseconds * 1000);
   #endif
}
      

void millisleep(float milliseconds) {
   #ifdef VISUAL
      // cannot convert to microseconds in Visual C++ yet.
      // Tell me how and I'll fix the following line
      Sleep((unsigned long)milliseconds);
   #else
      usleep((int)(milliseconds * 1000.0));
   #endif
}
      

// md5sum: e962dda5001d3454650ee32c65457470 SigTimer.cpp [20030102]

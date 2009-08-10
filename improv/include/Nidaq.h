//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Oct 26 15:01:37 PST 1998
// Last Modified: Mon Oct 26 15:01:51 PST 1998
// Last Modified: Tue Oct 13 12:48:57 PDT 1998
// Filename:      ...sig/maint/code/control/Nidaq/Nidaq.h
// Web Address:   http://www-ccrma.stanford.edu/~craig/improv/include/Nidaq.h
// Syntax:        C++; LabWindows/CVI libraries
//
// Description:   This class can handle continuous sampling from the NIDAQ.
//                Will work with up to 16 channels possible on the card.
//                There is probably a driver bug in that the multiple channels
//                must be consecutive starting at channel 0.
// 

#ifndef _NIDAQ_H_INCLUDED
#define _NIDAQ_H_INCLUDED

#ifdef VISUAL 
#ifdef NIDAQ 

#include "SigTimer.h"
#include "sigCVI.h"

typedef unsigned short ushort;
typedef unsigned long ulong;

class Nidaq {
   public:
                  Nidaq              (void);
                 ~Nidaq              ();

      void        initialize         (char** argv);

  	   void        activateAllChannels(void);
      int         aquireData         (void);
      int         checkPoll          (void);
      const char* getChannelString   (void) const;
      int         getBufferSize      (void) const;
      int         getChannelState    (int index) const;
      double      getDefaultMaxVoltage (void) const;
      double      getDefaultMinVoltage (void) const;
      int         getDevice          (void) const;
      int         getFrameCount      (void) const;
      int         getFrameSize       (void) const;
      int         getMode            (void) const;
      double      getPollPeriod      (void) const;
      double      getSrate           (void) const;
      int         is_running         (void) const;
      double*     operator[]         (int index);
      void        selectChannel      (int aChannel);
      void        setFrameSize       (int aSize);
      void        setDevice          (int aDevice);
      void        setModeLatest      (void);
      void        setModeContinuous  (void);
      void        setMode            (int aMode);
      void        setFrameCount      (int aCount);
      void        setPollPeriod      (double aPeriod);
      void        setScaleRange      (int aChannnel, double min, double max);
      void        setSrate           (double anSrate);
      void        start              (void);
      void        stop               (void);
      void        unselectChannel    (int aChannel);


   protected:
      short    device;              // number of the DAQ board (from 1 to 8)
      double   targetSampleRate;    // desired sampling rate
      double   actualSampleRate;    // actual sampling rate on NIDAQ
      ulong    taskID;              // for "Easy I/O" task descriptor
      short    error;               // for getting error flag from NIDAQ func
      int      samplingState;       // 0 = not sampling, 1 = sampling
      int      channelState[16];    // boolean for if channel is on or off
      double   minvolt[16];         // minimum value of voltage samples
      double   maxvolt[16];         // minimum value of voltage samples
      double   defaultMaxVoltage;   // for AIStartAcquisition
      double   defaultMinVoltage;   // for AIStartAcquisiton
      char     channelString[1024]; // for AIStartAcquisition
      int      channelIndex[16];    // for coordination with string
      int      numRunningChannels;  // number of active channels
      int      indexmap[16];        // 
      int      grouping;            // GROUP_BY_CHANNEL, GROUP_BY_SCAN
      int      tempFrameSize;       // size of the current frame
      int      frameSize;           // size of the current frame
      double  *frameData;           // frame data
      int      frameCount;          // count of the current frame
      SigTimer pollTimer;           // for period to check for data
      ulong    sampleCount;         // number of samples in transfer frame
      int      mode;                // LATEST_MODE, CONTINUOUS_MODE
      int      internalBufferSize;  // for NIDAQ circular buffer size


      int      createChannelString  (char* string, int* chanStates, 
                                     double* min, double* max);
      void     verifyChannelIndex   (int anIndex) const;
}; 


#endif  /* NIDAQ */
#endif  /* VISUAL */

#endif  /* _NIDAQ_H_INCLUDED */


// md5sum: 7e9078ab6b52f46a34bab9259fd72f3f Nidaq.h [20020518]

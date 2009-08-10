//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul 16 13:39:10 PDT 2000
// Last Modified: Mon Jul 17 11:10:46 PDT 2000
// Filename:      ...sig/code/control/AdamsStick/AdamsStick.h
// Web Address:   http://sig.sapp.org/include/sig/AdamsStick.h
// Syntax:        C++
//
// Description:   A structure for handling the Interval stick variables
//                and the MIDI connections from the computer to 
//                the stick.
//

#ifndef _ADAMSSTICK_H_INCLUDED
#define _ADAMSSTICK_H_INCLUDED

#include "MidiIO.h"              /* Inheritance of MIDI in/out class funcs */
#include "MidiMessage.h"         /* for processing incoming MIDI messages  */
#include "CircularBuffer.h"      /* for storage of state variables         */
#include "SigTimer.h"            /* for the poll timer functions           */

#define STICK_POLL_MODE    0
#define STICK_STREAM_MODE  1
#define STICK_DEFAULT_POLL_PERIOD  50

// The define below is for the size of the state variable storage buffers.
// By default, all buffers will be the size given below.  You can later
// change each buffer size independently, 
#define STICK_DEFAULT_STATE_SIZE (100) /* Num of previous positions to store  */


class AdamsStick : public MidiIO {
   public:
                 AdamsStick                 (void);
                 AdamsStick                 (int outputDevice, int inputDevice);
                ~AdamsStick                 ();

      int        checkPoll                  (void);
      int        getLevel                   (int fsrnumber);
      double     getPollPeriod              (void);
      int        getMode                    (void);
      int        getState                   (int fsrnumber);
      int        getStateSize               (void);
      int        getThreshold               (int fsrnumber);
      int        getVersion                 (void);
      int        is_connected               (void);
      void       poll                       (void);
      void       processIncomingMessages    (void);
      void       setLevel                   (int fsrnumber, int aValue);
      void       setLevel                   (int aValue);
      void       setMode                    (int aMode);
      void       setPollPeriod              (double aPeriod);
      void       setPollMode                (void);
      void       setStreamMode              (void);
      void       setStateSize               (int aSize);
      void       setThreshold               (int fsrnumber, int aValue);
      void       setThreshold               (int aValue);
      int        toggleMode                 (void);

      // public state variables

      // this function will be called when one set of values has
      // been reported by the stick.
      void       (*response)                (void);
      void       (*fsr1ontrig)              (void);
      void       (*fsr1offtrig)             (void);
      void       (*fsr2ontrig)              (void);
      void       (*fsr2offtrig)             (void);
      void       (*fsr3ontrig)              (void);
      void       (*fsr3offtrig)             (void);

      // Instantaneous position/force variables.  These variables get
      // updated as soon as the information is received from the stick.

      long  t1s;                   // FSR 1 position reporting time buffer
      short s1p;                   // FSR 1 position buffer
      short s1f;                   // FSR 1 force buffer
      short loc1;                  // FSR 1 last known position
      long  loc1t;                 // FSR 1 last known position time

      long  t2s;                   // FSR 2 position reporting time buffer
      short s2p;                   // FSR 2 position buffer
      short s2f;                   // FSR 2 force buffer
      short loc2;                  // FSR 2 last known position
      long  loc2t;                 // FSR 2 last known position time

      long  t3s;                   // FSR 3 position reporting time buffer
      short s3p;                   // FSR 3 position buffer
      short s3f;                   // FSR 3 force buffer
      short loc3;                  // FSR 3 last known position
      long  loc3t;                 // FSR 3 last known position time

      // storage buffers for old data.  These buffers can be used as
      // a circular buffer from which you can process data one by one
      // without ever losing/skipping over incoming data.  Use the 
      // buffer.extract() function to extract data, and buffer.getCount() 
      // to see how many elements can be extracted.

      CircularBuffer<long>  t1sb;  // FSR 1 position reporting time buffer
      CircularBuffer<short> s1pb;  // FSR 1 position buffer
      CircularBuffer<short> s1fb;  // FSR 1 force buffer

      CircularBuffer<long>  t2sb;  // FSR 2 position reporting time
      CircularBuffer<short> s2pb;  // FSR 2 position
      CircularBuffer<short> s2fb;  // FSR 2 force

      CircularBuffer<long>  t3sb;  // FSR 3 position reporting time
      CircularBuffer<short> s3pb;  // FSR 3 position
      CircularBuffer<short> s3fb;  // FSR 3 force

      // scaling functions: change the range of data from 0..2^15-1 
      // to another range

      int s1ps(int min, int max);
      int s1fs(int min, int max);
      int s2ps(int min, int max);
      int s2fs(int min, int max);
      int s3ps(int min, int max);
      int s3fs(int min, int max);

      // scale values to 7 bit numbers:

      int s1p7(void);
      int s1f7(void);
      int loc1_7(void);
      int s2p7(void);
      int s2f7(void);
      int loc2_7(void);
      int s3p7(void);
      int s3f7(void);
      int loc3_7(void);


      double s1ps(double min, double max);
      double s1fs(double min, double max);
      double s2ps(double min, double max);
      double s2fs(double min, double max);
      double s3ps(double min, double max);
      double s3fs(double min, double max);

      // changing functions: returns the amount of change between
      // successive values of the input data.

      int s1pc(int min, int max);
      int s1fc(int min, int max);
      int s2pc(int min, int max);
      int s2fc(int min, int max);
      int s3pc(int min, int max);
      int s3fc(int min, int max);

      double      fscale14x             (int value, double min, double max);
      int         scale14x              (int value, int min, int max);


   private: 

      int         currentMode;          // 0 = poll mode, 1 = stream mode
      int         connectedQ;           // 0 = not connected, 1 = connected
      int         versionInfo;          // -1 = unknown version
      SigTimer    pollTimer;

      void        interpretCommand      (MidiMessage aMessage);
      void        sendVersionMessage    (void);
      void        sendStreamingMessage  (void);
      void        sendPollingMessage    (void);
      void        sendPollMessage       (void);

      int         convertTo14bits       (unsigned char msb, unsigned char lsb);

      // triggering variables

      void        determineTriggers     (void);

      int trigger1level;    // pressure level which causes a trigger on FSR1
      int trigger2level;    // pressure level which causes a trigger on FSR2
      int trigger3level;    // pressure level which causes a trigger on FSR3

      int trigger1delta;    // histeresis threshold value
      int trigger2delta;    // histeresis threshold value
      int trigger3delta;    // histeresis threshold value

      int state1;           // 0 = FSR1 is being not pressed, 1 = pressed
      int state2;           // 0 = FSR2 is being not pressed, 1 = pressed
      int state3;           // 0 = FSR3 is being not pressed, 1 = pressed

      int state1on;         // 0 = onstate is not allowed
      int state2on;         // 1 = onstate is allowed
      int state3on;

      int state1off;        // 0 = offstate is not allowed
      int state2off;        // 1 = offstate is allowed
      int state3off;

};



//////////////////////////////
//
// AdamsStickEmptyBehavior -- the following function will be the default 
//     function for the radio baton behavior function pointers. 
//

void AdamsStickEmptyBehavior(void);


#endif  /* _ADAMSSTICK_H_INCLUDED */



// md5sum: f73a0fd2ca8673efd0bfa1d54c36f7bb AdamsStick.h [20020518]

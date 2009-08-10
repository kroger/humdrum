//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 21 December 1997
// Last Modified: Sun Feb 15 18:44:35 GMT-0800 1998
// Last Modified: Wed Sep 30 14:26:16 PDT 1998
// Last Modified: Mon Mar 15 03:53:06 PST 1999
// Last Modified: Mon Nov 29 13:44:52 PST 1999 (name RadioDrum->RadioBaton)
// Last Modified: Wed Apr 19 16:02:27 PDT 2000 (added axis reverse options)
// Last Modified: Thu Apr 20 16:27:05 PDT 2000 (added scaling functions)
// Last Modified: Sun Oct  1 15:19:13 PDT 2000 (revised for firmware "AE")
// Filename:      ...sig/code/control/RadioBaton/RadioBaton.h
// Web Address:   http://sig.sapp.org/include/sig/RadioBaton.h
// Syntax:        C++
//
// Description:   A structure for handling the Radio Baton variables
//                and the MIDI connections from the computer to 
//                the Radio Batons.
//

#ifndef _RADIOBATON_H_INCLUDED
#define _RADIOBATON_H_INCLUDED

#include "batonprotocol.h"       /* October 2000 communication protocol    */
#include "CircularBuffer.h"      /* for storage of state variables         */
#include "MidiMessage.h"         /* for MIDI input from the drivers        */
#include "MidiIO.h"              /* Inheritance of MIDI in/out class funcs */


// The define below is for the size of the state variable storage buffers.
// By default, all buffers will be the size given below.  You can later
// change each buffer size independently, 


#define DEFAULT_STATE_SIZE (100) /* Number of previous positions to store  */


// The following defines are for RadioBaton private functions:
#define DATA_X             (0)
#define DATA_Y             (1)
#define DATA_Z             (2)
#define DATA_W             (2)


// Following defines are for labeling baton variables when recording to a file:
#define TRIGGER1RECORD     "t1"   /* trigger value for stick 1     */
#define TRIGGER2RECORD     "t2"   /* trigger value for stick 2     */
#define POSITION1RECORD    "p1"   /* position value for stick 1    */
#define POSITION2RECORD    "p2"   /* position value for stick 2    */
#define DIAL1RECORD        "d1"   /* position value for dial 1     */
#define DIAL2RECORD        "d2"   /* position value for dial 2     */
#define DIAL3RECORD        "d3"   /* position value for dial 3     */
#define DIAL4RECORD        "d4"   /* position value for dial 4     */
#define FOOTPEDAL1RECORD   "f1"   /* b14- pedal                    */
#define FOOTPEDAL2RECORD   "f2"   /* b15- pedal                    */
#define BUTTON1RECORD      "b1"   /* b14+ button                   */
#define BUTTON2RECORD      "b2"   /* b15+ button                   */

// recording of raw antenna/potentiometer data:
#define ANTENNA0RECORD     "a0"   /* stick 1 left ant 0 strength (buf[0])    */
#define ANTENNA1RECORD     "a1"   /* stick 1 right ant 1 strength (buf[1])   */
#define ANTENNA2RECORD     "a2"   /* stick 1 near ant 2 strength (buf[2])    */
#define ANTENNA3RECORD     "a3"   /* stick 1 far ant 3 strength (buf[3])     */
#define ANTENNA4RECORD     "a4"   /* stick 1 center ant 4 strength (buf[4])  */
#define ANTENNA5RECORD     "a5"   /* stick 2 left 0 strength (buf[6])        */
#define ANTENNA6RECORD     "a6"   /* stick 2 right 1 strength (buf[7])       */
#define ANTENNA7RECORD     "a7"   /* stick 2 near 2 strength (buf[8])        */
#define ANTENNA8RECORD     "a8"   /* stick 2 far 3 strength (buf[9])         */
#define ANTENNA9RECORD     "a9"   /* stick 2 center 4 strength (buf[10])     */
#define POT1RECORD         "e1"   /* dial 1 potentiometer strength (buf[11]) */
#define POT2RECORD         "e2"   /* dial 2 potentiometer strength (buf[12]) */
#define POT3RECORD         "e3"   /* dial 3 potentiometer strength (buf[13]) */
#define POT4RECORD         "e4"   /* dial 4 potentiometer strength (buf[5])  */
#define B14RECORD          "c1"   /* B14+ button state record                */
#define B15RECORD          "c2"   /* B15+ button state record                */


class RadioBaton : public MidiIO {
   public:
                  RadioBaton                (void);
                  RadioBaton                (int outputDevice, int inputDevice);
                 ~RadioBaton                ();


      // general functions affecting object (No MIDI communication with drum):
      int         getBuf                   (int index);
      int         getError                 (void) const;
      int         getPositionReporting     (void) const;
      int         getReportStatus          (void) const;
      long        getWhack1Time            (void) const;
      long        getWhack2Time            (void) const;
      int         getXaxisDirection        (void) const;
      int         getYaxisDirection        (void) const;
      int         getZaxisDirection        (void) const;
      void        processIncomingMessages  (void);
//    void        requestBufData           (void);
      void        sendMessage              (int aMessage);
      void        setError                 (int errorState);
      int         recordingQ               (void) const;
      void        recordStateStart         (const char* aFilename);
      void        recordStateStop          (void);
      void        setReportStatus          (int aStatus);
      void        setStateSize             (int aSize);
      void        setXaxisDirection        (int aDirection);
      void        setYaxisDirection        (int aDirection);
      void        setZaxisDirection        (int aDirection);
      int         toggleXAxisDirection     (void);
      int         toggleYAxisDirection     (void);
      int         toggleZAxisDirection     (void);
      void        toggleError              (void);

      // commands which send/receive MIDI messages to/from the radio drum
      int         positionReportingOff     (void);
      int         positionReportingOn      (void);
      void        togglePositionReporting  (void);
               
      // baton behavior functions: These functions are to be defined by the 
      // user, or creator, or a batonImprov or similar program.
      // These functions are called by a radio baton object when
      // a certain event such as a trigger or position update has occurred.
      void (*stick1trig)(void);
      void (*stick2trig)(void);
      void (*stick1position)(void);
      void (*stick2position)(void);
      void (*b14plustrig)(void);
      void (*b15plustrig)(void);
      void (*b14minusuptrig)(void);
      void (*b14minusdowntrig)(void);
      void (*b15minusuptrig)(void);
      void (*b15minusdowntrig)(void);

      void (*dial1position)(void);
      void (*dial2position)(void);
      void (*dial3position)(void);
      void (*dial4position)(void);
   

   // public state variables

      // Instantaneous position/trigger variables.  These variables get
      // updated as soon as the information is received from the Radio 
      // Batons.  There will be times when the new data and old data for
      // baton position x, y, & z values might be from different position
      // reportings; for example, a new x-value could overwrite an old
      // x-value while the old z-value is still written.  The storage
      // buffers for the position data will always contain a complete
      // set for a given index.

      long  t1p;                    // baton 1 position reporting time
      ushort x1p;                   // baton 1 x-axis position
      ushort y1p;                   // baton 1 y-axis position
      ushort z1p;                   // baton 1 z-axis position

      long  t2p;                    // baton 2 position reporting time
      ushort x2p;                   // baton 2 x-axis position
      ushort y2p;                   // baton 2 y-axis position
      ushort z2p;                   // baton 2 z-axis position

      ushort d1p;                   // dial 1 position
      ushort d2p;                   // dial 2 position
      ushort d3p;                   // dial 3 position
      ushort d4p;                   // dial 4 position

      // trigger variables

      long  t1t;                    // baton 1 position reporting time
      ushort x1t;                   // baton 1 x-axis position
      ushort y1t;                   // baton 1 y-axis position
      ushort w1t;                   // baton 1 z-axis position

      long  t2t;                    // baton 2 position reporting time
      ushort x2t;                   // baton 2 x-axis position
      ushort y2t;                   // baton 2 y-axis position
      ushort w2t;                   // baton 2 z-axis position

      long b14pt;                   // b14+ button trigger time
      long b15pt;                   // b15+ button trigger time
      long b14mdt;                  // b14- pedal down trigger time
      long b14mut;                  // b14- pedal up trigger time
      long b15mdt;                  // b15- pedal down trigger time
      long b15mut;                  // b15- pedal up trigger time

      // scaling functions: change the range of data from 0-127 to
      // another range
 
      int x1ps(int min, int max);
      int y1ps(int min, int max);
      int z1ps(int min, int max);
      int x2ps(int min, int max);
      int y2ps(int min, int max);
      int z2ps(int min, int max);
      int x1ts(int min, int max);
      int y1ts(int min, int max);
      int x2ts(int min, int max);
      int y2ts(int min, int max);
      int d1ps(int min, int max);
      int d2ps(int min, int max);
      int d3ps(int min, int max);
      int d4ps(int min, int max);
 
      double x1psf(double min, double max);
      double y1psf(double min, double max);
      double z1psf(double min, double max);
      double x2psf(double min, double max);
      double y2psf(double min, double max);
      double z2psf(double min, double max);
      double x1tsf(double min, double max);
      double y1tsf(double min, double max);
      double x2tsf(double min, double max);
      double y2tsf(double min, double max);
      double d1psf(double min, double max);
      double d2psf(double min, double max);
      double d3psf(double min, double max);
      double d4psf(double min, double max);

      // changing functions: returns the amount of change between
      // successive values of the input data.

      int x1pc(int min, int max);
      int y1pc(int min, int max);
      int z1pc(int min, int max);
      int x2pc(int min, int max);
      int y2pc(int min, int max);
      int z2pc(int min, int max);
      int x1tc(int min, int max);
      int y1tc(int min, int max);
      int x2tc(int min, int max);
      int y2tc(int min, int max);
      int d1pc(int min, int max);
      int d2pc(int min, int max);
      int d3pc(int min, int max);
      int d4pc(int min, int max);


      // storage buffers for old data.  These buffers can be used as
      // a circular buffer from which you can process data one by one
      // without ever losing/skipping over incoming data.  Use the 
      // buffer.extract() function to extract data, and buffer.getCount() 
      // to see how many elements can be extracted.
      // If you are using the storage buffers, you should set the ones 
      // you are using to the same size if using the buffer.extract() command;
      // otherwise after overflowing the buffers, the data sets (e.g.,
      // baton 1 trigger values) will be out of index alignment.

      CircularBuffer<long>   t1pb;   // stick1 time of position (stores t1p)
      CircularBuffer<uchar>  x1pb;   // stick1 x-axis position (stores x1p)
      CircularBuffer<uchar>  y1pb;   // stick1 y-axis position (stores y1p)
      CircularBuffer<uchar>  z1pb;   // stick1 z-axis position (stores z1p)

      CircularBuffer<long>   t2pb;   // stick2 time of position (stores t2p)
      CircularBuffer<uchar>  x2pb;   // stick2 x-axis position (stores x2p)
      CircularBuffer<uchar>  y2pb;   // stick2 y-axis position (stores y2p)
      CircularBuffer<uchar>  z2pb;   // stick2 z-axis position (stores z2p)

      CircularBuffer<uchar>  d1pb;   // dial 1 position (stores d1p)
      CircularBuffer<uchar>  d2pb;   // dial 2 position (stores d2p)
      CircularBuffer<uchar>  d3pb;   // dial 3 position (stores d3p)
      CircularBuffer<uchar>  d4pb;   // dial 4 position (stores d4p)

      CircularBuffer<long>   t1tb;   // trigger stick 1 time (stores t1t)
      CircularBuffer<uchar>  x1tb;   // stick1 x-axis trigger pos. (stores x1t)
      CircularBuffer<uchar>  y1tb;   // stick1 y-axis trigger pos. (stores y1t)
      CircularBuffer<uchar>  w1tb;   // stick1 wack at trigger time (stores w1t)

      CircularBuffer<long>   t2tb;   // trigger stick 2 time (stores t2t)
      CircularBuffer<uchar>  x2tb;   // stick2 x-axis trigger pos. (stores x2t)
      CircularBuffer<uchar>  y2tb;   // stick2 y-axis trigger pos. (stores y2t)
      CircularBuffer<uchar>  w2tb;   // stick2 whack at trig time (stores w2t)

      CircularBuffer<long> b14ptb;   // b14+ button trigger time buffer
      CircularBuffer<long> b15ptb;   // b15+ button trigger time buffer
      CircularBuffer<long> b14mdtb;  // b14- pedal down trigger time buffer
      CircularBuffer<long> b14mutb;  // b14- pedal up trigger time buffer
      CircularBuffer<long> b15mdtb;  // b15- pedal down trigger time buffer
      CircularBuffer<long> b15mutb;  // b15- pedal up trigger time buffer


      // lower-level baton state/maintenance variables:

      ushort buf[16];          // antenna and potentiometer measurements
      int   completeBufQ[16];  // true if buffer index is not half complete
      int reportingQ;          // ???
      int errorQ;              // bool to print err. mes. for unknown MIDI cmds
   
      int trigger1;            // when a complete trigger has arrived, then
      int trigger2;            // these variables are set to 1.  User can
                               //   set to 0 so they can check when another
                               //   trigger arrives.
 
	  
      // here are some MIDI debugging values which MVM added to code.
      int lastmidi;
      int currentmidi;
      int midierrors;
      int miditests;
	  
      // variables which should be removed:
      int whack1x;
      int whack1y;
      int whack2x;
      int whack2y;


      // variables for recording position/trigger variables to a file:
      long        recordTimeOffset; 
      int         recordStateQ;
      fstream     recordOutput;
      void        recordState    (long aTime, const char* aState);
      void        recordState    (long aTime, const char* aState, int value);
      void        recordState    (long aTime, const char* aState,
                                    int value1, int value2, int value3);


   ///////////////////////////////////////////////////////////////////////////
   //
   // private functions:
   // 

   private: 
      void        interpretCommand      (MidiMessage aMessage);
      void        s1ts                  (long aTime);
      void        s1td                  (int flag, uchar aValue);
      void        s2ts                  (long aTime);
      void        s2td                  (int flag, uchar aValue);
      void        s1ps                  (long aTime);
      void        s1pd                  (int flag, uchar aValue);
      void        s2ps                  (long aTime);
      void        s2pd                  (int flag, uchar aValue);

      int         storedReportStatus;   // variable used for next two functions
      void        pushReportingStatus   (void);
      void        popReportingStatus    (void);

      void        stoprunningstatus     (void);

      int s1tf;     // stick 1 trigger flag
      int s2tf;     // stick 2 trigger flag
      int s1pf;     // stick 1 trigger flag
      int s2pf;     // stick 2 trigger flag

      int xDirection;   // flag for flipping the x-axis values or not
      int yDirection;   // flag for flipping the x-axis values or not
      int zDirection;   // flag for flipping the x-axis values or not
      
};


//////////////////////////////
//
// RadioBatonEmptyBehavior -- the following function will be the default 
//     function for the radio baton behavior function pointers. 
//

void RadioBatonEmptyBehavior(void);


#endif  /* _RADIOBATON_H_INCLUDED */



// md5sum: 9880d1b5243b781acf662461fbb6b0ad RadioBaton.h [20020518]

//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  9 13:08:12 PDT 2003
// Last Modified: Sat Aug  9 13:08:05 PDT 2003
// Filename:      ...sig/code/control/Tablet/RadioBatonTablet.h
// Web Address:   http://sig.sapp.org/include/sig/RadioBatonTablet.h
// Syntax:        C++
//
// Description:   A structure for handling the Radio Baton variables
//                and the MIDI connections from the computer to 
//                the Radio Batons simulated with a Wacom Tablet.
//

#ifndef _RADIOBATONTABLET_H_INCLUDED
#define _RADIOBATONTABLET_H_INCLUDED

#include "Tablet.h"

typdef ushort unsigned short;

class RadioBatonTablet : protected Tablet {
   public:
      RadioBatonTablet     (void);
      RadioBatonTablet     (int outputDevice, int inputDevice);
     ~RadioBatonTablet     ();

      int toggleXAxisDirection(void);
      int toggleYAxisDirection(void);
      int toggleZAxisDirection(void);
      int yDirection;
      int xDirection;
      int zDirection;
      int toggleError(void);

      void pause(void);
      void unpause(void);
      int  getInputPort(void);
      void setInputPort(int aPort);
      int  getOutputPort(void);
      void setOutputPort(int aPort);
      int  recordingQ(void);
      void openInput(void);
      void openOutput(void);
      void recordStateStop(void);
      void recordStateStart(const char* filename);
      void togglePositionReporting(void);
      int  getPositionReporting(void);
      void positionReportingOff(void);
      void positionReportingOn(void);
      void toggleOutputTrace(void);
      void toggleInputTrace(void);
      void getOutputTrace(void);
      void getInputTrace(void);
      int  getNumInputPorts(void);
      int  getNumOutputPorts(void);

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

      void mypen1trigon(void);
      void mypen1trigoff(void);
      void mypen2trigon(void);
      void mypen2trigoff(void);
      void mypen1button1on(void);
      void mypen1button1off(void);
      void mypen1button2on(void);
      void mypen1button2off(void);
      void mypen2button1on(void);
      void mypen2button1off(void);
      void mypen2button2on(void);
      void mypen2button2off(void);

      // public state variables
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

      // useless variables needed for compatibility
      int trigger1;
      int trigger2;
      int whack1x;
      int whack1y;
      int whack2x;
      int whack2y;

      ushort buf[16];          // antenna and potentiometer measurements
};
 


#endif  /* _RADIOBATONTABLET_H_INCLUDED */



// md5sum: a15c84c436fc0eb9fe90efd2b16e9e79 RadioBatonTablet.h [20050403]

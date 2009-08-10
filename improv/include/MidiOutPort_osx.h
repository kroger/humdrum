//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jun 10 17:17:09 PDT 2009
// Last Modified: Wed Jun 10 17:17:14 PDT 2009
// Filename:      ...sig/maint/code/control/MidiOutPort/linux/MidiOutPort_osx.h
// Web Address:   http://sig.sapp.org/include/sig/MidiOutPort_osx.h
// Syntax:        C++
//
// Description:   Operating-System specific interface for basic MIDI output 
//                capabilities in Apple Macintosh using the CoreMIDI 
//                interface.  This class is Privately inherited by the
//                MidiOutPort class.
// 

#ifndef _MIDIOUTPORT_OSX_H_INCLUDED
#define _MIDIOUTPORT_OSX_H_INCLUDED

#ifdef OSXPC || OSXOLD

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

#include "Array.h"

#include <CoreMIDI/CoreMIDI.h> /* interface to MIDI in Macintosh OS X */

typedef unsigned char uchar;

class MidiOutPort_osx {
   public:
                      MidiOutPort_osx     (void);
                      MidiOutPort_osx     (int aPort, int autoOpen = 1);
                     ~MidiOutPort_osx     ();

      void            close               (void);
      static void     closeAll            (void);
      int             getChannelOffset    (void) const;
      const char*     getName             (void);
      static const char* getName          (int i);
      int             getPort             (void);
      static int      getNumPorts         (void);
      int             getPortStatus       (void);
      int             getTrace            (void);
      int             rawsend             (int command, int p1, int p2);
      int             rawsend             (int command, int p1);
      int             rawsend             (int command);
      int             rawsend             (uchar* array, int size);
      int             open                (void);
      void            setChannelOffset    (int aChannel);
      void            setPort             (int aPort);
      int             setTrace            (int aState);
      int             sysex               (uchar* array, int size);
      void            toggleTrace         (void);

   protected:
      int    port;     // the port to which this object belongs
 
      static int      objectCount;     // num of similar objects in existence
      static int*     portObjectCount; // objects connected to particular port
      static int      numDevices;      // number of output ports
      static int*     trace;           // for printing messages to output
      static ostream* tracedisplay;    // for printing trace messages

   private:
      static void     deinitialize        (void); 
      static void     initialize          (void); 
      void            setPortStatus       (int aStatus);

      static int      channelOffset;   // channel offset, either 0 or 1.
                                       // not being used right now.

      // usually found in Sequencer classes:
      static int      getNumOutputs       (void);
      int             is_open             (void);
      static int                 initializeQ; // True if database was initialize
      static MIDIClientRef       midiclient;  // CoreMIDI main interface to MIDI
      static Array<MIDIPortRef>  midioutputs; // CoreMIDI MIDI output interfaces
      static Array<Array<char> > outputnames; // CoreMIDI derived port names 
};


#endif  /* OSXPC || OSXOLD */

#endif  /* _MIDIOUTPUT_OSX_H_INCLUDED */


// md5sum: f60183e23c49741e93d9b965bbe9a6d8 MidiOutPort_osx.h [20030102]

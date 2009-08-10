//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun 11 16:43:04 PDT 2009
// Last Modified: Thu Jun 11 16:43:12 PDT 2009
// Filename:      ...sig/maint/code/control/MidiInPort/osx/MidiInPort_osx.h
// Web Address:   http://sig.sapp.org/include/sig/MidiInPort_osx.h
// Syntax:        C++ 
//
// Description:   Operating-System specific interface for basic MIDI input 
//                capabilities in Apple Macintosh using the CoreMIDI 
//                interface.  This class is Privately inherited by the
//                MidiInPort class.
//

#ifndef _MIDIINPORT_OSX_H_INCLUDED
#define _MIDIINPORT_OSX_H_INCLUDED

#ifdef OSXPC || OSXOLD

#include "MidiMessage.h"
#include "CircularBuffer.h"
#include "Array.h"
#include "SigTimer.h"
#include <CoreMIDI/CoreMIDI.h>

typedef unsigned char uchar;
typedef void (*MIDI_Callback_function)(int arrivalPort);


class MidiInPort_osx {
   public:
                      MidiInPort_osx             (void);
                      MidiInPort_osx             (int aPort, int autoOpen = 1);
                     ~MidiInPort_osx             ();

      void            clearSysex                 (int buffer);
      void            clearSysex                 (void);
      void            close                      (void);
      void            close                      (int i) { close(); }
      void            closeAll                   (void);
      MidiMessage     extract                    (void);
      int             getBufferSize              (void);
      int             getChannelOffset           (void) const;
      int             getCount                   (void);
      const char*     getName                    (void);
      static const char* getName                 (int i);
      static int      getNumPorts                (void);
      int             getPort                    (void);
      int             getPortStatus              (void);
      uchar*          getSysex                   (int buffer);
      int             getSysexSize               (int buffer);
      int             getTrace                   (void);
      void            insert                     (const MidiMessage& aMessage);
      int             installSysex               (uchar* anArray, int aSize);
      MidiMessage&    message                    (int index);
      int             open                       (void);
      void            pause                      (void);
      void            setBufferSize              (int aSize);
      void            setChannelOffset           (int anOffset);
      void            setPort                    (int aPort);
      int             setTrace                   (int aState);
      void            toggleTrace                (void);
      void            unpause                    (void);

   protected:
      int    port;     // the port to which this object belongs

      static MIDI_Callback_function  callbackFunction;

      static int      installSysexPrivate        (int port, 
                                                    uchar* anArray, int aSize);
 
      static int        objectCount;     // num of similar objects in existence
      static int*       portObjectCount; // objects connected to particular port
      static int*       trace;           // for verifying input
      static ostream*   tracedisplay;    // stream for displaying trace
      static int        numDevices;      // number of input ports
      static CircularBuffer<MidiMessage>** midiBuffer; // MIDI storage frm ports
      static int        channelOffset;   // channel offset, either 0 or 1
                                         // not being used right now.
      static int*       pauseQ;          // for adding items to Buffer or not
      static SigTimer   midiTimer;       // for timing MIDI input
      static int*       sysexWriteBuffer; // for MIDI sysex write location
      static Array<uchar>** sysexBuffers; // for MIDI sysex storage

   private:
      void            deinitialize               (void); 
      void            initialize                 (void); 

      // Functions/Data normally found in Seqencer object (which is not needed):
      int             is_open                    (void);
      static MIDIClientRef       midiclient;  // CoreMIDI main interface to MIDI
      static Array<MIDIPortRef>  midiinputs; // CoreMIDI MIDI output interfaces
      static Array<Array<char> > inputnames;
 
   //friend void *interpretMidiInputStreamPrivate(void *x);
   friend   void improvReadProc(const MIDIPacketList *packetList, 
            void* readProcRefCon, void* srcConnRefCon);
    
};

//void *interpretMidiInputStreamPrivate(void * x);

void improvReadProc(const MIDIPacketList *packetList, void* readProcRefCon,
   void* srcConnRefCon);



#endif  /* OSXPC || OSXOLD */

#endif  /* _MIDIINPORT_OSX_H_INCLUDED */



// md5sum: 56e32a58ab82ca13f56a3d5c30e8615a MidiInPort_osx.h [20050403]

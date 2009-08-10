//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu May 11 21:06:21 PDT 2000
// Last Modified: Wed Oct  3 22:21:11 PDT 2001 (frozen for ALSA 0.5)
// Last Modified: Sat Jun 13 21:16:29 PDT 2009 (renamed SigCollection)
// Filename:      ...sig/maint/code/control/MidiOutPort/Sequencer_alsa05.h
// Web Address:   http://sig.sapp.org/include/sig/Sequencer_alsa05.h
// Syntax:        C++ 
//
// Description:   Basic MIDI input/output functionality for the 
//                Linux ALSA midi device /dev/snd/midiXX.  This class
//                is inherited by the classes MidiInPort_alsa and
//                MidiOutPort_alsa.
//

#ifndef _SEQUENCER_ALSA_H_INCLUDED
#define _SEQUENCER_ALSA_H_INCLUDED

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

#ifdef ALSA05

#include <sys/asoundlib.h>
#include "SigCollection.h"

#define MIDI_EXTERNAL  (1)
#define MIDI_INTERNAL  (2)

typedef unsigned char uchar;


class Sequencer_alsa05 {
   public:
                    Sequencer_alsa05     (int autoOpen = 1);
                   ~Sequencer_alsa05     ();

      void          close                (void);
      void          displayInputs        (ostream& out = cout, 
                                            char* initial = "\t");
      void          displayOutputs       (ostream& out = cout, 
                                            char* initial = "\t");
      static const char*   getInputName  (int aDevice);
      static const char*   getOutputName (int aDevice);
      static int    getNumInputs         (void);
      static int    getNumOutputs        (void);
      int           is_open              (int mode, int index);
      int           is_open_in           (int index);
      int           is_open_out          (int index);
      int           open                 (void);
      void          read                 (int dev, uchar* buf, int count);
      void          rebuildInfoDatabase  (void);
      int           write                (int aDevice, int aByte);
      int           write                (int aDevice, uchar* bytes, int count);
      int           write                (int aDevice, char* bytes, int count);
      int           write                (int aDevice, int* bytes, int count);
      
      int           getInCardValue       (int aDevice) const;
      int           getOutCardValue      (int aDevice) const;
   protected:
      static int    class_count;            // number of existing classes using
      static int    indevcount;             // number of MIDI input devices
      static int    outdevcount;            // number of MIDI output devices
      static int    initialized;            // for starting buileinfodatabase

      static SigCollection<snd_rawmidi_t*> Sequencer_alsa05::rawmidi_in;
      static SigCollection<snd_rawmidi_t*> Sequencer_alsa05::rawmidi_out;
      static SigCollection<int> Sequencer_alsa05::midiincard;
      static SigCollection<int> Sequencer_alsa05::midioutcard;
      static SigCollection<int> Sequencer_alsa05::midiindevice;
      static SigCollection<int> Sequencer_alsa05::midioutdevice;
      static SigCollection<char*> Sequencer_alsa05::midiinname;
      static SigCollection<char*> Sequencer_alsa05::midioutname;

   private:
      static void   buildInfoDatabase     (void);
      static int    getFd                 (void);   
      int           getInDeviceValue      (int aDevice) const;
      int           getInputType          (int aDevice) const;
      int           getOutDeviceValue     (int aDevice) const;
      int           getOutputType         (int aDevice) const;
      void          removeInfoDatabase    (void);


   friend void *interpretMidiInputStreamPrivateALSA05(void * x);

};

#else  /* ALSA05 is not defined */

typedef unsigned char uchar;

class Sequencer_alsa05 {
   public:
                    Sequencer_alsa05     (int autoOpen = 1) { }
                   ~Sequencer_alsa05     () { }

      void          close                (void) { };
      void          displayInputs        (ostream& out = cout, 
                                            char* initial = "\t") 
                                         { out << initial << "NONE\n"; }
      void          displayOutputs       (ostream& out = cout, 
                                            char* initial = "\t") 
                                         { out << initial << "NONE\n"; }
      static const char*   getInputName  (int aDevice) { return ""; }
      static const char*   getOutputName (int aDevice) { return ""; }
      static int    getNumInputs         (void) { return 0; }
      static int    getNumOutputs        (void) { return 0; }
      int           is_open              (int mode, int index) { return 0; }
      int           is_open_in           (int index) { return 0; }
      int           is_open_out          (int index) { return 0; }
      int           open                 (void) { return 0; }
      void          read                 (int dev, uchar* buf, int count) { }
      void          rebuildInfoDatabase  (void) { }
      int           write                (int aDevice, int aByte) { return 0; }
      int           write                (int aDevice, uchar* bytes, int count) { return 0; }
      int           write                (int aDevice, char* bytes, int count) { return 0; }
      int           write                (int aDevice, int* bytes, int count) { return 0; }
      int           getInCardValue       (int aDevice) const { return 0; }
      int           getOutCardValue      (int aDevice) const { return 0; }

};


#endif /* ALSA05 */


#endif  /* _SEQUENCER_ALSA05_H_INCLUDED */


// md5sum: b014161aa0b644880b59a88b79db87e8 Sequencer_alsa05.h [20050403]

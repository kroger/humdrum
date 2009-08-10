//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu May 11 21:06:21 PDT 2000
// Last Modified: Sat Oct 13 14:50:04 PDT 2001 (updated for ALSA 0.9 interface)
// Last Modified: Tue May 26 12:29:15 EDT 2009 (updated for ALSA 1.0 interface)
// Last Modified: Sat Jun 13 21:16:29 PDT 2009 (renamed SigCollection)
// Filename:      ...sig/maint/code/control/MidiOutPort/Sequencer_alsa.h
// Web Address:   http://sig.sapp.org/include/sig/Sequencer_alsa.h
// Syntax:        C++ 
//
// Description:   Basic MIDI input/output functionality for the 
//                Linux ALSA raw midi devices. This class
//                is inherited by the classes MidiInPort_alsa and
//                MidiOutPort_alsa.
//
// To get information of status of ALSA hardware & software
//    cat /proc/asound/version
//    cat /proc/asound/devices
//    cat /proc/asound/card0/midi0
//

#ifndef _SEQUENCER_ALSA_H_INCLUDED
#define _SEQUENCER_ALSA_H_INCLUDED

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

#ifdef ALSA

// use this include for older versions of ALSA before 0.9:
// #include <sys/asoundlib.h>
// use this include for newer versions of ALSA 0.9 and higher:
#include <alsa/asoundlib.h>

#include "SigCollection.h"

#define MIDI_EXTERNAL  (1)
#define MIDI_INTERNAL  (2)

class ALSA_ENTRY {
   public:
           ALSA_ENTRY(void) { clear(); };
      void clear     (void) 
                     { card = device = subdevice = -1; 
                       input = output = 0;
                       strcpy(name, ""); };
      int  card;
      int  device;
      int  subdevice;
      int  input;
      int  output;
      char name[1024];
};


typedef unsigned char uchar;

class Sequencer_alsa {
   public:
                    Sequencer_alsa       (int autoOpen = 1);
                   ~Sequencer_alsa       ();

      void          close                (void);
      void          closeInput           (int index);
      void          closeOutput          (int index);
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
      int           open                 (int direction, int index);
      int           openInput            (int index);
      int           openOutput           (int index);
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

      static SigCollection<snd_rawmidi_t*> rawmidi_in;
      static SigCollection<snd_rawmidi_t*> rawmidi_out;
      static SigCollection<ALSA_ENTRY>     rawmidi_info;
      static SigCollection<int>            midiin_index;
      static SigCollection<int>            midiout_index;

   private:
      static void   buildInfoDatabase     (void);
      static void   getDeviceInfo         (SigCollection<ALSA_ENTRY>& info);
      static void   searchForMidiDevicesOnCard(int card,
                                           SigCollection<ALSA_ENTRY>& info);
      static void   searchForMidiSubdevicesOnDevice(snd_ctl_t* ctl, 
                                           int card, int device, 
                                           SigCollection<ALSA_ENTRY>& info);
      static int    is_input              (snd_ctl_t *ctl, int card, 
                                           int device, int sub);
      static int    is_output             (snd_ctl_t *ctl, int card, 
                                           int device, int sub);
      int           getInSubdeviceValue   (int aDevice) const;
      int           getInDeviceValue      (int aDevice) const;
      int           getInputType          (int aDevice) const;
      int           getOutSubdeviceValue  (int aDevice) const;
      int           getOutDeviceValue     (int aDevice) const;
      int           getOutputType         (int aDevice) const;
      void          removeInfoDatabase    (void);


   friend void *interpretMidiInputStreamPrivateALSA(void * x);

};

#else  /* LINUX or ALSA is not defined */

typedef unsigned char uchar;

class Sequencer_alsa {
   public:
                    Sequencer_alsa       (int autoOpen = 1) { }
                   ~Sequencer_alsa       () { }

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


#endif /* ALSA */


#endif  /* _SEQUENCER_ALSA_H_INCLUDED */


// md5sum: 9f915472e655ca80a1c81a1581fff1c3 Sequencer_alsa.h [20050403]

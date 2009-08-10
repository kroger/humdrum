//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu May 11 21:10:02 PDT 2000
// Last Modified: Sat Oct 13 14:51:43 PDT 2001 (updated for ALSA 0.9 interface)
// Last Modified: Tue May 26 12:38:18 EDT 2009 (updated for ALSA 1.0 interface)
// Filename:      ...sig/maint/code/control/Sequencer_alsa.cpp
// Web Address:   http://sig.sapp.org/src/sig/Sequencer_alsa.cpp
// Syntax:        C++ 
//
// Description:   MIDI input/output capability for the 
//                Linux ALSA raw midi devices.  This class
//                is inherited by the classes MidiInPort_alsa and
//                MidiOutPort_alsa.
//

#if defined(LINUX) && defined(ALSA)
 
#include "SigCollection.h"
#include "Sequencer_alsa.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>           /* for reading filename for MIDI info */

// use the following include for older then ALSA 0.9:
//#include <sys/asoundlib.h>
// use the following include for ALSA 0.9 and later:
#include <alsa/asoundlib.h>

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

typedef unsigned char uchar;

// define static variables:
int    Sequencer_alsa::class_count          =  0;
int    Sequencer_alsa::initialized          =  0;

// static variables for MIDI I/O information database
int    Sequencer_alsa::indevcount      = 0;
int    Sequencer_alsa::outdevcount     = 0;

SigCollection<snd_rawmidi_t*> Sequencer_alsa::rawmidi_in;
SigCollection<snd_rawmidi_t*> Sequencer_alsa::rawmidi_out;
SigCollection<ALSA_ENTRY>     Sequencer_alsa::rawmidi_info;
SigCollection<int>            Sequencer_alsa::midiin_index;
SigCollection<int>            Sequencer_alsa::midiout_index;


///////////////////////////////
//
// Sequencer_alsa::Sequencer_alsa --
//	default value: autoOpen = 1;
//

Sequencer_alsa::Sequencer_alsa(int autoOpen) {
   if (class_count < 0) {
      cerr << "Unusual class instantiation count: " << class_count << endl;
      exit(1);
   } else if (class_count == 0) {
      buildInfoDatabase();
   }

   // will not autoOpen

   class_count++;
}



//////////////////////////////
//
// Sequencer_alsa::~Sequencer_alsa --
//

Sequencer_alsa::~Sequencer_alsa() {

   if (class_count == 1) {
      close();
      removeInfoDatabase();
   } else if (class_count <= 0) {
      cerr << "Unusual class instantiation count: " << class_count << endl;
      exit(1);
   }

   class_count--;
}



//////////////////////////////
//
// Sequencer_alsa::close -- close the sequencer device.  The device
//   automatically closes once the program ends.
//

void Sequencer_alsa::close(void) {
   int i;

   for (i=0; i<getNumInputs(); i++) {
      if (rawmidi_in[i] != NULL) {
         snd_rawmidi_close(rawmidi_in[i]);
         rawmidi_in[i] = NULL;
      }
   }

   for (i=0; i<getNumOutputs(); i++) {
      if (rawmidi_out[i] != NULL) {
         snd_rawmidi_close(rawmidi_out[i]);
         rawmidi_out[i] = NULL;
      }
   }
}


void Sequencer_alsa::closeInput(int index) {
   if (index < 0 || index >= rawmidi_in.getSize()) {
      return;
   }

   if (rawmidi_in[index] != NULL) {
      snd_rawmidi_close(rawmidi_in[index]); 
      rawmidi_in[index] = NULL;
   }
}


void Sequencer_alsa::closeOutput(int index) {
   if (index < 0 || index >= rawmidi_out.getSize()) {
      return;
   }

   if (rawmidi_out[index] != NULL) {
      snd_rawmidi_close(rawmidi_out[index]); 
      rawmidi_out[index] = NULL;
   }
}



//////////////////////////////
//
// Sequencer_alsa::displayInputs -- display a list of the
//     available MIDI input devices.
//	default values: out = cout, initial = "\t"
//
 
void Sequencer_alsa::displayInputs(ostream& out, char* initial) {
   for (int i=0; i<getNumInputs(); i++) {
      out << initial << i << ": " << getInputName(i) << '\n';
   }
}



//////////////////////////////
//
// Sequencer_alsa::displayOutputs -- display a list of the
//     available MIDI output devices.
//	default values: out = cout, initial = "\t"
//
 
void Sequencer_alsa::displayOutputs(ostream& out, char* initial) {
   for (int i=0; i<getNumOutputs(); i++) {
      out << initial << i << ": " << getOutputName(i) << '\n';
   }
}



//////////////////////////////
//
// Sequencer_alsa::getInputName -- returns a string to the name of
//    the specified input device.  The string will remain valid as
//    long as there are any sequencer devices in existence.
//

const char* Sequencer_alsa::getInputName(int aDevice) {
   if (initialized == 0) {
      buildInfoDatabase();
   }
   return rawmidi_info[midiin_index[aDevice]].name;
}



//////////////////////////////
//
// Sequencer_alsa::getNumInputs -- returns the total number of
//     MIDI inputs that can be used.
//

int Sequencer_alsa::getNumInputs(void) {
   if (initialized == 0) {
      buildInfoDatabase();
   }
   return indevcount;
}



//////////////////////////////
//
// Sequencer_alsa::getNumOutputs -- returns the total number of
//     MIDI inputs that can be used.
//

int Sequencer_alsa::getNumOutputs(void) {
   if (initialized == 0) {
      buildInfoDatabase();
   }
   return outdevcount;
}



//////////////////////////////
//
// Sequencer_alsa::getOutputName -- returns a string to the name of
//    the specified output device.  The string will remain valid as
//    long as there are any sequencer devices in existence.
//

const char* Sequencer_alsa::getOutputName(int aDevice) {
   if (initialized == 0) {
      buildInfoDatabase();
   }
   return rawmidi_info[midiout_index[aDevice]].name;
}



//////////////////////////////
//
// Sequencer_alsa::is_open -- returns true if the
//     sequencer device is open, false otherwise.
//

int Sequencer_alsa::is_open(int mode, int index) {
   if (mode == 0) {
      // midi output
      if (rawmidi_out[index] != NULL) {
         return 1;
      } else {
         return 0;
      }
   } else {
      if (rawmidi_in[index] != NULL) {
         return 1;
      } else {
         return 0;
      }
   }
}


int Sequencer_alsa::is_open_in(int index) {
   return is_open(1, index);
}


int Sequencer_alsa::is_open_out(int index) {
   return is_open(0, index);
}



/////////////////////////////
//
// Sequencer_alsa::open -- returns true if the device
//	was successfully opended (or already opened)
//

int Sequencer_alsa::open(int direction, int index) {
   if (direction == 0) {
      return openOutput(index);
   } else {
      return openInput(index);
   }
}


int Sequencer_alsa::openInput(int index) {
   if (rawmidi_in[index] != NULL) {
      return 1;
   }
   int status;
   char devname[128] = {0};
   int card = rawmidi_info[midiin_index[index]].card;
   int device = rawmidi_info[midiin_index[index]].device;
   int subdevice = rawmidi_info[midiin_index[index]].subdevice;
   if (subdevice >= 0) {
      sprintf(devname, "hw:%d,%d,%d", card, device, subdevice);
   } else {
      sprintf(devname, "hw:%d,%d", card, device);
   }

   int mode = 0;
   status = snd_rawmidi_open(&rawmidi_in[index], NULL, devname, mode);
   if (status == 0) {
      return 1;
   } else { 
      return 0;
   }
}


int Sequencer_alsa::openOutput(int index) {
   if (rawmidi_out[index] != NULL) {
      return 1;
   }
   int status;
   char devname[128] = {0};
   int card = rawmidi_info[midiout_index[index]].card;
   int device = rawmidi_info[midiout_index[index]].device;
   int subdevice = rawmidi_info[midiout_index[index]].subdevice;
   
   if (subdevice >= 0) {
      sprintf(devname, "hw:%d,%d,%d", card, device, subdevice);
   } else {
      sprintf(devname, "hw:%d,%d", card, device);
   }
   int mode = SND_RAWMIDI_SYNC;
   status = snd_rawmidi_open(NULL, &rawmidi_out[index], devname, mode);
   if (status == 0) {
      return 1;
   } else { 
      return 0;
   }
}



//////////////////////////////
//
// Sequencer_alsa::read -- reads MIDI bytes and also stores the 
//     device from which the byte was read from.  Timing is not
//     saved from the device.  If needed, then it would have to 
//     be saved in this function, or just return the raw packet
//     data (use rawread function).
//
 
void Sequencer_alsa::read(int dev, uchar* buf, int count) {
   if (is_open_in(dev)) {
      snd_rawmidi_read(rawmidi_in[dev], buf, count);
   } else {
      cout << "Warning: MIDI input port " << dev << " is not open for reading" 
           << endl;
   }
}



//////////////////////////////
//
// Sequencer_alsa::rebuildInfoDatabase -- rebuild the internal
//   database that keeps track of the MIDI input and output devices.
//

void Sequencer_alsa::rebuildInfoDatabase(void) {
   removeInfoDatabase();
   buildInfoDatabase();
}



///////////////////////////////
//
// Sequencer_alsa::write -- Send a byte out the specified MIDI
//    port which can be either an internal or an external synthesizer.
//

int Sequencer_alsa::write(int aDevice, int aByte) {
   uchar byte[1];
   byte[0] = (uchar)aByte;
   return write(aDevice, byte, 1);   
}


int Sequencer_alsa::write(int aDevice, uchar* bytes, int count) {
   if (is_open_out(aDevice)) {
      int status = snd_rawmidi_write(rawmidi_out[aDevice], bytes, count);
      return status == count ? 1 : 0;
   } else {
      cerr << "Warning: MIDI output port " << aDevice << " is not open for writing" 
           << endl;
      return 0;
   }

   return 0;
}


int Sequencer_alsa::write(int aDevice, char* bytes, int count) {
   return write(aDevice, (uchar*)bytes, count);
}


int Sequencer_alsa::write(int aDevice, int* bytes, int count) {
   uchar *newBytes;
   newBytes = new uchar[count];
   for (int i=0; i<count; i++) {
      newBytes[i] = (uchar)bytes[i];
   }
   int status = write(aDevice, newBytes, count);
   delete [] newBytes;
   return status;
}



///////////////////////////////////////////////////////////////////////////
//
// private functions
//

//////////////////////////////
//
// Sequencer_alsa::buildInfoDatabase -- determines the number
//     of MIDI input and output devices and determines their names.
//

void Sequencer_alsa::buildInfoDatabase(void) {
   if (initialized) {
      return;
   }
   initialized = 1;
  
   if (indevcount != 0 || outdevcount != 0) {
      cout << "Error: Sequencer_alsa is already running" << endl;
      cout << "Indevcout = " << indevcount << " and " 
           << " outdevcount = " << outdevcount << endl;
      exit(1);
   }

   indevcount  = 0;
   outdevcount = 0;

   // read number of MIDI inputs/output available 
   getDeviceInfo(rawmidi_info);

   // store data into separate input/output arrays:
   midiin_index.setSize(rawmidi_info.getSize());
   midiin_index.setSize(0);
   midiin_index.allowGrowth(1);
   midiout_index.setSize(rawmidi_info.getSize());
   midiout_index.setSize(0);
   midiout_index.allowGrowth(1);

   int i;
   for (i=0; i<rawmidi_info.getSize(); i++) {
      if (rawmidi_info[i].output) {
         midiout_index.append(i);
      }
      if (rawmidi_info[i].input) {
         midiin_index.append(i);
      }
   }
   midiin_index.allowGrowth(0);
   midiout_index.allowGrowth(0);
   indevcount  = midiin_index.getSize();
   outdevcount = midiout_index.getSize();

   rawmidi_in.setSize(indevcount);
   for (i=0; i<rawmidi_in.getSize(); i++) {
      rawmidi_in[i] = NULL;
   }
   rawmidi_out.setSize(outdevcount);
   for (i=0; i<rawmidi_out.getSize(); i++) {
      rawmidi_out[i] = NULL;
   }
}



//////////////////////////////
//
// Sequencer_alsa::getInDeviceValue --
//

int Sequencer_alsa::getInDeviceValue(int aDevice) const {
   return rawmidi_info[midiin_index[aDevice]].device;
}



//////////////////////////////
//
// Sequencer_alsa::getInSubdeviceValue --
//

int Sequencer_alsa::getInSubdeviceValue(int aDevice) const {
   return rawmidi_info[midiin_index[aDevice]].subdevice;
}



//////////////////////////////
//
// Sequencer_alsa::getInCardValue --
//

int Sequencer_alsa::getInCardValue(int aDevice) const {
   return midiin_index[aDevice];
}



//////////////////////////////
//
// Sequencer_alsa::getOutDeviceValue --
//

int Sequencer_alsa::getOutDeviceValue(int aDevice) const {
   return rawmidi_info[midiout_index[aDevice]].device;
}



//////////////////////////////
//
// Sequencer_alsa::getOutSubdeviceValue --
//

int Sequencer_alsa::getOutSubdeviceValue(int aDevice) const {
   return rawmidi_info[midiout_index[aDevice]].subdevice;
}



//////////////////////////////
//
// Sequencer_alsa::getOutCardValue --
//

int Sequencer_alsa::getOutCardValue(int aDevice) const {
   return rawmidi_info[midiout_index[aDevice]].card;
}



//////////////////////////////
//
// Sequencer_alsa::removeInfoDatabase --
//

void Sequencer_alsa::removeInfoDatabase(void) {
   if (rawmidi_in.getSize() != 0) {
      close();
   }

   if (rawmidi_out.getSize() != 0) {
      close();
   }

   rawmidi_in.setSize(0);
   rawmidi_out.setSize(0);
   rawmidi_info.setSize(0);
   midiin_index.setSize(0);
   midiout_index.setSize(0);

   indevcount = 0;
   outdevcount = 0;
   initialized = 0;
}



//////////////////////////////
//
// getDeviceInfo --  
//

void Sequencer_alsa::getDeviceInfo(SigCollection<ALSA_ENTRY>& info) {
   info.setSize(0);
   info.allowGrowth(1);

   int status;
   int card = -1;

   if ((status = snd_card_next(&card)) < 0) {
      cerr << "Cannot read MIDI information" << endl;
      cerr << "Reason: " << snd_strerror(status) << endl;
      return;
   }
   if (card < 0) {
      cerr << "No sound cards found" << endl;
      return;
   }
   while (card >= 0) {
      searchForMidiDevicesOnCard(card, info);
      status = snd_card_next(&card);
      if (status < 0) {
         break;
      }
   }

   info.allowGrowth(0);
}



////////////////////////
//
// searchForMidiDevicesOnCard -- for a particular "card" look at all
//   of the "devices/subdevices" on it and store the information about
//   triplets of numbers which can handle MIDI input and/or output.
//

void Sequencer_alsa::searchForMidiDevicesOnCard(int card, 
      SigCollection<ALSA_ENTRY>& info) {
   snd_ctl_t *ctl;
   char name[64] = {0};
   int device = -1;
   int status;

   sprintf(name, "hw:%d", card);
   if ((status = snd_ctl_open(&ctl, name, 0)) < 0) {
      cerr << "Cannot open control for card " << card << ": " 
           << snd_strerror(status) << endl;
      return;
   }
   do {
      status = snd_ctl_rawmidi_next_device(ctl, &device);
      if (status < 0) {
         cerr << "Cannot determine device number: " 
              << snd_strerror(status) << endl;
         break;
      }
      if (device >= 0) {
         searchForMidiSubdevicesOnDevice(ctl, card, device, info);
      }
   } while (device >= 0);
   snd_ctl_close(ctl);
}



//////////////////////////////
//
// Sequencer_alsa::searchForMidiSubdevicesOnDevice --
//

void Sequencer_alsa::searchForMidiSubdevicesOnDevice(snd_ctl_t* ctl, int card, 
      int device, SigCollection<ALSA_ENTRY>& rawmidi_info) {
   snd_rawmidi_info_t *info;
   const char *name;
   const char *sub_name;
   int subs, subs_in, subs_out;
   int sub, in, out;
   int status;

   snd_rawmidi_info_alloca(&info);
   snd_rawmidi_info_set_device(info, device);

   snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
   snd_ctl_rawmidi_info(ctl, info);
   subs_in = snd_rawmidi_info_get_subdevices_count(info);
   snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
   snd_ctl_rawmidi_info(ctl, info);
   subs_out = snd_rawmidi_info_get_subdevices_count(info);
   subs = subs_in > subs_out ? subs_in : subs_out;

   sub = 0;
   in = out = 0;
   if ((status = is_output(ctl, card, device, sub)) < 0) {
      cerr << "Cannot get rawmidi information " << card << ":"
           << device << ": " << snd_strerror(status) << endl;
      return;
   } else if (status)
      out = 1;

   if (status == 0) {
      if ((status = is_input(ctl, card, device, sub)) < 0) {
         cerr << "Cannot get rawmidi information " << card << ":" 
              << device <<": " << snd_strerror(status) << endl;
         return;
      }
   } else if (status)
      in = 1;

   if (status == 0)
      return;

   int index;
   name = snd_rawmidi_info_get_name(info);
   sub_name = snd_rawmidi_info_get_subdevice_name(info);
   if (sub_name[0] == '\0') {
      if (subs == 1) {

         rawmidi_info.setSize(rawmidi_info.getSize()+1);
         index = rawmidi_info.getSize()-1;
         rawmidi_info[index].card = card;
         rawmidi_info[index].device = device;
         rawmidi_info[index].subdevice = -1;
         strcpy(rawmidi_info[index].name, name);
         rawmidi_info[index].input = in;
         rawmidi_info[index].output = out;
      } else

         rawmidi_info.setSize(rawmidi_info.getSize()+1);
         index = rawmidi_info.getSize()-1;
         rawmidi_info[index].card = card;
         rawmidi_info[index].device = device;
         rawmidi_info[index].subdevice = -1;
         strcpy(rawmidi_info[index].name, name);
         rawmidi_info[index].input = in;
         rawmidi_info[index].output = out;

   } else {
      sub = 0;
      for (;;) {
         rawmidi_info.setSize(rawmidi_info.getSize()+1);
         index = rawmidi_info.getSize()-1;
         rawmidi_info[index].card = card;
         rawmidi_info[index].device = device;
         rawmidi_info[index].subdevice = sub;
         strcpy(rawmidi_info[index].name, sub_name);
         rawmidi_info[index].input = in;
         rawmidi_info[index].output = out;

         if (++sub >= subs)
            break;

         in = is_input(ctl, card, device, sub);
         out = is_output(ctl, card, device, sub);
         snd_rawmidi_info_set_subdevice(info, sub);
         if (out) {
            snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
            if ((status = snd_ctl_rawmidi_info(ctl, info)) < 0) {
               cerr << "Cannot get rawmidi information " << card << ":"
                    << device << ":" << sub << ": " << snd_strerror(status)
                    << endl;
               break;
            }
         } else {
            snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
            if ((status = snd_ctl_rawmidi_info(ctl, info)) < 0) {
               cerr << "Cannot get rawmidi information " << card << ":" 
                    << device << ":" << sub << ": " 
                    << snd_strerror(status) << endl;
               break;
            }
         }
         sub_name = snd_rawmidi_info_get_subdevice_name(info);
      }
   }
}



//////////////////////////////
//
// Sequencer_alsa::is_input -- returns true if specified card/device/sub 
//      can read MIDI data.
//

int Sequencer_alsa::is_input(snd_ctl_t *ctl, int card, int device, int sub) {
   snd_rawmidi_info_t *info;
   int status;

   snd_rawmidi_info_alloca(&info);
   snd_rawmidi_info_set_device(info, device);
   snd_rawmidi_info_set_subdevice(info, sub);
   snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
  
   if ((status = snd_ctl_rawmidi_info(ctl, info)) < 0 && status != -ENXIO) {
      return status;
   } else if (status == 0) {
      return 1;
   }

   return 0;
}



//////////////////////////////
//
// Sequencer_alsa::is_output -- returns true if specified card/device/sub 
//     can write MIDI data.
//

int Sequencer_alsa::is_output(snd_ctl_t *ctl, int card, int device, int sub) {
   snd_rawmidi_info_t *info;
   int status;

   snd_rawmidi_info_alloca(&info);
   snd_rawmidi_info_set_device(info, device);
   snd_rawmidi_info_set_subdevice(info, sub);
   snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);

   if ((status = snd_ctl_rawmidi_info(ctl, info)) < 0 && status != -ENXIO) {
      return status;
   } else if (status == 0) {
      return 1;
   }

   return 0;
}



#endif   /* LINUX and ALSA */

// md5sum: 22b8e7ca6c14c4f0a708d8eaacc0e910 Sequencer_alsa.cpp [20050403]

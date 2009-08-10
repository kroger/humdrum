//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jan 27 19:49:32 PST 2000
// Last Modified: Sun Jan 30 13:27:31 PST 2000
// Filename:      sysexio.cpp
// Syntax:        C++ 
//
// Description:   This program is an example program for
//                handling SYSEX messages with the improv library.
//
// System Exclusive messages are stored in a separate buffer from 
// regular MIDI input messages.  They are stored in a Circular Buffer
// which has a length of 128 (or 256?).  The MidiInPort class will
// collect a system exclusive message.  When it has finished receiving
// the sysex message, it will store it into the sysex circular buffer
// and then place a regular MIDI message into the regular MIDI input
// buffer.  The format of the MIDI input message for the sysex is:
//   parameter 0 = 0xf0  (the command for the start of a sysex)
//   parameter 1 = the sysex buffer index for the data of the sysex
//
//   If you extract a MIDI message from a MidiInPort and the 
//   MIDI command byte is 0xf0, then you know you have a system
//   exclusive message available in the MIDI input's sysex buffer
//
//   Here is the basic code to check for sysex messages, and how
//   to determine the length of the sysex message and how to get the
//   actual data:
//
//   MidiInput midiinput;
//   MidiMessage message;
//   int sysexSize;
//   int bufferNum;
//   unsigned char *sysexData;
//
//   if (midiinput.getCount() > 0) {
//      message = midiinput.extract();
//      if (message.p0() == 0xf0) {
//         // we have a system exclusive message
//         bufferNum = message.p1();
//         sysexData = midiinput.getSysex(bufferNum);
//         sysexSize = midiinput.getSysexSize(bufferNum);
//      }
//   }
//
//   Sending a System Exclusive message through MIDI output is more
//   simple.  The starting 0xf0 byte and the ending command 0xf7
//   should be present in the sysex data.
//
//   unsigned char *sysexData;
//   int sysexLength;
//   MidiOutput midiout;
//   
//   //fill the sysex array and set the size variable, then to send:
//   midiout.sysex(sysexData, sysexLength);
//   midiout.rawsend(sysexData, sysexLength);     // equivalent to last line
//

#include "improv.h"

#ifndef OLDCPP
   #include <iostream>
   #include <iomanip>
   using namespace std;
#else
   #include <iostream.h>
   #include <iomanip.h>
#endif


int main(void) {
   MidiInput midiin;
   MidiOutput midiout;
   midiout.setPort(0);
   midiout.open();
   midiin.setPort(0);
   midiin.open();

   MidiMessage message;
   int sysexloc;
   unsigned char *sysexdata = NULL;
   int sysexsize = 0;
   int i;
   int running = 1;
   cout << "sysexio -- display and echo SYSEX messages from MIDI input" << endl;
   cout << "Press Middle C to quit." << endl;
   while (running) {
      if (midiin.getCount() > 0) {
         message = midiin.extract();
         if (message.p0() == 0xf0) {
            sysexloc = message.p1();
            sysexdata = midiin.getSysex(sysexloc);
            sysexsize = midiin.getSysexSize(sysexloc);

            // print out the sysex data to the screen:
            for (i=0; i<sysexsize; i++) {
               cout << hex << (int)sysexdata[i] << " ";
               if ((i + 1) % 30 == 0) {
                  cout << endl;
               }
            }
            cout << endl;

            // Now echo the messages to MIDI output (as a demo
            // for how to send Sysex outputs)
            midiout.rawsend(sysexdata, sysexsize);

            // As a courtesy, mark the midiin sysex buffer free
            // but this is not necessay (it will be erased when
            // more space is needed for storing a sysex.
            midiin.clearSysex(sysexloc);
            
         } else if ((message.p0() & 0xf0) == 0x90) {
            // Exit the program when a middle C note is pressed.
            if (message.p1() == 60 && message.p2() > 0) {
               running = 0;
            }
         }
      }
   }

   return 0;
}



// md5sum: 039d02d9762c1876ba5ec1d1a3da7fe5 sysexio.cpp [20050403]

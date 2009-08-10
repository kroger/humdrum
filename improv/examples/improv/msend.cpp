//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Dec  9 20:42:10 PST 1997
// Last Modified: Tue Jan 27 21:16:02 GMT-0800 1998
// Filename:      ...sig/doc/examples/improv/improv/msend/msend.cpp
// Syntax:        C++; improv
//
// Description:   Send the specified MIDI command to a MIDI synthesizer.
//                If the message is a note-on message, then the note
//                will be turned off most likely by the MIDI driver
//                of the computer.
//

#include "sigControl.h"
#include <stdlib.h>
#include <ctype.h>

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

int atohd(const char* aNumber);
void exitUsage(const char* command);


int main(int argc, char* argv[]) {
   int port = 0;

   if (argc < 2) exitUsage(argv[0]);

   MidiOutput midi;
   if (midi.getNumPorts() <= port) {
      cout << "Error: highest available port is: " << midi.getNumPorts()-1
           << endl;
      exit(1);
   }
   midi.setPort(port);
   midi.open();
   int MIDIbyte;
   for (int i=1; i<argc; i++) {
      MIDIbyte = atohd(argv[i]);
      if (MIDIbyte < 0 || MIDIbyte > 255) exitUsage(argv[0]);
      midi.send(MIDIbyte);
   }
   midi.close();
   return 0;
}



int atohd(const char* aNumber) {
   if (aNumber[0] == '0' && tolower(aNumber[1]) == 'x') {
      return (int)strtol(aNumber, (char**)NULL, 16);
   } else {
      return atoi(aNumber);
   }
}



void exitUsage(const char* command) {
      cout << endl;
      cout << "Sends MIDI bytes to a synthesizer.\n";
      cout << endl;
      cout << "Usage: " << command << " port byte1 [byte2 ...]\n";
      cout << endl;
      cout << "   port = MIDI output port (usually 0)\n";
      cout << "   byte = [0..255] MIDI data or command\n";
      cout << "          [0x0..0xff]\n";
      cout << endl;
      exit(1);
}



// md5sum: 65e81d1ab26edfe34e29d0b019896522 msend.cpp [20050403]

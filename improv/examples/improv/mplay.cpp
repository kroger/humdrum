//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Dec  1 22:01:22 PST 1997
// Last Modified: Tue Jan 27 21:10:05 GMT-0800 1998
// Filename:      ...sig/doc/examples/improv/improv/mplay/mplay.cpp
// Syntax:        C++; improv
//
// Description:   Plays the specified note on a MIDI synthesizer.
//                most MIDI drivers will automatically turn the note
//                off when the program is exited.
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
   int channel = 0;
   int keyno = 0; 
   int velocity = 0;

   if (argc == 5) {
      port = atohd(argv[1]);
      channel = atohd(argv[2]);
      keyno = atohd(argv[3]);
      velocity = atohd(argv[4]);
   } else {
      exitUsage(argv[0]);
   }

   if (channel  < 0 || channel  > 15)   exitUsage(argv[0]);
   if (keyno    < 0 || keyno    > 127)  exitUsage(argv[0]);
   if (velocity < 0 || velocity > 127)  exitUsage(argv[0]);


   MidiOutput midi;
   if (midi.getNumPorts() <= port) {
      cout << "Error: highest available port is: " << midi.getNumPorts()-1
           << endl;
      exit(1);
   }
   midi.setPort(port);
   midi.open();
   midi.setTrace(1);
   midi.play(channel, keyno, velocity);
   midi.close();
   cout << endl;
   
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
      cout << "Sends a MIDI note message to a synthesizer.\n";
      cout << endl;
      cout << "Usage: " << command 
           << " port channel keyNumber velocity" << endl;
      cout << endl;
      cout << "   port      = MIDI output port (usually 0)\n";
      cout << "   channel   = [0..15] the MIDI channel to play note on.\n";
      cout << "               default is 0.\n";
      cout << "   keyNumber = [0..127] MIDI key, Middle C = 60, C# = 61, ...\n";
      cout << "   velocity  = [0..127] attack velocity of the note, 0 = off.\n";
      cout << "               default is 127.\n";
      cout << endl;
      exit(1);
}



// md5sum: d420fcbaeacd7b45a1338f536462ade2 mplay.cpp [20050403]

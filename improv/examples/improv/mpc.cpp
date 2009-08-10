//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Dec  1 22:01:22 PST 1997
// Last Modified: Tue Jan 27 21:04:04 GMT-0800 1998
// Filename:      ...sig/doc/examples/sigmidi/mpc/mpc.cpp
// Syntax:        C++; improv
//
// Description:   Sends a patch change (program change) to set the 
//                specified instrument number on a MIDI synthesizer.
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
   int timbre = 0;

   if (argc == 4) {
      port = atohd(argv[1]);
      timbre = atohd(argv[2]);
      channel = atohd(argv[3]);
   } else {
      exitUsage(argv[0]);
   }

   if (channel  < 0 || channel  > 15)   exitUsage(argv[0]);
   if (timbre   < 0 || timbre > 127)    exitUsage(argv[0]);

   MidiOutput midi;
   if (midi.getNumPorts() <= port) {
      cout << "Error: highest available port is: " << midi.getNumPorts()-1
           << endl;
      exit(1);
   }
   midi.setPort(port);
   midi.open();
   midi.setTrace(1);
   midi.pc(channel, timbre);
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
      cout << "Sends a patch-change command to a MIDI synthesizer which\n";
      cout << "changes the channel instrument.\n";
      cout << endl;
      cout << "Usage: " << command << " port channel timbre" << endl;
      cout << endl;
      cout << "   port      = MIDI output port (usually 0)\n";
      cout << "   channel   = [0..15] the MIDI channel to play note on.\n";
      cout << "               default is 0.\n";
      cout << "   timbre    = [0..127] timbre number to change to.\n";
      cout << endl;
      exit(1);
}


// md5sum: a8c09d859a36be1c3dc7ccff93df181e mpc.cpp [20050403]

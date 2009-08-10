//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Dec  9 20:10:26 PST 1997
// Last Modified: Tue Jan 27 20:44:46 GMT-0800 1998
// Filename:      ...sig/doc/examples/improv/improv/mcont/mcont.cpp
// Syntax:        C++; improv
//  
// Description:   This program sends out a continuous control
//                message to a MIDI synthsizer.
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
   int controller = 0;
   int data = 0;

   if (argc == 5) {
      port = atohd(argv[1]);
      channel = atohd(argv[2]);
      controller = atohd(argv[3]);
      data = atohd(argv[4]);
   } else {
      exitUsage(argv[0]);
   }

   if (controller < 0 || controller > 127)  exitUsage(argv[0]);
   if (data < 0       || data > 127)        exitUsage(argv[0]);
   if (channel  < 0   || channel  > 15)     exitUsage(argv[0]);

   MidiOutput midi;
   if (midi.getNumPorts() <= port) {
      cout << "Error: highest available port is: " << midi.getNumPorts()-1
           << endl;
      exit(1);
   }
   midi.setPort(port);
   midi.open();
   midi.setTrace(1);
   midi.cont(channel, controller, data);
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
      cout << "Sends a MIDI controller message to a synthesizer.\n";
      cout << endl;
      cout << "Usage: " << command 
           << " port channel controller data\n";
      cout << endl;
      cout << "   port       = MIDI output port (usually 0)\n";
      cout << "   channel    = [0..15] the MIDI channel to play note on.\n";
      cout << "                default is 0.\n";
      cout << "   controller = [0..127] controller number\n";
      cout << "   data       = [0..127] controller data\n";
      cout << "                default is 0.\n";
      cout << endl;
      exit(1);
}



// md5sum: 663dc1dc2aaffb994f22c20f09c40e90 mcont.cpp [20050403]

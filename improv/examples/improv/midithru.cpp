//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 4 June 2002
// Last Modified: 4 June 2002
// Filename:      ...sig/doc/examples/improv/improv/midithru/midithru.cpp
// Syntax:        C++; improv
//
// Description:   Have the computer act as a Patch Bay.  Send MIDI input
//                data to any MIDI output port.  MIDI input can be sent
//                to multiple output.
//

#include "improv.h"

typedef Array<int> ArrayInt;
typedef Array<MidiOutPort> MidiOutputArray;

// function declarations:
void displayPatchBay(Array<ArrayInt>& connections);
void sendData(MidiMessage& message, MidiOutPort& output, 
    MidiInput& input);

/////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   int incount = MidiInPort::getNumPorts();
   int outcount = MidiOutPort::getNumPorts();

   Array<ArrayInt> connections;
   connections.setSize(incount);
   int i, j;
   for (i=0; i<incount; i++) {
      connections[i].setSize(outcount);
      connections[i].setAll(1);
   }
 
   // temporary connections for now:
   //  connections[0][2] = 0;
   //  connections[1][2] = 0;
 
   cout << "There are " << incount  << " MIDI inputs" << endl;
   cout << "and " << outcount << " MIDI outputs" << endl;

   MidiMessage message;
   Array<MidiInput> midiins;
   for (i=0; i<incount; i++) {
      midiins[i].setPort(i);
      midiins[i].open();
   }

   Array<MidiOutPort> midiouts;
   midiouts.setSize(outcount);
   for (i=0; i<outcount; i++) {
      midiouts[i].setPort(i);
      midiouts[i].open();
      cout << "opened MIDI output number " << i << endl;
   }
 
   connections[0][3] = 0;

   displayPatchBay(connections);
   int done = 0;
   while (!done) {
      for (i=0; i<incount; i++) {
         while (midiins[i].getCount() > 0) {
            message = midiins[i].extract();
            if (message.p1() == A0) {
               done = 1;
            }
            cout << "[" << i << ":";
            for (j=0; j<outcount; j++) {
               if (connections[i][j]) {
                  cout << j << " ";
                  sendData(message, midiouts[j], midiins[i]);
               }
            } 
            cout << "]" << flush;
         }
      }
   }

   return 0;
}


//////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// sendData --
//

void sendData(MidiMessage& m, MidiOutPort& output, 
     MidiInput& input) {
   int count = m.getParameterCount();
   if (count >= 0 && count <= 2) {
      if (count >= 0) output.rawsend(m.p0());
      if (count >= 1) output.rawsend(m.p0(), m.p1());
      if (count >= 2) output.rawsend(m.p0(), m.p1(), m.p2());
   }
}


//////////////////////////////
//
// displayPatchBay --
//

void displayPatchBay(Array<ArrayInt>& connections) {
   int i, j;
   for (i=0; i<connections.getSize(); i++) {
	  cout << "input " << i;
	  if (i>9) {
         cout << " : ";
      } else {
         cout << "  : ";
      }
      for (j=0; j<connections[i].getSize(); j++) {
         cout << connections[i][j];
      }
      cout << endl;
   }
}



// md5sum: 7f162ea0d9b51b188d4b3607a7248373 midithru.cpp [20050403]

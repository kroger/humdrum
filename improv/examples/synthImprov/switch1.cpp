//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 24 January  1998
// Last Modified: 09 November 1998
// Filename:      ...sig/doc/examples/improv/synthImprov/switch1/switch1.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: Inverts pitch and attack parameters of keys played on 
//     the synthesizer keyboard.  On the computer keyboard, the 
//     keys "-" and "=" will change the timbre of the notes which
//     are being switched.
//

#include "synthImprov.h" 


/*----------------- beginning of improvization algorithms ---------------*/

int channel = 0;         // MIDI channel (offset 0) on which to play notes 
MidiMessage message;     // for reading keyno and velocity (and time)
int offNote[128] = {0};  // for keeping track of note-offs
int instrument = GM_VIBRAPHONE; // initial timbre of output notes


/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();
   psl(
   "  Switch - by Craig Stuart Sapp <craig@ccrma.stanford.edu> - 24 Jan 1998");
   psl("");
   psl("  Description: Inverts pitch and attack parameters of keys ");
   psl("    played on the synthesizer keyboard.");
   printintermediateline();
   psl("  User commands:");
   psl("     \"-\" = decrement timbre no.   \"=\" = increment timbre no. ");
   printboxbottom();
} 



void initialization(void) { 
   synth.pc(channel, instrument);   // send patch change to synthesizer
}

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

void mainloopalgorithms(void) { 
 
   // process all of the note messages waiting in the input buffer:
   while (synth.getNoteCount() > 0) {
      message = synth.extractNote();
      if (message.p2() == 0) {              // incoming note-off message
         synth.play(channel, offNote[message.p1()], 0);
      } else {                              // incoming note-on  message
         synth.play(channel, message.p2(), message.p1());
         offNote[message.p1()] = message.p2();
      }
   }

}
      

/*-------------------- triggered algorithms -----------------------------*/

void keyboardchar(int key) { 
   switch (key) {
      case '-':        // decrement timbre
         instrument = (instrument - 1 + 127) % 128;
         cout << "Instrument: " << instrument << endl;
         synth.pc(channel, instrument);
         break;
      case '=':        // increment timbre
         instrument = (instrument + 1) % 128;
         cout << "Instrument: " << instrument << endl;
         synth.pc(channel, instrument);
         break;
   }
}



/*------------------ end improvization algorithms -----------------------*/


// md5sum: a9e0e341e44cd9d928d7aadfe44ae9c3 switch1.cpp [20050403]

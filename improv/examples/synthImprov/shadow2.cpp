//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 9 January 1998
// Last Modified: 9 January 1998
// Filename:      ...sig/doc/examples/improv/synthImprov/shadow2/shadow2.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: echos a note played on the synthesizer keyboard
//     Unlike shadow.cpp, this program also imitates the duration
//     of the echoing note and is polyphonic.

#include "synthImprov.h" 


/*----------------- beginning of improvization algorithms ---------------*/

int shadowDistance = 12;     // number of half-steps to place shadow at
int shadowSide     = 1;      // 1 = higher, -1 = lower
MidiMessage noteMessage;     // for reading keyno and velocity (and time)
int shadowNote = 0;          // note to play as a shadow
int keyray[128] = {0};       // keyboard array which keeps track of notes

/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();
   psl(
"   Shadow2 -- by Craig Stuart Sapp <craig@ccrma.stanford.edu> -- 5 Jan 1998");
   psl("");
   psl(
"   This program will echo the input notes at a certain distance from the");
   psl(
"   original notes.  The range of the shadow is from 1 to 24 half-steps on");
   psl("   either side of the original note.");
   printintermediateline();
   psl("  User commands:");
   psl("      \"-\" = decrease shadow length   \"=\" = increase shadow length");
   psl("      \"\\\" = switch shadow side");
   printboxbottom();
} 

void initialization(void) { }

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

void interpretNote(MidiMessage message);

void mainloopalgorithms(void) { 
   while (synth.getNoteCount() > 0) {
      interpretNote(synth.extractNote());
   }
}

void interpretNote(MidiMessage message) {
   int newNote = message.p1();
   int newVelocity = message.p2();

   if (newVelocity != 0) {
      // pick a shadow note
      shadowNote = newNote + shadowSide * shadowDistance;
      // ignore the shadow note if it is out of keyboard range:
      if (shadowNote > 0 && shadowNote < 128) {
         // turn off any old note (should never happen, but be safe):
         if (keyray[newNote] != 0) {
            synth.play(0, keyray[newNote], 0);
         }
         keyray[newNote] = shadowNote;
         synth.play(0, shadowNote, newVelocity);
      }
   } else {
      // a note off command
      if (keyray[newNote] != 0) {
         synth.play(0, keyray[newNote], 0);
         keyray[newNote] = 0;
      }
   }
}
      


/*-------------------- triggered algorithms -----------------------------*/

void keyboardchar(int key) { 
   switch (key) {
      case '-': 
         shadowDistance--; 
         if (shadowDistance < 1) shadowDistance = 1;
         cout << "Shadow = " << shadowSide * shadowDistance << endl;
         break;
      case '=':
         shadowDistance++; 
         if (shadowDistance > 24) shadowDistance = 24;
         cout << "Shadow = " << shadowSide * shadowDistance << endl;
         break;
      case '\\':
         shadowSide *= -1;
         cout << "Shadow = " << shadowSide * shadowDistance << endl;
         break;
      default:
         charsynth(key);
   }
}


/*------------------ end improvization algorithms -----------------------*/


/*
   some functions and variables provided by the support program

   program_change(channel, instrument); -------- sets the timbre for a channel
   control_change(channel, controller, value); - sends a continuous controller 
   note_on(channel, keynumber, keyvelocity); --- plays a MIDI note
   note_off(channel, keynumber); --------------- same as note_on with 0 vel.
   raw_send(channel, command, data1, data2); --- send some midi command
   long t_time; -------------------------------- current time in milliseconds

*/

// md5sum: 6c9160fd4f09124be14ede7eec7ecbe9 shadow2.cpp [20050403]

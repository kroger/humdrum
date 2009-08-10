//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 5 January 1998
// Last Modified: 5 January 1998
// Filename:      ...sig/doc/examples/improv/synthImprov/shadow1/shadow1.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: echos a note played on the synthesizer keyboard
//

#include "synthImprov.h" 


/*----------------- beginning of improvization algorithms ---------------*/

int shadowDistance = 12;     // number of half-steps to place shadow at
int shadowSide     = 1;      // 1 = higher, -1 = lower
MidiMessage noteMessage;     // for reading keyno and velocity (and time)
int shadowNote = 0;          // note to play as a shadow

/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();
   psl(
"   Shadow -- by Craig Stuart Sapp <craig@ccrma.stanford.edu> -- 5 Jan 1998");
   psl("");
   psl(
"   This program will echo the input notes at a certain distance from the");
   psl(
"   original notes.  The range of the shadow is from 1 to 24 half-steps on");
   psl(
"   either side of the original note.");
   printintermediateline();
   psl(
"  User commands:");
   psl(
"      \"-\" = decrease shadow length   \"=\" = increase shadow length ");
   psl(
"      \"\\\" = switch shadow side                                     ");
   printboxbottom();
} 

void initialization(void) { }

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

void mainloopalgorithms(void) { 
   while (synth.getNoteCount() > 0) {
      noteMessage = synth.extractNote();
      if (noteMessage.p2() != 0) {              // ignore note off commands
         synth.play(0, shadowNote, 0);          // turn off last note
         shadowNote = noteMessage.p1() + shadowSide * shadowDistance;
         if (shadowNote > 0 && shadowNote < 128) {
            synth.play(0, shadowNote, noteMessage.p2());
         } else {
            shadowNote = 0;
         }
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


// md5sum: 8833a5542523f14b45681482d324b7bf shadow1.cpp [20050403]

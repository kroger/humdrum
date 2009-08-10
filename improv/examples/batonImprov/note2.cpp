//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 26 December 1997
// Last Modified: Sat Jan 16 11:03:46 PST 1999
// Filename:      ...improv/doc/examples/batonImprov/note2/note2.cpp
// Syntax:        C++; batonImprov 2.0
//  
// Description: Plays a note when a baton stick is triggered
//	Both sticks behave the same.  Only one note at a time.
//	Unlike the note.cpp program, this program maps the
//	x-axis to pitch.  Whack is mapped to attack velocity.
//

#include "batonImprov.h" 


/*----------------- beginning of improvization algorithms ---------------*/

int note1;       // the current note being played (keep track for turning off).
int note2;       // the current note being played on stick 2
int channel = 1; // the MIDI channel to use for playing notes
int instrument;  // the current instrument being played on


/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   cout 
   << "\nThis program demonstrates how to play notes with baton triggers.\n"
   << "Only one pitch is played at a time.\n"
   << "\nThe b14+ and b15+ buttons control the instrument being played.\n"
   << "\nPress the key \"X\" to turn on the MIDI out trace and see what\n"
   << "MIDI commands are being sent to the synthesizer.\n"
   << endl;
}

void initialization(void) { 
   note1 = note2 = 0;
   instrument = 0;
   program_change(channel, instrument);
}

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

void mainloopalgorithms(void) { }


/*-------------------- triggered algorithms -----------------------------*/

void stick1trig(void) { 
   cout << "Stick 1 triggered:  \txw1 =  " << xw1 << "\tyw1 =  " << yw1 
        << "\twh1 =  " << wh1 << endl;
   note_off(channel, note1);
   note1 = xw1;
   note_on(channel, note1, yw1);
}

void stick2trig(void) { 
   cout << "Stick 2 triggered:  \txw2 =  " << xw2 << "\tyw2 =  " << yw2 
        << "\twh2 =  " << wh2 << endl;
   note_off(channel, note2);
   note2 = xw2;
   note_on(channel, note2, yw2);
}

void b14plustrig(void) {
   instrument++;
   if (instrument > 127) {
      instrument = 0;
   }
   program_change(channel, instrument);
   cout << "instrument = " << GeneralMidiName(instrument)
        << " (" << instrument << ")" << endl;
}

void b15plustrig(void) { 
   instrument--;
   if (instrument < 0) {
      instrument = 127;
   }
   program_change(channel, instrument);
   cout << "instrument = " << GeneralMidiName(instrument)
        << " (" << instrument << ")" << endl;
}

void b14minusuptrig(void) { }

void b14minusdowntrig(void) { }

void b15minusuptrig(void) { }

void b15minusdowntrig(void) { }

void keyboardchar(int key) { charsynth(key); }


/*------------------ end improvization algorithms -----------------------*/


/*
   some functions and variables provided by the support program

   program_change(channel, instrument); -------- sets the timbre for a channel
   control_change(channel, controller, value); - sends a continuous controller 
   note_on(channel, keynumber, keyvelocity); --- plays a MIDI note
   note_off(channel, keynumber); --------------- same as note_on with 0 vel.
   raw_send(command, data1, data2); ------------ send some midi command
   long t_time; -------------------------------- current time in milliseconds
   short pt1, pt2, pt3, pt4; ------------------- current pot (dial) positions
   short xt1,yt1,zt1,xt2,yt2,zt2; -------------- current stick positions
   short wh1, wh2; ----------------------------- whack strength of a trigger
   short xw1, yw1, xw2, yw2; ------------------- x and y positions at trigger
   short hit1, set1, hit2, set2; --------------- hit and set levels of sticks
   short whack1,whack2; ------------------------ registers set by stick triggers
   long  trigtime1, trigtime2; ----------------- times of stick triggers
*/


// md5sum: b2fba8da1bf0600b5dc27fb3d8d28a7e note2.cpp [20050403]

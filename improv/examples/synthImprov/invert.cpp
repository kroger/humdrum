//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Dec 14 14:08:59 PST 2002
// Last Modified: Sat Dec 14 14:09:02 PST 2002
// Filename:      ...sig/doc/examples/improv/invert/invert.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: invert the MIDI key coming in.
//

#include "synthImprov.h" 


/*----------------- beginning of improvization algorithms ---------------*/

/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << "plays music backwards" << endl;
} 



//////////////////////////////
//
// initialization -- this function is called by the improv
//     interface once at the start of the program.  Put items
//     here which need to be initialized at the beginning of
//     the program.
//

void initialization(void) { }



//////////////////////////////
//
// finishup -- this function is called by the improv interface
//     whenever the program is exited.  Put items here which
//     need to be taken care of when the program is finished.
//

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

//////////////////////////////
//
// mainloopalgorithms -- this function is called by the improv interface
//   continuously while the program is running.  The global variable t_time
//   which stores the current time is set just before this function is
//   called and remains constant while in this functions.
//

MidiMessage m;
void mainloopalgorithms(void) { 
   while (synth.getNoteCount() > 0) {
      m = synth.extractNote();
      synth.send(m.p0(), 127 - m.p1(), m.p2());
   }
}


/*-------------------- triggered algorithms -----------------------------*/

///////////////////////////////
//
// keyboardchar -- this function is called by the improv interface
//     whenever a key is pressed on the computer keyboard.
//     Put commands here which will be executed when a key is
//     pressed on the computer keyboard.
//

void keyboardchar(int key) { }


/*------------------ end improvization algorithms -----------------------*/

// md5sum: df1a8d38b0742e486fe1e2aa9b417b88 invert.cpp [20050403]

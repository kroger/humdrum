//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jul  2 11:55:32 PDT 1999
// Last Modified: Fri Jul  2 19:14:05 PDT 1999
// Filename:      ...sig/doc/examples/improv/batonImprov/midiclock.cpp
// Syntax:        C++; batonImprov 2.0
//  
// Description:   This program will start and stop MIDI timing information.
//                With this program you can make the computer an external
//                time keeper for an external sequencer connected via MIDI.
//                

#include "batonImprov.h"     
#include <ctype.h>

CONVERT_SYNTH_TO_BATON_IMPROV  // define empty radio baton functions

/*----------------- beginning of improvization algorithms ---------------*/

// global variables:
double   tempo = 60.0 * 24;  // for keeping track of the current tempo.
                             // There are 24 MIDI clocks per quarter note.
SigTimer timer;              // keep track of when to send MIDI clock messages
int      status = 0;         // boolean for sending MIDI clocks or not
Voice    voice;              // for playing notes from computer keyboard

/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << 
   "\nMIDI clock: start/stop the MIDI clock and control the tempo.\n"
   "\n"
   "This program will start and stop MIDI timing information.\n"
   "With this program you can make the computer an external\n"
   "time keeper for an external sequencer connected via MIDI.\n"
   "\n"
   "Commands:\n"
   "   \"-\" = start MIDI clock\n"
   "   \"=\" = stop MIDI clock\n"
   "   \"[\" = slow down tempo\n"
   "   \"]\" = speed up tempo\n"
   "   \"0\"-\"9\" = play a note\n"
   "   \" \" = turn off a note\n"
   << endl;
} 



//////////////////////////////
//
// initialization -- this function is called by the improv
//     interface once at the start of the program.  Put items
//     here which need to be initialized at the beginning of
//     the program.
//

void initialization(void) { 
   timer.setTempo(tempo);         // initialize the tempo of the timer
   timer.reset();                 // start the timer at time zero
}



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

void mainloopalgorithms(void) { 
   if (status && timer.expired()) { // if we are sending timing bytes and
                                    // it is time to send a time byte:
      synth.send(0xf8);             // send a MIDI clock message
      timer.update();               // set up the next time that a 
                                    // clock message will be sent.
      if (timer.expired() > 100) {
         // if the computer is too far behind 
         // (more than 4 quarter notes) playing 
         // MIDI clocks skip over some of them:
         timer.reset();
      }
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

void keyboardchar(int key) { 
   switch (key) {
      case '-':                                   // start MIDI clock
         cout << "starting MIDI clock" << endl;
         synth.send(0xfa);
         status = 1;
         timer.reset();
         break;
      case '=':                                   // stop MIDI clock
         cout << "stopping MIDI clock" << endl;
         status = 0;
         synth.send(0xfc);
         break;
      case '[':                                   // slow down tempo
         cout << "slowing down tempo: ";
         tempo *= 0.95;
         timer.setTempo(tempo);
         cout << tempo/24 << endl;
         break;
     case ']':                                    // speed up tempo
         cout << "speeding up tempo: ";
         tempo *= 1.05;
         timer.setTempo(tempo);
         cout << tempo/24 << endl;
         break;
      case ' ':                                   // turn off note
         voice.off();
         break;
   }

   if (isdigit(key)) {
      voice.play(48 + key - '0', 127);
   }
}


/*------------------ end improvization algorithms -----------------------*/

// md5sum: 6076e3c3508f7bea02d08aef71a4ba7b midiclock.cpp [20050403]

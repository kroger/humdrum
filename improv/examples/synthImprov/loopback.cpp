//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Nov 19 17:08:53  2002
// Last Modified: Tue Nov 19 17:08:56  2002
// Filename:      ...sig/doc/examples/improv/synthImprov/loopback.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: This program is used to time the loopback of data from
//              the PianoDisc so that a program can be written to ignore
//              this loopback so that interactive programs can be written
//              for the PianoDisc.  See pdiscal.cpp for final calibration
//              program.
//

#include "synthImprov.h"      // includes the default Win95 console interface
                              // for the synthImprov environment

int basetime = 0;
Voice voice;
Voice voice2;
/*----------------- beginning of improvization algorithms ---------------*/

/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << 
      "This program is used to time the loopback of data from\n"
      "the PianoDisc so that a program can be written to ignore\n"
      "this loopback so that interactive programs can be written\n"
      "for the PianoDisc." << endl;
} 



//////////////////////////////
//
// initialization -- this function is called by the improv
//     interface once at the start of the program.  Put items
//     here which need to be initialized at the beginning of
//     the program.
//

void initialization(void) { 
   voice.setPort(synth.getInputPort());
   voice2.setPort(synth.getInputPort());
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

MidiMessage message;
void mainloopalgorithms(void) { 
   while (synth.getNoteCount() > 0) {
      message = synth.extractNote();
      cout << "Received Message: " << message << " at time: " << t_time-basetime << endl;
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
      case '0': 
         basetime = t_time;
         cout << "Computer Sends a note at time: " << t_time << endl;
         voice.play(0, A0, 100);
         break;
      case '1':
         basetime = t_time;
         cout << "Computer Sends a note at time: " << t_time << endl;
         voice.play(0, A1, 100);
         break;
     case '2': 
         basetime = t_time;
         cout << "Computer Sends a note at time: " << t_time << endl;
         voice.play(0, A2, 100);
         break;
      case '3':
         basetime = t_time;
         cout << "Computer Sends a note at time: " << t_time << endl;
         voice.play(0, C4, 100);
         voice2.play(0, E4, 100);
         break;
     case '4':
         basetime = t_time;
         cout << "Computer Sends a note at time: " << t_time << endl;
         voice.play(0, A4, 100);
         break;
     case '5': 
         basetime = t_time;
         cout << "Computer Sends a note at time: " << t_time << endl;
         voice.play(0, A5, 100);
         break;
      case '6':
         basetime = t_time;
         cout << "Computer Sends a note at time: " << t_time << endl;
         voice.play(0, A6, 100);
         break;
      case '7':
         basetime = t_time;
         cout << "Computer Sends a note at time: " << t_time << endl;
         voice.play(0, A7, 100);
         break;
      case 'f':
         cout << "Turning off the note" << endl;
         voice.off();
         voice2.off();
         break;
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

// md5sum: ab7ee0f4604b2835bc1e23a2783db0f3 loopback.cpp [20050403]

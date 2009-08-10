//
// Programmer:    Leland Stanford, Jr. <leland@stanford.edu>
// Creation Date: 29 February 1999
// Last Modified: 29 February 1999
// Filename:      ...sig/doc/examples/improv/synthImprov/synthTemplate.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: This is a template which you fill with algorithms 
//              in the following functions to create a synthImprov program.
//

#include "synthImprov.h"      // includes the default Win95 console interface
                              // for the synthImprov environment


/*----------------- beginning of improvization algorithms ---------------*/

/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << "Write the program description here" << endl;
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

void mainloopalgorithms(void) { }


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


/*
   some functions and variables provided by the support program

   program_change(channel, instrument); -------- sets the timbre for a channel
   control_change(channel, controller, value); - sends a continuous controller 
   note_on(channel, keynumber, keyvelocity); --- plays a MIDI note
   note_off(channel, keynumber); --------------- same as note_on with 0 vel.
   raw_send(channel, command, data1, data2); --- send some midi command
   long t_time; -------------------------------- current time in milliseconds

*/

// md5sum: 0a8f6631899ce83ed0fbbceee204f44a synthTemplate.cpp [20050403]

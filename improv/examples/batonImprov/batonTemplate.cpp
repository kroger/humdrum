//
// Programmer:    Leland Stanford <leland@stanford.edu>
// Creation Date: 29 February 1999
// Last Modified: 29 February 1999
// Filename:      ...sig/doc/examples/all/batonTemplate.cpp
// Syntax:        C++; batonImprov 2.0
//  
// Description: Fill in the following functions to create an improv program.
//

#include "batonImprov.h" 


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

//////////////////////////////
//
// stick1trig -- this function is called automatically whenever
//   a baton stick #1 trigger is received.
//

void stick1trig(void) { }



//////////////////////////////
//
// stick2trig -- this function is called automatically whenever
//   a baton stick #2 trigger is received.
//

void stick2trig(void) { }



//////////////////////////////
//
// b14plustrig -- this function is called automatically whenever
//   the b14+ button is pressed.
//

void b14plustrig(void) { }



//////////////////////////////
//
// b15plustrig -- this function is called automatically whenever
//   the b15+ button is pressed.
//

void b15plustrig(void) { }




//////////////////////////////
//
// b14minusuptrig -- this function is called automatically whenever
//   the b14- foot trigger is pressed.
//

void b14minusuptrig(void) { }



//////////////////////////////
//
// b14minusdowntrig -- this function is called automatically whenever
//   the b14- foot trigger is released.
//

void b14minusdowntrig(void) { }



//////////////////////////////
//
// b15minusuptrig -- this function is called automatically whenever
//   the b15- foot trigger is pressed.
//

void b15minusuptrig(void) { }



//////////////////////////////
//
// b15minusdowntrig -- this function is called automatically whenever
//   the b15- foot trigger is released.
//

void b15minusdowntrig(void) { }



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

// md5sum: a789c07ca2a11890b2eebb12888c3dcf batonTemplate.cpp [20050403]

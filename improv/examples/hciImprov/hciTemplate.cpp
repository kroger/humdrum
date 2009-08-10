//
// Programmer:    Leland Stanford, Jr. <leland@stanford.edu>
// Creation Date: 29 February 2001
// Last Modified: 29 February 2001
// Filename:      ...sig/doc/examples/improv/hciImprov/hciTemplate.cpp
// Syntax:        C++; hciImprov 2.3
//  
// Description: This is a template which you fill with algorithms 
//              in the following functions to create an hciImprov program.
//

#include "hciImprov.h"      

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
// mididata -- this function is called by the improv interface
//     whenever a MIDI message comes from MIDI input.
//     Put commands here which will be executed when 
//     a MIDI message arrives from the sensors.
//
// int  intime   = arrival time of the MIDI message
// int  command  = command byte of the MIDI message
// int  p1       = first parameter of the MIDI message
// int  p2       = second parameter of the MIDI message
//

void mididata(int intime, int command, int p1, int p2) { }



///////////////////////////////
//
// keyboardchar -- this function is called by the improv interface
//     whenever a key is pressed on the computer keyboard.
//     Put commands here which will be executed when a key is
//     pressed on the computer keyboard.
//

void keyboardchar(int key) { }


/*------------------ end improvization algorithms -----------------------*/


// md5sum: c55162572ab32ad7bd7c5861a2b15868 hciTemplate.cpp [20050403]

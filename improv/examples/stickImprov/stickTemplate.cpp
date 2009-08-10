//
// Programmer:    Leland Stanford, Jr. <leland@stanford.edu>
// Creation Date: 30 February 2000
// Last Modified: 30 February 2000
// Filename:      ...sig/examples/all/stickImprov.cpp
// Syntax:        C++; stickImprov 2.3
//  
// Description: This is a template which you fill with algorithms 
//              in the following functions to create a stickImprov program.
//

#include "stickImprov.h"      // includes the default Win95/Linux console 
                              // interface for the stickImprov environment


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



///////////////////////////////
//
// stickresponse -- this function is called whenever a complete set
//     of stick positoin/pressure values have been received.
//

void stickresponse(void) { }



///////////////////////////////
//
// fsr 1 triggers -- fsr1ontrig will be called when the FSR is
//    pressed, and fsr1offtrig will be called when the FSR is
//    released.
//

void fsr1ontrig(void) { }
void fsr1offtrig(void) { }



///////////////////////////////
//
// fsr 2 triggers -- fsr2ontrig will be called when the FSR is
//    pressed, and fsr2offtrig will be called when the FSR is
//    released.
//

void fsr2ontrig(void) { }
void fsr2offtrig(void) { }



///////////////////////////////
//
// fsr 3 triggers -- fsr3ontrig will be called when the FSR is
//    pressed, and fsr3offtrig will be called when the FSR is
//    released.
//

void fsr3ontrig(void) { }
void fsr3offtrig(void) { }


/*------------------ end improvization algorithms -----------------------*/



// md5sum: b9e1f756a8d03c0542ab8059e65f88b4 stickTemplate.cpp [20050403]

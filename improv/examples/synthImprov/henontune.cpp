//
// Programmer:    Craig Sapp <craig@ccrma.stanford.edu>
// Programmer:    Juan Reyes <juanig@ccrma.stanford.edu>
// Creation Date: 12 April 2000
// Last Modified: 15 April 2000
// Filename:      ...sig/doc/examples/improv/synthImprov/henontune.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description:   This is a template which you fill with algorithms 
//                in the following functions to create a synthImprov program.
//
// See Also:      http://henon.sapp.org 
//                for a non-realtime musical implementation of the Henon map.
//

#include "synthImprov.h"      // includes the default Win95 console interface
                              // for the synthImprov environment

void startHenon(int prealpha, int prebeta);
int nextHenon(void);

/*----------------- beginning of improvization algorithms ---------------*/

MidiMessage message;
double y = 0.0;
double x = 0.0;
double newx;
double newy;
int key;
int nextnotetime;
double alpha = -1.56693;
double beta = -0.011811;

Voice voice;

/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << 
   "Some good results:\n" 
   "Alpha = -1.81496 (58)   Beta = -0.208661   (37) \n"
   "Alpha = -1.56693 (79)   Beta = -0.011811   (62) \n"
   "Alpha = -1.85039 (55)   Beta =  0.00393701 (64) \n"
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
   nextnotetime = t_time;
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
   if (synth.getNoteCount() > 0) {
      message = synth.extractNote();
      if (message.p2() != 0) {
         startHenon(message.p1(), message.p2());
      }
   }

   if (nextnotetime < t_time) {
      nextnotetime += 100;
      key = nextHenon();
      voice.play(key, 64);
   }
}


void startHenon(int prealpha, int prebeta) {
    alpha = prealpha/127.0 * 1.5 - 2.5;
    beta = prebeta/127.0 - 0.5;
    x = 0.0;
    y = 0.0;
    cout << "Alpha = " << alpha << " (" << prealpha << ")"
         << "\tBeta = " << beta << " (" << prebeta << ")" << endl;
}

int nextHenon(void) {
   newx = 1 + alpha * x * x + beta * y;
   newy = x;
   x = newx;
   y = newy;
   int output;
   output = (int)((x + 1.0)/2.0 * 127.0 + 0.5);
   if (output < 0) {
      output = 0;
   }
   if (output > 127) {
      output = 127;
   }
   return output;
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

// md5sum: 0f8aa5ec5c782a4439742a50fe3dc213 henontune.cpp [20050403]

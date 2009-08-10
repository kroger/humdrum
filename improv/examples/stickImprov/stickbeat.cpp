//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 17 14:40:30 PDT 2000
// Last Modified: Mon Jul 17 14:40:33 PDT 2000
// Filename:      ...sig/examples/all/drumstick.cpp
// Syntax:        C++; stickImprov 2.3
//  
// Description: This is a template which you fill with algorithms 
//              in the following functions to create a stickImprov program.
//

#include "stickImprov.h"      // includes the default Win95/Linux console 
                              // interface for the stickImprov environment
#ifndef OLDCPP
   #include <fstream>
   using namespace std;
#else
   #include <fstream.h>
#endif

/*----------------- beginning of improvization algorithms ---------------*/

int channel = 9;              // channel to play notes on
Voice voice;                  // controls note-offs
SigTimer beat;                // for controlling the beat location
int beattime = 0;             // last time a beat was played
int current = 0;              // current beat subdivision being played
fstream infile;

int lowtarget = 3;            // position that defines the lower beat
int hitarget = 123;           // position that defined the upper beat
int clocktick = -1;           // current time location
double ipredict = -1;         // instantaneous prediction
int slowdirection = 0;        // low-passed direction info
double alpha = 0.0;           // for linear prediction
double beta = 0.0;            // for linear prediction

CircularBuffer<int> location;  // history of the positions
CircularBuffer<int> direction; // history of the positions

// function definitions:
void analyzeTempo(void);
int determineDirection(CircularBuffer<int>& history, int curr);

/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << 
   " STICKBEAT -- Craig Stuart Sapp <craig@ccrma.stanford.edu>, 19 July 2000\n";
   cout << endl;
} 



//////////////////////////////
//
// initialization -- this function is called by the improv
//     interface once at the start of the program.  Put items
//     here which need to be initialized at the beginning of
//     the program.
//

void initialization(void) { 
   voice.setChannel(channel);
   infile.open("test.dat", ios::out);
   location.setSize(1000);
   direction.setSize(1000);
   location.reset();
   direction.reset();
   location.insert(0);
   direction.insert(0);
}


//////////////////////////////
//
// finishup -- this function is called by the improv interface
//     whenever the program is exited.  Put items here which
//     need to be taken care of when the program is finished.
//

void finishup(void) { 
   infile.close();
}


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


void stickresponse(void) { 
   clocktick++;

   // don't monitor for beats if not pressing FSR3
   if (stick.getState(3) != 1) {
      return;
   }

   location.insert(stick.loc3_7());

   if (location[0] < location[1]) {
      direction.insert(-1);
   } else if (location[0] > location[1]) {
      direction.insert(1);
   } else {
      direction.insert(0);
   }

   slowdirection = determineDirection(direction, location[0]);
   if (slowdirection == 0) {
      // don't predict if uncertain of direction
      ipredict = -100;
   } else if (slowdirection == 1) {
      alpha = hitarget - location[1];
      beta  = hitarget - location[0];
      ipredict = alpha / (alpha - beta); // + clocktick;
      // going up on the fsr
   } else if (slowdirection == -1) {
      // going down on the fsr
      alpha = location[1] - lowtarget;
      beta  = location[0] - lowtarget;
      ipredict = alpha / (alpha - beta); // + clocktick;
   } else {
      cout << "Unknown direction: " << slowdirection << endl;
      exit(1);
   }

   cout << clocktick     << "\t" 
        << location[0]   << "\t" 
//      << direction[0]  << "\t" 
        << slowdirection << "\t"
        << ipredict 
        << endl;

   infile << clocktick   << "\t" 
        << location[0]   << "\t" 
//      << direction[0]  << "\t" 
        << slowdirection << "\t"
        << ipredict 
        << endl;


}



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



//////////////////////////////
//
// determineDirection -- determine the direction that you are heading
//

int determineDirection(CircularBuffer<int>& history, int curr) {
   int highregion = 115;    // place likely to be 0
   int lowregion = 12;      // place likely to be 0
   int decision = 0;

   int a = history[0];
   int b = history[1];
   int c = history[2];
   int d = history[3];

   if (a == b && a == c) {
      decision = a;
   } else {
      decision = 0;
   }

   if (decision == 0 && curr > highregion || curr < lowregion) {
      if (a == b && a == c && a == d) {
         decision = a;
      } else {
         decision = 0;
      }
   }

   return decision;
}





// md5sum: 82df9097e3ef8e68aab8120b2d4a8fc2 stickbeat.cpp [20050403]

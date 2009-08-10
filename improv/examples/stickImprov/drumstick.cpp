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

Voice voice1;
Voice voice2;
Voice voice3;
int instrument = 0;

/*----------------- beginning of improvization algorithms ---------------*/

#define DRUM_CHANNEL 9         /* General MIDI channel 10 for percussion */

/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << 
   " DRUMSTICK -- Craig Stuart Sapp <craig@ccrma.stanford.edu>, 17 July 2000\n";
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
   voice1.setChannel(0);
   voice1.pc(instrument);
   voice2.setChannel(1);
   voice2.pc(42);
   voice3.setChannel(DRUM_CHANNEL);
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

void mainloopalgorithms(void) { }


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
      case '[':   
         instrument++;
         if (instrument > 127) {
            instrument = 127;
         }
         cout << "Instrument: " << instrument << endl;
         voice1.pc(instrument);
         break; 
      case ']':   
         instrument--;
         if (instrument < 0) {
            instrument = 0;
         }
         cout << "Instrument: " << instrument << endl;
         voice1.pc(instrument);
         break; 
   }

 }



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

void fsr1ontrig(void) { 
   int notenumber = stick.s1ps(0, 24);
   int velocity   = stick.s1fs(30, 127);
   voice1.play(48 + notenumber, velocity);
}
void fsr1offtrig(void) { 
   voice1.off();
}



///////////////////////////////
//
// fsr 2 triggers -- fsr2ontrig will be called when the FSR is
//    pressed, and fsr2offtrig will be called when the FSR is
//    released.
//

void fsr2ontrig(void) {
   int notenumber = stick.s2ps(0, 24);
   int velocity   = stick.s2fs(30, 127);
   voice2.play(36 + notenumber, velocity);
}
void fsr2offtrig(void) {
   voice2.off();
}



///////////////////////////////
//
// fsr 3 triggers -- fsr3ontrig will be called when the FSR is
//    pressed, and fsr3offtrig will be called when the FSR is
//    released.
//

void fsr3ontrig(void) { 
   int notenumber = stick.s3ps(0, 24);
   int velocity   = stick.s3fs(30, 127);
   voice3.play(72 + notenumber, velocity);
}
void fsr3offtrig(void) { 
   voice3.off();
}


/*------------------ end improvization algorithms -----------------------*/



// md5sum: 92be13ad6d6aa67e84eaf20087a3d6c5 drumstick.cpp [20050403]

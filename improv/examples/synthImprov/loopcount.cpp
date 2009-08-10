//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul  2 15:23:31 PDT 2000
// Last Modified: Sun Jul  2 15:23:35 PDT 2000
// Filename:      ...sig/doc/examples/all/loopcount/loopcount.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: Counts the number of times the mainloop is called during
//	one second.  You can controll the speed of the looping up to the
//	maximum possible looping speed.
//

#include "batonImprov.h" 


/*----------------- beginning of improvization algorithms ---------------*/

SigTimer timer;               // for keeping track of time
double tempo = 60.0;          // tempo for fixed tempo algorithms
int counter = 0;              // count the loop iterations
double idletime = 0.0;        // pause between loops in milliseconds

int getMaxCycle(void);

/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << 
   "LOOPCOUNT -- Craig Stuart Sapp <craig@ccrma.stanford.edu> July 2000\n"
   " Description: Counts the number of times the mainloop is called during\n"
   "	one second.  You can controll the speed of the looping up to the\n"
   "	maximum possible looping speed.\n"
   " Commands:\n"
   "    < = slow down report tempo     > = speed up report tempo \n"
   "    [ = speed up idle duration     ] = slow down idle duration \n"
   "    r = perform a raw loop count\n"
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
   timer.setTempo(tempo);
   timer.reset();
   eventIdler.setPeriod(idletime);
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
   if (timer.expired()) {
      cout << "beat loop count = " << counter << endl;
      counter = 0;
      timer.update();
   } else {
      counter++;
   }
}


/*-------------------- triggered algorithms -----------------------------*/

void stick1trig(void) { }
void stick2trig(void) { }
void b14plustrig(void) { }
void b15plustrig(void) { }
void b14minusuptrig(void) { }
void b14minusdowntrig(void) { }
void b15minusuptrig(void) { }
void b15minusdowntrig(void) { }


///////////////////////////////
//
// keyboardchar -- this function is called by the improv interface
//     whenever a key is pressed on the computer keyboard.
//     Put commands here which will be executed when a key is
//     pressed on the computer keyboard.
//

void keyboardchar(int key) { 
   switch (key) {
      case '<':      // slow the tempo down for fixed tempo options
      case ',':      // slow the tempo down for fixed tempo options
         tempo /= 1.02;
         timer.setTempo(tempo);
         cout << "Fixed tempo set to: " << tempo << endl;
         break;
      case '>':      // speed up the tempo down for fixed tempo options
      case '.':      // speed up the tempo down for fixed tempo options
         tempo *= 1.02;
         timer.setTempo(tempo);
         cout << "Fixed tempo set to: " << tempo << endl;
         break;
      case '[':      // lower idle duration
      case '{':      // lower idle duration
         idletime -= 1.0;
         if (idletime < 0.0) {
            idletime = 0.0;
         }
         eventIdler.setPeriod(idletime);
         cout << "Idle duration is " << idletime << " milliseconds" << endl;
         break;
      case ']':      // raise idle duration
      case '}':      // raise idle duration
         idletime += 1.0;
         eventIdler.setPeriod(idletime);
         cout << "Idle duration is " << idletime << " milliseconds" << endl;
         break;
      case 'r':      // raw loop cycle count
         cout << "Maximum possible cyclecount = " << getMaxCycle() << endl;
   }
}


/*------------------ end improvization algorithms -----------------------*/



//////////////////////////////
//
// getMaxCycle -- returns the cycle count that occurs when
//    nothing is done except continuously access the 
//    CPUs cyclecount register in the SigTimer::expired() function.
//

int getMaxCycle(void) {
   SigTimer x;
   x.setTempo(60);
   x.reset();
   int counter = 0;
   while (!x.expired()) {
      counter++;
   }
   return counter;
}








// md5sum: 60e0f70cde269f4f3b48f1fcec863a92 loopcount.cpp [20050403]

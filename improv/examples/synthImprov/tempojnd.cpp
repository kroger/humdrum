//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jul  5 09:27:14 PDT 2000
// Last Modified: Sun Jul  9 14:54:38 PDT 2000
// Filename:      ...sig/doc/examples/all/tempojnd/tempojnd.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description:   This program is used to collect data for
//		  tempo JND experiments.
//		  The experiment is as follows:
//		  A. Play a set of evenly spaced notes at Tempo 1
//		  B. Play an empty beat (or more)
//		  C. Play a set of evenly spaced notes at Tempo 2
//

#include "synthImprov.h"      

#define NOTE     60
#define VELOCITY 64

#define MINSETA     2   /* Minimum beat count for set A */
#define MINSETB     2   /* Minimum beat count for set B */
#define MINSILENCE  1   /* Minimum spacing between set A and B */

/*----------------- beginning of improvization algorithms ---------------*/

int stage = 0;                 // 0=off,1=set1,2=silence,3=set2,4=answerwait
int laststage = 0;             // 0=off,1=set1,2=silence,3=set2,4=answerwait

int setAtotal = 10;            // number of beats to play in set A
int setAcount = 10;            // the number of beats at Tempo 1
int setAcurrent = 0;           // current location in set A

int silencetotal = 1;          // number of beats to play in set A
int silencecount = 10;         // the number of empty beats after set A
int silencecurrent = 0;        // current location in silence

int setBtotal = 10;            // number of beats to play in set A
int setBcount = 10;            // the number of beats at Tempo 2
int setBcurrent = 0;           // current location in set B

double startTempo;             // the start tempo for the experiment
double startPeriod;            // converted value of startTempo
double variation = 1.0;       // duration difference from old tempo

double nexteventtime = 0;      // time to perform another event in milliseconds
int runCount = 0;              // the trial number
int answerCount = 0;           // answers for the trials
int correctQ = 0;              // true if the last answer was correct

Voice voice;                   // for playing notes


// function declarations:
void performEvent(void);
void playbeat(int state);
double generateVariation(double lastone, int correct);
int randomsign(void);


/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << 
   "TEMPOJND -- Craig Stuart Sapp <craig@ccrma.stanford.edu>, July 2000\n"
   "Commands:\n"
   "   space = initiate a test sequence\n"
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
   voice.setPort(synth.getOutputPort());
   options.define("t|tempo=d:80", "starting tempo");
   options.define("o|output=s:/tmp/tempojnd.dat", "data reporting file");
   options.process();

//   outputfile = options.getString("output");
   startTempo = options.getDouble("tempo");
   cout << "Base tempo is: " << startTempo << endl;
   startPeriod = 60000.0/startTempo;

   eventIdler.setPeriod(0);     // high quality timing
   srand48(time(NULL) * int(mainTimer.getPeriodCount() * 1000000));

   cout << "Press the space bar to begin" << endl;
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
   if (stage && nexteventtime <= t_time) {
      performEvent();
   }
}



//////////////////////////////
//
// performEvent --
//

void performEvent(void) {
   switch (stage) {
      case 1:                 // tempo set A
         setAcurrent++;
         if (setAcurrent >= setAtotal) {
            setAcurrent = 0;
            stage = 2;
         }
         playbeat(1);
         nexteventtime  = nexteventtime + startPeriod;
         break;
      case 2:                 // tempo silence
         silencecurrent++;
         if (silencecurrent >= silencetotal) {
            silencecurrent = 0;
            stage = 3;
         }
         playbeat(0);
         nexteventtime  = nexteventtime + startPeriod;
         break;
      case 3:                 // tempo set B
         setBcurrent++;
         if (setBcurrent >= setBtotal) {
            stage = 4;
            cout << "\nPress 1 if set B is slower than set A" << endl;
            cout << "Press 2 if set B is faster than set A" << endl;
            cout << "Press r to hear the trial again" << endl;
         }
         playbeat(1);
         nexteventtime  = nexteventtime + startPeriod + variation;
         break;
      case 4:
         // do nothing: waiting for an answer
         break;
   }
}


//////////////////////////////
//
// playbeat --
//

void playbeat(int state) {
   if (state) {
     voice.play(NOTE, VELOCITY);
   } else {
      voice.off();
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
      case 'r':     // replay the test sequence
         stage = 1;
         setAcurrent = 0;
         setBcurrent = 0;
         silencecurrent = 0;
         nexteventtime = t_time;
         cout << "Restarting the trial" << endl;
         break;
      case ' ':     // start a test sequence
         if (runCount != answerCount || stage != 0) {
            cout << "Cannot start a new trial" << endl;
            break;
         } else {
            cout << "Starting trial: " << runCount + 1 << endl;
         }
         runCount++;
         variation = generateVariation(variation, correctQ);
         cout << "Delta = " << fabs(variation) << " milliseconds " << endl;
         stage = 1;
         setAcurrent = 0;
         setBcurrent = 0;
         silencecurrent = 0;
         nexteventtime = t_time;
         break;
      case '1':
         cout << "Set B was chosen as slower than set A" << endl;
         answerCount++;
         cout << "Press the spacebar to start a new test" << endl;
         if (variation > 0) {
            correctQ = 1;
            cout << "Correct" << endl;
         } else {
            correctQ = 0;
            cout << "Incorrect" << endl;
         }
         nexteventtime = t_time + 1000000;
         stage = 0;
         break;
      case '2':
         cout << "Set B was chosen as faster than set A" << endl;
         answerCount++;
         cout << "Press the spacebar to start a new test" << endl;
         if (variation < 0) {
            correctQ = 1;
            cout << "Correct" << endl;
         } else {
            correctQ = 0;
            cout << "Incorrect" << endl;
         }
         stage = 0;
         nexteventtime = t_time + 1000000;
         break;
      case 'q':   // length set A
         setAtotal++;
         cout << "Beats in set A = " << setAtotal << endl;
         break;
      case 'a':   // shorten set A
         setAtotal--;
         if (setAtotal < MINSETA) {
            setAtotal = MINSETA;
         }
         cout << "Beats in set A = " << setAtotal << endl;
         break;
      case 'w':   // length set B
         setBtotal++;
         cout << "Beats in set B = " << setBtotal << endl;
         break;
      case 's':   // shorten set B
         setBtotal--;
         if (setBtotal < MINSETB) {
            setBtotal = MINSETB;
         }
         cout << "Beats in set B = " << setAtotal << endl;
         break;
   }

}


//////////////////////////////
//
// generateVariation -- generate a new tempo variation based on the 
//    last answer -- if correct, then decrease the difference,
//    otherwise, increase the variation.
//

double generateVariation(double lastone, int correct) {
   double increment = 1.0;
   if (correct) {
      return randomsign() * (fabs(lastone) - increment);
   } else {
      return randomsign() * (fabs(lastone) + increment);
   }
}



//////////////////////////////
//
// generateVariation -- generate a new tempo variation based on the 
//    last answer -- if correct, then decrease the difference,
//    otherwise, increase the variation.
//

int randomsign(void) {
   if (drand48() < 0.50) {
      return -1;
   } else {
      return 1;
   }
}



/*------------------ end improvization algorithms -----------------------*/

// md5sum: 7daf168760d98281896f3cd184b72eac tempojnd.cpp [20050403]

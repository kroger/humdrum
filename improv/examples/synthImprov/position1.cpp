//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Oct 26 17:00:30 PST 1998
// Last Modified: Thurs 30 Oct 1998
// Last Modified: 22 Nov 1998 (changed Nidaq class to be multi channel input)
// Filename:      ...sig/doc/examples/improv/synthImprov/position1/position1.cpp
// Syntax:        C++; synthImprov 2.0; (Windows 95 only)
//  
// Description: plays notes according to 1D input data coming from NIDAQ card.
//

#include "synthImprov.h"    /* basic MIDI I/O interface with MIDI synthesizer */
#include "Nidaq.h"          /* interface with NIDAQ Card (Windows 95 only)    */


/*----------------- beginning of improvization algorithms ---------------*/

// global variables:
Nidaq       sensor;          // NIDAQ card interface for sensor
int         channel = 0;     // the NIDAQ channel to listen to
Voice       voice;           // for MIDI output (auto handles note off messages)

double      min = 10.0;      // miniumum value from NIDAQ, adapts
double      max = -10.0;     // maximum value from NIDAQ, adapts

int         keyno;           // MIDI keynumber to play
int         velocity;        // MIDI attack velocity
int         minDur = 20;     // minimum duration of output notes

SigTimer    displayTimer;    // for displaying positions on the screen
int         display = 0;     // boolean for display sensor data on screen


// non-interface function declarations:
void makeNote(void);


/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface 
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << '\n';
   printboxtop();
   pcl("POSITION: by Craig Stuart Sapp <craig@ccrma.stanford.edu> 26 Oct 1998");
   pcl("version 22 November 1998");
   printintermediateline();
   psl(" NIDAQ device initially on channel 0. Initial data range is");
   psl(" 2.4 to 2.5, but range adapts.  So go through whole range of sensor");
   psl(" to set the software range.");
   psl(" Commands:");
   psl(" \"0\"-\"9\" = select NIDAQ analog input channel");
   psl(" \"d\" = toggle sensor display.");
   printboxbottom();
} 



//////////////////////////////
//
// initialization -- this function is called by the improv
//     intervace once at the start of the program.  Put items
//     here which need to be initialized at the beginning of 
//     the program.
//

void initialization(void) { 
   sensor.initialize(options.argv()); // start CVIRTE stuff for NIDAQ card
   sensor.setPollPeriod(1);      // check for new data every 1 millisecond   
   sensor.setFrameSize(1);       // data transfer size from NIDAQ card
   sensor.setModeLatest();       // just look at most recent data in buffer
   sensor.setSrate(500);         // set NIDAQ sampling rate to X Hz
   sensor.activateAllChannels(); // turn on all channels for sampling 
   cout << "starting data aquisition ... " << flush;
   sensor.start();              // start aquiring data from NIDAQ card
   cout << "ready." << endl;

   voice.setPort(synth.getOutputPort());   // specify output port of voice
   voice.setChannel(0);         // specify output chan of voice

   displayTimer.setPeriod(200); // display position every X milliseconds
}



//////////////////////////////
//
// finishup -- this function is called by the improv interface
//     whenever the function is exited.  Put items here which
//     need to be taken care of when the program is finished.
//

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/


//////////////////////////////
//
// mainloopalgorithms -- this function is called by the improv interface
//   continuously while the program is running.  The variable t_time 
//   which stores the current time is set just before this function is 
//   called and remains constant while in this functions.
//

void mainloopalgorithms(void) { 
   sensor.checkPoll();   // see if it is time to check for new data frame
   makeNote();           // check if it is time to play a MIDI note

   if (display && displayTimer.expired()) {
      displayTimer.reset();
      cout << "\rmin = " << min << "\tmax = " << max << "\tcurrent = "
           << sensor[channel][0] << "\t\t" << flush;
   }
}



//////////////////////////////
//
// makeNote -- plays a new note whenever sensor detects
//     a new note value, but limit the minimum time between
//     notes.

void makeNote(void) {
   // if the duration of the previous note is long enough and
   // the sensor is not at its min or max value, then play a note
   // if the interval distance has been traversed since the last
   // note which was played.
   if (t_time - voice.getOnTime() > minDur) {

      // adjust the range of the sensor values if necessary:
      if (sensor[channel][0] > max)   max = sensor[channel][0];
      if (sensor[channel][0] < min)   min = sensor[channel][0];
 
      // keyno is the note to be played.  It is in the range from 0 to 127
      keyno = 127 - (int)((sensor[channel][0] - min) / (max - min) * 127);
   
      // limit the range of keyno to valid range, in case max/min not correct
      if (keyno < 0)  keyno = 0;
      if (keyno > 127)  keyno = 127;

      if (keyno < 2 || keyno > 125) { // turn off notes out of sensor range
         voice.off();
      } else {
         velocity = 120;
         voice.play(keyno, velocity);
      }
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

   // select a NIDAQ channel if the key is a number from 0 to 9:
   if (isdigit(key)) {
      cout << "\nresetting data aquisition ... " << flush;
      channel = key - '0';
      cout << "done." << endl;
      cout << "Channel " << key - '0' << " selected" << endl;
      return;
   }

   switch (key) {
      case 'd':          // toggle display of sensor data on screen
         display = !display;
         break;
   }
}


/*------------------ end improvization algorithms -----------------------*/


// md5sum: 3a695dbb5cacad35e71d160268d345ec position1.cpp [20050403]

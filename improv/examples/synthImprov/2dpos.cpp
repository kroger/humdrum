//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 22 Nov 1998
// Last Modified: 22 Nov 1998
// Last Modified: Sat Mar 27 18:14:41 PST 1999
// Filename:      ...sig/doc/examples/improv/synthImprov/2dpos/2dpos.cpp
// Syntax:        C++; synthImprov 2.0; NIDAQ (Windows 95 only)
//  
// Description: plays notes according to 2D input data coming from NIDAQ card.
//     the keys (pitches) of the output are based on one channel of 
//     input and their velocities are based on another channel of input
//

#include "synthImprov.h"    /* basic MIDI I/O interface with MIDI synthesizer */
#include "Nidaq.h"          /* interface with NIDAQ Card (Windows 95 only)    */


/*----------------- beginning of improvization algorithms ---------------*/

// global variables:
Nidaq       sensor;          // NIDAQ card interface for sensor
int         keychan = 0;     // the NIDAQ channel to listen to for keys
int         velchan = 0;     // the NIDAQ channel to listen to for velocities
int         keyselect = 0;   // boolean to listen to keyboard for key chann
int         velselect = 0;   // boolean to listen to keyboard for vel chann
int         keydir = 1;      // for changing sensor direction of key numbers
int         veldir = 1;      // for changing sensor direction of velocities
Voice       voice;           // for MIDI output (auto handles note off messages)
int         inst = GM_VIBRAPHONE; // initial instrument to play notes on
int         sustain = 0;     // for sustain pedal

double      keymin = 10.0;   // miniumum value from NIDAQ for keys, adapts
double      keymax = -10.0;  // maximum value from NIDAQ for keys, adapts
double      velmin = 10.0;   // miniumum value from NIDAQ for vel, adapts
double      velmax = -10.0;  // maximum value from NIDAQ for vel, adapts

int         keyno;           // MIDI keynumber to play
int         velocity;        // MIDI attack velocity
int         minDur = 40;     // minimum duration of output notes

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
   pcl("2dpos: by Craig Stuart Sapp <craig@ccrma.stanford.edu> 22 Nov 1998");
   printintermediateline();
   psl(" NIDAQ device initially on channel 0. Initial sensor range will ");
   psl(" adapt as sensor moves.  So go through whole range of sensor");
   psl(" to fix the software range.");
   psl(" Commands:");
   psl(" \"0\"-\"9\" = select NIDAQ analog input channel");
   psl(" type \"k\" before number to select key channel");
   psl(" type \"v\" before number to select velocity channel");
   psl(" \"-\", \"=\" = change instruments, \" \" = sustain pedal");
   psl(" \"z\" = reset key sensor range, \"x\" = reset vel sensor range");
   psl(" \"a\" = switch key sensor range, \"s\" = switch vel sensor range");
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
   voice.pc(inst);

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
      cout << "\r\t\t\t\t\t\t\t\t\t";
      cout << "\rkey0= " << (int)(100 * keymin)/100.0 
           << "  keyx= " << (int)(100 * keymax)/100.0
           << "  keyc= " << (int)(100 * sensor[keychan][0])/100.0 
           << "   \tvel0= " << (int)(100 * velmin)/100.0 
           << "  velx= " << (int)(100 * velmax)/100.0 
           << "  velc= " << (int)(100 * sensor[velchan][0])/100.0  
           << "   " << flush;
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
      if (sensor[keychan][0] > keymax)   keymax = sensor[keychan][0];
      if (sensor[keychan][0] < keymin)   keymin = sensor[keychan][0];
      if (sensor[velchan][0] > velmax)   velmax = sensor[velchan][0];
      if (sensor[velchan][0] < velmin)   velmin = sensor[velchan][0];
 
      // keyno is the note to be played.  It is in the range from 0 to 127
      keyno = (int)((sensor[keychan][0] - keymin) / 
            (keymax - keymin) * 127);
      if (keydir == -1) {
         keyno = 127 - keyno;
      }
 
      velocity = (int)((sensor[velchan][0] - velmin) / 
            (velmax - velmin) * 127);
      if (veldir == -1) {
         velocity = 127 - velocity;
      }

      // limit the range of keyno to valid range, in case max/min not correct
      if (keyno < 0)  keyno = 0;
      if (keyno > 127)  keyno = 127;
      if (velocity < 0)  velocity = 0;
      if (velocity > 127)  velocity = 127;

      if (keyno < 2 || keyno > 125) { // turn off notes out of sensor range
         voice.off();
      } else {
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
      if (keyselect) {
         keychan = key - '0';
         keyselect = 0;
         cout << "\nKey Channel " << key - '0' << " selected" << endl;
      }
      if (velselect) {
         velchan = key - '0';
         velselect = 0;
         cout << "\nVelocity Channel " << key - '0' << " selected" << endl;
      }
      return;
   }

   switch (key) {
      case 'd':          // toggle display of sensor data on screen
         display = !display;
         break;
      case 'k':          // toggle key number channel select option
         keyselect = !keyselect;
         break;
      case 'v':          // toggle velocity channel select option
         velselect = !velselect;
         break;
      case 'z':          // reset the key sensor range
         keymax = -10.0;
         keymin = 10.0;
         break;
      case 'x':          // reset the velocity sensor range
         velmax = -10.0;
         velmin = 10.0;
         break;
      case 'a':          // reverse the key axis of the sensor data
         keydir = -keydir;
         break;
      case 's':          // reverse the veolcity sensor direction
         veldir = -veldir;
         break;    
      case '-':          // decrement instrument
         inst--;
         if (inst < 0)   inst = 127;
         voice.pc(inst);
         cout << "\ninst = " << inst << endl;
         break;
      case '=':          // increment instrument
         inst++;
         if (inst > 127)   inst = 0;
         voice.pc(inst);
         cout << "\ninst = " << inst << endl;
         break;
      case ' ':          // sustain pedal on/off
         sustain = !sustain;
         voice.sustain(sustain);
         cout << "\nsustain = " << sustain << endl;
         break;
   }
}


/*------------------ end improvization algorithms -----------------------*/


// md5sum: 7039d0f8b8c44c88411361e93b892257 2dpos.cpp [20050403]

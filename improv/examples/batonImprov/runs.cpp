//
// Programmer:    Max Mathews <mvm@ccrma.stanford.edu>
// Modified by:   Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jan 12 12:44:12 PST 1996 
// Last Modified: Wed Jun 23 10:48:31 PDT 1999
// Filename:      ...improv/doc/examples/batonImprov/runs/runs.cpp
// Syntax:        C++; batonImprov 2.0
//
// Description:   runs -- play a sequence of notes pitch follows x1, y1p->tempo
//

#include "batonImprov.h"

/*--------------------beginning improvization algorithms------------------*/

// Global Variables
Voice voice;
int play = 0;           // boolean for arpeggios on/off
int noteon = 0;         // boolean for whether a note is being played
int interval = 7;       // the arpeggio interval
int sustain = 0;        // boolean for turning sustain on/off
int keyno = 64;         // the current note being played
int batonkey = 64;      // the note that the baton's want to play
int nextnotetime = 0;   // the time to play the next note
int dur = 50;           // the duration between notes being played

// User functions
void programhelp(void);

/*---------------- Documentation functions -------------------------------*/

void description(void) {
   cout
   << "______________________________________________________________ \n"
   << "| Description:                                                |\n"
   << "|    A sequence of notes follows baton 1 around the surface.  |\n"
   << "| Hit the surface with baton 1 to start the arpeggios.  Lift  |\n"
   << "| baton 1 to stop the arpeggios.                              |\n"
   << "---------------------------------------------------------------\n"
   << endl;
}

/*---------------- Maintainance functions --------------------------------*/

void initialization(void) {
   description();              // print the description of the program
   program_change(1,0);        // set MIDI channel 1 instrument to piano
   control_change(1,7,127);    // set the main volume on chan 1 to max
   control_change(1,11,127);   // set the secondary vol on chan 1 to max
   control_change(1,10,64);    // set pan location to center of stereo
   voice.setChannel(1);
}

void finishup() {
   if (noteon) {               // if a note is on
      note_on(1, keyno, 0);    // turn it off
   }
}

/*---------------- Main loop functions -----------------------------------*/

void mainloopalgorithms(void){
   if (play == 1 && zt1 < 70) {      // if playing notes but baton 1 too high
      play = 0;                        // stop playing notes
      voice.off();                     // turn off the current note
   }

   if (whack1y && zt1 > 90) {        // if stick 1 triggered and baton 1 low
      keyno = xt1/2 + 30;              // set note in range 30 to 93
      voice.play(keyno,yt1);           // play that note
      noteon = 1;                      // indicate that a note is being played
      nextnotetime = t_time+dur;       // calculate next time to play a note
      play = 1;                        // indicate that we are playing runs
      whack1y = 0;                     // deactivate whack1y trigger value
   }

   if (zt2 > 70){                    // if stick 2 is low
      if (!sustain) {                  // if not sustaining
         sustain = 1;                  // set sustain flag on
         control_change(1,64,127);     // turn on MIDI sustain
      }
   } else if (sustain) {               // if stick 2 is high and sustain is on
      sustain = 0;                     // set sustain flag to off
      control_change(1,64,0);          // turn off MIDI sustain
   }

   if (play && t_time>nextnotetime) {  // if runs on and time for a new note

      dur = 500-(xt2*4);               // set duration to range 50 to 500 ms
      if (dur < 50) {                  // minimum duration is 50 ms
         dur = 50;
      }
      interval = yt2/8;                // set intervl in range 0..16 half-steps

      batonkey = xt1/2+30;             // get baton's note in range 30 to 93
      if (noteon) {                    // if a note is already on
         note_on(1, keyno, 0);         // turn off that note
         noteon=0;                     // set the noteon flag to off
      }
      if (batonkey < keyno) {          // if baton note is less than currnt one
         keyno -= interval;            // subtract interval from last note
      } else {
         keyno += interval;            // otherwise add interval to last note
      }
      note_on(1, keyno, yt1);          // play the new note
      noteon = 1;                      // set the noteon flag to on
      nextnotetime = t_time + dur;     // determine next time to play a note
   }
}

/*--------------------triggered algorithms--------------------------------*/

void stick1trig(void){}
void stick2trig(void){}
void b14plustrig(void){}
void b15plustrig(void){}
void b14minusuptrig(void){}
void b14minusdowntrig(void){}
void b15minusuptrig(void){}
void b15minusdowntrig(void){}
void keyboardchar(int key){}

/*------------------end improvization algorithms----*/

void programhelp(void) {
   cout
   << ">                                                              \n"
   << "> Baton 1:  trig: Starts playing of runs                       \n"
   << ">           x1p: Pitch of notes                                \n"
   << ">           y1p: Loudness                                      \n"
   << ">           z1p: on/off                                        \n"
   << "> Baton 2:  trig: Stops playing of runs                        \n"
   << ">           x2p: tempo                                         \n"
   << ">           y2p: interval                                      \n"
   << ">           z2p: sustain on/off                                \n"
   << ">                                                              \n"
   << ">   +b14: Command mode                                         \n"
   << ">   +b15: This help screen                                     \n"
   << ">                                                              \n"
   << endl;
   ;
}

// md5sum: 10a38f08dbc960896e7b2d8f71e62b1a runs.cpp [20050403]

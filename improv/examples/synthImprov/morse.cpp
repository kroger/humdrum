//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Mar 12 12:52:36 PST 2001
// Last Modified: Tue Mar 13 14:10:10 PST 2001
// Filename:      ...sig/doc/examples/improv/synthImprov/morse/morse.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: Convert Morse Code taps from a MIDI keyboard into text.
//

#include "synthImprov.h"  
#include <string.h>

/* 
  A = .-	B = -...	C = -.-.        D = -..
  E = .		F = ..-.	G = --.	        H = ....
  I = ..	J = .---	K = -.-	        L = .-..
  M = --	N = -.		O = ---	        P = .--.
  Q = --.-	R = .-.		S = ...	        T = -
  U = ..-       V = ...-        W = .--         X = -..-
  Y = -.--	Z = --..

     In Morse code, timing is important to ensure that a coherent
   message is received.  The actual length of a dot can be as long as
   you like, but obviously it should be short enough that messages can
   be sent quickly and long enough to be heard over great distances.
   Once this standard unit of time has been established, a dash should
   last three times as long as a dot.  The pause between individual dots
   and dashes should take as long as a single dot, the pause between two
   characters should take as long as a dash and the pause between two
   words should be twice as long as a dash (six times as long as a dot).

*/



/*----------------- beginning of improvization algorithms ---------------*/

MidiMessage message;
CircularBuffer<int> attacktimes; // time of attack
CircularBuffer<int> durations;   // time between attack and release
CircularBuffer<int> iois;        // time between attacks
CircularBuffer<int> rois;        // rest time between notes
Array<int> notestates;           // 0 = off, 1 = on
Array<int> actiontimes;          // attack or release time

double speed = 80.0;             // basic tempo for detecting beats
char buffer[1024] = {0};         // storage for identifying events
int interpreted = 0;             // 1 = data already interpreted
int tdelta = -1000;

// function declarations:
void  processEvent(int key, int vel, int mtime);
char  identifyEvent(const char* string);
char* interpretEvent(void);


/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
  cout << 
"     In Morse code, timing is important to ensure that a coherent\n"
"   message is received.  The actual length of a dot can be as long as\n"
"   you like, but obviously it should be short enough that messages can\n"
"   be sent quickly and long enough to be heard over great distances.\n"
"   Once this standard unit of time has been established, a dash should\n"
"   last three times as long as a dot.  The pause between individual dots\n"
"   and dashes should take as long as a single dot, the pause between two\n"
"   characters should take as long as a dash and the pause between two\n"
"   words should be twice as long as a dash (six times as long as a dot).\n\n"
"     A = .-       B = -...        C = -.-.     D = -..\n"
"     E = .        F = ..-.        G = --.      H = ....\n"
"     I = ..       J = .---        K = -.-	L = .-..\n"
"     M = --       N = -.          O = ---	P = .--.\n"
"     Q = --.-     R = .-.         S = ...	T = -\n"
"     U = .._      V = ...-        W = .--      X = -..-\n"
"     Y = -.--     Z = --..\n"
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
   notestates.setSize(128);
   actiontimes.setSize(128);
   notestates.allowGrowth(0);
   actiontimes.allowGrowth(0);
   notestates.zero();
   actiontimes.zero();
   
   attacktimes.setSize(1004);
   durations.setSize(1003);
   iois.setSize(1002);
   rois.setSize(1001);
   attacktimes.reset();
   durations.reset();
   iois.reset();
   rois.reset();
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
      int key = message.p1();
      int vel = message.p2();
      if ((message.p0() & 0xf0) == 0x80) {
         vel = 0;
      }
      if (tdelta == -1000) {
         tdelta = t_time - message.time;
      }
      if (key == 21) {
         if (vel != 0) {
            cout << endl;
         }
      } else {
         processEvent(key, vel, message.time);
      }
   }

   if ((!interpreted) && (attacktimes.getSize() > 0) && 
       ((t_time - tdelta - attacktimes[0]) > speed * 6)) {
      // cout << "Interpreting event " << endl;
      char *event = interpretEvent();
      if (strcmp(event, "") != 0) {
         cout << event << "\t= " << identifyEvent(event) << endl;
      }
      interpreted = 1;
   }
}



//////////////////////////////
//
// interpretEvent --
//

char* interpretEvent(void) {
   int eventcount = 1;
   int ioicount = iois.getCount();
   int i = 0;
   while (i < ioicount && i < iois.getCount()) {
      if (iois[i] < 6 * speed) {
         eventcount++;
      } else {
         break;
      }
      i++;
   }

   // cout << "Number of events = " << eventcount << endl;

   if (eventcount < 1 || eventcount > 100) {
      // nothing to do
      interpreted = 1;
      buffer[0] = '\0';
      return buffer;
   }

   // event count keeps track of how many events occurred,
   // now identify each even as a dot or a dash 
   buffer[0] = '\0';
   for (i=eventcount-1; i>0; i--) {
      if (i-1 < iois.getSize() && iois[i-1] < speed * 3) {
         strcat(buffer, ".");
      } else {
         strcat(buffer, "-");
      }
   }

   if (durations.getCount() == 0) {
      cout << "Duration size is zero" << endl;
      return buffer;
   } else {
      // cout << "Duration = " << durations[0] << endl;
   }

   // the last item does not have an ioi, so check the duration:
   if (durations[0] < speed * 1.5) {
      strcat(buffer, ".");
   } else {
      strcat(buffer, "-");
   }

   return buffer;
}



//////////////////////////////
//
// processEvent --
//

void processEvent(int key, int vel, int mtime) {
   if (vel > 0 && attacktimes.getSize() > 0) {
      iois.insert(mtime - attacktimes[0]);
      rois.insert(mtime - attacktimes[0] + durations[0]);
   }

   if (vel > 0) {
      attacktimes.insert(mtime);
      notestates[key] = 1;
   } else {
      durations.insert(mtime - actiontimes[key]);
      notestates[key] = 0;
   }

   actiontimes[key] = mtime;

   interpreted = 0;
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
      case ' ': cout << endl;
   }
}


/*------------------ end improvization algorithms -----------------------*/



////////////////////////////////
//
// identifyEvent --
//

char identifyEvent(const char* string) {
   if (strcmp(string, ".-"  ) == 0)   return 'A';
   if (strcmp(string, "-...") == 0)   return 'B';
   if (strcmp(string, "-.-.") == 0)   return 'C';
   if (strcmp(string, "-.." ) == 0)   return 'D';
   if (strcmp(string, "."   ) == 0)   return 'E';
   if (strcmp(string, "..-.") == 0)   return 'F';
   if (strcmp(string, "--." ) == 0)   return 'G';
   if (strcmp(string, "....") == 0)   return 'H';
   if (strcmp(string, ".."  ) == 0)   return 'I';
   if (strcmp(string, ".---") == 0)   return 'J';
   if (strcmp(string, "-.-" ) == 0)   return 'K';
   if (strcmp(string, ".-..") == 0)   return 'L';
   if (strcmp(string, "--"  ) == 0)   return 'M';
   if (strcmp(string, "-."  ) == 0)   return 'N';
   if (strcmp(string, "---" ) == 0)   return 'O';
   if (strcmp(string, ".--.") == 0)   return 'P';
   if (strcmp(string, "--.-") == 0)   return 'Q';
   if (strcmp(string, ".-." ) == 0)   return 'R';
   if (strcmp(string, "..." ) == 0)   return 'S';
   if (strcmp(string, "-"   ) == 0)   return 'T';
   if (strcmp(string, "..-"  ) == 0)  return 'U';
   if (strcmp(string, "...-") == 0)   return 'V';
   if (strcmp(string, ".--" ) == 0)   return 'W';
   if (strcmp(string, "-..-") == 0)   return 'X';
   if (strcmp(string, "-.--") == 0)   return 'Y';
   if (strcmp(string, "--..") == 0)   return 'Z';

   return ' ';
}


// md5sum: 79b2895f7c1ec4aa285100f7346e8106 morse.cpp [20050403]

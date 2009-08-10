//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jun  3 10:04:21 PDT 2000
// Last Modified: Tue Jul  4 17:11:21 PDT 2000
// Filename:      ...sig/doc/examples/all/taptempo/taptempo.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description:   A demonstration of various tempo beating algorithms.
//                which use a binary switch.
//

#include "synthImprov.h" 
#include <string.h>

/*----------------- beginning of improvization algorithms ---------------*/

#define MAXLAGTIME 70            /* max latency of beat simultaneaity */

// various note algorithms
#define GEN_MONO  1
#define GEN_CHROM 2

// various tempo algorithms
#define BEAT_AUTO  0              /* computer controlls tempo */
#define BEAT_CONST 1              /* next beat is always constant */
#define BEAT_FIXED 2              /* next beat is always fixed */
#define BEAT_LAST  3              /* next beat is last's beats tempo */

int generation = GEN_CHROM;       // note generation algorithm to use
int control    = BEAT_AUTO;       // tempo generation algorithm to use
int divisions  = 4;               // number of notes per beat
int divisionChange = 0;           // true when divisions change
int newdivisions = 4;             // new division setting

int keyflag = 0;                  // for options control from computer keyboard
int keypresstime = 0;             // for elegance of interface
CircularBuffer<int> beattime(1000); // time that the last beat occured

EventBuffer constcase;            // for BEAT_CONST tempo case

Voice voice;
int running = 0;                  // 0 = off, 1 = on

SigTimer beatlocation;            // for keeping track of time
double tempo = 60.0;              // tempo for fixed tempo algorithms
double beatfraction = 0.0;        // location in the beat
int displayBeatQ = 1;             // boolean for displaying subbeat locations

// function declarations
void             beat             (void);
void             checkfornote     (double fraction);
const char*      genString        (int generation);
const char*      tempoString      (int tempotype);
void             playnote         (int subbeat, int subcount);
void             constcaseinsert  (int count, double totaldur);
void             displaySubbeat   (int subbeat, int div);

/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << 
   "TEMPODEMO -- Craig Stuart Sapp <craig@ccrma.stanford.edu> June 2000\n"
   "Commands:\n"
   "   g# = choose a note generation algorithm\n"
   "   d# = choose a beat division amount\n"
   "   t# = choose a tempo tracking algorithm\n"
   "   b  = print the current beat fraction\n"
   "   s  = toggle subbeat display\n"
   "   p  = toggle periodic tempo display\n"
   "   z  = toggle performance on/off\n"
   "   <,>  = slow/speed tempo for certain tempo types\n"
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
   voice.setChannel(0);
   voice.setPort(synth.getInputPort());
   beatlocation.setTempo(tempo);
   eventIdler.setPeriod(0);
   constcase.setPollPeriod(10);     
   displayBeatQ = 1;
   beattime.reset();
   beattime[0] = 0;
   beattime[1] = 0;
   beattime[2] = 0;
   beattime[3] = 0;
}



//////////////////////////////
//
// finishup -- this function is called by the improv interface
//     whenever the program is exited.  Put items here which
//     need to be taken care of when the program is finished.
//

void finishup(void) { 
   voice.off();
}


/*-------------------- main loop algorithms -----------------------------*/

//////////////////////////////
//
// mainloopalgorithms -- this function is called by the improv interface
//   continuously while the program is running.  The global variable t_time
//   which stores the current time is set just before this function is
//   called and remains constant while in this functions.
//

void mainloopalgorithms(void) { 
   if (control == BEAT_AUTO) {
      beatfraction = beatlocation.getPeriodCount() - beatlocation.expired();
   } else {
      beatfraction = beatlocation.getPeriodCount();
   }

   if (running) {
      if (control == BEAT_CONST) {
         constcase.checkPoll();
      } else {
         checkfornote(beatfraction);
      }
   } else {
      beatfraction = -1;
   }
}



//////////////////////////////
//
// checkfornote -- determine if it is time to play a new note
//     based on the beat location.
//

void checkfornote(double fraction) {
   // always wait for the next beat to occur
   if (fraction > 1.0) {
      return;
   }

   static double lastfraction = 0.0;
   static int lastnote = -1;
   static int subbeat = 0;

   if (fraction < lastfraction && divisions == 1 && control != BEAT_CONST) {
      playnote(0, 1);
      lastfraction = fraction;
      displaySubbeat(0, divisions);
      if (divisionChange) {
         divisions = newdivisions;
         divisionChange = 0;
      }
      return;
   }


   if (control == BEAT_CONST) {
      lastfraction = fraction;
      return;
   }

   if (fraction >= 1.0 && control == BEAT_FIXED) {
      lastfraction = fraction;
      return;
   }

   subbeat = (int)(fraction * divisions);

   if (lastnote != subbeat) {
      if (subbeat == 0 && divisionChange) {
         divisions = newdivisions;
         divisionChange = 0;
      }
      if (subbeat == 0 && lastnote != divisions - 1) {
         for (int j=0; j<divisions - 1 - lastnote; j++) {
            playnote(lastnote + j + 1, divisions);
            displaySubbeat(lastnote + j + 1, divisions);
         }
      }
      playnote(subbeat, divisions);
      lastnote = subbeat;
      displaySubbeat(subbeat, divisions);
   } 

   if (subbeat > divisions) {
      subbeat = divisions - 1;
   }
   if (lastnote > divisions) {
      lastnote = divisions - 1;
   }

   lastfraction = fraction;
}


void displaySubbeat(int subbeat, int div) {
   if (displayBeatQ == 0) {
      return;
   }

   if (subbeat == 0) {
      cout << endl;
   } else {
      cout << " ";
   }

   cout << subbeat << flush;
}



/*-------------------- triggered algorithms -----------------------------*/


/* uncomment these line to use with the Radio Baton
   void stick1trig(void) { beat(); }
   void stick2trig(void) { beat(); }
   void b14plustrig(void) { beat(); }
   void b15plustrig(void) { beat(); }
   void b14minusuptrig(void) { beat(); }
   void b14minusdowntrig(void) { beat(); }
   void b15minusuptrig(void) { beat(); }
   void b15minusdowntrig(void) { beat(); }
*/



///////////////////////////////
//
// keyboardchar -- this function is called by the improv interface
//     whenever a key is pressed on the computer keyboard.
//     Put commands here which will be executed when a key is
//     pressed on the computer keyboard.
//

void keyboardchar(int key) { 
   keypresstime = t_time;

   switch (key) {
      case 'b':   // print the current beat fraction
         cout << "Beat fraction = " << beatfraction << endl;
         break;
      case 's':   // toggle showing of subbeat locations
         displayBeatQ = !displayBeatQ;
         if (displayBeatQ) {
            cout << "Subbeat display is turned ON" << endl;
         } else {
            cout << "Subbeat display is turned OFF" << endl;
         }
         break;
      case 'z':   // toggle performance
         running = !running;
         if (running) {
            cout << "Performing" << endl;
            beatlocation.reset();
         } else {
            cout << "Performance stopped" << endl;
         }
         break;
      case ' ':
         beat();
         break;
      case 'g':
	 cout << "Choose a number between 1 and 9 to control note algorithm" 
              << endl;
         keyflag = 'g';
         break;
      case 'd':
	 cout << "Choose a number between 1 and 9 to select beat division" 
              << endl;
         keyflag = 'd';
         break;
      case 't':
	 cout << "Choose a number between 1 and 9 to control tempo algorithm" 
              << endl;
         keyflag = 't';
         break;
      case '<':  // slow the tempo down for auto/fixed/const tempo options
      case ',':  // slow the tempo down for auto/fixed/const tempo options
         tempo /= 1.02;
         beatlocation.setTempo(tempo);
         cout << "Fixed tempo set to: " << tempo << endl;
         break;
      case '>':  // speed up the tempo down for auto/fixed/const tempo options
      case '.':  // speed up the tempo down for auto/fixed/const tempo options
         tempo *= 1.02;
         beatlocation.setTempo(tempo);
         cout << "Auto/const/fixed tempo set to: " << tempo << endl;
         break;
   }


   if (isdigit(key)) {
      switch (keyflag) {
         case 'g':
            generation = key - '0';  // note generation algorithm to use
            cout << "Generation algorithm is: " << genString(generation) 
                 << endl;
            break;
         case 'd':
            if (key == '0') {
               break;
            }
            if (keypresstime - beattime[0] < MAXLAGTIME) {
               divisions = key - '0';   // number of notes per beat
               cout << "Time diff: " << keypresstime - beattime[0] << endl;
               cout << "Beat division: " << divisions << endl;
               divisionChange = 0;
            } else {
               newdivisions = key - '0';   // number of notes per beat
               cout << "Time diff: " << keypresstime - beattime[0] << endl;
               cout << "Beat division: " << divisions << endl;
               divisionChange = 1;
            }
            break;
         case 't':
            control = key - '0';     // tempo generation algorithm to use
            cout << "Tempo algorithm is: " << tempoString(control) << endl;
            break;
         case 'q':                   // quiet
            running = 0;
            break;
         default:
            keyflag = 0;
      }
   }

}


/*------------------ end improvization algorithms -----------------------*/


//////////////////////////////
//
// beat -- cause a new beat to occur.
//

void beat(void) {
   beattime.insert(t_time);
   beatlocation.reset();

   if (running == 0) {
      running = 1;
      beatlocation.reset();
   }

   if (control == BEAT_CONST) {
      constcaseinsert(divisions, beatlocation.getPeriod());
      return;
   }

   int diff = beattime[0] - beattime[1];
   
   // ignore initial beats
   if (diff > 5000) {
      return;
   }

   return;
   beatlocation.setPeriod(diff);
}



//////////////////////////////
//
// genString -- print the note generation algorithm type.
//

const char* genString(int generation) {
   static char string[128] = {0}; 
   switch (generation) {
      case GEN_MONO:
         strcpy(string, "Unison");
         break;
      case GEN_CHROM:
         strcpy(string, "Chromatic");
         break;
      default:
         strcpy(string, "Unknown");
         break;
   }

   return string;
}



//////////////////////////////
//
// playnote -- play notes for beating
//

void playnote(int subbeat, int subcount) {
   if (control == BEAT_AUTO && subbeat == 0) {
      beattime.insert(t_time);
   }

   if (subbeat == subcount) {
      cout << "Illegal beat: " << subbeat << "\tdivisions = " 
           << subcount << endl;
   }

   voice.play(60 + subbeat, 64);
}



//////////////////////////////
//
// tempoString -- print the note generation algorithm type.
//

const char* tempoString(int tempotype) {
   static char string[128] = {0}; 
   switch (tempotype) {
      case BEAT_AUTO:
         strcpy(string, "Auto Tempo");
         break;
      case BEAT_CONST:
         strcpy(string, "Const Tempo");
         break;
      case BEAT_FIXED:
         strcpy(string, "Fixed Tempo");
         break;
      case BEAT_LAST:
         strcpy(string, "Last Beat-duration Tempo");
         break;
      default:
         strcpy(string, "Unknown");
         break;
   }

   return string;
}



//////////////////////////////
//
// constcaseinsert -- add all notes
//

void constcaseinsert(int count, double totaldur) {
   static NoteEvent note;           // temporary note for placing in buffer
   double starttime = mainTimer.getTime();
   double duration;
   if (count > 0) {
      duration = totaldur / count;
   } else {
      duration = 0.0;
   }
   int basenote = 60;
   int i;
   for (i=0; i<count; i++) {
      note.setOnDur((int)starttime, (int)duration);
      note.setKey(basenote + i);
      note.setVel(64);
      note.setChan(0);
      note.activate();
      constcase.insert(note);
      starttime += duration;
   }
}



// md5sum: b1cbaa2e0800e5e5e10f804ebd01e97f taptempo.cpp [20050403]

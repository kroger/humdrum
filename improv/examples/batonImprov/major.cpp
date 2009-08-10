// Test of simultaneous triggering with chords
//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Mar  9 13:23:13 PST 1996
// Last Modified: Tue Apr 23 00:20:01 PDT 1996
// Last Modified: Tue Apr 18 18:31:15 PDT 2000 (syntax updated)
// Filename:      ...sig/doc/examples/all/major/major.cpp
// Syntax:        C++; batonImprov 2.0
//
// Description: //	 A major triad will be played if both batons
//	 are triggered at the same time.  The root of the triad
//	 is controlled by the paragraph below.  If baton 1
//	 is hit hard, a minor third below the root will be
//	 played; if hit very hard, a major third below will be
//	 played.  In a similar manner, if baton 2 is hit hard,
//	 notes will be played above the fifth of the triad.
//

#include "batonImprov.h"

Voice  bass;          // submediant note
Voice  tenor;         // tonic note
Voice  alto;          // mediant note
Voice  soprano;       // dominant note
Voice  quintus;       // subtonic note

const int HitHard = 80;
const int HitMedium = 50;

int   time1 = 0;
int   time2 = 0;

void dial1response(void);

/***************** Beginning of user functions ****************************/


int makeDigit(int x, int y) {
   static const int digitHolder[5][2] =                     // 1 2 3 4 5
                     {{6,1},{7,2},{8,3},{9,4},{0,5}};       // 6 7 8 9 0
   return digitHolder[x][y];
}

int makeKeyno(void) {
   return 10 * makeDigit(midiscale(baton.x1t,0,4), midiscale(baton.y1t,0,1)) +
           1 * makeDigit(midiscale(baton.x2t,0,4), midiscale(baton.y2t,0,1));
}


/*---------------- Documentation functions -------------------------------*/

void description(void) {
   cout
   << "______________________________________________________________ \n"
   << "| Description:                                                |\n"
   << "|    A major triad will be played if both batons are          |\n"
   << "|    triggered at the same time.  The root of the triad       |\n"
   << "|    is controlled by the paragraph below.  If baton 1        |\n"
   << "|    is hit hard, a minor third below the root will be        |\n"
   << "|    played; if hit very hard, a major third below will be    |\n"
   << "|    played.  In a similar manner, if baton 2 is hit hard,    |\n"
   << "|    notes will be played above the fifth of the triad.       |\n"
   << "|                                                             |\n"
   << "|         The radio baton is divided into 10 locations:        |\n"
   << "|    5 divisions along the x direction and 2 divisions along  |\n"
   << "|    the y direction.  Batons  control the key number of the  |\n"
   << "|    MIDI note, Baton 1 controlling the tens digits of the    |\n"
   << "|    key number, and Baton 2 controlling the ones digit of    |\n"
   << "|    the key number. Digits are arranged on the radio baton as:|\n"
   << "|              1  2  3  4  5                                  |\n"
   << "|              6  7  8  9  0                                  |\n"
   << "|                          -- Craig Stuart Sapp, 10 Mar 1996  |\n"
   << "---------------------------------------------------------------\n"
   ;
}


void programhelp(void) {
   cout
   << ">                                                              \n"
   << "> Baton 1: Tens digit of keyno to play                         \n"
   << ">    x1t: Five divisions                                       \n"
   << ">    y1t: Two divisions                                        \n"
   << ">    w1t: Averages with w2t for key velocity of some notes.    \n"
   << "> Baton 2: Ones digit of keyno to play                         \n"
   << ">    x2t: Five divisions                                       \n"
   << ">    y2t: Two divisions                                        \n"
   << ">    w2t: Averages with w1t for key velocity of some notes.    \n"
   << ">                                                              \n"
   << ">    p1p: Channel Number to play chord on.                     \n"
   << ">    p2p: Timber to play chord with.                           \n"
   << ">   +b14: Command mode.                                        \n"
   << ">   +b15: This help screen.                                    \n"
   << ">                                                              \n"
   ;
}

/*---------------- Initialization functions ------------------------------*/

void initialization(void) { 
   description(); 
   baton.dial1position = dial1response;
}

/*---------------- Main loop functions -----------------------------------*/

void mainloopalgorithms(void) { }

/*---------------- Trigger response functions ----------------------------*/

int togetherQ(int separation = 50) {
   if (abs(time1 - time2) < separation) {
      return 1;
   } else {
      return 0;
   }
}


void ActIfTogether(void) {
   if (togetherQ(60)) {
      int triggerKey, average;

      triggerKey = makeKeyno();
      average = (baton.w1t + baton.w2t)/2;

      cout << "Keyno = " << triggerKey
           << "\t w1t = " << baton.w1t << "\tw2t = " << baton.w2t << endl;

      //            key           vel
      tenor.play   (triggerKey,   baton.w1t);
      alto.play    (triggerKey+4, average);
      soprano.play (triggerKey+7, baton.w2t);

      // determine bass note:
      if (baton.w1t > HitMedium && triggerKey > 3) {
         if (baton.w1t > HitHard) {
            bass.play(triggerKey-4, average);
         } else {
            bass.play(triggerKey-3, average);
         }
      } else {
            bass.off();
      }

      // determine quintus note:
      if (baton.w2t > HitMedium) {
         if (baton.w2t > HitHard) {
            quintus.play(triggerKey+11, average);
         } else {
            quintus.play(triggerKey+10, average);
         }
      } else {
         quintus.off();
      }

   }
}


void stick1trig(void) {
   cout << "Baton 1 triggered." << endl;
   time1 = t_time;
   ActIfTogether();
}

void stick2trig(void) {
   cout << "Baton 2 triggered." << endl;
   time2 = t_time;
   ActIfTogether();
}

void b14plustrig(void) { cout << "B14+ triggered\n"; programhelp(); }
void b15plustrig(void) { cout << "B15+ triggered\n"; programhelp(); }

void b14minusuptrig(void) { }
void b14minusdowntrig(void) { }
void b15minusuptrig(void) { }
void b15minusdowntrig(void) { }

void keyboardchar(int key) { }

/*---------------- Poll response functions -------------------------------*/

void stick1pollresponse(void) { }
void stick2pollresponse(void) { }
void dial1response(void) { 
   if (midiscale(baton.d2pb[0],0,63) - midiscale(baton.d2pb[1],0,63)) {
      cout << "Timbre = " << GeneralMidiName(midiscale(baton.d2p, 0, 63)) 
           << endl;
      tenor.pc(midiscale(baton.d2p, 0, 63));
   }
}

/*---------------- Actions to take just before leaving program -----------*/

void finishup(void) { }

/***************** End of user functions **********************************/

// md5sum: e7907a885c97362ebc02c06eb81a41a8 major.cpp [20050403]

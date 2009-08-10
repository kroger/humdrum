//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Mar  9 00:43:13 PST 1996
// Last Modified: Tue Apr 23 00:04:22 PDT 1996
// Last Modified: Tue Apr 18 12:29:16 PDT 2000 (syntax updated)
// Filename:      .../sig/doc/examples/all/hundred/hundred.cpp
// Syntax:        C++; batonImprov 2.0
//
// Description: Test of the detection of simultaneous triggering 
//              of both batons.  Baton divided into 10 locations:
//              5 divisions along the x direction and 2 divisions along
//              the y direction.  Batons  control the key number of the
//              MIDI note: Baton 1 controlling the tens digits of the
//              key number, and Baton 2 controlling the ones digit of
//              the key number. Digits are arranged on the radio drum as:
//                       1  2  3  4  5
//                       6  7  8  9  0
//

#include "batonImprov.h"

Voice voice;               // voice for output notes
int   time1, time2;        // for determining whether batons trigger together
int   timbre = 0;          // instrument to play voice upon
int   keyno = 0;           // the MIDI key number to play

void dial1position(void);  // channel changer
void dial2position(void);  // timbre changer

/***************** Beginning of user functions ****************************/

int makeDigit(int x, int y) {
   static const int digitHolder[5][2] =                     // 1 2 3 4 5
                 {{6,1},{7,2},{8,3},{9,4},{0,5}};           // 6 7 8 9 0
   return digitHolder[x][y];
}

int makeKeyno(int a1, int a2, int b1, int b2) {
   return  10 * makeDigit(midiscale(a1,0,4), midiscale(a2,0,1)) +
            1 * makeDigit(midiscale(b1,0,4), midiscale(b2,0,1));
}



/*---------------- Documentation functions -------------------------------*/

void description(void) {
   cout
   << "______________________________________________________________ \n"
   << "| Description:                                                |\n"
   << "|    Test of the detection of simultaneous triggering         |\n"
   << "|    of both batons.  Baton divided into 10 locations:        |\n"
   << "|    5 divisions along the x direction and 2 divisions along  |\n"
   << "|    the y direction.  Batons  control the key number of the  |\n"
   << "|    MIDI note: Baton 1 controlling the tens digits of the    |\n"
   << "|    key number, and Baton 2 controlling the ones digit of    |\n"
   << "|    the key number. Digits are arranged on the radio drum as:|\n"
   << "|              1  2  3  4  5                                  |\n"
   << "|              6  7  8  9  0                                  |\n"
   << "|                           -- Craig Stuart Sapp, 9 Mar 1996  |\n"
   << "---------------------------------------------------------------\n"
   ;
}


void programhelp(void) {
   cout
   << ">                                                              \n"
   << "> Baton 1: Tens digit of keyno to play                         \n"
   << ">    x1t: Five divisions                                       \n"
   << ">    y1t: Two divisions                                        \n"
   << ">    w1t: Averages with w2t for key velocity of note.          \n"
   << "> Baton 2: Ones digit of keyno to play                         \n"
   << ">    x2t: Five divisions                                       \n"
   << ">    y2t: Two divisions                                        \n"
   << ">    w2t: Averages with w1t for key velocity of note.          \n"
   << ">                                                              \n"
   << ">    p1p: Channel Number to play note on.                      \n"
   << ">    p2p: Timber of note.                                      \n"
   << ">   +b14: Command mode.                                        \n"
   << ">   +b15: This help screen.                                    \n"
   << ">                                                              \n"
   ;
}

/*---------------- Initialization functions ------------------------------*/

void initialization(void) { 
   description(); 
   baton.dial1position = dial1position;
   baton.dial2position = dial2position;
   voice.pc(timbre);
}

/*---------------- Main loop functions -----------------------------------*/

void mainloopalgorithms(void) { }

/*---------------- Trigger response functions ----------------------------*/

int togetherQ(void) {
   if (abs(time1-time2) < 50) {
      return 1;
   } else {
      return 0;
   }
}

void ActIfTogether(void) {
   int a1, a2, b1, b2;
   if (togetherQ()) {
      a1 = baton.x1t;
      a2 = baton.y1t;
      b1 = baton.x2t;
      b2 = baton.y2t;
      keyno = makeKeyno(a1, a2, b1, b2);
      cout << "Keyno = " << keyno << endl;
      voice.play(keyno, (baton.w1t+baton.w2t)/2);
   }
}

void stick1trig(void) {
   cout << "Baton 1 triggered. " << endl;
   time1 = t_time;
   ActIfTogether();
}

void stick2trig(void) {
   cout << "Baton 2 triggered. " << endl;
   time2 = t_time;
   ActIfTogether();
}


void b14minusdowntrig(void) { }
void b14minusuptrig(void) { }
void b14plustrig(void) { cout << "B14+ triggered\n"; programhelp(); }
void b15minusdowntrig(void) { }
void b15minusuptrig(void) { }
void b15plustrig(void) { cout << "B15+ triggered\n"; programhelp(); }

/*---------------- Poll response functions -------------------------------*/

void dial1position(void) {
   if (midiscale(baton.d1p,0,15) != voice.getChannel()) {
      voice.setChannel(midiscale(baton.d1p,0,15));
      cout << "Channel = " << voice.getChannel() << endl;
   }
}

void dial2position(void) {
   if (baton.d2p != timbre) {
      timbre = baton.d2p;
      voice.pc(timbre);
      cout << "Timber = " << GeneralMidiName(timbre) << endl;
   }
}

/*---------------- Actions to take just before leaving program -----------*/

void finishup(void) { }
void keyboardchar(int key) { }

/***************** End of user functions **********************************/

// md5sum: d6ee6cec1844aa238d21d15daa7caa6d hundred.cpp [20050403]

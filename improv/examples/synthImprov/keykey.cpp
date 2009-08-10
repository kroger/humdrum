//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 15 October 1998
// Last Modified: 15 October 1998
// Filename:      ...sig/doc/examples/improv/synthImprov/keykey/keykey.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: Using a synthesizer keyboard as a typewriter
//     by using combination (pairs) of keys to generate a single
//     character.  This is an example of how to generate a
//     large number of events (2^10-1 from only 10 keys).              
//

#include "synthImprov.h" 

/*----------------- beginning of improvization algorithms ---------------*/

int lastEventTime  = 0;   // last message that came in
int nextActionTime = 0;   // last time an event was created

int lastOnTime[11] = {0};  // for monitoring key on times
int lastOffTime[11] = {0}; // for monitoring key off times
int keyCount = 0;          // number of keys currently being pressed
int repeatRate = 500;      // rate at which to repeate keys
int onKey1;
int onKey2;
int event = -1;
int shift = 0;

MidiMessage message;       // for processing incoming messages
CircularBuffer<int> keysOn(100); 

// function declarations:
int makeEventDecision(void);

/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   cout << "\nKeykey:  (Craig Stuart Sapp <craig@ccrma.stanford.edu>)"
           "15 Oct 1998\n"
        << "This program is a demonstration of how to use a few keys\n"
        << "to generate a lot of events (10 keys --> 2^10 - 1 events\n"
        << "Use the white midi keys C-G below middle C for the left hand\n"
        << "and middle C-G white keys for the right hand\n"
        << "left hand: C=5, D=4, E=3, F=2, G=1\n"
        << "right hand: C=1, D=4, E=3, F=4, G=5\n"
        << "Here are the key combinations (LH col, RH row):\n\n"
        << "     5 E J O T Y\n"
        << "     4 D I N S X        For example:\n"
        << "     3 C H M R W           LH3+RH2 = F\n"
        << "     2 B G L Q V           LH5+RH3 = O\n"
        << "     1 A F K P U           LH1+RH1 = A\n"
        << "       1 2 3 4 5"
        << endl;
} 

void initialization(void) { }

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

void mainloopalgorithms(void) { 
   if (synth.getNoteCount() > 0) {
      nextActionTime = t_time;
      message = synth.extractNote();
      if (message.p2() == 0) {   // note off
         keyCount--;
         if (keyCount < 0) {
            keyCount = 0;
         }
         switch (message.p1()) {
            case C3:   lastOffTime[1]  = t_time;   break;
            case D3:   lastOffTime[2]  = t_time;   break;
            case E3:   lastOffTime[3]  = t_time;   break;
            case F3:   lastOffTime[4]  = t_time;   break;
            case G3:   lastOffTime[5]  = t_time;   break;
            case C4:   lastOffTime[6]  = t_time;   break;
            case D4:   lastOffTime[7]  = t_time;   break;
            case E4:   lastOffTime[8]  = t_time;   break;
            case F4:   lastOffTime[9]  = t_time;   break;
            case G4:   lastOffTime[10] = t_time;   break;
         }
      } else {                   // note on
         keyCount++;
         keysOn.insert(message.p1());
         onKey2 = onKey1;
         switch (message.p1()) {
            case C3:   lastOnTime[1]  = t_time;  onKey1 = 1;   break;
            case D3:   lastOnTime[2]  = t_time;  onKey1 = 2;   break;
            case E3:   lastOnTime[3]  = t_time;  onKey1 = 3;   break;
            case F3:   lastOnTime[4]  = t_time;  onKey1 = 4;   break;
            case G3:   lastOnTime[5]  = t_time;  onKey1 = 5;   break;
            case C4:   lastOnTime[6]  = t_time;  onKey1 = 6;   break;
            case D4:   lastOnTime[7]  = t_time;  onKey1 = 7;   break;
            case E4:   lastOnTime[8]  = t_time;  onKey1 = 8;   break;
            case F4:   lastOnTime[9]  = t_time;  onKey1 = 9;   break;
            case G4:   lastOnTime[10] = t_time;  onKey1 = 10;  break;
         }
      }
   }

   if (nextActionTime + repeatRate <= t_time) {
      nextActionTime = t_time;
   }    

   event = makeEventDecision();
   if (event > 0) {
      if (shift) {
         event = toupper(event);
      }
      cout << (char)event << flush;
   }

}


int makeEventDecision(void) {
   int event = -1;
   int combination;
   if (keyCount == 2 && nextActionTime <= t_time) {
      nextActionTime = t_time + repeatRate;
      if (onKey1 < onKey2) {
         combination = 100 * (onKey1-1) + (onKey2-1);
      } else {
         combination = 100 * (onKey2-1) + (onKey1-1);
      }
      switch (combination) {
         case 5:   event = 'e';   break;
         case 6:   event = 'j';   break;
         case 7:   event = 'o';   break;
         case 8:   event = 't';   break;
         case 9:   event = 'y';   break;

         case 105:   event = 'd';   break;
         case 106:   event = 'i';   break;
         case 107:   event = 'n';   break;
         case 108:   event = 's';   break;
         case 109:   event = 'x';   break;

         case 205:   event = 'c';   break;
         case 206:   event = 'h';   break;
         case 207:   event = 'm';   break;
         case 208:   event = 'r';   break;
         case 209:   event = 'w';   break;

         case 305:   event = 'b';   break;
         case 306:   event = 'g';   break;
         case 307:   event = 'l';   break;
         case 308:   event = 'q';   break;
         case 309:   event = 'v';   break;

         case 405:   event = 'a';   break;
         case 406:   event = 'f';   break;
         case 407:   event = 'k';   break;
         case 408:   event = 'p';   break;
         case 409:   event = 'u';   break;

         default:   event = -1; nextActionTime = t_time;
      }
   } else if (keyCount == 1 && nextActionTime <= t_time) {
      switch (keysOn[0]) {
         case Cs3:  event = ' ';  nextActionTime = t_time + repeatRate; break;
         case Ds3:  event = '\n'; nextActionTime = t_time + repeatRate; break;
         case Fs3:  event = 'z';  nextActionTime = t_time + repeatRate; break; 
         case Gs3:  event = ',';  nextActionTime = t_time + repeatRate; break;
         case As3:  event = '.';  nextActionTime = t_time + repeatRate; break;
         case Cs4:  shift = !shift; nextActionTime = t_time + repeatRate;
                    event = -1; break;
         default:    event = -1;  nextActionTime = t_time;
      }
   } else if (keyCount > 2) {
      nextActionTime = t_time;
   }

   return event;
}


/*-------------------- triggered algorithms -----------------------------*/

void keyboardchar(int key) { }


/*------------------ end improvization algorithms -----------------------*/


// md5sum: 845dfc6b7d455f8e3e713897f5368552 keykey.cpp [20050403]

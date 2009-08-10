//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 4 January 1998
// Last Modified: Fri Jan  9 18:30:17 GMT-0800 1998
// Filename:      ...sig/doc/examples/improv/synthImprov/loop2/loop2.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: This is a (rhythm) looping program.
//

#include "synthImprov.h" 

#ifndef OLDCPP
   #include <fstream>
   using namespace std;
#else
   #include <fstream.h>
#endif


/*----------------- beginning of improvization algorithms ---------------*/

#define TRIGVEL 100

int      beats;                 // number of beats in the loop
int      subdivisions;          // number of subdivisions per beat
int      tempo;                 // number of beats per minute
SigTimer metronome;             // keeps track of curnt beat and subdivision
int      instrument[10];        // instruments, midi keynumbers
int      size = 0;              // size of loop array = beats * subdivisions
char*    storage[10] = {NULL};  // array for storing looped notes, store vel
int      silenceQ[10] = {0};    // whether or not to have track sound
int      silenceHitQ = 0;       // whether silence shift key was hit
int      unsilenceHitQ = 0;     // whether silence shift key was hit
int      clickTrack = 0;        // click track, 1=on, 0=off
int      defaultinst[10] = {0, 
   GM_LOW_TOM, GM_LOW_MID_TOM, GM_HIGH_MID_TOM, GM_HIGH_TOM, GM_HI_BONGO, 
   GM_MUTE_CUICA, GM_OPEN_CUICA, GM_MUTE_TRIANGLE, GM_ACOUSTIC_BASS_DRUM};



/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();
   psl("   Rhythm loops 2 -- Craig Stuart Sapp <craig@ccrma.stanford.edu>");
   psl("   9 January 1998 -- version 22 November 1998");
   printintermediateline();
   psl("   Attack Trigger row:     1 2 3 4 5 6 7 8 9 ");
   psl("   Silence Trigger row:    q w e r t y u i o ");
   psl("   Clear Notes row:        a s d f g h j k l");
   psl("   ");
   psl("   Commands:");
   psl("      z = clear all notes");
   psl("      x = change beat pattern");
   psl("      c = toggle click track");
   psl("      - = slow down tempo      = = speed up tempo");
   psl("      v = load loop 1          , = save loop 1");
   psl("      b = load loop 2          . = save loop 2");
   psl("      n = load loop 3          / = save loop 3");
   psl("      0 = silence all notes on all tracks at current beat location");
   psl("   ");
   psl("   Two-key commands:");
   psl("      p + number = silence a rhythm track");
   psl("      ; + number = unsilence a rhythm track");
   psl("");
   printboxbottom();
   cout << endl;
} 


void zero(void) {
   for (int inst=0; inst<10; inst++) {
      for (int i=0; i<size; i++) {
         storage[inst][i] = 0;
      }
   }
}


void initialization(void) { 
   int inst;

   cout << "How many beats in the loop: ";
   echoKeysOn();
   cin  >> beats;
   echoKeysOff();
   if (beats < 1 || beats > 100) {
      cout << "beats set to 1." << endl;
      beats = 1;
   }

   cout << "How many subdivisions per beat: ";
   echoKeysOn();
   cin  >> subdivisions;
   echoKeysOff();
   if (subdivisions < 1 || subdivisions > 100) {
      cout << "subdivisions set to 1." << endl;
      subdivisions = 1;
   }

   cout << "How many beats per minute: ";
   echoKeysOn();
   cin  >> tempo;
   echoKeysOff();
   if (tempo < 1 || tempo > 1000) {
      cout << "Tempo set to 120 beats per minute." << endl;
      tempo = 120;
   }

   size = beats * subdivisions;
   for (inst=0; inst<10; inst++) {
      if (storage[inst] != NULL) delete [] storage[inst];
      storage[inst] = new char[size];
   }
   zero();

   metronome.setPeriod(30000.0/(tempo*subdivisions));
   metronome.reset();

   for (inst=1; inst<10; inst++) {
      if (instrument[inst] == 0) {
         instrument[inst] = defaultinst[inst];
      }
   }
}


void finishup(void) { 
   for (int inst=0; inst<10; inst++) {
      delete [] storage[inst];
   }
}


void saveLoop(const char* aFilename) {
   fstream output;
   output.open(aFilename, ios::out);
   if (!output) {
      cout << "Cannot write file: " << aFilename << endl;
      return;
   }
   output << "Comment: Loop2 " << endl << endl;
   output << "Beats: " << beats << endl;
   output << "Subdivisions: " << subdivisions << endl;
   output << "Tempo: " << tempo << endl << endl;

   for (int inst=1; inst<10; inst++) {
      output << "Instrument: " << inst << " " << instrument[inst];
      for (int i=0; i<size; i++) {
         if (i % subdivisions == 0) output << endl;
         output << setw(4) << (int)storage[inst][i];
      }
      output << endl << endl;
   }
   output << endl;
}


void loadLoop(const char* aFilename) {
   int inst, i, temp;
   int instSetQ = 0;
   fstream input;
   #ifndef OLDCPP
      input.open(aFilename, ios::in);
   #else
      input.open(aFilename, ios::in | ios::nocreate);
   #endif
   if (!input) {
      cout << "Cannot read file: " << aFilename << endl;
      return;
   }
   static char buffer[1000];

   while (!input.eof()) {
      // get command
      input >> buffer;
      switch (tolower(buffer[0])) {
         case 'b':                // beats per loop
            if (instSetQ) {
               cerr << "Error: beat command after instrument loop" << endl;
               return;
            }
            input >> beats;
            break;
         case 'c':                // comment
            // get rest of line
            input.getline(buffer, 1000, '\n');
            cout << buffer << endl;
            break;
         case 's':                // subdivisions per beat
            if (instSetQ) {
               cerr << "Error: subdivision command after instrument loop" << endl;
               return;
            }
            input >> subdivisions;
            break;
         case 't':                // tempo
            input >> tempo;
            break;
         case 'i':                // instrument track
            if (instSetQ == 0) {
               instSetQ = 1;
               for (i=0; i<10; i++) {
                  if (storage[i] != NULL) delete [] storage[i];
                  size = beats * subdivisions;
                  storage[i] = new char[size];
               }
            }
            // first read the instrument number
            input >> inst;
            if (inst < 0 || inst >= 10) {
               cerr << "Error: invalid instrument number: " << inst << endl;
               return;
            }
            // then read the rhythm instrument to assign to that instrument
            input >> instrument[inst];
            // now read the attack velocities for the instrument
            for (i=0; i<size; i++) {
               input >> temp;
               storage[inst][i] = (char)temp;
            }
            break;
         default: ;
             // ignore the command
      }
   }

   // set the metronome
   metronome.setPeriod(30000.0/(tempo * subdivisions));
   metronome.reset();
}


/*-------------------- main loop algorithms -----------------------------*/
void playTracks(int position);

void checkloop(void) {
   static int lastposition = -1;
   int current = metronome.expired();
   if (current >= size*2) {
      metronome.update(size*2);
      // tempo updated only at barlines
      metronome.setPeriod(30000.0/(tempo*subdivisions));  
      current -= size*2;
   }
   if (current/2 != lastposition) {
      lastposition = current/2;
      if (lastposition >= size) {
         cerr << "Error: out of bounds in array: " << lastposition << endl;
         exit(1);
      } else {
         playTracks(lastposition);
      }
      if (clickTrack && (lastposition % subdivisions == 0)) {
         if (lastposition == 0) {
            synth.play(9, GM_CLAVES, 100);
         } else {
            synth.play(9, GM_CLAVES, 50);
         }
      }
   }
}


void playTracks(int position) {
   static int inst;
  
   for (inst=1; inst<10; inst++) {
      if (storage[inst][position] != 0 && silenceQ[inst] != 1) {
         synth.play(9, instrument[inst], storage[inst][position]);
      }
   }
}


void mainloopalgorithms(void) { 
   checkloop();
}



/*-------------------- triggered algorithms -----------------------------*/

void processPercussion(int inst);
void restPercussion(int inst);
void clearPercussion(int inst);

void keyboardchar(int key) { 

   if (silenceHitQ) {
      int track = key - '0';
      silenceHitQ = 0;
      if (track >=0 && track < 10) {
         silenceQ[track] = 1;
         return;
      }
   } else if (unsilenceHitQ) {
      int track = key - '0';
      unsilenceHitQ = 0;
      if (track >=0 && track < 10) {
         silenceQ[track] = 0;
         return;
      }
   }
      

   // determine if this is a new note
   switch (key) {
      case '1': processPercussion(1);  break;
      case '2': processPercussion(2);  break;
      case '3': processPercussion(3);  break;
      case '4': processPercussion(4);  break;
      case '5': processPercussion(5);  break;
      case '6': processPercussion(6);  break;
      case '7': processPercussion(7);  break;
      case '8': processPercussion(8);  break;
      case '9': processPercussion(9);  break;
      case 'q': restPercussion(1);     break;
      case 'w': restPercussion(2);     break;
      case 'e': restPercussion(3);     break;
      case 'r': restPercussion(4);     break;
      case 't': restPercussion(5);     break;
      case 'y': restPercussion(6);     break;
      case 'u': restPercussion(7);     break;
      case 'i': restPercussion(8);     break;
      case 'o': restPercussion(9);     break;
      case '0': 
         restPercussion(1);  restPercussion(2);  restPercussion(3);
         restPercussion(4);  restPercussion(5);  restPercussion(6);
         restPercussion(7);  restPercussion(8);  restPercussion(9);
         return;
      case 'a': clearPercussion(1);    break;
      case 's': clearPercussion(2);    break;
      case 'd': clearPercussion(3);    break;
      case 'f': clearPercussion(4);    break;
      case 'g': clearPercussion(5);    break;
      case 'h': clearPercussion(6);    break;
      case 'j': clearPercussion(7);    break;
      case 'k': clearPercussion(8);    break;
      case 'l': clearPercussion(9);    break;
      case '-': 
         tempo--; 
         if (tempo < 1 ) tempo = 1;       
         cout << "\rTempo = " << tempo << "           ";
         cout.flush();
         return;
      case '=': 
         tempo++; 
         if (tempo > 1000 ) tempo = 1000;
         cout << "\rTempo = " << tempo << "           ";
         cout.flush();
         return;
      case 'v': loadLoop("loop1.txt");  return;
      case 'b': loadLoop("loop2.txt");  return;
      case 'n': loadLoop("loop3.txt");  return;
      case 'x': initialization();       return;
      case ',': saveLoop("loop1.txt");  return;
      case '.': saveLoop("loop2.txt");  return;
      case '/': saveLoop("loop3.txt");  return;
      case 'c': clickTrack = !clickTrack;                 
                if (clickTrack) {
                   cout << "Click track ON." << endl;
                } else {
                   cout << "Click track OFF." << endl;
                }
                return;
      case 'z': zero();  return;
      case 'p': silenceHitQ = 1; return;
      case ';': unsilenceHitQ = 1; return;
      default:
         // ignore the key or play the computer keyboard synth keys:
         charsynth(key);
         return;
   }
}



void restPercussion(int inst) {
   // determine where in the loop this rest goes
   int current = metronome.expired();
   if (current % 2 == 1) {
      current++;
   } else if (storage[inst][current] != 0) {  
      // this rhythm position already played, but turn off before finished
      synth.play(9, instrument[inst], 0); 
   }
   current /= 2;
   if (current >= size) current = 0;
   storage[inst][current] = 0;
}


void clearPercussion(int inst) {
   for (int i=0; i<size; i++) {
      storage[inst][i] = 0;
   }
}


void processPercussion(int inst) {
   if (silenceQ[inst] == 1) {
      synth.play(9, instrument[inst], TRIGVEL);
      // don't store in loop since paused
      return;
   }
   // determine where in the loop this note goes
   int current = metronome.expired();
   if (current % 2 == 1) {
      current++;

   } else {  
      // this rhythm position already played, but play new as well
      synth.play(9, instrument[inst], TRIGVEL); 
   }
   current /= 2;
   if (current >= size) current = 0;
   storage[inst][current] = TRIGVEL;
}


/*------------------ end improvization algorithms -----------------------*/


// md5sum: e5016c0509c904fbe8c441d9323850d4 loop2.cpp [20050403]

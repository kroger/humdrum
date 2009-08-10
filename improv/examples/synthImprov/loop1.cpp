//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 4 January 1998
// Last Modified: Mon Nov 23 16:37:22 PST 1998
// Filename:      ...sig/doc/examples/improv/synthImprov/loop1/loop1.cpp
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

int      beats;                // number of beats in the loop
int      subdivisions;         // number of subdivisions per beat
int      tempo;                // number of beats per minute
SigTimer metronome;            // keeps track of current beat and subdivision
int*     storage = NULL;       // array for storing looped notes
int      size = 0;             // number of elements in loop array
int      clickTrack = 0;       // for click track toggle, 1=on, 0=off


// non-interface functions declarations:
void zero(void);
void loadLoop(const char* aFilename);
void saveLoop(const char* aFilename);
void checkloop(void);


/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();
   psl("   Rhythm loops -- Craig Stuart Sapp <craig@ccrma.stanford.edu>");
   psl("   4 January 1998 -- version 23 Nov 1998");
   psl("");
   psl("   Press \"-\" to slow tempo, or \"=\" to speed tempo.");
   psl( "   Here are the notes which can be played on the "
      "computer keyboard:");
   psl("");
   pcl("1 LOW_TOM           6 MUTE_CUICA        ");
   pcl("2 LOW_MID_TOM       7 OPEN_CUICA        ");
   pcl("3 HIGH_MID_TOM      8 MUTE_TRIANGLE     ");
   pcl("4 HIGH_TOM          9 ACOUSTIC_BASS_DRUM");
   pcl("5 HI_BONGO          0 REST              ");
   psl("");
   psl("   Additional commands:");
   psl("      c = toggle click track           z = clear (zero) loop");
   psl("      s = save loop                    l = load loop");
   printboxbottom();
   cout << endl;
} 



void initialization(void) { 
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
   if (storage != NULL) {
      delete [] storage;
   }
   storage = new int[size];
   zero();

   metronome.setTempo(tempo * subdivisions);
   metronome.reset();
}



void finishup(void) { 
   if (storage != NULL) {
      delete [] storage;
      storage = NULL;
   }
}



void zero(void) {
   for (int i=0; i<size; i++) {
      storage[i] = 0;
   }
}



void saveLoop(const char* aFilename) {
   fstream output;
   output.open(aFilename, ios::out);
   if (!output) {
      cout << "Cannot write file: " << aFilename << endl;
      return;
   }
   output << "Beats: " << beats << endl;
   output << "Subdivisions: " << subdivisions << endl;
   output << "Tempo: " << tempo << endl;
   for (int i=0; i<size; i++) {
      if (i % subdivisions == 0) output << endl;
      output << setw(4) << storage[i];
   }
   output << endl;
}



void loadLoop(const char* aFilename) {
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
   input >> buffer;               // Beats:
   input >> beats;
   input >> buffer;               // Subdivisions:
   input >> subdivisions;
   input >> buffer;               // Tempo:
   input >> tempo; 

   if (storage != NULL) delete [] storage;
   size = beats * subdivisions;
   storage = new int[size];
   for (int i=0; i<size; i++) {
      input >> storage[i];
   }

   metronome.setTempo(tempo * subdivisions);
   metronome.reset();
}
   



/*-------------------- main loop algorithms -----------------------------*/


void mainloopalgorithms(void) { 
   checkloop();
}



void checkloop(void) {
   static int lastposition = -1;
   int current = metronome.expired();
   if (current >= size) {
      metronome.update(size);
      // tempo updated only at barlines
      metronome.setTempo(tempo * subdivisions);
      current -= size;
   }
   if (current != lastposition) {
      lastposition = current;
      if (lastposition >= size) {
         cerr << "Error: out of bounds in array: " << lastposition << endl;
         exit(1);
      } else if (storage[lastposition] != 0) {
        synth.play(9, storage[lastposition], 100); 
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



/*-------------------- triggered algorithms -----------------------------*/

void keyboardchar(int key) { 
   int note;

   // determine if this is a new note
   switch (key) {
      case '1': note = GM_LOW_TOM;            break;
      case '2': note = GM_LOW_MID_TOM;        break;
      case '3': note = GM_HIGH_MID_TOM;       break;
      case '4': note = GM_HIGH_TOM;           break;
      case '5': note = GM_HI_BONGO;           break;
      case '6': note = GM_MUTE_CUICA;         break;
      case '7': note = GM_OPEN_CUICA;         break;
      case '8': note = GM_MUTE_TRIANGLE;      break;
      case '9': note = GM_ACOUSTIC_BASS_DRUM; break;
      case '0': note = 0;                     break;
      case '-': tempo--; if (tempo < 1 ) tempo = 1;       return;
      case '=': tempo++; if (tempo > 1000 ) tempo = 1000; return;
      case 's': saveLoop("loop.txt");                     return;
      case 'l': loadLoop("loop.txt");                     return;
      case 'c': clickTrack = !clickTrack;                 
                if (clickTrack) {
                   cout << "Click track ON." << endl;
                } else {
                   cout << "Click track OFF." << endl;
                }
                return;
      case 'z': zero();                                   return;
      default:
         // ignore the key or play the computer keyboard synth keys:
         charsynth(key);
         return;
   }

   // determine where in the loop this note goes by finding the 
   // closest attack point by rounding to the nearest subdivision 

   int current = (int)(metronome.getPeriod() + 0.5);

   if (note != 0) {  
      synth.play(9, note, 80); 
   }
   if (current >= size) current = 0;
   storage[current] = note;
}


/*------------------ end improvization algorithms -----------------------*/



// md5sum: 8ce35f8f47c5ff6bfe1cd30c120c5cb0 loop1.cpp [20050403]

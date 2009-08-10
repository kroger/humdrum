//
// Programmer:    Leland Stanford, Jr. <leland@stanford.edu>
// Creation Date: Wed May 12 00:18:31 PDT 1999
// Last Modified: Wed May 12 01:47:06 PDT 1999
// Filename:      .../improv/examples/synthImprov/markov1/markov1.cpp
// Syntax:        C++; Visual C++ 6.0; synthImprov
//  
// Description: Generate/create markov transition tables for playing a 
//              melody.
//

#include "synthImprov.h"      
#include <math.h>

#ifndef OLDCPP
   #include <iostream>
   #include <iomanip>
   using namespace std;
#else
   #include <iostream.h>
   #include <iomanip.h>
#endif

#ifdef VISUAL
   double drand48(void) {
      return (double)rand()/RAND_MAX;
   }
#endif


/*----------------- beginning of improvization algorithms ---------------*/

#define MARKOV_SIZE 12

double markov[MARKOV_SIZE][MARKOV_SIZE] = {
   {0.10, 0.05, 0.10, 0.10, 0.50, 0.10, 0.10, 0.10, 0.10, 0.10, 0.05, 0.05},
   {0.10, 0.05, 0.10, 0.10, 0.50, 0.10, 0.10, 0.10, 0.10, 0.10, 0.05, 0.05},
   {0.10, 0.05, 0.10, 0.10, 0.50, 0.10, 0.10, 0.10, 0.10, 0.10, 0.05, 0.05},
   {0.10, 0.05, 0.10, 0.10, 0.50, 0.10, 0.10, 0.10, 0.10, 0.10, 0.05, 0.05},
   {0.10, 0.05, 0.10, 0.10, 0.50, 0.10, 0.10, 0.10, 0.10, 0.10, 0.05, 0.05},
   {0.10, 0.05, 0.10, 0.10, 0.50, 0.10, 0.10, 0.10, 0.10, 0.10, 0.05, 0.05},
   {0.10, 0.05, 0.10, 0.10, 0.50, 0.10, 0.10, 0.10, 0.10, 0.10, 0.05, 0.05},
   {0.10, 0.05, 0.10, 0.10, 0.50, 0.10, 0.10, 0.10, 0.10, 0.10, 0.05, 0.05},
   {0.10, 0.05, 0.10, 0.10, 0.50, 0.10, 0.10, 0.10, 0.10, 0.10, 0.05, 0.05},
   {0.10, 0.05, 0.10, 0.10, 0.50, 0.10, 0.10, 0.10, 0.10, 0.10, 0.05, 0.05},
   {0.10, 0.05, 0.10, 0.10, 0.50, 0.10, 0.10, 0.10, 0.10, 0.10, 0.05, 0.05},
   {0.10, 0.05, 0.10, 0.10, 0.50, 0.10, 0.10, 0.10, 0.10, 0.10, 0.05, 0.05}};

int    markovRecord  = 0;      // true if recording markov chain data
double tempo         = 120;    // the tempo of the performed melody
int    markovNote    = 0;      // the markov note for performance
int    octave        = 4;      // the octave of the markov note for performance
Voice  markovVoice;            // for keeping track of note-offs
int    lastMarkovNote = 0;     // for counting markov notes
int    newMarkovNote = 0;      // for counting markov notes
int    i, j;                   // for counting
SigTimer metronome;            // for determing the rhythm
int    constant = 0;           // for constant/non constant rhythm

// function declarations:
void processNoteForMarkov(void);
void performNoteFromMarkov(void);
void setNextEventTime(double tempo, SigTimer& metronome);
int  chooseNoteFromMarkovArray(int markovNote);
void zeroMarkovTable(void);
void generateMarkovTable(void);


/*--------------------- maintenance algorithms --------------------------*/


//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << 
   "Markov -- play/record markov melodies\n"
   "   keyboard commands: \n"
   "      , = slow down tempo\n"
   "      . = speed up tempo\n"
   "      p = print markov table\n"
   "    1-7 = set the octave to play markov note\n"
   "    ' ' = switch between recording and playing a markov melody\n"
   << endl;

} 



//////////////////////////////
//
// initialization -- this function is called by the improv
//     intervace once at the start of the program.  Put items
//     here which need to be initialized at the beginning of
//     the program.
//

void initialization(void) { 
   markovVoice.pc(0);
   markovVoice.setChannel(0);
   metronome.setTempo(60);
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
   if (markovRecord) {
      processNoteForMarkov();
   } else if (metronome.expired()) {
      performNoteFromMarkov();
      setNextEventTime(tempo, metronome);
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
   if (isdigit(key)) {
      octave = key - '0';
      if (octave < 1) {
         octave = 1;
      }
      if (octave > 7) {
         octave = 7;
      }
      return;
   }

   switch (key) {
      case ',':            // slow down the tempo which will be
         tempo /= 1.05;    // activated at the next note to be played
         if (tempo < 20) {
            tempo = 20;
         }
         cout << "Tempo is: " << tempo << endl;
         break;

      case '.':            // speed up the tempo which will be
         tempo *= 1.05;    // activated at the next note to be played
         if (tempo > 500) {
            tempo = 500;
         }
         cout << "Tempo is: " << tempo << endl;
         break;

      case 'p':            // print the markov table
         cout << "Markov transition table:" << endl;
         for (i=0; i<MARKOV_SIZE; i++) {
            for (j=0; j<MARKOV_SIZE; j++) {
               cout << setw(5) << markov[i][j] << " ";
            }
            cout << endl;
         }
         break;
 
      case ' ':            // switch between recording and playing 
         markovRecord = !markovRecord;
         if (markovRecord) {
            cout << "Starting to record Markov table..." << endl;
            zeroMarkovTable();
            newMarkovNote = -100;
            markovVoice.off();
         } else {
            cout << "Finished recording Markov table." << endl;
            generateMarkovTable();
         }
         break;
            
      case 'c':            // constant/not constant rhythm
         constant = !constant;
         cout << "The rhythm set is: " << constant << endl;
         break;

   }
}


/*------------------ begining of assisting functions --------------------*/


/////////////////////////////
//
// processNoteForMarkov -- read any MIDI input notes and
//    store occurances for generating a Markov transition table.
//

void processNoteForMarkov(void) {
   MidiMessage message;
   while (synth.getNoteCount() > 0) {
      message = synth.extractNote();
      if (message.p2() != 0) {
         lastMarkovNote = newMarkovNote;
         newMarkovNote = message.p1() % 12;
         if (newMarkovNote == (21%12) && message.p1() == 21) {
            newMarkovNote = -1;
         } else if (lastMarkovNote >= 0) {
            markov[lastMarkovNote][newMarkovNote] += 1;
         } 
      }
   }
}



/////////////////////////////
//
// performNoteFromMarkov -- generate a note from the last played note.
//

void performNoteFromMarkov(void) {
   MidiMessage message;
   int newnote = 0;
   if (synth.getNoteCount() != 0) {
      while (synth.getNoteCount() > 0) {
         message = synth.extractNote();
         if (message.p2() != 0) {
            newnote = message.p1() % 12;
         } 
      }
      newnote = chooseNoteFromMarkovArray(newnote);
   } else {
      newnote = chooseNoteFromMarkovArray(markovNote);
   }
   cout << "Last note: " << markovNote << "\tNew note: " << newnote << endl;
   markovVoice.play(newnote + octave * 12, rand()%20 + 60);
   markovNote = newnote;
}



/////////////////////////////
//
// chooseNoteFromMarkovArray -- get the next markov note, returning
//   B if any under-valuing the probabilities.
//

int chooseNoteFromMarkovArray(int markovNote) {
   markovNote = markovNote % 12;
   double targetSum = drand48();
   double sum = 0.0;
   int targetNote = 0;

   while (targetNote < MARKOV_SIZE &&
      sum+markov[markovNote][targetNote] < targetSum) {
      sum += markov[markovNote][targetNote];
      targetNote++;
   }

   return targetNote;
}



/////////////////////////////
//
// setNextEventTime -- choose the duration of the current note.
//


void setNextEventTime(double tempo, SigTimer& metronome) {
   int random = (int)(drand48() * 100);
   if (constant) {
      metronome.setTempo(tempo*2);   
   } else if (random < 10) {
      metronome.setTempo(tempo/4.0);   
   } else if (random < 20) {
      metronome.setTempo(tempo/4.0*2);
   } else if (random < 30) {
      metronome.setTempo(tempo/4.0*3);
   } else if (random < 40) {
      metronome.setTempo(tempo/4.0*4);
   } else if (random < 50) {
      metronome.setTempo(tempo/4.0*6);
   } else if (random < 60) {
      metronome.setTempo(tempo/4.0*8);
   } else if (random < 70) {
      metronome.setTempo(tempo/4.0*12);
   } else if (random < 80) {
      metronome.setTempo(tempo/4.0*16);
   } else if (random < 90) {
      metronome.setTempo(tempo/4.0*4.5);
   } else {
      metronome.setTempo(tempo/4.0*0.5);
   }
   metronome.reset();
}



//////////////////////////////
//
// zeroMarkovTable -- replace all values with zeros.
//

void zeroMarkovTable(void) {
   int i, j;
   for (i=0; i<MARKOV_SIZE; i++) {
      for (j=0; j<MARKOV_SIZE; j++) {
         markov[i][j] = 0.0;
      }
   }
}



//////////////////////////////
//
// generateMarkovTable -- calculate markov table from counting array.
//

void generateMarkovTable(void) {
   double total;
   int i, j;

   for (i=0; i<MARKOV_SIZE; i++) {
      total = 0;
      for (j=0; j<MARKOV_SIZE; j++) {
         total += markov[i][j];
      }
      if (total == 0.0) {
         markov[i][0] = 1.0;
      } else {
         for (j=0; j<MARKOV_SIZE; j++) {
            markov[i][j] /= total;
         }
      }
   }
}


/*------------------ end improvization algorithms -----------------------*/



// md5sum: 6f41d97973624cb0dcb4eac9cdaee698 markov1.cpp [20050403]

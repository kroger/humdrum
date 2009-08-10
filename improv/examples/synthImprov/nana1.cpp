//
// Programmer:    Dan Gang <dang@ccrma.stanford.edu>
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed May  5 21:42:45 PDT 1999
// Last Modified: Sun May  9 14:49:50 PDT 1999
// Filename:      ...sig/doc/examples/all/nana1/nana1.cpp
// Syntax:        C++; synthImprov 2.1; sigNet 1.0
//  
// Description:   Auto acompaniment of a simple melody.
//                Initial conversion of rule-based automatic
//                accompaniment.
//

#include "synthImprov.h"           /* MIDI interface handling functions  */


// program defines:
#define ON  1                      /* play a chord                       */
#define OFF 0                      /* turn off a chord                   */

#define CH_1  0                    /* MIDI channel 1                     */
#define CH_10 9                    /* MIDI channel 10                    */

#define LOUD_TICK 115              /* Metronome accented (beat 1)        */
#define SOFT_TICK  75              /* Metronome unaccented (beats 2-4)   */
#define VEL        75              /* default attack velocity of chords  */

#define HARMONICRHYTHM 2           /* 2=harmonic rhythm of half-notes    */
                                   /* note that this program probably    */
                                   /* cannot handle any changes to this  */
                                   /* define other than original value 2 */

#define LAGMAXINSEC 0.075          /* maximum 50 millisecond decision    */
                                   /* delay in output of chord from      */
                                   /* occurance of chordal beat          */

#define DIMINISHED_TRIAD  0         /* the following defines are for      */
#define MAJOR_TRIAD       1         /* determining the chord quality      */
#define MINOR_TRIAD       2         /* in the playChord functions         */
#define HALF_DIM_7TH      3
#define Mm_7TH            4


// program typedefs:
typedef void (*ChordPlayFunction)(int note, int chordstate);


// non-improv assistant function declarations defined at bottom of program:
int    chordBeat                (int beat, int meter);
void   playChordByNet           (int note, int chordstate);
void   playChordByRules         (int note, int chordstate);
double calculateMaxWait         (double tempo);
void   playMetronome            (int beat);


// global variables for the program:
double   tempo = 80;            // tempo of the accompaniment
int      currentnote = 60;      // note which was used to play the current chord
int      note = 0;              // last notemessage note coming from synth
SigTimer metronome;             // object to handle time for beats
int      meter = 4;             // meter to play chords in
int      beat;                  // the current beat in the meter (offset by 1)
double   beatfraction;          // the fractional part of the beat
int      notestates[128] = {0}; // booleans for keyboard notes on/off
int      oldstate;              // previous state position in beat
int      state;                 // current state position in beat
int      stateChange;           // boolean for testing if state has changed
int      notescan = 0;          // boolean for looking for a new currentnote
double   maxwait = 0.10;        // maximum wait time after a beat before 
                                //    choosing a default chord
MidiMessage notemessage;        // for extracting notes from the Synthesizer
double   lagmaxinsec =  0.075;  // maximum 75 millisecond decision 
                                //    delay in output of chord from  
                                //    occurance of chordal beat
ChordPlayFunction playChord = NULL; // func for playing chordal accompaniment

/*----------------- beginning of improvization algorithms ---------------*/

/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << 
   "NaNa ver. 6 May 1999: Automatic Chordal Accompaniment of simple melodies\n"
   "   Computer Keyboard commands: \n"
   "      , or < = slow down the tempo\n"
   "      . or > = speed up the tempo\n"
   "      [ = decrease chord decision lag time\n"
   "      ] = increase chord decision lag time\n"
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
   cout << "Enter a tempo for melody performance: ";
   echoKeysOn();
   cin  >> tempo;
   echoKeysOff();

   metronome.setTempo(tempo);
   maxwait = calculateMaxWait(tempo);

   playChord = playChordByRules;
   cout << "Using rules for playing accompaniment" << endl;
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
//   called and remains constant while in this function.
//

void mainloopalgorithms(void) { 
   // 1. check to see if we are in a new measure and update the
   // metronome accordingly.  If in 4/4, then the metronome will
   // be guarenteed to be between 0 and 3.99999 after the following
   // code is run.  The update will make sure that the metronome remains
   // synced exactly in time with the absolute beat. (Useful for 
   // polyphony, not really necessary in monophonic cases).
   if (metronome.expired() >= meter) {
      metronome.update(meter);
   }

   // 2. Determine the current beat of the meter.
   // We will want to play automated chords on beats one and three.
   beatfraction = metronome.getPeriodCount();
   beat = (int)beatfraction + 1;
   beatfraction -= beat - 1;

   // 3. Process the incoming MIDI note messages (if any), keeping track 
   // of the last note, and whether it is currently on or off.
   while (synth.getNoteCount() > 0) {
      notemessage = synth.extractNote();
      if (notemessage.p2() != 0) {
         note = notemessage.p1();
         notestates[note] = 1;
      } else {
         notestates[notemessage.p1()] = 0;
      }
   }

   // 4. Determine the position in time in the current beat.
   // There are two beat-parts which are called states:
   //    state == 0: we are at the start of the beat and may need to
   //                choose a new chord.
   //    state == 1: we are past the maximum wait time for a chord decision
   // Also, check to see if the state has changed from 0 to 1 or 1 to 0.
   oldstate = state;
   state = beatfraction < maxwait ? 0 : 1;
   stateChange = (state != oldstate);

   // 5. Check to see if a chord needs to be played.
   if (stateChange && state == 0) {
      playMetronome(beat);
      if (chordBeat(beat, meter)) {
         notescan = 1;
      } else {
         playChord(currentnote, OFF);
      }
   }

   if (notescan && notestates[note]) {   // if note played in beat window
      currentnote = note;
      playChord(currentnote, ON);
      notescan = 0;
   } else if (notescan && state == 1) {  // if too late for a new note
      playChord(currentnote, ON);
      notescan = 0;
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
   switch (key) {
      case ',':      // slow the tempo down
      case '<':
         tempo *= 0.95;
         metronome.setTempo(tempo);
         cout << "Tempo = " << tempo << endl;
         maxwait = calculateMaxWait(tempo);
         break;

      case '.':      // speed the tempo up
      case '>':
         tempo *= 1.05;
         metronome.setTempo(tempo);
         cout << "Tempo = " << tempo << endl;
         maxwait = calculateMaxWait(tempo);
         break;

      case '[':    // decrease the beat lag time in determing a chord
         lagmaxinsec -= 0.05;
         if (lagmaxinsec < 0.05) {
            lagmaxinsec = 0.05;
         }
         cout << "Chord decision time set to " << lagmaxinsec << endl;
         maxwait = calculateMaxWait(tempo);
         break;

      case ']':    // increase the beat lag time in determing a chord
         lagmaxinsec += 0.05;
         if (lagmaxinsec > 60.0/tempo - 0.05) {
            lagmaxinsec = 60.0/tempo - 0.05;
         }
         cout << "Chord decision time set to " << lagmaxinsec << endl;
         maxwait = calculateMaxWait(tempo);
         break;

      default:
         cout << "Undefined keyboard command" << endl;
   }

}


/*------------------ end improvization algorithms -----------------------*/

/*------------------ assisting functions for NaNa program ---------------*/

//////////////////////////////
//
// chordBeat -- returns true if we need to play a chord on this
//     beat, otherwise return false.  False will be used to
//     turn off the current chord in the mainloopalgorithms() function.
//

int chordBeat(int beat, int meter) {
   if ((beat-1) % HARMONICRHYTHM == 0) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// playChordByRules -- choose a chord and play the chosen chord.  If
//     note == 0, then turn off the current chord.  Chooses a chord to 
//     play as accompaniment according to the input note which was played 
//     on the beat (or last chord beat if no note is being played now).  
//     This function uses a neural network to decide the output chord.
//

void playChordByRules(int note, int chordstate) {
   static int rootNote = 60;
   static int chordQuality = MAJOR_TRIAD;
   int numNotes = 0;               // the number of notes to play
   static int chordNote[4] = {0};  // the notes of the chord to be calculated
   int i;

   // turn off the last chord notes if necessary:
   for (i=0; i<4; i++) {
      if (chordNote[i] != 0) {
         synth.play(CH_1, chordNote[i], 0);
         chordNote[i] = 0;
      }
   }
   if (chordstate == OFF) {
      return;
   }

   switch (note % 12) {
      case 0:
      case 4:
      case 7:
         rootNote = 48;
         chordQuality = MAJOR_TRIAD;
         break;
      case 5:
      case 9:
         rootNote = 53;
         chordQuality = MAJOR_TRIAD;
         break;
      case 2:
      case 11:
         rootNote = 55;
         chordQuality = Mm_7TH;
         break;
      default: ;
         // use the last chord if playing some chromatic notes
   }

   chordNote[0] = rootNote;
   switch (chordQuality) {

      case DIMINISHED_TRIAD:
         chordNote[1] = chordNote[0] + 3;
         chordNote[2] = chordNote[0] + 6;
         numNotes = 3;
         break;

      case MINOR_TRIAD:
         chordNote[1] = chordNote[0] + 3;
         chordNote[2] = chordNote[0] + 7;
         numNotes = 3;
         break;

      case MAJOR_TRIAD:
         chordNote[1] = chordNote[0] + 4;
         chordNote[2] = chordNote[0] + 7;
         numNotes = 3;
         break;

      case HALF_DIM_7TH:
         chordNote[1] = chordNote[0] + 3;
         chordNote[2] = chordNote[0] + 6;
         chordNote[3] = chordNote[0] + 10;
         numNotes = 4;
         break;

      case Mm_7TH:
         chordNote[1] = chordNote[0] + 4;
         chordNote[2] = chordNote[0] + 7;
         chordNote[3] = chordNote[0] + 10;
         numNotes = 4;
         break;
   
      default:
         return;        // unknown chord quality
   }


   for (i=0; i<numNotes; i++) {
      synth.play(CH_1, chordNote[i], VEL);
   }

}



//////////////////////////////
//
// calculateMaxWait -- use lagmaxinsec to calculate the beat fraction
//   of the wait period
//

double calculateMaxWait(double tempo) {
   double output = tempo/60.0 * lagmaxinsec;
   return output;
}


//////////////////////////////
//
// playMetronome --
//

void playMetronome(int beat) {
   synth.play(CH_10, GM_CLAVES, 0);
   if (beat == 1) {
      synth.play(CH_10, GM_CLAVES, LOUD_TICK);
   } else {
      synth.play(CH_10, GM_CLAVES, SOFT_TICK);
   }
}



// md5sum: 7fd948192cd5b3c1b5f070fd70742da4 nana1.cpp [20050403]

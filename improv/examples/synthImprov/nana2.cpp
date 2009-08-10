//
// Programmer:    Dan Gang <dang@ccrma.stanford.edu>
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed May  5 21:42:45 PDT 1999
// Last Modified: Mon May 10 22:29:39 PDT 1999
// Last Modified: Tue May 18 17:31:56 PDT 1999
// Filename:      ...sig/doc/examples/all/nana2/nana2.cpp
// Syntax:        C++; synthImprov 2.1; sigNet 1.0
//  
// Description:   Auto acompaniment of a simple melody.
//                Initial conversion of rule-based automatic accompaniment
//                to integrated version that includes automatic accompaniment
//                using a Jordan Neural Network
//

#include "synthImprov.h"           /* MIDI interface handling functions  */
#include "sigNet.h"                /* basic defintions for C++ NN's      */


// program defines:
#define ON  1                      /* play a chord                       */
#define OFF 0                      /* turn off a chord                   */

#define CH_1  0                    /* MIDI channel 1                     */
#define CH_2  1                    /* MIDI channel 2                     */
#define CH_10 9                    /* MIDI channel 10                    */

#define LOUD_TICK 105              /* Metronome accented (beat 1)        */
#define SOFT_TICK  85              /* Metronome unaccented (beats 2-4)   */
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
void   displayTrace      (int phrase, int measure, int beat, int pc, int chord);
const char* name                (int pitchclass);
const char* chordName           (int chord);


// global variables for the program:
double   tempo = 80;            // tempo of the accompaniment
int      currentnote = 60;      // note which was used to play the current chord
int      note = 0;              // last notemessage note coming from synth
SigTimer metronome;             // object to handle time for beats
int      meter = 4;             // meter to play chords in
int      beat;                  // the current beat in the meter (offset by 1)
double   beatfraction;          // the fractional part of the beat
int      notestates[128] = {0}; // booleans for keyboard notes on/off
int      oldstate = 1;          // previous state position in beat
int      state = 1;             // current state position in beat
int      stateChange;           // boolean for testing if state has changed
int      notescan = 0;          // boolean for looking for a new currentnote
double   maxwait = 0.10;        // maximum wait time after a beat before 
                                //    choosing a default chord
MidiMessage notemessage;        // for extracting notes from the Synthesizer
MidiMessage keymessage;         // for spoofing MIDI input frm computer keyboard
double   lagmaxinsec =  0.075;  // maximum 75 millisecond decision 
                                //    delay in output of chord from  
                                //    occurance of chordal beat
ChordPlayFunction playChord = NULL; // func for playing chordal accompaniment
SigTimer timer;                 // for timing the network activation etc.
int      time1, time2;          // for timing the network activation etc.
int      pauseQ = 0;            // for pausing the program

int      autoQ = 0;             // for automatic accompaniment on Yamaha PSR-520
int      syncQ = 0;             // for metronome synchronization with keyboard
SigTimer autoTimer;             // for automatic accompaniment on Yamaha PSR-520
int      displayQ = 1;          // true to display input note and decision


// Neural Network variables for automatic chordal accompaniment:
ChordNet ann;                   // Jordan Neural Network for auto accompaniment
int      phraseCount = 0;       // the current phrase number
int      measureCount = 0;      // the current measure number
int      beatCount = 0;         // the current beat count (0=1, 1=3)
int      clickCount = 0;        // for counting out the start of the piece


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
   "NaNa ver. 18 May 1999: Automatic Chordal Accompaniment of simple melodies\n"
   "   Computer Keyboard commands: \n"
   "      n = use neural network to play chords\n"
   "      r = use rules to play chords\n"
   "      d = toggle display of input note and chord decisions\n"
   "      , or < = slow down the tempo\n"
   "      . or > = speed up the tempo\n"
   "      z = reset the metrical variables to beginning of piece\n"
   "      [ = decrease chord decision lag time\n"
   "      ] = increase chord decision lag time\n"
   "      p = pause/unpause the program       \n"
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

   if (tempo > 220) {
      tempo = 220;
   } else if (tempo < 40) {
      tempo = 40;
   }

   metronome.setTempo(tempo);
   metronome.reset();
   autoTimer.setTempo(tempo*24);       // 24 clock ticks per quarter note
   maxwait = calculateMaxWait(tempo);

   playChord = playChordByNet;
   cout << "Using network for playing accompaniment" << endl;

   keymessage.p0() = 0x90; 

/*
   char netfilename[1024] = {0};
   fstream netfile;
   netfile.open("chord.net", ios::in | ios::nocreate);
   echoKeysOn();
   while (!netfile.is_open()) {
      cout << "Enter filename of network connection weights";
      cin  >> netfilename;
   }
   echoKeysOff();
   cout << "Reading neural network connection weights, etc..." << flush;
   ann.initialize(netfile);
   cout << " Done." << endl;
*/
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
   if (pauseQ) {
      return;
   }
 
   if (syncQ && autoQ && autoTimer.expired()) {
      synth.rawsend(0xf8);
      autoTimer.update();
   }

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
      if (autoQ && syncQ == 0) {
         syncQ = 1;
         synth.rawsend(0xf8);
         autoTimer.reset();
      }
      clickCount++;
      if (clickCount == 5) {
         clickCount = 10;
         metronome.update(4);
      }
      if (clickCount > 4) {
         if (chordBeat(beat, meter)) {
            notescan = 1;
         } else {
            playChord(currentnote, OFF);
         }
      } else {
         return;
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
         if (tempo > 220) {
            tempo = 220;
         } else if (tempo < 40) {
            tempo = 40;
         }
         metronome.setTempo(tempo);
         autoTimer.setTempo(tempo*24);
         cout << "Tempo = " << tempo << endl;
         maxwait = calculateMaxWait(tempo);
         break;

      case '.':      // speed the tempo up
      case '>':
         tempo *= 1.05;
         if (tempo > 220) {
            tempo = 220;
         } else if (tempo < 40) {
            tempo = 40;
         }
         metronome.setTempo(tempo);
         autoTimer.setTempo(tempo*24);
         cout << "Tempo = " << tempo << endl;
         maxwait = calculateMaxWait(tempo);
         break;

      case 'a':     // start/stop the autoaccompaniment
         autoQ = !autoQ;
         if (autoQ) {
            synth.rawsend(0xfa);
            cout << "\nStarting auto accompaniment mode" << endl;
         } else {
            syncQ = 0;
            synth.rawsend(0xfc);
            cout << "\nStopping auto accompaniment mode" << endl;
         }
         break;

      case 'n':     // use the neural network to play the chordal accompaniment
         playChord = playChordByNet;
         cout << "Using network to play chordal accompaniment" << endl;
         break;

      case 'r':     // use rules to play the chordal accompaniment
         playChord = playChordByRules;
         cout << "Using rules to play chordal accompaniment" << endl;
         break;

      case 'z':     // reset the metrical variables to start of piece
         cout << "resetting metrical information to beginning of piece" << endl;
         phraseCount = 0;
         measureCount = 0;
         beatCount = 0;
         clickCount = 0;
         break;
       
      case 'd':    // toggle display of input note and chord decisions
         displayQ = !displayQ;
         if (displayQ) {
            cout << "\nDisplay turned on" << endl;
         } else {
            cout << "\nDisplay turned off" << endl;
         }
         break;

      case '[':    // decrease the beat lag time in determing a chord
         lagmaxinsec -= 0.005;
         if (lagmaxinsec < 0.020) {
            lagmaxinsec = 0.020;
         }
         cout << "Chord decision time set to " 
              << lagmaxinsec*1000 << " ms"<< endl;
         maxwait = calculateMaxWait(tempo);
         break;

      case ']':    // increase the beat lag time in determing a chord
         lagmaxinsec += 0.005;
         if (lagmaxinsec > 60.0/tempo - 0.005) {
            lagmaxinsec = 60.0/tempo - 0.005;
         }
         cout << "Chord decision time set to " 
              << lagmaxinsec*1000 << " ms" << endl;
         maxwait = calculateMaxWait(tempo);
         break;

      case 'p':   // pause/unpause
         pauseQ = !pauseQ;
         if (pauseQ) {
            cout << "\nProgram paused..." << endl;
         } else {
            cout << "\nProgram unpaused." << endl;
         }
         break;

      case '1':   // C note
         keymessage.p2() = 0;
         synth.insert(keymessage);
         keymessage.p1() = 60;
         keymessage.p2() = VEL;
         synth.insert(keymessage);
         break;

      case '2':   // D note
         keymessage.p2() = 0;
         synth.insert(keymessage);
         keymessage.p1() = 62;
         keymessage.p2() = VEL;
         synth.insert(keymessage);
         break;

      case '3':   // E note
         keymessage.p2() = 0;
         synth.insert(keymessage);
         keymessage.p1() = 64;
         keymessage.p2() = VEL;
         synth.insert(keymessage);
         break;

      case '4':   // F note
         keymessage.p2() = 0;
         synth.insert(keymessage);
         keymessage.p1() = 65;
         keymessage.p2() = VEL;
         synth.insert(keymessage);
         break;

      case '5':   // G note
         keymessage.p2() = 0;
         synth.insert(keymessage);
         keymessage.p1() = 67;
         keymessage.p2() = VEL;
         synth.insert(keymessage);
         break;

      case '6':   // A note
         keymessage.p2() = 0;
         synth.insert(keymessage);
         keymessage.p1() = 69;
         keymessage.p2() = VEL;
         synth.insert(keymessage);
         break;

      case '7':   // B note
         keymessage.p2() = 0;
         synth.insert(keymessage);
         keymessage.p1() = 71;
         keymessage.p2() = VEL;
         synth.insert(keymessage);
         break;

      case '8':   // C octave
         keymessage.p2() = 0;
         synth.insert(keymessage);
         keymessage.p1() = 72;
         keymessage.p2() = VEL;
         synth.insert(keymessage);
         break;

      case '9':   // Rest
         keymessage.p2() = 0;
         synth.insert(keymessage);
         break;

      default: ;
         // cout << "Undefined keyboard command" << endl;
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
// chordName -- display the current chord being played
//

const char* chordName(int chord) {
   switch (chord) {
      case 0:        return "C";
      case 1:        return "Dm";
      case 2:        return "Em";
      case 3:        return "F";
      case 4:        return "G";
      case 5:        return "Am";
      case 6:        return "Bdim";
      case 7:        return "C7";
      case 8:        return "D7";
      case 9:        return "E7";
      case 10:       return "F7";
      case 11:       return "G7";
      case 12:       return "A7";
      case 13:       return "Bb5m7";
      case 14:       return "Fm";
   }

   return "unknown";
}


//////////////////////////////
//
// displayTrace -- display the current note and playing choice
//

void displayTrace(int phrase, int measure, int beat, int pc, int chord) {
   int mphrase = phrase % 4;
   int mmeasure = measure % 4;
   int mbeat = (beat-1) % 2;
   
   if (mphrase == 0 && mmeasure == 0 && mbeat == 0) {
      cout << "\n\n++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
   }
   
   if (mmeasure == 0 && mbeat == 0) {
      cout << endl;
   }

   if (mbeat == 0) {
      cout << "\nbar:";
      cout.width(2);
      cout << measure % 16 + 1 << ", ";
   }

   cout << "note = " << name(pc) << ", output chord = "
        << chordName(chord) << ",\t" << flush;

}



//////////////////////////////
//
// name -- for printing the ascii note name of a given MIDI note.
//

const char* name(int pc) {
   pc = pc % 12;
   switch (pc) {
      case 0:      return "C ";
      case 1:      return "C#";
      case 2:      return "D ";
      case 3:      return "Ef";
      case 4:      return "E ";
      case 5:      return "F ";
      case 6:      return "F#";
      case 7:      return "G ";
      case 8:      return "Af";
      case 9:      return "A ";
      case 10:      return "Bf";
      case 11:      return "B ";
   }
 
   return "unknown";
}



//////////////////////////////
//
// playChordByNet -- chooses a chord to play as accompaniment according
//    to the input note which was played on the beat (or last chord beat
//    if no note is being played now).  This function uses a neural network
//    to decide the output chord.
//

void playChordByNet(int note, int chordstate) {
   static int rootNote = 48;
   static int chordQuality = MAJOR_TRIAD;
   int numNotes = 0;               // the number of notes to play
   static int chordNote[4] = {0};  // the notes of the chord to be calculated
   int i;
   int winnerUnit;

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

   // 1. Map note to internal neural network representation

   // 1a. Set the external input note
   int pitchclass = note % 12;
   ann.Melody.zero();
   ann.Melody[pitchclass] = 1.0;

   // 1b. Set the external meter input
   // 1b1: set phrase number (first two units: 1=0a, 2=aa, 3=00, 4=aa)
   // 1b2: set measure number in phrase (next two units: 1=00, 2=0a, 3=a0, 4=aa)
   // 1b3: set beat number in measure (4/4 meter) (last two units: 1=a0, 3=0a)

////////
   ann.Meter.zero();
   switch (phraseCount % 4) {
      case 0: ann.Meter[1] = 1;  break;            // phrase 1
      case 1: ann.Meter[0] = 1; ann.Meter[1] = 1;  // phrase 2
      case 2: break;                               // phrase 3
      case 3: ann.Meter[0] = 1; ann.Meter[1] = 1;  // phrase 4
   }
   switch (measureCount % 4) {
      case 0: break;                               // measure 1
      case 1: ann.Meter[3] = 1;                    // measure 2
      case 2: ann.Meter[2] = 1;                    // measure 3
      case 3: ann.Meter[2] = 1; ann.Meter[3] = 1;  // measure 4
   }
   switch (beatCount % 2) {
      case 0: ann.Meter[4] = 1;       // beat 1
      case 1: ann.Meter[5] = 1;       // beat 3
   }
////////

/*
   switch (phraseCount % 4) {
      case 0: ann.Meter.set(0, "0a");  break;   // phrase 1
      case 1: ann.Meter.set(0, "aa");  break;   // phrase 2
      case 2: ann.Meter.set(0, "00");  break;   // phrase 3
      case 3: ann.Meter.set(0, "aa");  break;   // phrase 4
   }
   switch (measureCount % 4) {
      case 0: ann.Meter.set(2, "00");  break;   // measure 1
      case 1: ann.Meter.set(2, "0a");  break;   // measure 2
      case 2: ann.Meter.set(2, "a0");  break;   // measure 3
      case 3: ann.Meter.set(2, "aa");  break;   // measure 4
   }
   switch (beatCount % 2) {
      case 0: ann.Meter.set(4, "a0");  break;   // beat 1
      case 1: ann.Meter.set(4, "0a");  break;   // beat 3
   }
*/


   // increment the meter tracking variables:
   beatCount++;
   if (beatCount % 2 == 0) {
      measureCount++;
      if (measureCount % 4 == 0) {
         phraseCount++;
      }
   }

   // 2. Activate the Neural Network
   ann.activate();
 
   // 3. Map from Net Output to chord.  Get the rootNote from the Network.
   winnerUnit = 0;
   double max = -2;
   for (int z=0; z<ann.Output.getSize(); z++) {
      if (max < ann.Output[z]) {
         max = ann.Output[z];
         winnerUnit = z;
      }
   }

   if (displayQ) {
      displayTrace(phraseCount, measureCount, beatCount, 
            pitchclass, winnerUnit);
   }

   // 4. Now that the rootNote has been determined, choose the correct
   // chord quality and play.

 switch (winnerUnit) {
      case 0:    rootNote = 0;   chordQuality = MAJOR_TRIAD;      break;
      case 1:    rootNote = 2;   chordQuality = MINOR_TRIAD;      break;
      case 2:    rootNote = 4;   chordQuality = MINOR_TRIAD;      break;
      case 3:    rootNote = 5;   chordQuality = MAJOR_TRIAD;      break;
      case 4:    rootNote = 7;   chordQuality = MAJOR_TRIAD;      break;
      case 5:    rootNote = 9;   chordQuality = MINOR_TRIAD;      break;
      case 6:    rootNote = 11; chordQuality = DIMINISHED_TRIAD;  break;
      case 7:    rootNote = 0;   chordQuality = Mm_7TH;           break;
      case 8:    rootNote = 2;   chordQuality = Mm_7TH;           break;
      case 9:    rootNote = 4;   chordQuality = Mm_7TH ;          break;
      case 10:  rootNote = 5;   chordQuality = Mm_7TH ;           break;  
      case 11:  rootNote = 7;   chordQuality = Mm_7TH ;           break; 
      case 12:  rootNote = 9;   chordQuality = Mm_7TH ;           break; 
      case 13:  rootNote = 11; chordQuality = Mm_7TH ;            break; 
      case 14:  rootNote = 5;   chordQuality = HALF_DIM_7TH;      break;  
      default: ;
   }

   chordNote[0] = rootNote + 48;
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



// md5sum: 24ebd1688a49e4b8fc5b584c5c969e4c nana2.cpp [20050403]

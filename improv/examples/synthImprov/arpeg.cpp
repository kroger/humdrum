//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Feb  8 11:56:42 GMT-0800 1998
// Last Modified: Sun Feb  8 14:59:09 GMT-0800 1998
// Filename:      ...sig/doc/examples/improv/synthImprov/arpeg/arpeg.cpp
// Syntax:        C++; synthImprov 0.5
//
// Description:  You press a key on the keyboard, and a chord will
//      be played according to the selected quality, onsets, and durations.
//
//      Chord Qualities are selected from the computer keyboard from these keys:
//              "q" = diminished triad
//              "w" = minor triad
//              "e" = major triad
//              "r" = augmented triad
//              "t" = fully-diminished seventh chord
//              "y" = half-diminished seventh chord
//              "u" = minor-minor seventh chord
//              "i" = minor-major seventh chord
//              "o" = major-minor (dominant) seventh chord
//              "p" = major-major seventh chord
//

#include "synthImprov.h"


#define  DIMINISHED_TRIAD (1)
#define  MINOR_TRIAD      (2)
#define  MAJOR_TRIAD      (3)
#define  AUGMENTED_TRIAD  (4)
#define  FULLY_DIM_7TH    (5)
#define  HALF_DIM_7TH     (6)
#define  mm_7TH           (7)
#define  mM_7TH           (8)
#define  Mm_7TH           (9)
#define  MM_7TH           (10)


/*----------------- beginning of improvization algorithms ---------------*/

int octave = 4;                 // octave range for computer keyboard notes
int keyboardnote = 0;           // computer keyboard note
MidiMessage noteMessage;        // for reading keyno and velocity (and time)
EventBuffer eventBuffer(2000);  // for future notes
int chordType = MAJOR_TRIAD;    // the type of chord to play when key pressed
int onset[4] = {0};             // the rhythm of the chord
int duration[4] = {0};          // the duration of the chord notes
int attack[4] = {0};            // attack velocity for chords
int offset = 0;                 // for delays (of Disklavier)


void description(void) {
   printboxtop();
   printstringline(
"   Arpeg -- by Craig Stuart Sapp <craig@ccrma.stanford.edu> -- 8 Feb 1998");
   printstringline("");
   printstringline(
"  Description:  You press a key on the keyboard, and a chord will");
   printstringline(
"    be played according to the selected quality, onsets, and durations.");
   printstringline(
"    Computer keyboard keys are assigned random attack velocities.");
   printintermediateline();
   printstringline("  User commands:");
   printstringline(
"            \"\\\" = record rhythms, durations     \"=\" = display rhythms");
   printstringline(
"            \"[\" = lower timing offset           \"]\" = raise timing offset");
   printstringline(
"    Chords: \"q\"=dim       \"w\"=minor   \"e\"=major   \"r\"=aug   \"t\"=full dim");
   printstringline(
"            \"y\"=half dim  \"u\"=mm7     \"i\"=mM7     \"o\"=Mm7   \"p\"=MM");
   printstringline(" ");
   printstringline(
"      \"0\"-\"9\" = octave number of computer keyboard notes");
   printstringline(
"      Notes:           s   d      g    h   j   ");
   printstringline(
"                     z   x   c   v   b   n   m  ");
   printboxbottom();
}



void initialization(void) {
   eventBuffer.setPollPeriod(10);

   onset[0] = 0;
   onset[1] = 100;
   onset[2] = 200;
   onset[3] = 300;

   duration[0] = 600;
   duration[1] = 500;
   duration[2] = 400;
   duration[3] = 300;
}



void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/


void playchord(MidiMessage aMessage, int chordQuality, 
      int* rhythm, int* dur) {
   int numNotes = 0;             // the number of notes to play
   NoteEvent tempNote;           // temporary Note for copying into eventBuffer
   int chordNote[4];             // the notes of the chord to be calculated
   int rootNote = aMessage.p1(); // root of chord to be created

   chordNote[0] = rootNote;
   switch (chordQuality) {
      case DIMINISHED_TRIAD:
         chordNote[1] = rootNote + 3;  chordNote[2] = rootNote + 6;
         numNotes = 3;
         break;
      case MINOR_TRIAD:
         chordNote[1] = rootNote + 3;  chordNote[2] = rootNote + 7;
         numNotes = 3;
         break;
      case MAJOR_TRIAD:
         chordNote[1] = rootNote + 4;  chordNote[2] = rootNote + 7;
         numNotes = 3;
         break;
      case AUGMENTED_TRIAD:
         chordNote[1] = rootNote + 4;  chordNote[2] = rootNote + 8;
         numNotes = 3;
         break;
      case FULLY_DIM_7TH:
         chordNote[1] = rootNote + 3;
         chordNote[2] = rootNote + 6;
         chordNote[3] = rootNote + 9;
         numNotes = 4;
         break;
      case HALF_DIM_7TH:
         chordNote[1] = rootNote + 3;
         chordNote[2] = rootNote + 6;
         chordNote[3] = rootNote + 10;        
         numNotes = 4;
         break;
      case mm_7TH:
         chordNote[1] = rootNote + 3;
         chordNote[2] = rootNote + 7;
         chordNote[3] = rootNote + 10;        
         numNotes = 4;
         break;
      case mM_7TH:
         chordNote[1] = rootNote + 3;
         chordNote[2] = rootNote + 7;
         chordNote[3] = rootNote + 11;        
         numNotes = 4;
         break;
      case Mm_7TH:
         chordNote[1] = rootNote + 3;
         chordNote[2] = rootNote + 4;
         chordNote[3] = rootNote + 10;        
         numNotes = 4;
         break;
      case MM_7TH:
         chordNote[1] = rootNote + 4;
         chordNote[2] = rootNote + 7;
         chordNote[3] = rootNote + 10;        
         numNotes = 4;
         break;
      default:                                    // invalid quality
         return;
   }

   cout << "Chord: (";
   for (int i=0; i<numNotes; i++) {
      tempNote.setKeyno(chordNote[i]);
      if (tempNote.getKeyno() < 0 || tempNote.getKeyno() > 127)  continue;

      if (attack[i] == 0) {
         tempNote.setVelocity(aMessage.p2());
      } else {
         tempNote.setVelocity(attack[i]);
      }

      tempNote.setOnDur(t_time+rhythm[i]+offset, dur[i]);
      tempNote.setStatus(0);                   // note hasn't been played yet
      eventBuffer.insert(&tempNote);

      cout << tempNote.getKeyno();
      if (i != numNotes-1)  cout << ",";
   }
   cout << ")" << endl;
}



void mainloopalgorithms(void) {
   eventBuffer.checkPoll();             // see if any notes need playing

   while (synth.getNoteCount() > 0) {
      noteMessage = synth.extractNote();
      if (noteMessage.p2() != 0) {
         playchord(noteMessage, chordType, onset, duration);
      }
   }
}



/*-------------------- triggered algorithms -----------------------------*/

void recordRhythms(void) {
   // remove all previous notes from input
   while (synth.getNoteCount() > 0) synth.extractNote();

   cout << "Play four notes to record rhythms and durations for chords" << endl;
   int oncount = 0;
   int offcount = 0;
   int finished[4] = {0};
   int offnote;
   int offtime;
   int playedNotes[4] = {0};
   int startTime = 0;
   MidiMessage noteMessage;
   while (oncount <= 4 && offcount <=4 ) {
      if (interfaceKeyboard.hit())  checkKeyboard();
      synth.processIncomingMessages();
      if (synth.getNoteCount() > 0) {
         noteMessage = synth.extractNote();
         if (oncount < 4 && noteMessage.p2() != 0) {
            if (oncount == 0)  startTime = noteMessage.time;
            playedNotes[oncount] = noteMessage.p1();
            attack[oncount] = noteMessage.p2();
            onset[oncount] = noteMessage.time - startTime;
            oncount++;
         } else if (noteMessage.p2() == 0) {
            offtime = noteMessage.time;
            offnote = noteMessage.p1();
            for (int i=0; i<oncount; i++) {
               if ((offnote == playedNotes[i]) && (finished[i] == 0)) {
                  duration[i] = offtime - onset[i] - startTime;
                  offcount++;
                  finished[i] = 1;
                  break;
               }
            }
         }
      }
      if ((oncount >= 4) && (offcount >=4))  break;
   }

   cout << "Finished recording new rhythms and durations" << endl;

   cout << "Onsets:    \t" << onset[0] << "\t" << onset[1]
        << "\t" << onset[2] << "\t" << onset[3] << endl;
   cout << "durations: \t" << duration[0] << "\t" << duration[1]
        << "\t" << duration[2] << "\t" << duration[3] << endl;

}



void keyboard(int key) {
   synth.play(0, keyboardnote, 0);
   noteMessage.time = mainTimer.getTime();
   noteMessage.command() = 0x90;
   noteMessage.p1() = keyboardnote;
   noteMessage.p2() = 0;
   synth.insert(noteMessage);
   switch (key) {
      case 'z': keyboardnote = 12 * octave + 0;  break;    // C
      case 's': keyboardnote = 12 * octave + 1;  break;    // C#
      case 'x': keyboardnote = 12 * octave + 2;  break;    // D
      case 'd': keyboardnote = 12 * octave + 3;  break;    // D#
      case 'c': keyboardnote = 12 * octave + 4;  break;    // E
      case 'v': keyboardnote = 12 * octave + 5;  break;    // F
      case 'g': keyboardnote = 12 * octave + 6;  break;    // F#
      case 'b': keyboardnote = 12 * octave + 7;  break;    // G
      case 'h': keyboardnote = 12 * octave + 8;  break;    // G#
      case 'n': keyboardnote = 12 * octave + 9;  break;    // A
      case 'j': keyboardnote = 12 * octave + 10;  break;   // A#
      case 'm': keyboardnote = 12 * octave + 11;  break;   // B
      case ',': keyboardnote = 12 * octave + 12;  break;   // C
      default: return;
   }
   if (keyboardnote < 0)  keyboardnote = 0;
   else if (keyboardnote > 127)  keyboardnote = 127;
   noteMessage.time = mainTimer.getTime();
   noteMessage.command() = 0x90;
   noteMessage.p1() = keyboardnote;
   noteMessage.p2() = rand()%47 + 80;      // random int from 1 to 127
   synth.play(0, noteMessage.p1(), noteMessage.p2());
   synth.insert(noteMessage);
}



void keyboardchar(int key) { 
   if (isdigit((char)key)) {
      octave = key - '0';   
      return;
   }
   switch (key) {
      case 'q':
         chordType = DIMINISHED_TRIAD;
         break;
      case 'w':
         chordType = MINOR_TRIAD;
         break;
      case 'e':
         chordType = MAJOR_TRIAD;
         break;
      case 'r':
         chordType = AUGMENTED_TRIAD;
         break;
      case 't':
         chordType = FULLY_DIM_7TH;
         break;
      case 'y':
         chordType = HALF_DIM_7TH;
         break;
      case 'u':
         chordType = mm_7TH;
         break;
      case 'i':
         chordType = mM_7TH;
         break;
      case 'o':
         chordType = Mm_7TH;
         break;
      case 'p':
         chordType = MM_7TH;
         break;
      case '[':
         offset--;
         break;
      case ']':
         offset++; 
         break;
      case '\\':
         recordRhythms();
         break;
      case '=':
         cout << "Onsets:    \t" << onset[0] << "\t" << onset[1]
              << "\t" << onset[2] << "\t" << onset[3] << endl;
         cout << "durations: \t" << duration[0] << "\t" << duration[1]
              << "\t" << duration[2] << "\t" << duration[3] << endl;
         cout << "Offset: " << offset << endl;
         break;
      default:
         keyboard(key);
   }
}


/*------------------ end improvization algorithms -----------------------*/



// md5sum: 811c54faae4ee5940c11c0a7dc1f5edb arpeg.cpp [20050403]

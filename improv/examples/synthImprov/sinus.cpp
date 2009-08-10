//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 23 January 1998
// Last Modified: Sat Jan 16 15:08:32 PST 1999
// Filename:      ...sig/doc/examples/improv/synthImprov/sinus/sinus.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: you press a key on the synthesizer keyboard, and a pitch 
//	pattern shaped like a sinewave is generated.  Attack velocity give 
//	amplitude of sine wave.  Alternatively, there is a synthesizer
//	keyboard mapped to the computer keyboard: the octave is controlled
//	by the numbers 0-9, and the chromatic pitches are on the bottom
//	two rows of the keyboard (e.g., 'z' is the note C).  '-' and '='
//	control the period size which controls how many notes to play
//	in one sinewave cycle.  '[' and ']' controls the tempo of the
//	sinewave notes in 100 millisecond increments.  If you play
//	a note loudly, then the sinewave pattern will be soft, and 
//	if you play a note softly, then the pattern will be loud.
//	The pattern is played as long as you are holding down a note.
//	If you are playing notes from the computer keyboard, then
//	their attack velocities are assigned random values from 1 to
//	127 since computer keyboards don't record attack velocity.
//

#include "synthImprov.h" 

/*----------------- beginning of improvization algorithms ---------------*/

#define TWOPI 6.283185307

int centernote = 0;          // played note which centers sinewave pattern
int amplitude = 0;           // greatest deviation amplitude of sine function
int sinenote = 0;            // current note of the sinewave
int sinevelocity = 0;        // current velocity of the sinewave
int period = 5;              // how many notes to play in the sinewave pattern
int counte = 0;               // which note of the period we are on
int duration = 200;          // duration of sine notes
int octave = 4;              // octave range for computer keyboard notes
int keyboardnote = 0;        // computer keyboard note
int lastoff = 0;             // last note which was turned off
int nextnotetime = 0;        // time for the next note
MidiMessage noteMessage;     // for reading keyno and velocity (and time)
Voice sinevoice;             // for handling note-offs


/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();
   psl("Sinus -- by Craig Stuart Sapp <craig@ccrma.stanford.edu>, 24 Jan 1998");
   psl("");
   psl(" Description: press a key on the synthesizer keyboard, and a pitch ");
   psl(" pattern shaped like a sinewave is generated.  Attack velocity gives");
   psl(" inverse amplitude of sine wave.  Computer keyboard keys are assigned");
   psl(" random attack velocities");
   printintermediateline();
   psl("  User commands:");
   psl("     \"-\" = decrease period length   \"=\" = increase period length ");
   psl("     \"[\" = decrease duration        \"]\" = increase duration");
   psl(" ");
   psl("     \"0\"-\"9\" = octave number of computer keyboard notes");
   psl("         Notes:           s   d      g    h   j   ");
   psl("                        z   x   c   v   b   n   m  ");
   printboxbottom();
} 



void initialization(void) { 
   sinevoice.setPort(synth.getOutputPort());
   sinevoice.setChannel(0);
}

void finishup(void) { 
   sinevoice.off();
}


/*-------------------- main loop algorithms -----------------------------*/


void mainloopalgorithms(void) { 
   while (synth.getNoteCount() > 0) {
      noteMessage = synth.extractNote();
      if (noteMessage.p2() != 0) {
         sinevelocity = 127 - noteMessage.p2();
      } else  {
         break;
      }
      centernote = noteMessage.p1();
      counte = 0;
      amplitude = midiscale(noteMessage.p2(), 0, 24);
      cout << "\rcenter = " << setw(4) << centernote
           << " amplitude = " << setw(4) << amplitude << "\t\t\t\t";
      cout.flush();
   }

   if (nextnotetime <= t_time && sinevelocity != 0) {
      nextnotetime = t_time + duration;
      sinenote = (int)(centernote + amplitude * 
            sin(TWOPI * counte / period) + 0.5);
      sinevoice.play(sinenote, sinevelocity);
      counte++;
   }
}
      

/*-------------------- triggered algorithms -----------------------------*/

void keyboard(int key) {
   synth.play(0, keyboardnote, 0);
   noteMessage.command() = 0x90;
   noteMessage.p1() = keyboardnote;
   noteMessage.p2() = 0;
   synth.insert(noteMessage);
   switch (key) {
      case 'z': keyboardnote = 12 * octave + 0;  break;   // C
      case 's': keyboardnote = 12 * octave + 1;  break;   // C#
      case 'x': keyboardnote = 12 * octave + 2;  break;   // D
      case 'd': keyboardnote = 12 * octave + 3;  break;   // D#
      case 'c': keyboardnote = 12 * octave + 4;  break;   // E
      case 'v': keyboardnote = 12 * octave + 5;  break;   // F
      case 'g': keyboardnote = 12 * octave + 6;  break;   // F#
      case 'b': keyboardnote = 12 * octave + 7;  break;   // G
      case 'h': keyboardnote = 12 * octave + 8;  break;   // G#
      case 'n': keyboardnote = 12 * octave + 9;  break;   // A
      case 'j': keyboardnote = 12 * octave + 10;  break;   // A#
      case 'm': keyboardnote = 12 * octave + 11;  break;   // B
      case ',': keyboardnote = 12 * octave + 12;  break;   // C
      default: return;
   }
   if (keyboardnote < 0)  keyboardnote = 0;
   else if (keyboardnote > 127)  keyboardnote = 127;
   noteMessage.command() = 0x90;
   noteMessage.p1() = keyboardnote;
   noteMessage.p2() = rand()%127 + 1;      // random int from 1 to 127
   synth.play(0, noteMessage.p1(), noteMessage.p2());
   synth.insert(noteMessage);
}



void keyboardchar(int key) { 
   if (isdigit((char)key)) {
      octave = key - '0';   
      return;
   }
   switch (key) {
      case '-': 
         period--; 
         if (period < 1)  period = 1;
         cout << "\r period = " << period << "   \t\t\t\t\t";
         cout.flush();
         break;
      case '=':
         period++; 
         if (period > 50)  period = 50;
         cout << "\r period = " << period << "   \t\t\t\t\t";
         cout.flush();
         break;
      case '[':
         duration -= 25;
         if (duration < 25)  duration = 25;
         break;
      case ']':
         duration += 25;
         break;
      default:
         keyboard(key);
   }
}


/*------------------ end improvization algorithms -----------------------*/


// md5sum: 50a059b97e63b61ce970727ba865e4fe sinus.cpp [20050403]

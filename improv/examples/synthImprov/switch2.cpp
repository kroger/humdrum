//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 24 January 1998
// Last Modofied:  9 November 1998
// Filename:      ...sig/doc/examples/improv/synthImprov/switch2/switch2.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: Inverts pitch and attack parameters of keys played on 
//     the synthesizer keyboard.  On the computer keyboard, the 
//     keys "-" and "=" will change the timbre of the notes which
//     are being switched.  On the computer keyboard, the notes
//     z x c v b n m , . / represent the notes C D E F G A B C D E
//     and the notes s d g h j l ; represent the black notes
//     associated with the lower row of notes.
//

#include "synthImprov.h" 


/*----------------- beginning of improvization algorithms ---------------*/

int channel = 0;         // MIDI channel (offset 0) on which to play notes 
MidiMessage message;     // for reading keyno and velocity (and time)
int offNote[128] = {0};  // for keeping track of note-offs
int instrument = GM_VIBRAPHONE; // initial timbre of output notes
int sustain = 0;         // for sustain control

// function declarations:
void sillyKeyboard(int key, int chan = 0);


/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();
   psl(
   "  SWITCH2 - by Craig Stuart Sapp <craig@ccrma.stanford.edu> - 9 Nov 1998");
   psl("");
   psl("  Description: Inverts pitch and attack parameters of keys ");
   psl("    played on the synthesizer keyboard.");
   printintermediateline();
   psl("  User commands:");
   psl("     \"-\" = decrement timbre no.   \"=\" = increment timbre no. ");
   psl("     tab = sustain toggle.");
   psl("     \"0\"-\"9\" = octave number of computer keyboard notes");
   psl("     Computer notes:   s   d      g    h   j       l   ;");
   psl("                     z   x   c   v   b   n   m   ,   .   /");
   printboxbottom();
} 



void initialization(void) { 
   synth.pc(channel, instrument);   // send patch change to synthesizer
}


void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

void mainloopalgorithms(void) { 
 
   // process all of the note messages waiting in the input buffer:
   while (synth.getNoteCount() > 0) {
      message = synth.extractNote();
      if (message.p2() == 0) {              // incoming note-off message
         synth.play(channel, offNote[message.p1()], 0);
      } else {                              // incoming note-on  message
         synth.play(channel, message.p2(), message.p1());
         offNote[message.p1()] = message.p2();
      }
   }

}
      

/*-------------------- triggered algorithms -----------------------------*/

void keyboardchar(int key) { 
   switch (key) {
      case '-':        // decrement timbre
         instrument = (instrument - 1 + 127) % 128;
         cout << "Instrument: " << instrument << endl;
         synth.pc(channel, instrument);
         break;
      case '=':        // increment timbre
         instrument = (instrument + 1) % 128;
         cout << "Instrument: " << instrument << endl;
         synth.pc(channel, instrument);
         break;
      case '\t':       // sustain pedal on/off
         sustain = !sustain;
         synth.sustain(channel, sustain);
         cout << "Sustain: " << sustain << endl;
         break;
      default:  
         sillyKeyboard(key);
   }
}



//////////////////////////////
//
// sillyKeyboard -- converts the bottom two rows of the 
//     computer keyboard into a synthesizer keyboard.  The attack
//     velocities are not controllable, so they are randomly generated.
//     The octave number is controlled by the characters 0 through 9.
//     This function will send out the specified note to the 
//     external synthsizer for playing and also put the note into the
//     synth's input MIDI buffer (so that the synth object thinks that
//     a note from an external MIDI synthesizer keyboard has been pressed.
//

void sillyKeyboard(int key, int chan /* = 0 */) {
   static int octave = 4;
   static int newkey = 0;
   static Voice voice;
   static MidiMessage message;

   // check to see if adjusting the octave:
   if (isdigit(key)) {
      octave = key - '0';
      return;
   }

   switch (key) {
      case 'z':  newkey = 12 * octave + 0;   break;   // C
      case 's':  newkey = 12 * octave + 1;   break;   // C#
      case 'x':  newkey = 12 * octave + 2;   break;   // D
      case 'd':  newkey = 12 * octave + 3;   break;   // D#
      case 'c':  newkey = 12 * octave + 4;   break;   // E
      case 'v':  newkey = 12 * octave + 5;   break;   // F
      case 'g':  newkey = 12 * octave + 6;   break;   // F#
      case 'b':  newkey = 12 * octave + 7;   break;   // G
      case 'h':  newkey = 12 * octave + 8;   break;   // G#
      case 'n':  newkey = 12 * octave + 9;   break;   // A
      case 'j':  newkey = 12 * octave + 10;  break;   // A#
      case 'm':  newkey = 12 * octave + 11;  break;   // B
      case ',':  newkey = 12 * octave + 12;  break;   // C
      case 'l':  newkey = 12 * octave + 12;  break;   // C#
      case '.':  newkey = 12 * octave + 12;  break;   // D
      case '\'': newkey = 12 * octave + 12;  break;   // D#
      case '/':  newkey = 12 * octave + 12;  break;   // E
      default: return;         // don't do anything if not a key
   }

   // prevent any invalid key numbers:
   if (newkey < 0) {
      newkey = 0;
   } else if (newkey > 127) {
      newkey = 127;
   }
   
   // put note-off message in synth's input buffer:
   message.time = t_time;
   message.p0() = 0x90 | voice.getChan();
   message.p1() = voice.getKey();
   message.p2() = 0;
   synth.insert(message);

   // turn off the last note:
   voice.off();

   // set parameters for next note-on:
   voice.setChan(chan & 0x0f);      // limit channel to range from 0 to 15
   voice.setVel(rand() % 127 +1);   // random attack in range from 1 to 127
   voice.setKey(newkey);            // use the newly selected key number

   // play the MIDI note:
   voice.play();

   // insert the played note into synth's input MIDI buffer:
   message.command() = 0x90 | voice.getChan();
   message.p1() = voice.getKey();
   message.p2() = voice.getVel();
   synth.insert(message);

}


/*------------------ end improvization algorithms -----------------------*/


// md5sum: 38dabcb1469bd417459c7140fdc79010 switch2.cpp [20050403]

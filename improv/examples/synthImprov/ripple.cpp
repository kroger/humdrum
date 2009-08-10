//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Apr 21 20:13:47 PDT 2000
// Last Modified: Fri Apr 21 20:13:50 PDT 2000
// Filename:      ...sig/doc/examples/all/ripple/ripple.cpp
// Syntax:        C++; synthImprov 2.0
//  
//

#include "synthImprov.h" 


/*----------------- beginning of improvization algorithms ---------------*/

EventBuffer eventBuffer;  // for future notes 
MidiMessage message;      // for reading keyno and velocity (and time)
CircularBuffer<int> lastnotes(100);  // for keeping track of melody interval
CircularBuffer<int> lasttimes(100);  // for keeping track of melody rate
int duration;             // duration of the glissandos
int distancee;             // step size of the glissandos
int channel;              // channel to play the glissandos on


// function declarations:
void sillyKeyboard(int key, int chan = 0);
void playgliss(int basenote, int loudness, int channel, int duration, 
      int distancee);


/*--------------------- Event Algorithms --------------------------------*/


//////////////////////////////
//
// EnhanceFunction -- 
//
// Global variables needed by this function:
//     None.
//
// Local variables needed by this function:
//     14 short = interval step between notes
//

static void EnhanceFunction(FunctionEvent& p, EventBuffer& midiOutput) {
   static NoteEvent note;            // temporary note before placing in buffer

   // set the parameters for the output note:
   note.setOnDur(t_time, p.getOffTime()); // off time holds dur
   note.setVel(p.getVel());
   note.setChan(p.getChan());
   note.setKey(p.getKey());

   // if note is too quiet
   if (p.getVel() <= 5) {
      p.off(midiOutput);
   }

   // update the parameters for the function:
   p.setKey(p.getKey()+p.shortValue(14));
   p.setVel(p.getVel()-5);
   p.setOnTime(p.getOnTime() + p.getDur());  // OffTime stores duration

   note.activate();
   note.action(midiOutput);       // start right now, avoiding any buffer delay
   midiOutput.insert(note);       // the note off message is being buffered


   // check wether to kill the algorithm or not:

   // if note is off the range of the keyboard
   if (p.getKey() > C8 || p.getKey() < A0) {
      p.off(midiOutput);
   }

}




/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();
   psl(
   "ripple -- by Craig Stuart Sapp <craig@ccrma.stanford.edu> -- 21 April 2000");
   psl("");
   psl(
   "  Description: Creates glissandos when you press a key on the keyboard.");
   psl("    Computer keyboard keys are assigned random attack velocities.");
   printintermediateline();
   psl("  User commands:");
   psl(
   "     \"-\" = decrease step   \"=\" = increase step   \"\\\" = change "
   "direction");
   psl("     \"[\" = increase rate   \"]\" = decrease rate ");
   psl("      \"0\"-\"9\" = octave number of computer keyboard notes");
   psl("      Notes:           s   d      g    h   j   ");
   psl("                     z   x   c   v   b   n   m  ");
   printboxbottom();
} 


void initialization(void) { 
   eventBuffer.setPollPeriod(10);
   lastnotes.reset();
   lasttimes.reset();
}


void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/


void mainloopalgorithms(void) { 
   if (eventBuffer.checkPoll());        // see if any notes to play

   while (synth.getNoteCount() > 0) {
      message = synth.extractNote();
      if (message.p2() != 0) {
         lastnotes.insert(message.p1());
         lasttimes.insert(message.time);
         distancee = lastnotes[0] - lastnotes[1];
         duration = lasttimes[0] - lasttimes[1];
         channel = 0x0f & message.p0();
         if (distancee != 0) {
            playgliss(message.p1(), message.p2(), channel,  duration, distancee);
         }
      }
   }
}
      


////////////////////////////
//
// playgliss -- inserts a FunctionEvent into the eventBuffer which
//     plays a glissando.  The glissando will die after the 
//     note falls off of the keyboard or the volume gets too low.
//

void playgliss(int basenote, int loudness, int channel, int duration, 
      int distancee) { 
   static FunctionEvent tn;   // a Temporary Note for copying into eventBuffer
   
   // setting the fields of the function note
   tn.shortValue(14) = distancee;

   tn.setFunction(EnhanceFunction);
   tn.setChannel(channel);
   tn.setKeyno(basenote + distancee);
   tn.setVelocity(loudness - 5);
 
   tn.setStatus(EVENT_STATUS_ACTIVE);

   // start time of function and the duration between calling it
   tn.setOnDur(t_time, duration);

   eventBuffer.insert(tn);

   cout << "StartKey =    "  << basenote
        << "\tLoudness =  "  << loudness
        << "\tRate =      "  << duration
        << "\tDirection = "  << distancee
        << endl;
}



/*-------------------- triggered algorithms -----------------------------*/

void keyboardchar(int key) { 
   switch (key) {
      default:
         sillyKeyboard(key);
   }
}



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


// md5sum: 0e5fdaf792274b27bc634faf8d4c8ce5 ripple.cpp [20050403]

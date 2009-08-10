//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Mar  2 22:32:12 PST 1996
// Last Modified: 5 February 1998
// Last Modified: Tue Nov 10 16:59:59 PST 1998
// Last Modified: Sat Oct 13 18:14:07 PDT 2001 (allow for 0x80 note-offs)
// Filename:      ...sig/doc/examples/improv/synthImprov/gliss/gliss.cpp
// Syntax:        C++; synthImprov 0.5
//  
//

#include "synthImprov.h" 


/*----------------- beginning of improvization algorithms ---------------*/

EventBuffer eventBuffer;  // for future notes (2048 notes max)
int direction = 1;        // direction of glissandos, 1=up, -1=down
int step = 1;		  // step to take for each note in glissando
int rate = 300;		  // tempo of gliss notes
int channel = 0;          // channel to play the glisses on.
MidiMessage message;      // for reading keyno and velocity (and time)


// function declarations:
void sillyKeyboard(int key, int chan = 0);
void playgliss(MidiMessage aMessage);
int limit(int value, int min, int max);


/*--------------------- Event Algorithms --------------------------------*/


//////////////////////////////
//
// GlissNoteFunction -- creates glissandos.  To be
//      used with the FunctionNote class; Notes are generated one at a
//      time in the EventBuffer from a FunctionNote.
//
// Global variables needed by this function:
//      direction : The direction of the glissando (up or down)
//      step:       The interval step size for the glissando
//

static void GlissNoteFunction(FunctionEvent& p, EventBuffer& midiOutput) {
   static NoteEvent note;            // temporary note before placing in buffer

   // set the parameters for the output note:
   note.setOnDur(t_time, p.getOffTime()); // off time holds dur
   note.setVel(p.getVel());
   note.setChan(p.getChan());
   note.setKey(p.getKey());

   // update the parameters for the gliss function:
   p.setKey(p.getKey()+step*direction);
   p.setOnTime(p.getOnTime() + p.getDur());  // OffTime stores duration

   note.activate();
   note.action(midiOutput);       // start right now, avoiding any buffer delay
   midiOutput.insert(note);       // the note off message is being buffered

   if (p.getKey() > C8 || p.getKey() < A0) {
      p.off(midiOutput);
   }
}




/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();
   psl(
   "   GLISS -- by Craig Stuart Sapp <craig@ccrma.stanford.edu> -- 5 Feb 1998");
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
}


void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/


void mainloopalgorithms(void) { 
   if (eventBuffer.checkPoll());        // see if any notes to play

   while (synth.getNoteCount() > 0) {
      message = synth.extractNote();
      if (((message.p0() & 0xf0) == 0x90) && (message.p2() != 0)) {
         playgliss(message);

         // lowest A on the keyboard will switch the gliss directions
         if ((message.p0() & 0xf0 == 0x90) && (message.p1() == A0)) {
            direction = -direction;
         }
      }
   }
}
      


////////////////////////////
//
// playgliss -- inserts a FunctionEvent into the eventBuffer which
//     plays a glissando.  The glissando will die after the 
//     note falls off of the keyboard.
//

void playgliss(MidiMessage aMessage) { 
   static FunctionEvent tn;   // a Temporary Note for copying into eventBuffer

   // setting the fields of the function note
   tn.setFunction(GlissNoteFunction);
   tn.setChannel(channel);
   tn.setKeyno(aMessage.p1());
   tn.setVelocity(aMessage.p2());
 
   tn.setStatus(EVENT_STATUS_ACTIVE);

   // start time of function and the duration between calling it
   tn.setOnDur(t_time, rate);

   int location = eventBuffer.insert(tn);

   cout << "GLISS: StartKey = " << (int)aMessage.p1()
        << "\tVelocity = "      << (int)aMessage.p2()
        << "\tRate = "          << rate
        << "\tLocation = "      << location
        << endl;
}



/*-------------------- triggered algorithms -----------------------------*/

void keyboardchar(int key) { 
   switch (key) {
      case '-':
         step = limit(--step, 1, 12);
         cout << "Step = " << step << endl;
         break;
      case 'p':
         cout << "current list in eventBuffer: " << endl;
         eventBuffer.print();
         cout << endl;
         cout << "Event[0] status: " << eventBuffer[0].getStatus() << endl;
         break;
      case '=':
         step = limit(++step, 1, 12);
         cout << "Step = " << step << endl;
         break;
      case '\\':
         direction *= -1;
         if (direction == 1) {
            cout << "Up" << endl;
         } else {
            cout << "Down" << endl;
         }
         break;
      case '[':
         rate -= 20;
         rate = limit(rate, 20, 200000);
         cout << "Rate = " << rate << endl;
         break;
      case ']':
         rate += 20;
         rate = limit(rate, 20, 200000);
         cout << "Rate = " << rate << endl;
         break;
      default:
         sillyKeyboard(key);
   }
}



int limit(int value, int min, int max) {
   if (value < min) {
      return min;
   } else if (value > max) {
      return max;
   } else {
      return value;
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


// md5sum: 7c0d8687dbdfec710196ff88639060a6 gliss.cpp [20050403]

//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Oct 15 14:46:35 PDT 2001
// Last Modified: Wed Oct 17 20:30:37 PDT 2001
// Filename:      ...sig/doc/examples/improv/synthImprov/tumble/tumble.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description:   Melodic sequences with a uniform rhythm are played
//                to input a melodic pattern which is then repeated
//                until the algorith notes fall outside the range of
//                an 88-note keyboard.
//

#include "synthImprov.h"       /* include synthImprov environment */
#define  PARAMSIZE 256         /* number of max simultaneous algorithms */


// Tumble Parameters is a data structure for holding all of the data
// for a tumble algorithm.  It stores all of the necessary information
// for generating the tumble algorithm's notes.

class TumbleParameters {
   public:
      int max;                 // maximum number of notes in sequence
      int pos;                 // current position to play next note.
      int current;             // current note being played
      Array<char> n;           // note sequence; n0 = cycle delta
      Array<char> v;           // attack velocity of note n.
      Array<unsigned short> i; // duration between note attacks (ioi)
      Array<unsigned short> d; // duration of note n
      int active;              // 0=unused, 1=currently running algorithm
      int dir;                 // 1=normal, -1=inversion

      TumbleParameters(void) {
         clear();
      }

      TumbleParameters(TumbleParameters& b) {
         TumbleParameters& a = *this;
         a = b;      
      }

      void clear(void) {
         n.setSize(0);
         v.setSize(0);
         i.setSize(0);
         d.setSize(0);
         max = 0;
         pos = 0;
         active = 0;
         current = 0;
         dir = 1;
      }

      TumbleParameters& operator=(TumbleParameters& b) {
         TumbleParameters& a = *this;
         a.n       = b.n;
         a.v       = b.v;
         a.i       = b.i;
         a.d       = b.d;
         a.max     = b.max;
         a.pos     = b.pos;
         a.active  = b.active;
         a.current = b.current;
         a.dir     = b.dir;
         return a;
      }
         
};


/*----------------- beginning of improvization algorithms ---------------*/

EventBuffer eventBuffer;  // for future notes (2048 notes max)
int channel = 0;          // channel to play the notes on.
MidiMessage message;      // for reading keyno and velocity (and time)
int direction = 1;        // direction of algorithm generation notes
int length = 4;           // number of notes in a algorithm cycle
int anticipation = 125;   // anticipation of first note in tumble for
                          // use with the Yamaha Disklavier which has
                          // a slight delay in playing notes from the computer.
                          // Value is in milliseconds.
double tolerance = 0.90;  // allowable trigger rhythm tolerance. *// 0.90
SigCollection<TumbleParameters> tparam;  // data storage for tumble functions


// function declarations:
void    processNote         (MidiMessage message, int seqLength, int direction);
int     startAlgorithm      (TumbleParameters& p);
int     storeParameters     (SigCollection<TumbleParameters>& params,
                             TumbleParameters& p);
void    randomizeDirections (SigCollection<TumbleParameters>& p);
void    reverseDirections   (SigCollection<TumbleParameters>& p);
void    forwardDirections   (SigCollection<TumbleParameters>& p);
void    invertDirections    (SigCollection<TumbleParameters>& p);
void    sillyKeyboard       (int key, int chan = 0);

template<class type>
type    limit               (type value, type min, type max);

/*--------------------- Event Algorithms --------------------------------*/



//////////////////////////////
//
// TumbleNoteFunction -- creates the tumble note sequence.  To be
//     used with the FunctionEvent class; NoteEvents are generated
//     one at a time in the EventBuffer from a FunctionEvent.
//

static void TumbleNoteFunction(FunctionEvent& p, EventBuffer& midiOutput) {
   static NoteEvent note;           // temporary note for placing in buffer

   TumbleParameters& param = tparam[p.charValue(0)];
   int newnote = limit(param.current + param.dir * param.n[param.pos], 0, 127);

   // turn off algorithm if someone turned the algorithm off externally
   // or if the current note is too large or too small.
   if (param.active == 0 || newnote < A0 || newnote > C7) {
      param.active = 0;
      p.off(midiOutput);
      return;
   }

   // set the parameters of the output note:
   note.setOnDur(t_time, param.d[param.pos]);   // off time holds dur
   note.setVel(param.v[param.pos]);
   note.setChan(p.getChan());
   note.setKey(newnote);
   note.activate();
   note.action(midiOutput);     // start right now, avoiding any buffer delay
   midiOutput.insert(note);     // store the note for turning off later

   // update the parameters for the tumble algorithm
   p.setOnTime(p.getOnTime() + param.i[param.pos]);
   param.current = newnote;
   param.pos++;
   if (param.pos > param.n.getSize()) {
      param.pos = 0;
   }
}


/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();
   psl(
   "   TUMBLE -- by Craig Stuart Sapp <craig@ccrma.stanford.edu> 16 Oct 2001");
   psl("");
   psl("  Description: continues a melodic sequence of a given length.");
   psl("    Computer keyboard keys are assigned random attack velocities.");
   printintermediateline();
   psl("  User commands:");
   psl(
   "     \"-\" = decrease seq.   \"=\" = increase seq.   \"\\\" = change "
                                                                  "direction");
   psl("      \"0\"-\"9\" = octave number of computer keyboard notes");
   psl("      Notes:           s   d      g    h   j   ");
   psl("                     z   x   c   v   b   n   m  ");
   printboxbottom();
} 


void initialization(void) { 
   eventBuffer.setPollPeriod(10);  // look in the algorithm buffer every 10 ms.
   tparam.setSize(PARAMSIZE);      // 256 simultaneous algorithms at once.
   tparam.allowGrowth(0);
}


void finishup(void) { 
   for (int i=0; i<tparam.getSize(); i++) {
      tparam[i].active = 0;
   }
}


/*-------------------- main loop algorithms -----------------------------*/

void mainloopalgorithms(void) {
   eventBuffer.checkPoll();

   while (synth.getNoteCount() > 0) {
      message = synth.extractNote();
      if (message.is_note_on() && message.p1() == A0) {
         direction = -direction;
         cout << "Direction = " << direction << endl;
      } else if (message.is_note_on() && message.p1() == C7) {
         // add one to the length of the tumble sequence
         length = limit(length+1, 2, 200);
         cout << "Sequence length = " << length << endl;
      } else if (message.is_note_on() && message.p1() == B6) {
         // subtract one from the length of the tumble sequence
         length = limit(length-1, 2, 200);
         cout << "Sequence length = " << length << endl;
      } else {
         processNote(message, length, direction);
      }
   }
}



//////////////////////////////
//
// processNote -- processes notes for algorithm and starts
//     the algorithm if it is time to do so.
//

void processNote(MidiMessage message, int seqLength, int direction) {
   static Array<char>         notes;
   static Array<char>         velocities;
   static Array<int>          durations;
   static Array<int>          iois;
   static Array<int>          ontimes;
   static CircularBuffer<int> attacktimes;
   static int                 init = 0;
   static TumbleParameters    temparam;
   char vel;

   if (!init) {
      attacktimes.setSize(256);
      attacktimes.reset();
      notes.setSize(0);
      velocities.setSize(0);
      durations.setSize(0);
      iois.setSize(0);
      ontimes.setSize(128);
      ontimes.zero();
      init = 1;
   }

   char note;
   int deltatime;
   int ioi0;
   int ioix;
   if (message.is_note_on()) {
      attacktimes.insert(message.time);

      // check to see if the ioi is in the correct range
      if (notes.getSize() == 0) {
         // no notes yet, so don't know the first ioi
      } else {
         deltatime = attacktimes[0] - attacktimes[1];
         iois.append(deltatime);
      }
      if (iois.getSize() > 1) {
         ioi0 = iois[0];
         ioix = iois[iois.getSize()-1];
         if ((ioix < ioi0 * tolerance) || (ioix > ioi0 / tolerance)) {
            goto resettrigger;
         }
      }

      // at this point the note can be added to the sequence
      if (notes.getSize() + 1 >= seqLength) {
         // time to trigger an algorithm
         if (durations.getSize() < notes.getSize()) {
            // if the last note has not yet been turned off, approximate dur.
            deltatime = iois[iois.getSize()-1];
            durations.append(deltatime);
         }

         int i;
         for (i=0; i<seqLength; i++) {
            temparam.v[i] = velocities[i];
            temparam.i[i] = iois[i];
            temparam.d[i] = durations[i];
            temparam.n[i] = notes[i] - notes[0];
         }
         temparam.n[0]    = message.p1() - notes[0];
         temparam.current = message.p1();
         temparam.pos     = 1;
         temparam.max     = seqLength;
         temparam.active  = 1;
         
         startAlgorithm(temparam);
         goto resettrigger;
      } else {
         // add the note info to the algorithm pile
         note = message.p1();
         notes.append(note);
         vel = message.p2();
         velocities.append(vel);
         attacktimes[message.p1()] = message.time;
      }
   } else if (message.is_note_off()) {
      if (notes.getSize() > 0) {
         if (notes[notes.getSize()-1] == message.p1()) {
         deltatime = message.time - ontimes[message.p1()];
         durations.append(deltatime);
      } else {
         cout << "A funny error ocurred" << endl;
      }
   }

   return;

resettrigger:
   attacktimes.setSize(0);
   notes.setSize(0);
   velocities.setSize(0);
   durations.setSize(0);
   iois.setSize(0);

   if (message.is_note_on()) {
      note = message.p1();
      notes.append(note);
      ontimes[message.p1()] = message.time;
      vel = message.p2();
      velocities.append(vel);
   }
}



//////////////////////////////
//
// startAlgorithm -- start playing the tumble algorithm.  Inserts a
//     FunctionEvent into the eventBuffer which plays the tumble
//     algorithm sequence.  The algorithm will die after the notes
//     fall off of the 88-note keyboard.
//

}
int startAlgorithm(TumbleParameters& p) {
   static FunctionEvent tn;   // a Temporary Note for copying into eventBuffer

   int ploc = storeParameters(tparam, p);
   if (ploc < 0) {
      cout << "Warning: Parameter space is full.  Not adding new algorithm"
           << endl;
      return -1;
   }

   // setting the fields of the function note
   tn.setFunction(TumbleNoteFunction);
   tn.setChannel(channel);
   tn.setKeyno(0);
   tn.setVelocity(0);
   tn.charValue(0) = (char)ploc;         // store location of the parameters
   tn.setStatus(EVENT_STATUS_ACTIVE);
   tn.setOnTime(t_time + p.i[0] - anticipation);

   // display the basic algorithm info
   cout << "Tumble: Time: " << t_time << "\tStart = " << (int)p.current
        << "\tPattern = . ";
   for (int i=1; i<p.n.getSize(); i++) {
      cout << (int)p.n[i] << " ";
   }
   cout << "(" << (int)p.n[0] << ")";
   cout << " ioi: " << p.i[0];
   cout << endl;

   return eventBuffer.insert(tn);
}



///////////////////////////////
//
// storeParameters --
//

int storeParameters(SigCollection<TumbleParameters>& params, 
      TumbleParameters& p) {
   int start = rand() % PARAMSIZE;
   int position = start + 1;
   if (position >= PARAMSIZE) {
      position = 0;
   }
   while (position != start) {
      if (params[position].active == 0) {
         params[position] = p;  // copy parameters into storage
         return position;       // and return location of data
      }
      position++;
      if (position >= PARAMSIZE) {
         position = 0;
      }
   }

   // no free space found, abort search
   return -1;
}


/*-------------------- triggered algorithms -----------------------------*/

void keyboardchar(int key) { 
   switch (key) {
      case 'p':
         cout << "current list in eventBuffer: " << endl;
         eventBuffer.print();
         cout << endl;
         cout << "Event[0] status: " << eventBuffer[0].getStatus() << endl;
         break;
      case '\\':
         direction *= -1;
         if (direction == 1) {
            cout << "Up" << endl;
         } else {
            cout << "Down" << endl;
         }
         break;
      case 'r':                       // random direction to current algorithms
         randomizeDirections(tparam);
         cout << "Random directions" << endl;
         break;
      case 'f':                       // normal direction of algorithms
         forwardDirections(tparam);
         cout << "Normal directions" << endl;
         break;
      case 'i':                       // invert direction of algorithms
         invertDirections(tparam);
         cout << "Inverted directions" << endl;
         break;
      case 'c':                       // reverse direction of algorithms
         reverseDirections(tparam);
         cout << "Changed directions" << endl;
         break;
      case 's':	                      // increase rhythmic sensitivity
         tolerance = limit(tolerance * 1.02, 0.01, 0.99);
         cout << "Sensitivity = " << tolerance << endl;
         break;
      case 'x':	                      // decrease rhythmic sensitivity
         tolerance = limit(tolerance / 1.02, 0.01, 0.99);
         cout << "Sensitivity = " << tolerance << endl;
         break;
      default:
         sillyKeyboard(key);
   }
}



//////////////////////////////
//
// randomizeDirections -- change the direction of algorithms in a random
//      fashion.
//

void randomizeDirections(SigCollection<TumbleParameters>& p) {
   for (int i=0; i<p.getSize(); i++) {
      if (p[i].active) {
         if (rand() % 2) {
            p[i].dir = 1;
         } else {
            p[i].dir = -1;
         }
      }
   }
}



//////////////////////////////
//
// reverseDirections -- change the direction of algorithms from their
//     current states.
//

void reverseDirections(SigCollection<TumbleParameters>& p) {
   for (int i=0; i<p.getSize(); i++) {
      if (p[i].active) {
         p[i].dir = -p[i].dir;
      }
   }
}



//////////////////////////////
//
// forwardDirections -- go to the original direction of algorithms 
//

void forwardDirections(SigCollection<TumbleParameters>& p) {
   for (int i=0; i<p.getSize(); i++) {
      if (p[i].active) {
         p[i].dir = 1;
      }
   }
}



//////////////////////////////
//
// invertDirections -- change the direction of algorithms from their
//     natural states.
//

void invertDirections(SigCollection<TumbleParameters>& p) {
   for (int i=0; i<p.getSize(); i++) {
      if (p[i].active) {
         p[i].dir = -1;
      }
   }
}



//////////////////////////////
//
// limit -- limit the range of a variable
//

template<class type>
type limit(type value, type min, type max) {
   if (value < min) {
      return min;
   } else if (value > max) {
      return max;
   } else {
      return value;
   }
}



//////////////////////////////
//
// sillyKeyboard -- ASCII music keyboard
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

   newkey = limit(newkey, 0, 127);

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


// md5sum: 484b50f19ae8f2e72564170a860aaba2 tumble.cpp [20090615]

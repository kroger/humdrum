//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat May 27 20:16:58 PDT 2000
// Last Modified: Sat May 27 20:44:30 PDT 2000
// Filename:      ...sig/doc/examples/all/eco/eco.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description:   Generates echos
//                Echos are triggered whenever two notes of the same
//                pitch are played within 10 seconds of each other.
//
//                The duration of the echoed notes are related to the
//                duration of the first note that starts the echo
//
//                The decay amount of the echo is related to the
//                change in attack velocity between the two notes
//                which start the echo.
//

#include "synthImprov.h" 

/*----------------- beginning of improvization algorithms ---------------*/

#define MAXECHOTIME 10000   /* max time for activating echos */

EventBuffer eventBuffer;  // for future notes 
MidiMessage message;      // for reading keyno and velocity (and time)
int channel;              // channel to play the echos on
int key;                  // the current MIDI key number
int duration;             // duration of the echoing note
int period;               // time between echoing notes
int command;              // the current MIDI command
int velocity;             // the current MIDI attack velocity
int decay;                // decay rate for the echo algorithm

Array<int> ontimes(128);  // time when the last note turned on for each pitch
Array<int> offtimes(128); // time with the last note turned off for each pitch
Array<int> attack(128);   // attack velocity of the first note in the echo


// function declarations:
void sillyKeyboard(int key, int chan = 0);
void playecho(int key, int velocity, int decay, int channel, int duration, 
      int period);

/*--------------------- Event Algorithms --------------------------------*/


//////////////////////////////
//
// EchoFunction -- 
//
// Global variables needed by this function:
//     None.
//
// Local variables needed by this function:
//     12 int   = duration
//     8 float  = decay
//     4 float  = newvelocity
//

static void EchoFunction(FunctionEvent& p, EventBuffer& midiOutput) {
   static NoteEvent note;       // temporary note before placing in buffer

   // set the parameters for the output note:
   note.setOnDur(t_time, p.intValue(12)); // duration in int(12)
   note.setVel(p.getVel());
   note.setChan(p.getChan());
   note.setKey(p.getKey());

   // update the parameters for the function:
   p.floatValue(4) = p.floatValue(4) * p.floatValue(8);
   short newvelocity = (short)p.floatValue(4);
   if (newvelocity <= 7) {
      p.off(midiOutput);
   } else {
      p.setVel(newvelocity);
      p.setOnTime(p.getOnTime() + p.getDur());  // OffTime stores duration
   
      note.activate();
      note.action(midiOutput);    // start right now, avoiding any buffer delay
      midiOutput.insert(note);    // the note off message is being buffered
   }
}


/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();
   psl(
   "eco -- by Craig Stuart Sapp <craig@ccrma.stanford.edu> -- 28 May 2000");
   psl("");
   psl("      \"0\"-\"9\" = octave number of computer keyboard notes");
   psl("      Notes:           s   d      g    h   j   ");
   psl("                     z   x   c   v   b   n   m  ");
   printboxbottom();
} 


void initialization(void) { 
   eventBuffer.setPollPeriod(10);

   ontimes.zero();
   offtimes.zero();
   attack.zero();
}


void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/


void mainloopalgorithms(void) { 
   if (eventBuffer.checkPoll());        // see if any notes to play

   while (synth.getNoteCount() > 0) {
      message  = synth.extractNote();
      channel  = 0x0f & message.p0();
      command  = 0xf0 & message.p0();
      key      = message.p1();
      velocity = message.p2();

      if (command == 0x80 || velocity == 0) {
         // note off message
         offtimes[key] = t_time;
      } else {
         if (key == A0) {
            eventBuffer.off();
            return;
         }


         // note on message
         duration = offtimes[key] - ontimes[key];
         period = t_time - ontimes[key];
         decay = attack[key] - velocity;
         if (decay < 0) {
            decay = -decay;
            velocity = attack[key];
         }
         if (decay < 1) {
            decay = 1;
         }
         
         if (period < MAXECHOTIME && ontimes[key] != 0) {
            playecho(key, velocity, decay, channel, duration, period);
            ontimes[key] = 0;
         } else {
            ontimes[key] = t_time;
            attack[key] = message.p2(); 
         }
      }
   }
}



////////////////////////////
//
// playecho -- inserts a FunctionEvent into the eventBuffer which
//     plays an echo. 
//

void playecho(int key, int velocity, int decay, int channel, int duration, 
      int period) { 
   static FunctionEvent tn;   // a Temporary Note for copying into eventBuffer
   
   // setting the fields of the function note
   tn.intValue(12) = duration;
   tn.floatValue(8) = 1.0 - decay/128.0;
   tn.floatValue(4) = (float)velocity;

   tn.setFunction(EchoFunction);
   tn.setChannel(channel);
   tn.setKeyno(key);
   tn.setVelocity(velocity);
 
   tn.setStatus(EVENT_STATUS_ACTIVE);

   // start time of function and the duration between notes
   tn.setOnDur(t_time, period);

   eventBuffer.insert(tn);

   cout << "Key = "        << key
        << "\tLoudness = " << velocity
        << "\tdecay = "    << decay
        << "\tduration = " << duration
        << "\tperiod = "   << period
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



// md5sum: bf0d0a22200a94eb60ed265119a3094f eco.cpp [20050403]

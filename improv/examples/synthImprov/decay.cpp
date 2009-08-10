//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Apr 22 16:04:00 PDT 2000
// Last Modified: Sat Apr 22 16:04:04 PDT 2000
// Filename:      ...sig/doc/examples/all/decay/decay.cpp
// Syntax:        C++; synthImprov 2.0
//  
//

#include "synthImprov.h" 


/*----------------- beginning of improvization algorithms ---------------*/

EventBuffer   eventBuffer;      // for future notes 
MidiMessage   message;          // for reading keyno and velocity (and time)
Array<int>    notestates(128);  // for keeping track of note on/off
Array<double> decaystates(128); // for keeping track of note on/off
Array<int>    onvels(128);      // for keeping track of note on/off
int           gap = 100;        // spacing between notes (millisec)
double        decayrate = 0.87; // echo decay rate.


// function declarations:
void sillyKeyboard(int key, int chan = 0);
void processNote(MidiMessage& message);
void createDecay(int channel, int key, int duration, int velocity);


/*--------------------- Event Algorithms --------------------------------*/


//////////////////////////////
//
// EchoAlgorithm -- 
//
// Global variables needed by this function:
//     decaystates
//
// Local variables needed by this function:
//     14 short = gap between notes
//

static void EchoAlgorithm(FunctionEvent& p, EventBuffer& midiOutput) {
   static NoteEvent note;            // temporary note before placing in buffer

   // check if pausing
   if (decaystates[p.getKey()] < 0.0) {
      p.setOnTime(p.getOnTime() + p.getDur() + p.shortValue(14)); 
      return;
   }
      
   // set the parameters for the output note:
   note.setOnDur(t_time, p.getOffTime()); // off time holds dur
   note.setVel(p.getVel());
   note.setChan(p.getChan());
   note.setKey(p.getKey());

   // update the parameters for the function:
   decaystates[p.getKey()] *= decayrate;
   p.setVel((int)decaystates[p.getKey()]);

   // if note is too quiet, end the note
   if (p.getVel() <= 2) {
      p.off(midiOutput);
      decaystates[p.getKey()] = 0.0;
   }

   // next time includes a gap so that key can raise on keyboard
   p.setOnTime(p.getOnTime() + p.getDur() + p.shortValue(14)); 

   note.activate();
   note.action(midiOutput);       // start right now, avoiding any buffer delay
   midiOutput.insert(note);       // the note off message is being buffered

}




/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();
   psl(
   "DECAY -- by Craig Stuart Sapp <craig@ccrma.stanford.edu> -- 22 April 2000");
   psl("");
   printintermediateline();
   printboxbottom();
} 


void initialization(void) { 
   eventBuffer.setPollPeriod(10);
   for (int i=0; i<notestates.getSize(); i++) {
      notestates[i] = 0;
      onvels[i] = 0;
      decaystates[i] = 0.0;
   }
}


void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/


void mainloopalgorithms(void) { 
   if (eventBuffer.checkPoll());        // see if any notes to play

   while (synth.getNoteCount() > 0) {
      message = synth.extractNote();
      processNote(message);
   }
}
      


////////////////////////////
//
// processNote -- decide when to trigger a decay algorithm.
//

void processNote(MidiMessage& message) {
   int velocity = message.p2();
   int key = message.p1();
   int channel = message.p0() & 0x0f;
   int state = 1;
   int duration = 0;
   if (((message.p0() & 0xf0) == 0x80) || velocity == 0) {
      state = 0;
   }

   if (key == A0) {
      cout << "A0 Triggered" << endl;
      for (int i=0; i<decaystates.getSize(); i++) {
         decaystates[i] *= -1.0;
      }
      return;
   }

   if (state == 1) {
      notestates[key] = message.time;
      onvels[key] = velocity;
   } else {
      if (notestates[key] == 0) {
         // do nothing
      } else {
         duration = message.time - notestates[key];
         if (decaystates[key] == 0.0) {
            createDecay(channel, key, duration, onvels[key]);
         }
      }
      notestates[key] = 0;
   }
}



////////////////////////////
//
// createDecay -- start a decay algorithm
//

void createDecay(int channel, int key, int duration, int velocity) {
   static FunctionEvent tn;   // temporary function for copying into eventBuffer

   tn.shortValue(14) = gap;    // gap between successive notes

   tn.setFunction(EchoAlgorithm);
   tn.setChannel(channel);
   tn.setKeyno(key);
   decaystates[key] = velocity * decayrate;
   tn.setVelocity((int)decaystates[key]);
   tn.setStatus(EVENT_STATUS_ACTIVE);

   // start time of function and the duration between calling it
   tn.setOnDur(t_time, duration);

   eventBuffer.insert(tn);

   cout << "Key=    "  << key
        << "\tDuration =  "  << duration + gap
        << "\tVelocity =  "  << velocity
        << endl;
}



/*-------------------- triggered algorithms -----------------------------*/

void keyboardchar(int key) { 
   switch (key) {
      case 'p':                    // print eventbuffer info
         eventBuffer.print();
         cout << endl;
         break;
      case '[':                    // speed up echo rate
         decayrate *= 0.99;
         cout << "Decay rate = " << decayrate << endl;
         break;
      case ']':                    // slow down echo rate
         decayrate /= 0.99;
         cout << "Decay rate = " << decayrate << endl;
         break;
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


// md5sum: 2b73be6f711d1022982de4323f34f32f decay.cpp [20050403]

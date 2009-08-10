//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Apr 22 17:12:00 PDT 2000
// Last Modified: Mon Apr 24 14:31:56 PDT 2000
// Filename:      ...sig/doc/examples/all/trill/trill.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description:   a trill is started when two notes occur within
//		  1 second of each other.  The trill is stopped
//		  when a note is played inside or on the boundary
//		  of the trill (for minor 2nd, within one note outside
//                the range of the trill.
//         
//                There are several control keys on the keyboard.
//                If a control key is hit harder, then its effect
//                will occur at a faster rate.
//                   A0  = turn off all current trills
//                   A7  = make all trills quieter
//                   A#7 = make all trills louder
//                   B7  = make all trills faster
//                   C8  = make all trills slower
//

#include "synthImprov.h" 

#define TRIGTIME     1000     /* maximum time between trill notes */
#define MINTRIGTIME    75     /* minimum time between trill notes */

/*----------------- beginning of improvization algorithms ---------------*/

EventBuffer eventBuffer;            // used to store algorithms for performance
CircularBuffer<int> notetimes(100); // history of the last 100 note-on times
Array<int> noteontimes(128);        // list of the last time a note was played
CircularBuffer<int> notes(100);     // history of the pitches played
CircularBuffer<int> trills(100);    // history of which notes create a trill
int trillcorrection = 0;            // global speed control of trills
int velcorrection = 0;              // golbal velocity control of trills

// function declarations:
void sillyKeyboard(int key, int chan = 0);
void createTrill(int key1, int key2, int velocity, int channel, int duration);
void processNote(MidiMessage message);



/*--------------------- Event Algorithms --------------------------------*/


//////////////////////////////
//
// TrillAlgorithm -- Play the next note in the trill, adjusting
//    for the global control of 
//
// Global variables needed by this function:
//      notetimes : The time that keys are pressed on the keyboard
//      trillcorrection : adjustment to the trill speed
//      velcorrection : adjustment to the attack velocity 
//
// Local variables:
//    charValue(14) = upper key number of trill
//    charValue(15) = current note to play
//    intValue(10)  = start time of trill
//

static void TrillAlgorithm(FunctionEvent& p, EventBuffer& midiOutput) {
   static NoteEvent note;           // temporary note before placing in buffer

   int key1 = p.getKey();           // lower key of trill
   int key2 = p.charValue(14);      // upper key of trill
   int state = p.charValue(15);     // which note to play next
   int starttime = p.intValue(10);  // when trill was started
   int i;
  
   // turn off the trill if there is a note played inside the trill
   int range1 = key1;
   int range2 = key2;
   if (range2 - range1 == 1) {
      range1--;
      range2++;
   }
   for (i=range1; i<=range2; i++) {
      if (noteontimes[i] > starttime) {
         p.off(midiOutput);
         return;
      }
   }

   // set the next note to play
   int key = state ? key2 : key1;
   state = !state;
   p.charValue(15) = state;

   // set the parameters for the output note:
   note.setOnDur(t_time, p.getDur()); 
   note.setVel(p.getVel());
   note.setChan(p.getChan());
   note.setKey(key);

   // update the parameters for the gliss function:
   p.setOnTime(p.getOnTime() + p.getDur());  
   
   int value = p.getVel() + velcorrection;
   if (value < 100 && value > 3) {
      p.setVel(value);
   }
   if (p.getDur() + trillcorrection > MINTRIGTIME) {
      p.setDur(p.getDur() + trillcorrection);
   }

   note.activate();
   note.action(midiOutput);       // start right now, avoiding any buffer delay
   midiOutput.insert(note);       // the note off message is being buffered
}



/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();
   psl(
   "   TRILL -- by Craig Stuart Sapp <craig@ccrma.stanford.edu> -- 22 April 2000");
   psl("");
   printboxbottom();
} 


void initialization(void) { 
   eventBuffer.setPollPeriod(10);
   notetimes.reset();
   notetimes.insert(0);
   notetimes.insert(0);
   notes.reset();
   notes.insert(0);
   notes.insert(0);
   noteontimes.zero();
}


void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/


void mainloopalgorithms(void) { 
   eventBuffer.checkPoll();             // see if any notes need playing

   while (synth.getNoteCount() > 0) {
      processNote(synth.extractNote());
   }
}
      


////////////////////////////
//
// processNote -- 
//

void processNote(MidiMessage message) {
   int key = message.p1();
   int velocity = message.p2();
   int channel = message.p0() & 0x0f;

   int status = 1;
   if (message.p0() - channel == 0x80 || velocity == 0) {
      status = 0;
   }

   if (status == 0) {
      if (key == C8 || key == B7) {
         trillcorrection = 0;
      }
      if (key == As7 || key == A7) {
         velcorrection = 0;
      }
      return;
   }

   if (key == C8) {
      trillcorrection = velocity / 10;
      return;
   }

   if (key == B7) {
      trillcorrection = -(velocity / 10);
      return;
   }

   if (key == As7) {
      velcorrection = +(velocity / 10);
      return;
   }

   if (key == A7) {
      velcorrection = -(velocity / 10);
      return;
   }

   if (key == A0) {
      for (int j=0; j<128; j++) {
         noteontimes[j] = t_time;
      }
      return;
   }
  
   if (key == A0) {
      for (int j=0; j<128; j++) {
         noteontimes[j] = t_time;
      }
      return;
   }

   noteontimes[key] = t_time;

   notetimes.insert(message.time);
   notes.insert(key);

   if (notes[1] == 0) {
      trills.insert(0);
      return;
   }
  
   if (notes[2] != 0) {
      if (trills[0] == 1 && notetimes[1] - notetimes[2] < TRIGTIME) {
         trills.insert(0);
         return;
      }
   }

   trills.insert(1);

   int duration = notetimes[0] - notetimes[1];
   if (duration < TRIGTIME && duration > MINTRIGTIME &&
       notes[0] - notes[1] != 0) {
      createTrill(key, notes[1], velocity, channel, duration);
   }
}


////////////////////////////
//
// createTrill -- set up the parameters for a trill
//

void createTrill(int key1, int key2, int velocity, int channel, int duration) {
   static FunctionEvent tn;   // a Temporary Note for copying into eventBuffer

   // key1 should always be smaller than key2
   int temp;
   if (key1 > key2) {
      temp = key1;
      key1 = key2;
      key2 = temp;
   }

   // setting the fields of the function note
   tn.setFunction(TrillAlgorithm);
   tn.setChannel(channel);
   tn.setKeyno(key1);
   tn.setVelocity(velocity);

   // set extra parameters
   tn.charValue(15) = 0;        // 0 = play key1 next, 1 = play key2 next
   tn.charValue(14) = key2;     // secondary pitch
   tn.intValue(10) = t_time;    // initialization time

   tn.setStatus(EVENT_STATUS_ACTIVE);

   // start time of function and the duration between calling it
   tn.setOnDur(t_time + duration, duration);

   eventBuffer.insert(tn);

   cout << "Trill = " << key1 << " to " << key2 
        << "\tRate = " << duration
        << endl;
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


// md5sum: 967878f0601a855737b12812bbc08313 trill.cpp [20050403]

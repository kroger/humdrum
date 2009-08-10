//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Apr 28 14:37:28 PDT 2000
// Last Modified: Fri Apr 28 14:37:32 PDT 2000
// Filename:      ...sig/doc/examples/all/ghost/ghost.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description:   When a note is played on the keyboard, it generates
//		  a ghost image which will play sometime within the next
//		  minute on the same pitch, half as loud as the original pitch.
//                The ghost note has the same duration as the original
//                note.
//

#include "synthImprov.h" 

#define MAXTIME 60     /* maximum time to start ghost note */
#define MINTIME 10     /* minimum time to start ghost note */

/*----------------- beginning of improvization algorithms ---------------*/

EventBuffer eventBuffer;            // used to store algorithms for performance
Array<int> noteontimes(128);        // list of the last time a note was played
Array<int> noteonvels(128);         // list of the last time a note was played

// function declarations:
void sillyKeyboard(int key, int chan = 0);
void createGhost(int key, int velocity, int channel, int duration);
void processNote(MidiMessage message);


/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();
   psl(
   "   GHOST -- by Craig Stuart Sapp <craig@ccrma.stanford.edu> -- 28 April 2000");
   psl("Description:   When a note is played on the keyboard, it generates");
   psl("a ghost image which will play sometime within the next");
   psl("minute on the same pitch, half as loud as the original pitch.");
   psl("The ghost note has the same duration as the original");
   psl("note.");
   psl("");
   printboxbottom();
} 


void initialization(void) { 
   eventBuffer.setPollPeriod(10);
   noteontimes.zero();
   noteonvels.zero();
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
   int velocity = message.p2() / 2;
   int channel = message.p0() & 0x0f;
   int command = message.p0() & 0xf0;
   int duration;

   int status = 1;
   if (command == 0x80 || velocity == 0) {
      status = 0;
   }

   if (status == 0) {
      duration = t_time - noteontimes[key];
      createGhost(key, noteonvels[key], channel, duration);
   } else {
      noteontimes[key] = t_time;
      noteonvels[key] = velocity;
   }
}


////////////////////////////
//
// createGhost -- set up the parameters for the ghost note.
//

void createGhost(int key, int velocity, int channel, int duration) {
   static NoteEvent tn;   // a Temporary Note for copying into eventBuffer

   // setting the fields of the function note
   tn.setChannel(channel);
   tn.setKeyno(key);
   tn.setVelocity(velocity);

   tn.setStatus(EVENT_STATUS_ACTIVE);

   int starttime = (rand() % (MAXTIME - MINTIME) + MINTIME) * 1000;
   tn.setOnDur(t_time + starttime, duration);

   eventBuffer.insert(tn);

   cout << "Ghost = " << key << " in " << starttime/1000
        << " seconds " << endl;
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


// md5sum: 9931db184b6d501cc5ea77c4e9894a12 ghost.cpp [20050403]

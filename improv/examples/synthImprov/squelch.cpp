//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Nov 19 17:59:51  2002
// Last Modified: Tue Nov 19 17:59:54  2002
// Filename:      ...sig/doc/examples/improv/synthImprov/squelch.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: This is a test program to squelch the feedback notes
//              from a PianoDisc.
//

#include "synthImprov.h"      // includes the default Win95 console interface
                              // for the synthImprov environment


/*----------------- beginning of improvization algorithms ---------------*/

/*--------------------- maintenance algorithms --------------------------*/

CircularBuffer<MidiMessage> memory;
int mintime = 0;  // minimum time for squelch to occur
int maxtime = 0;  // maximum time for squelch to occur
int sstate = 1;    // toggle for squelching
int veladd = 25;   // add extra velocity to match input note

int checkForSquelch(MidiMessage& message, 
      CircularBuffer<MidiMessage>& memory, int mintime, 
      int maxtime, int curtime);


//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << "Press space to turn Squelching on/off" << endl;
} 



//////////////////////////////
//
// initialization -- this function is called by the improv
//     interface once at the start of the program.  Put items
//     here which need to be initialized at the beginning of
//     the program.
//

void initialization(void) { 
   memory.setSize(10000);   // up to 10 seconds of squelch possible
   memory.reset();
   mintime = 48;   // minimum delta time
   maxtime = 180;   // maximum delta time
   description();
}



//////////////////////////////
//
// finishup -- this function is called by the improv interface
//     whenever the program is exited.  Put items here which
//     need to be taken care of when the program is finished.
//

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

//////////////////////////////
//
// mainloopalgorithms -- this function is called by the improv interface
//   continuously while the program is running.  The global variable t_time
//   which stores the current time is set just before this function is
//   called and remains constant while in this functions.
//

MidiMessage message;
int oldnote = 0;
int outvel;
void mainloopalgorithms(void) { 
   if (synth.getNoteCount() > 0) {
      message = synth.extractNote();
      oldnote = checkForSquelch(message, memory, mintime, maxtime, t_time);
      if (!oldnote || sstate == 0) {
         cout << "New note from performer: " << message << endl;
         if (message.is_note_on()) {
            message.p1() += 7;
            outvel = message.p2() + veladd;
            if (outvel > 127) { outvel = 127; }
            synth.send(message.p0(), message.p1(), outvel);
            message.time = t_time;
            message.p2() = outvel;
            memory.insert(message);
         } else if (message.is_note_off()) {
            message.p1() += 7;
            synth.send(message.p0(), message.p1(), message.p2());
            message.time = t_time;
            memory.insert(message);
         }
      } else {
         cout << "Feedback note from piano: " << message << endl;
      }
   }

}



int checkForSquelch(MidiMessage& message, CircularBuffer<MidiMessage>& memory, 
      int mintime, int maxtime, int curtime) {
   int i;
   if (memory.getSize() == 0) {
      return 0;
   }

   for (i=0; i<memory.getSize(); i++) {
      if ((curtime - (int)memory[i].time) < mintime) {
         continue;
      }
      if ((curtime - (int)memory[i].time) > maxtime) {
         break;
      }
      if ((memory[i].p0() == message.p0()) && (memory[i].p1() == message.p1())) {
         return 1;
      }
   }

   return 0;
}



/*-------------------- triggered algorithms -----------------------------*/

///////////////////////////////
//
// keyboardchar -- this function is called by the improv interface
//     whenever a key is pressed on the computer keyboard.
//     Put commands here which will be executed when a key is
//     pressed on the computer keyboard.
//

void keyboardchar(int key) { 
   switch (key) {
      case ' ':  // switch state of squelch
         sstate = !sstate;
         if (sstate) {   cout << "Squelch is on" << endl; }
         else        {   cout << "Squelch is off" << endl; }
         break;
      case '-':  // lower veladd
         veladd--;
         if (veladd < 0) { veladd = 0; }
         cout << "Velocity add is: " << veladd << endl;
         break;
      case '=': // raise veladd
         veladd++;
         if (veladd > 64) {
            veladd = 64;
         }
         cout << "Velocity add is: " << veladd << endl;
         break;
   }

}


/*------------------ end improvization algorithms -----------------------*/

// md5sum: fec683c6ca9cd01239f3869eec40e02c squelch.cpp [20050403]

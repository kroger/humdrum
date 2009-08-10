//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Dec 19 13:52:11 PST 1999
// Last Modified: Wed Jan 12 15:21:15 PST 2000
// Filename:      ...improv/examples/synthImprov/testgliss.cpp
// Syntax:        C++; synthImprov 2.0
//
// Description: This program is designed for testing the speed of MIDI 
//              transfers and for checking the throughput of various
//              microcontrollers.  The program sends a continuous
//              stream of MIDI notes in a glissando pattern.  You
//              can control the speed of the glissando with the
//              "[" and "]" keys.  The range of the notes for the
//              glissando are controlled by the "-" and "=" keys.
//              The space bar toggles the program on/off.  
//
//              MIDI data can be returned to this program and it will be
//              compared to the output.  If any differences occur
//              between the input and the output, an error message
//              will be printed to the screen with the input and
//              output messages which were not similar.  At that point
//              the program will send out the MIDI command [0xaa 0x7f
//              0x00] and stop sending out the glissando data.
//           
//              There are also some neat ideas for controlling
//              glissandos in this program.
//
//              This program also demonstrates how to disable the 
//              Synthesizer input buffer for directly receiving
//              MIDI input data (since a lower-level MIDI input
//              interpretation might be more useful in the future).
//

#include "synthImprov.h" 
// #include <iomanip.h>

typedef unsigned char uchar;

/*----------------- beginning of improvization algorithms ---------------*/

SigTimer notetimer;          // timer for timing period between MIDI messages
double   period     = 200.0; // time in ms to wait between MIDI output messages
int      lowestnote = 60;    // lowest note in the glissando
int      highestnote = 72;   // highest note in the glissando
int      note = lowestnote;  // current note being played in the glissando
int      notestate = -1;     // note is either on=1, or off=0.
int      direction = 1;      // 1=gliss going up, -1 = gliss going down
int      step = 1;           // half-note step amount

int      comparestate = 0;   // boolean for comparing input to output MIDI
uchar    checkin[3]   = {0}; // temporary comparison storage for MIDI input 
uchar    checkout[3]  = {0}; // temporary comparison storage for MIDI output
uchar    data         = 0;   // dummy holder for data before inserting to buffer
MidiMessage message;         // for reading in MIDI data.
MidiInput midiinput;         // raw MIDI input interface.

CircularBuffer<uchar> sentout;    // storage copy of MIDI output data
CircularBuffer<uchar> receivedin; // storage of MIDI input data to compare 


// function declarations:
void help(void);
void start(void);
void stop(void);

/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   printboxtop();
   psl(" TESTGLISS - by Craig Stuart Sapp <craig@ccrma.stanford.edu>"
       " - 11 Jan 2000");
   psl("");
   printintermediateline();
   psl("  User commands:");
   psl("     \"[\" = speed up.                \"]\" = slow down. ");
   psl("     \"{\" = speed up by 5.           \"}\" = slow down by 5. ");
   psl("     \"1\" = lower bottom of range.   \"2\" = raise bottom of range. ");
   psl("     \"3\" = lower range.             \"4\" = raise range. ");
   psl("     \"5\" = lower step.              \"6\" = raise step. ");
   psl("     \"-\" = lower top of range.      \"=\" = raise top of range. ");
   psl("     \"u\" = gliss up.                \"d = gliss down.  ");
   psl(" ");
   psl(" Type 'h' for more information  ");
   printboxbottom();
} 


void initialization(void) { 
   notetimer.setPeriod(period); // set the period in ms between MIDI events.
   notetimer.reset();

   sentout.setSize(1024);       // store up to 1024 MIDI output bytes 
   receivedin.setSize(1024);    // store up to 1024 MIDI input bytes
   description();

   synth.makeOrphanBuffer();
   start();
}


void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

void mainloopalgorithms(void) { 
   if (comparestate && notetimer.expired()) {
      if (notetimer.expired() > 2) {
         notetimer.reset();
      } else {
         notetimer.update();
      }

      notestate = !notestate;
      if (notestate == 1 || notestate == -1) {
         synth.play(0, note, 64);
         data = 0x90; sentout.insert(data);
         data = note; sentout.insert(data);
         data = 64;   sentout.insert(data);
      } else {
         synth.play(0, note, 0);
         data = 0x90; sentout.insert(data);
         data = note; sentout.insert(data);
         data = 0;    sentout.insert(data);
         note += step * direction;
         if (note > highestnote) {
            note = lowestnote;
         }
         if (note < lowestnote) {
            note = highestnote;
         }
      }
   }

   if (midiinput.getCount() > 0) {
      message = midiinput.extract();
      receivedin.insert(message.p0());
      receivedin.insert(message.p1());
      receivedin.insert(message.p2());

      // check that the messages are identical
      if (receivedin.getCount() < 3) {
         cout << "Error: not enough received data" << endl;
      } else {
         checkin[0] = receivedin.extract();
         checkin[1] = receivedin.extract();
         checkin[2] = receivedin.extract();
      }

      if (sentout.getCount() < 3) {
         cout << "Error: not enough sent data" << endl;
      } else {
         checkout[0] = sentout.extract();
         checkout[1] = sentout.extract();
         checkout[2] = sentout.extract();
      }

      if ((checkout[0] != checkin[0]) || (checkout[1] != checkin[1]) ||
          (checkout[2] != checkin[2])) {
         synth.rawsend(0xaa, 0x7f, 0x00);
         cout << "Error " 
              << "output was = (" << hex << (int)checkout[0] << ") "
              << dec << (int)checkout[1] << " "
              << dec << (int)checkout[2] << "\tbut input is = ("
              << hex << (int)checkin[0] << ") "
              << dec << (int)checkin[1] << " "
              << dec << (int)checkin[2] << " "
              << endl;

         // assume that a note message was missed.
         if (sentout.getCount() < 3) {
            cout << "Error: not enough sent data during error" << endl;
         } else {
            checkout[0] = sentout.extract();
            checkout[1] = sentout.extract();
            checkout[2] = sentout.extract();
         }

         stop();  
         cout << "Press space to restart testing, "
                 "or press 'S' to silence synth" << endl;
      }

   }
 
}
      

/*-------------------- triggered algorithms -----------------------------*/

void keyboardchar(int key) { 
   switch (key) {
      case 'h':        // more help
         help();
         break;
      case '[':        // slow tempo
         period--;
         if (period < 1) {
            period = 1;
         }
         cout << "Period: " << period << endl;
         notetimer.setPeriod(period);
         break;
      case '{':        // slow tempo by 5
         period -= 5;
         if (period < 1) {
            period = 1;
         }
         cout << "Period: " << period << endl;
         notetimer.setPeriod(period);
         break;
      case ']':        // speed tempo
         period++;
         cout << "Period: " << period << endl;
         notetimer.setPeriod(period);
         break;
      case '}':        // speed tempo by 5
         period += 5;
         cout << "Period: " << period << endl;
         notetimer.setPeriod(period);
         break;
      case 'u':        // make glissando go up
         direction = 1;
         cout << "Glissandoing up" << endl;
         break;
      case 'd':        // make glissando go down
         direction = -1;
         cout << "Glissandoing down" << endl;
         break;
      case '1':        // lower bottom of glissando range
         if (lowestnote > 0) {
            lowestnote--;
            cout << "lowest note set to: " << lowestnote << endl;
         }
         break;
      case '2':        // raise bottom of glissando range
         if (lowestnote < highestnote - 1) {
            lowestnote++;
            cout << "lowest note set to " << lowestnote << endl;
         } 
         break;
      case '3':        // lower both the top and the bottom of range
         if (lowestnote > 0) {
            lowestnote--;
            highestnote--;
            cout << "Range lowered" << endl;
         }
         break;
      case '4':        // lower both the top and the bottom of range
         if (highestnote < 127) {
            lowestnote++;
            highestnote++;
            cout << "Range raised" << endl;
         }
         break;
      case '5':        // lower step amount
         step--;
         if (step < 1) {
            step = 1;
         }
         cout << "Step size = " << step << endl;
         break;
      case '6':        // raise step amount
         step++;
         cout << "Step size = " << step << endl;
         break;
      case '-':        // lower top of glissando range
         if (highestnote > lowestnote + 1) {
            highestnote--;
            cout << "highest note set to: " << highestnote << endl;
         }
         break;
      case '=':        // raise top or glissando range
         if (highestnote < 127) {
            highestnote++;
            cout << "highestnote note set to: " << highestnote << endl;
         }
         break;
      case ' ':       // toggle sending/comparing of data
         if (comparestate) {
            stop();
            cout << "Stopped data transmission/comparison" << endl;
         } else {
            cout << "Starting data transmission/comparison" << endl;
            start();
         }
   }
}



/*------------------ end improvization algorithms -----------------------*/

//////////////////////////////
// 
// start -- start sending gliss data and start recording/comparing.
//

void start(void) {
   comparestate = 1;

   sentout.reset();             // initialize the MIDI output storage
   receivedin.reset();          // initialize the MIDI input storage

   midiinput.setPort(synth.getInputPort());
   midiinput.open();
}



//////////////////////////////
// 
// stop -- stop comparing gliss data and stop comparing input to output.
//

void stop(void) {
   if (notestate) {
      synth.play(0, note, 0);
      notestate = 0;
   }
   comparestate = 0;
}



//////////////////////////////
// 
// help -- print more information about the program
//

void help(void) {
   cout << 
"\n\nThis program is designed for testing the speed of MIDI transfers and\n"
"for checking the throughput of various microcontrollers.  The program\n"
"sends a continuous stream of MIDI notes in a glissando pattern.  You can\n"
"control the speed of the glissando with the '[' and ']' keys.  The range\n"
"of the notes for the glissando are controlled by the '1/-' and '2/=' keys.\n"
"The space bar toggles the program on/off.\n\n"
"MIDI data can be returned to this program and it will be compared to\n"
"the output.  If any differences occur between the input and the output,\n"
"an error message will be printed to the screen with the input and output\n"
"messages which were not similar.  At that point the program will send out\n"
"the MIDI command [0xaa 0x7f 0x00] and stop sending out the glissando data.\n"
   << endl;
} 




// md5sum: de57ee927bb8a05ec5fca7dab2c7728d testgliss.cpp [20050403]

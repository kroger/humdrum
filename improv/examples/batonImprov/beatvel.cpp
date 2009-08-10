//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Last Modified: Fri Mar 31 13:47:54 PST 2000
// Last Modified: Sat Apr  1 11:54:55 PST 2000
// Filename:      ...improv/doc/examples/batonImprov/beatvel/beatvel.cpp
// Syntax:        C++; batonImprov
//  
// Description:   Determines musical beats according to the velocity of the
//                z-axis.  The space bar will record the data used to 
//                determine the beats and the beat states:
//                  beat state 0  = no beat is expected
//                  beat state 1  = beat threshold is exceeded, beat coming soon
//                  beat state 10 = a beat has occurred
//                  beat state 2  = a beat has just occurred, do not change 
//                                  state until threshold timeout occurs.
// 
//                The output pitched played when a beat is detected is related 
//                to the location on the z-axis at the time of the beat.
//

#include "batonImprov.h" 

#ifndef OLDCPP
   #include <fstream>
   using namespace std;
#else
   #include <fstream.h>
#endif


/*----------------- beginning of improvization algorithms ---------------*/

CircularBuffer<short> zposition(32); // a history of the past positions
CircularBuffer<short> zvelocity(32); // a history of the past velocities
double  threshold        = 4.0;      // Minimum activation velocity
double  thresholdTime    = 0;        // Time which threshold was reached
int     thresholdQ       = 0;        // Boolean for threshold region
double  thresholdTimeout = 75;       // ms timeout for threshold
int     beatnum          = 1;        // beat number

fstream datafile;                    // for recording beats
int     datawriteQ       = 0;        // boolean for writing datafile
int     datasession      = 0;        // for multiple recordings in one file
int     datamarker       = 1;        // for placing marker in recorded data
Voice   voice;                       // for playing a note on the beat

/*----------------- FUNCTIONS -------------------------------------------*/

//////////////////////////////
//
// writedata -- write z velocity data to a test file.
//

void writedata(int velocity, int state) {
   if (datawriteQ) {
      datafile << velocity << '\t' << state << '\n';
   }
}
   

//////////////////////////////
//
// writecomment -- write comment to the data file if recording.
//

#define writecomment(x) if (datawriteQ) { datafile << "# " << x; } 



//////////////////////////////
//
// stick1position -- callback function for stick1 position reporting.
//

void stick1position(void) {
   int beatQ = 0;
   zposition.insert(baton.z1p);
   if (zposition.getCount() <= 1) {
      return;
   } 
 
//   cout << baton.t1pb[0] - baton.t1pb[1] << endl;

   zvelocity.insert(zposition[0] - zposition[1]);
   if (zvelocity.getCount() <= 1) {
      return;
   } 

   if (thresholdQ == 2) {
      if (t_time > thresholdTime + thresholdTimeout) {
         thresholdQ = 0;
         writedata(zvelocity[0], 0);
         goto nextline;
      } else {
         writedata(zvelocity[0], 2);
         goto nextline;
      }
   }
   if (thresholdQ == 0) {
      if (zvelocity[0] > threshold) {
         thresholdQ = 1;
         thresholdTime = t_time;
         writedata(zvelocity[0], 1);
         goto nextline;
      } else {
         writedata(zvelocity[0], 0);
         goto nextline;
      }
   }
   if (thresholdQ == 1) {
      if (zvelocity[0] < zvelocity[1]) {
         writedata(zvelocity[0], 10);
         cout << "Beat: " << beatnum++ << endl;
         voice.play(baton.z1p - 30, 64);
         if (beatnum % 4 == 0) {
            cout << "\n";
         }
         thresholdQ = 2;
         goto nextline;
      } else {
         writedata(zvelocity[0], 1);
         goto nextline;
      }
   }

   writedata(zvelocity[0], beatQ);

nextline:
   ;
}


/*--------------------- maintenance algorithms --------------------------*/

void description(void) {
   cout 
   << "\nThis program detects musical beats according to the velocity of\n"
   << "the z-axis positions. \n"
   << "  Keyboard commands:\n"
   << "  -/=   = lower/raise the velocity trigger threshold\n"
   << "  [/]   = lower/raise the threshold blackout time\n"
   << "  space = start/stop data recording\n"
   << "  s     = display the status of data recording\n"
   << "  m     = put a marker in the data recording\n"
   << "the z-axis positions\n"
   << endl;
}


void initialization(void) { 
   description();
   baton.stick1position = stick1position;
   zposition.reset();
   zvelocity.reset();
   datafile.open("datafile", ios::out);
   if (!datafile.is_open()) {
      cerr << "Error: cannot open output file: datafile" << endl;
      exit(1);
   }
   voice.setChannel(0);        // default MIDI channel for beat performance
}

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/

void mainloopalgorithms(void) { }


/*-------------------- triggered algorithms -----------------------------*/

void stick1trig(void) { }

void stick2trig(void) { }

void b14plustrig(void) { }

void b15plustrig(void) { }

void b14minusuptrig(void) { }

void b14minusdowntrig(void) { }

void b15minusuptrig(void) { }

void b15minusdowntrig(void) { }

void keyboardchar(int key) { 
   switch (key) {
      case '-':                 // lower threshold by one
      case '_':                 // lower threshold by one
         threshold = threshold - 1;
         if (threshold < 1) {
            threshold = 1;
         }
         cout << "Velocity threshold is: " << threshold << endl;
         writecomment("Threshold: " << threshold << "\n");
         break;
      case '=':                 // raise threshold by one
      case '+':                 // raise threshold by one
         threshold = threshold + 1;
         if (threshold > 50) {
            threshold = 50;
         }
         cout << "Velocity threshold is: " << threshold << endl;
         writecomment("Threshold: " << threshold << "\n");
         break;

      case '[':                 // lower timeout threshold by 5
         thresholdTimeout = thresholdTimeout - 5;
         if (thresholdTimeout < 20) {
            thresholdTimeout = 20;
         }
         cout << "threshold timeout is: " << thresholdTimeout << endl;
         writecomment("Timeout: " << thresholdTimeout << "\n");
         break;
      case ']':                 // raise timeout threshold by 5
         thresholdTimeout = thresholdTimeout + 5;
         if (thresholdTimeout > 1000) {
            thresholdTimeout = 1000;
         }
         cout << "threshold timeout is: " << thresholdTimeout << endl;
         writecomment("Timeout: " << thresholdTimeout << "\n");
         break;

      case 's':                 // display recording status
         if (datawriteQ) {
            cout << "\nRecording data" << endl;
         } else {
            cout << "\nNOT Recording data" << endl;
         }
         break;
         
      case 'm':                 // place a data marker in the recording
         if (datawriteQ) {
            cout << "\nData Marker: " << datamarker << endl;
            writecomment("Marker: " << datamarker++ << "\n");
         }
         break;

      case ' ':                 // toggle writing of datafile
         datawriteQ = !datawriteQ;
         if (!datafile.is_open()) {
            datawriteQ = 0;
         }
         if (datawriteQ) {
            datasession++;
         }
         if (datawriteQ) {
            cout << "Starting to write data: session " << datasession << endl;
            writecomment("Session: " << datasession << "\n");
         } else {
            cout << "Stopping data session " << datasession << " recording" 
                 << endl;
         }
         break;
   }
}


/*------------------ end improvization algorithms -----------------------*/



// md5sum: 779aab60dbfae8df50fb56aee9cf7534 beatvel.cpp [20050403]

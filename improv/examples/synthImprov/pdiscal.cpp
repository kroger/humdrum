//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Nov 20 00:08:27  2002
// Last Modified: Wed Nov 20 00:08:39  2002
// Filename:      ...sig/doc/examples/improv/synthImprov/pdiscal.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: Generate calibration data for squelching echo data from
//              the PianoDisk player piano.
//

#include "synthImprov.h"      // includes the default Win95 console interface
                              // for the synthImprov environment


/*----------------- beginning of improvization algorithms ---------------*/


Array< Array< Array<short> > > deltatimes;
Array< Array< Array<char> > >  velchange;

int  currentnote 	=   0;
int  notecount 		=   0;
int  nextnotetime 	=   0;
int  duration 		= 100;
int  currvoice 		=   0;
int  returnpitch 	=   0;
int  returnvelocity 	=   0;
int  returntime 	=   0;
char vchange		=   0;
int  playstate          =   0;

int maxvoices = 16;
Array<Voice> voices;
Array<int>   outtimes;
Array<int>   outvels;


// function declarations:
double average(Array<short>& data);


/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << " spacebar starts test, c = print note count " << endl;
   cout << " p = print data file " << endl;
} 



//////////////////////////////
//
// initialization -- this function is called by the improv
//     interface once at the start of the program.  Put items
//     here which need to be initialized at the beginning of
//     the program.
//

void initialization(void) { 
   outtimes.setSize(128);
   outtimes.setAll(0);

   outvels.setSize(128);
   outvels.setAll(0);

   voices.setSize(maxvoices);
   int i, j;
   for (i=0; i<voices.getSize(); i++) {
      voices[i].setPort(synth.getInputPort());
   }

   nextnotetime = t_time;
   deltatimes.setSize(128);

   for (i=0; i<deltatimes.getSize(); i++) {
      deltatimes[i].setSize(128);
      for (j=0; j<deltatimes[i].getSize(); j++) {
         deltatimes[i][j].setSize(1000);
         deltatimes[i][j].setGrowth(1000);
         deltatimes[i][j].setSize(0);
      }
   }

   velchange.setSize(128);

   for (i=0; i<velchange.getSize(); i++) {
      velchange[i].setSize(128);
      for (j=0; j<velchange[i].getSize(); j++) {
         velchange[i][j].setSize(1000);
         velchange[i][j].setGrowth(1000);
         velchange[i][j].setSize(0);
      }
   }

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
int velocity;
short dtime;

void mainloopalgorithms(void) { 

   while (synth.getNoteCount() > 0) {
      message = synth.extractNote();
      if (message.is_note_off()) {
         continue;
      }
      returnpitch = message.p1();
      returnvelocity = message.p2();
      // returntime = message.time;
      returntime = t_time;
      
      dtime = (short)(returntime - outtimes[returnpitch]);
      if (dtime > 500) {   // ignore return times which take more than 0.5 sec
         continue;
      }

      vchange = returnvelocity - outvels[returnpitch];
      velchange[returnpitch][outvels[returnpitch]].append(vchange);
      deltatimes[returnpitch][outvels[returnpitch]].append(dtime);
   }

   if (!playstate) {
      return;
   }

   if (nextnotetime <= t_time) {
      currentnote += 11;
      if (currentnote > 127) {
         currentnote -= 127;
      }
      if (currentnote < A0 || currentnote > C8) {
         return;
      }
      velocity = rand() % 127 + 1;   // random velocity between 1 and 127

      voices[currvoice].off();
      outvels[currentnote] = velocity;
      outtimes[currentnote] = mainTimer.getTime();
      voices[currvoice++].play(0, currentnote, velocity);
      if (currvoice >= maxvoices) {
         currvoice = 0;
      }

      notecount++;
      nextnotetime = t_time + duration;
   }

}



void printDataFile(void) {
   const char* filename = "pdiscal.dat";
   fstream file;
   file.open(filename, ios::out);
   if (!file.is_open()) {
      cout << "ERROR: Cannot open file to write!" << endl;
      return;
   }
 
   int count;
   int i, j, k;
   for (i=0; i<127; i++) {
      for (j=0; j<127; j++) {
         file << "p" << i << "\tv" << j << "\t";
         count = deltatimes[i][j].getSize();
         for (k=0; k<count; k++) {
            file << "[" << (int) deltatimes[i][j][k] << ":" 
                 << (int) velchange[i][j][k] << "]";
            if (k<count-1) {
               file << " ";
            }
         }
         file << "\n";
      }
      
   }

}



void printMathFile(void) {
   const char* filename = "pdiscal.ma";
   fstream file;
   file.open(filename, ios::out);
   if (!file.is_open()) {
      cout << "ERROR: Cannot open file to write!" << endl;
      return;
   }
 
   file << "data = {";
   int i, j;
   for (i=0; i<128; i++) {
      file << "{";
      for (j=0; j<128; j++) {
         file << average(deltatimes[i][j]);
         if (j<deltatimes[i].getSize()-1) {
            file << ", ";
         }
      }
      if (i<deltatimes.getSize()-1) {
         file << ", ";
      }
      file << "},\n";
   }
   file << "};\n\n";
   file << "Print[\"Show[SurfaceGraphics[data]]\"]\n";
   file << "Show[SurfaceGraphics[data]]\n";
}



double average(Array<short>& data) {
   int i;
   double output = 0.0;
   for (i=0; i<data.getSize(); i++) {
      output += data[i];
   }
   if (i > 0) {
      output = output / data.getSize();
   }

   return output;
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
      case ' ':   // turn test on/off
         playstate = !playstate;
         cout << "PLAYSTATE: " << playstate << endl;
         break;
      case 'c':  // display current note count
         cout << "TOTAL NOTES: " << notecount << endl;
         break;
      case 'p':  // print data file
         printDataFile();
         cout << "DATA file written" << endl;
         break;
      case 'm': // print mathematica data
         printMathFile();
         cout << "MATHEMATICA file written" << endl;
         break;
      case '-':  // slow down duration
         duration++;
         cout << "Duration = " << duration << " milliseconds" << endl;
         break;
      case '+':  // speed up duration
         duration--;
         if (duration < 20) {
            duration = 20;
         }
         cout << "Duration = " << duration << " milliseconds" << endl;
         break;
   }
}


/*------------------ end improvization algorithms -----------------------*/


// md5sum: 78c25019bd1506271d30e4a78a61185a pdiscal.cpp [20050403]

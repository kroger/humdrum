//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 13 10:14:57 PDT 2000
// Last Modified: Tue Jul  4 21:37:29 PDT 2000
// Filename:      ...sig/doc/examples/all/batontap/batontap.cpp
// Syntax:        C++; batonImprov 2.0
//  
// Description:   Two general methods of tempo control for the radio
// 		  batons.
//

#include "batonImprov.h" 

#define MAXBEATPERIOD 2500  /* maximum time between beats, ignore if longer */
#define MAXLAGTIME      70  /* maximum time for same instant in time */
#define BASENOTE        60  /* MIDI base note for performance */
#define BATONSRATE    20.0  /* Radio Baton position reporting samping rate */


/*----------------- beginning of improvization algorithms ---------------*/

CircularBuffer<int> beattrigtime(1000); // times of previous beats
int avglen = 1;                         // beats to average for tempo
double idletime = 0.0;                  // time to sleep between iterations
SigTimer beatFraction;                  // for timing performance locations
double beatLocation = 0.0;              // for timing performance 
double lastBeatLocation = 0.0;          // for timing performance
int divisions = 4;                      // performance divisions per beat
int newdivisions = 4;                   // when changing the beat divisions
int divisionChange = 0;                 // for changing the beat divisions
int currentsub = -1;                    // currently playing subdivision
int numbermeaning = 'd';                // meaning of digits on keyboard
int runningQ = 0;                       // boolean for playing notes
int printsubQ = 1;                      // boolean for printing subbeats
int printbeatQ = 0;                     // boolean for printing beat info
int printpredictionQ = 0;               // boolean for printing beat pred. info
Voice voice;                            // for performance of subbeats

int predictionQ = 0;                    // boolean for beat prediction
int beatprediction = 0;                 // prediction location of next beat
int newprediction = 0;                  // newest predition of beat location
int correction = 0;                     // correction between old and new pred.
int triggerplane = 110;                 // trigger plane location


// function declarations:
double averagePeriod(CircularBuffer<int>& buffer, int avglen);
void   beattrig(int trigtime);
double periodToTempo(int mstime);
double periodToTempo(double mstime);
void   playnotes(double beatLocation, double lastBeatLocation, int& currentsub);
void   performnote(int subdivision, int totaldivision);
void   adjustTempoWithPrediction(void);
void   stick1position(void);
double tempoToPeriod(double tempo);


/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << 
   "BATONTAP by Craig Stuart Sapp, June 2000\n" 
   "Commands:\n"
   " [, ] = lower/raise the number of periods to average for tempo\n"
   " d#   = set the divisions per beat\n"
   " p    = toggle beat prediction\n"
   " u    = toggle display of subbeats on screen\n"
   " o    = toggle display of beat statistics\n"
   " i    = toggle display of beat prediction info\n"
   << endl;
} 



//////////////////////////////
//
// initialization -- this function is called by the improv
//     interface once at the start of the program.  Put items
//     here which need to be initialized at the beginning of
//     the program.
//

void initialization(void) { 
   baton.stick1position = stick1position;
   beattrigtime.reset();
   beattrigtime[0] = 0;
   beattrigtime[1] = 0;
   beattrigtime[2] = 0;
   beattrigtime[3] = 0;
   eventIdler.setPeriod(idletime);
   runningQ = 0;
   voice.setPort(synth.getPort());
   beatFraction.setTempo(80);
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

void mainloopalgorithms(void) { 
   lastBeatLocation = beatLocation;
   beatLocation = beatFraction.getPeriodCount();

   if (beattrigtime[1] != 0 && 
      beattrigtime[0] - beattrigtime[1] < MAXBEATPERIOD) {
      if (runningQ == 0) {
         lastBeatLocation = 0.0;
      }
      runningQ = 1;
   } else {
      runningQ = 0;
   }

   if (runningQ && beatLocation < 1.0) {
      playnotes(beatLocation, lastBeatLocation, currentsub);
   }
}



//////////////////////////////
//
// prepareNewPrediction -- make a new prediction when
//     the next beat will occur.
//

int prepareNewPrediction(void) {
   // get the instantaneous (average) velocity
   double velocity1 = baton.z1pb[0] - baton.z1pb[1];
   double velocity2 = baton.z1pb[1] - baton.z1pb[2];
   double velocity = (velocity1 + velocity2) / 2.0;

   // do stuff here with accelerations, etc., to improve prediction.

   // now know use trajectory velocity to figure 
   // out when we should arrive at the trigger plane

   // if heading in the wrong direction, then don't make
   // a prediction:
   if (velocity < -2) {
      return -1;
   } else if (velocity < 2) {
      return 0;
   }

   double tickcount = triggerplane / velocity;
   return t_time + (int)(BATONSRATE * tickcount);
}



//////////////////////////////
//
// playnotes -- 
//

void playnotes(double beatLocation, double lastBeatLocation, 
      int& currentsub) {
   int desiredsub = (int)(beatLocation * divisions);

   // if there is only one division per beat, then need
   // a special case:
   if (beatLocation < lastBeatLocation && divisions == 1) {
      performnote(0, 1);
      currentsub = 0;
      return;
   }

   int amount, j;
   if (desiredsub == currentsub) {
      return;
   } else if (desiredsub < currentsub) {
      // first play all notes upto the previous beat location:
      amount = divisions - 1 - currentsub;
      for (j=0; j<amount; j++) {
         performnote(currentsub + j + 1, divisions);
      }
      // now play all notes up to the desired subbeat location:
      for (j=0; j<=desiredsub; j++) {
         performnote(j, divisions);
      }
      currentsub = desiredsub;
   } else {
      amount = desiredsub - currentsub;
      for (j=0; j<amount; j++) {
         performnote(currentsub + j + 1, divisions);
      }
      currentsub = desiredsub;
   }
}


//////////////////////////////
//
// performnote -- 
//

void performnote(int subdivision, int totaldivision) {
   static int lasttime = 0;

   voice.play(BASENOTE + subdivision, 64);
   if (divisionChange && subdivision == 0) {
      divisions = newdivisions;
      divisionChange = 0;
   }

   if (printsubQ) {
      cout << subdivision << "\t(" << t_time - lasttime << ")" << endl;
   }

   lasttime = t_time;
}



/*-------------------- triggered algorithms -----------------------------*/

//////////////////////////////
//
// stick1trig -- this function is called automatically whenever
//   a baton stick #1 trigger is received.
//

void stick1trig(void) { beattrig(t_time); }



//////////////////////////////
//
// stick2trig -- this function is called automatically whenever
//   a baton stick #2 trigger is received.
//

void stick2trig(void) { beattrig(t_time); }



//////////////////////////////
//
// b14plustrig -- this function is called automatically whenever
//   the b14+ button is pressed.
//

void b14plustrig(void) { beattrig(t_time); }



//////////////////////////////
//
// b15plustrig -- this function is called automatically whenever
//   the b15+ button is pressed.
//

void b15plustrig(void) { beattrig(t_time); }




//////////////////////////////
//
// b14minusuptrig -- this function is called automatically whenever
//   the b14- foot trigger is pressed.
//

void b14minusuptrig(void) { beattrig(t_time); }



//////////////////////////////
//
// b14minusdowntrig -- this function is called automatically whenever
//   the b14- foot trigger is released.
//

void b14minusdowntrig(void) { beattrig(t_time); }



//////////////////////////////
//
// b15minusuptrig -- this function is called automatically whenever
//   the b15- foot trigger is pressed.
//

void b15minusuptrig(void) { beattrig(t_time); }



//////////////////////////////
//
// b15minusdowntrig -- this function is called automatically whenever
//   the b15- foot trigger is released.
//

void b15minusdowntrig(void) { beattrig(t_time); }



///////////////////////////////
//
// keyboardchar -- this function is called by the improv interface
//     whenever a key is pressed on the computer keyboard.
//     Put commands here which will be executed when a key is
//     pressed on the computer keyboard.
//

void keyboardchar(int key) { 
   switch(key) {
      case 'p':   // toggle beat predictions
         predictionQ = !predictionQ;
         if (predictionQ) {
            cout << "Beats are predicted" << endl;
         } else {
            cout << "Beats are NOT predicted" << endl;
         }
         break;
      case 'i':   // toggle printing of beat predictions
         printpredictionQ = !printpredictionQ;
         if (printpredictionQ) {
            cout << "Prediction printing is ON" << endl;
         } else {
            cout << "Prediction printing is OFF" << endl;
         }
         break;
      case 'o':   // toggle printing of beat period information
         printbeatQ = !printbeatQ;
         if (printbeatQ) {
            cout << "Beat information is ON" << endl;
         } else {
            cout << "Beat information is OFF" << endl;
         }
         break;
      case 'u':   // toggle printing of subbeats
         printsubQ = !printsubQ;
         break;
      case ' ':   // initiate a beat
         beattrig(t_time);
         break;
      case '[':   // lower the beat averaging period count
      case '{':   // lower the beat averaging period count
         avglen--;
         if (avglen < 1) {
            avglen = 1;
         }
         cout << "Beat period averaging is: " << avglen << endl;
         break;
      case ']':   // raise the beat averaging period count
      case '}':   // raise the beat averaging period count
         avglen++;
         if (avglen > 1000) {
            avglen = 1000;
         }
         cout << "Beat period averaging is: " << avglen << endl;
         break;
      case '-':   // lower idle period
         idletime -= 1.0;
         if (idletime < 0.0) {
            idletime = 0.0;
         }
         cout << "Idle time = " << idletime << endl;
         eventIdler.setPeriod(idletime);
         break;
      case '=':   // raise idle period
         idletime += 1.0;
         cout << "Idle time = " << idletime << endl;
         eventIdler.setPeriod(idletime);
         break;
      case 'd':   // set the meaning of the numbers to divisions
         numbermeaning = 'd';
         cout << "Digits mean divisions per beat" << endl;
         break;
   }

   if (isdigit(key)) {
      switch (numbermeaning) {
         case 'd':
            if (key == '0') {
               break;
            }
            if (t_time - beattrigtime[0] < MAXLAGTIME) {
               divisions = key - '0';   // number of notes per beat
               cout << "Beat division: " << divisions << endl;
               divisionChange = 0;
            } else {
               newdivisions = key - '0';   // number of notes per beat
               cout << "Next beat divisions: " << divisions << endl;
               divisionChange = 1;
            }
            break;
      }
   }

}



/*------------------ end improvization algorithms -----------------------*/

///////////////////////////////
//
// beattrig -- trigger a new beat.
//

void beattrig(int trigtime) {
   beattrigtime.insert(trigtime);
   double avgperiod = averagePeriod(beattrigtime, avglen);
   int diff = beattrigtime[0] - beattrigtime[1];

   if (printbeatQ && beattrigtime[1] != 0 && diff < MAXBEATPERIOD) {
      cout << "T:" << trigtime
           << "\tbeat: "  << diff;
      if (avglen > 1) {
         cout << "\tAvg: "   << avgperiod;
      }
      cout << "\tTempo: " << periodToTempo(diff);
      if (avglen > 1) {
         cout << "\tAvg: " << periodToTempo(avgperiod);
      }
      cout << endl;
   }

   if (avgperiod > 0) {
      beatFraction.setPeriod(avgperiod);
      beatFraction.reset();
    
      // make a prediction for when the next beat will occur
      beatprediction = t_time + (int)avgperiod;
   }
}


///////////////////////////////
//
// beattrig -- trigger a new beat.
//

double averagePeriod(CircularBuffer<int>& buffer, int avglen) {
   double sum = 0.0;
   int i = 0;
   int diff;
   for (i=1; i<=avglen; i++) {
      diff = buffer[i-1] - buffer[i];
      if (buffer[i] == 0) {
         break;
      } else if (diff > MAXBEATPERIOD) {
         break;
      } else {
         sum += diff;
      }
   }
   i--;
  
   if (i == 0) {
      return 0.0;
   } else {
      return sum/i;
   }
}



///////////////////////////////
//
// periodToTempo -- converts a time in milliseconds into a tempo
//    in beats per minute.
//

double periodToTempo(int mstime) {
   return (60000.0 / mstime);
}

double periodToTempo(double mstime) {
   return (60000.0 / mstime);
}

double tempoToPeriod(double tempo) {
   return (60000.0 / tempo);
}



//////////////////////////////
//
// stick1position -- called whenever a stick 1 position report comes in.
//

void stick1position(void) {
   if (predictionQ && beatLocation > 0.5) {
      adjustTempoWithPrediction();
   }
}



//////////////////////////////
//
// adjustTempoWithPrediction --
//

void adjustTempoWithPrediction(void) {
   newprediction = prepareNewPrediction();

   // don't know what is happening (stationary stick)
   if (newprediction == 0) {
      return;
   }

   // going in the wrong direction
   if (newprediction  < 0) {
      newprediction = beatprediction + 100;
      return;
   } 

   correction = beatprediction - newprediction;
   if (abs(correction) > beatFraction.getPeriod() * 0.7) {
      // error is too large, so ignore correction
      return;
   }

   // now have a half-way decent prediction, so adjust
   // tempo so that beatFraction timer will hit 1.0 at
   // newly predicted beat location.
   double oldTempo = beatFraction.getTempo();
   double newPeriod = beatFraction.getPeriod() + correction;
   double newTempo = periodToTempo(newPeriod);

   if (printpredictionQ) {
      cout << "Time: "         << t_time 
           << "\tError: "      << correction
           << "\tOld Tempo = " << oldTempo
           << "\tNew Tempo = " << newTempo
           << endl;
   }

   beatFraction.setTempo(newTempo); 
   beatprediction = newprediction;
}




// md5sum: 48f142d9e661d25f4c7746cf7651f787 batontap.cpp [20050403]

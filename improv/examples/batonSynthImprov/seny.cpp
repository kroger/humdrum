//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Programmer:    Seny Lee <senylee@ccrma.stanford.edu>
// Creation Date: Mon May 17 10:25:04 PDT 1999
// Last Modified: Tue May 25 20:46:16 PDT 1999
// Last Modified: Fri Jun 25 16:34:54 PDT 1999
// Filename:      .../improv/examples/synthImprov/seny/seny.cpp
// Syntax:        C++; batonSynthImprov 2.1
//  

#include "batonSynthImprov.h"      
#include <ctype.h>

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

#ifdef VISUAL
   double drand48(void) {
      return (double)rand()/RAND_MAX;
   }
#endif


#define BOUND1    50              /* boundary note of low section of keyboard */
#define BOUND2    79              /* boundary note of high section of keybd   */
#define VARIATION  3.0            /* std devs to traverse w/drum controls     */
#define MAXVOICES  12             /* maximum drum voices at one time          */
#define MINVEL     20             /* minimum attack velocity output           */
#define MAXVEL     90             /* maximum attack velocity output           */


/*----------------- beginning of improvization algorithms ---------------*/

// Global variables:
int activeQ = 1;                  // true if outputing notes false otherwise
int controlDisplayQ = 1;          // for displaying baton control variables

// variables for keeping track of performer's notes with which baton will
// use to generate notes on the piano in an algorithmic fashion.
int lastPerformerTime = 0;        // time of last performer note on/off
int performerNotes[127] = {0};    // all of the performer's notes being played
int performerPC[12] = {0};        // pitch class of the performer
int performerPCHistory[12] = {0}; // performer pitch class history
int keyDuration = 5000;           // how long to remember performers notes
CircularBuffer<char> keys;        // store keys for memory of previous notes
CircularBuffer<long> keytimes;    // note times for keys buffer

// the variables for stick 1 x-axis density control
double density = 0;               // 0=thick, 1=thin

// keep track of performer's notes for stick 1 y-axis register control
int performerRegion[3] = {0};     // location that the performer is playing
double pregister = 0;             // 0=low, 1=high

// the variables for stick 1 z-axis controlling consonance and dissonance
double consonance = 0.0;         // 0 = consonant, 1 = dissonant
// cintervals are a list of the intervals in an octave starting with
// the most consonant ones and then going to the most dissonant ones.
int    cintervals[12] = {0,7,5,4,9,8,3,10,2,11,1,6};



// loudness variables for baton stick 2 x-axis control of velocity volumes
double avgVol = 0;                // average volume
double avgVolRange = 0;           // standard deviation of the durations
CircularBuffer<double> volumes;   // duration of notes being held by performer
CircularBuffer<double> voltimes;  // duration of notes being held by performer
int volDuration = 5000;           // how long to keep track of volumes
double velocity = 0;              // 0=similar, 1=inverted

// duration variables for baton stick 2 y-axis control of articulations
double avgDur = 0;                // average duration
double avgDurRange = 0;           // standard deviation of the durations
CircularBuffer<long> durations;   // duration of notes being held by performer
CircularBuffer<long> durtimes;    // duration of notes being held by performer
int durDuration = 5000;          // how long to keep track of durations
double articulation = 0;          // 0=staccato, 1=tenuto, 0.5=normal

// the variables for stick 2 z-axis controlling rhythm 
double rhythmType = 0;            // 0=constant, 1=random wide variation
int    zmax = 120;                // for z-axis normalizing
int    zmin = 25;                 // for z-axis normalizing


// baton and buffer checking variables
SigTimer batonTimer;             // time to get new state of baton
SigTimer offTimer;               // time to check buffer for forgetting
SigTimer controlDisplayTimer;    // time to check buffer for forgetting


//////////////////////////////
//
// Radio Drum performance variables
//

Voice    computer;
MidiMessage computerMessage;
Voice    voice[MAXVOICES];
int      voiceState[MAXVOICES] = {0};
int      voiceOnTime[MAXVOICES] = {0};
int      voiceOffTime[MAXVOICES] = {0};
SigTimer voiceTimer;
int      voicePeriod = 100;          
int      displayOutput = 0;            // for displaying output notes of baton


// function declarations:
void checkBuffers(void);
void checkOffNotes(void);
int  countActiveVoices(void);
void displayVariables(void);
void processBaton(void);
void processKeyboard(void);
void updateDuration(void);
void updatePerformRegions(void);
void updateVolume(void);
int  midiLimit(int aNumber);


//////////////////////////////
//
// Radio Drum performance variables
//

void generateVoices(void);


/*--------------------- maintenance algorithms --------------------------*/


//////////////////////////////
//
// description -- this function is called by the improv interface
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << 
   "Seny 1 -- project for \n"
   " Keyboard commands:  \n"
   "    n = display piano's history input pitch set\n"
   "    m = display piano's currently on pitch set\n"
   "    d = display baton's output notes\n"
   "    c = display baton's control variables\n"
   "    r = display piano's register tracking variables\n"
   "  ' ' = activate/deactivate program\n"
   "    v = display active voice count of baton's notes\n"
   "    z = forget the piano's current key history\n"
   "    [ = shorten note memory by 1 second\n"
   "    ] = length note memory by 1 second\n"
   << endl;
} 



//////////////////////////////
//
// initialization -- this function is called by the improv
//     intervace once at the start of the program.  Put items
//     here which need to be initialized at the beginning of
//     the program.
//

void initialization(void) { 
   batonTimer.setPeriod(50);           // time to get new state of baton
   offTimer.setPeriod(200);            // time to check buffer for forgetting
   controlDisplayTimer.setPeriod(200); // time to check buffer for forgetting

   // set the voice channels all to be 0 for the disklavier and so
   // the channels do not have to be specified when playing the note.
   for (int i=0; i<MAXVOICES; i++) {
      voice[i].setChannel(0);
   }
   computer.setChannel(0);
   computerMessage.time = t_time;
   computerMessage.p0() = 0x90;
   computerMessage.p1() = 0;
   computerMessage.p2() = 0;

   keys.setSize(1000);        // store keys for memory of previous notes
   keytimes.setSize(1000);    // note times for keys buffer
   volumes.setSize(1000);     // duration of notes being held by performer
   voltimes.setSize(1000);    // duration of notes being held by performer
   durations.setSize(1000);   // duration of notes being held by performer
   durtimes.setSize(1000);    // duration of notes being held by performer
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
   if (synth.getNoteCount() > 0) {
      processKeyboard();
   }

   if (offTimer.expired()) {
      checkBuffers();
      checkOffNotes();
      offTimer.reset();
   }

   if (batonTimer.expired()) {
      processBaton();
      batonTimer.reset();
   }

   voicePeriod = (int)(avgDur - avgDurRange);
   if (voicePeriod <= 50) {
      voicePeriod = 50;
   }

   voiceTimer.setPeriod(voicePeriod);
   if (voiceTimer.expired()) {
      generateVoices();
      voiceTimer.reset();
   }

   if (controlDisplayQ && controlDisplayTimer.expired()) {
      displayVariables();
      controlDisplayTimer.reset();
   }

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
   if (isdigit(key)) {
      int mkey = 5 * 12 + key - '0';
      int mvel = (int)(drand48() * 40 + 40);
       
      //turn off old note from computer's point of view
      computerMessage.time = t_time;
      computerMessage.p2() = 0;
      synth.insert(computerMessage);

      cout << "played note: " << mkey << " with velocity: " << mvel << endl;
      computer.play(mkey, mvel);

      computerMessage.time = t_time;
      computerMessage.p0() = 0x90;           // midi note-on command, channel 1
      computerMessage.p1() = mkey;
      computerMessage.p2() = mvel;
      synth.insert(computerMessage);
      cout << "played note: " << mkey << " with velocity: " << mvel << endl;

      return;
   }

   switch (key) {
      case ' ':           // turn output notes on/off
         activeQ = !activeQ;
         if (activeQ) {
            cout << "Program Activated" << endl;
         } else {
            cout << "Program Deactivated" << endl;
         }
         break;

      case 'c':           // toggle baton control display
         controlDisplayQ = !controlDisplayQ;
         if (controlDisplayQ == 0) {
            cout << endl;
         }
         break;
         
      case 'd':           // display baton's output notes
         displayOutput = !displayOutput;
         if (displayOutput) {
            cout << "Baton output notes display turned ON" << endl;
         } else {
            cout << "Baton output notes display turned OFF" << endl;
         }
         break;

      case 'n':           // display performerPCHistory pitches
         {
         cout << "Piano notes: ";
         for (int i=0; i<11; i++) {
            cout << performerPCHistory[i] << ", ";
         }
         cout << performerPCHistory[11] << endl;
         }
         break;

      case 'm':           // display performerPC pitches
         {
         cout << "Notes on:    ";
         for (int i=0; i<11; i++) {
            cout << performerPC[i] << ", ";
         }
         cout << performerPC[11] << endl;
         }
         break;

      case 'r':           // display the register information
         {
         cout << "Register: "
              << performerRegion[0]
              << performerRegion[1]
              << performerRegion[2]
              << endl;
         }
         break;

      case 'v':           // display the active voice count for baton
         {
         cout << endl << "Active voices = " << countActiveVoices();
         cout << ": ";
         for (int z=0; z<MAXVOICES; z++) {
            if (voiceState[z]) {
               cout << " " << (int)voice[z].getKey();
            } else {
               cout << "  0";
            }
         }
         cout << endl;
         }
         break;

      case 'z':           // forget the current piano's key history
         {
         for (int z=0; z<12; z++) { 
            performerPCHistory[z] = 0;
         }
         keys.reset();
         keytimes.reset();
         }
         break;

      case '[':           // decrease the note memory time
         keyDuration -= 1000;
         if (keyDuration < 1000) {
            keyDuration = 1000;
         }
         cout << "Note memory time set to : " << keyDuration/1000 
              << " seconds" << endl;
         break;

      case ']':           // increase the note memory time
         keyDuration += 1000;
         if (keyDuration > 60000) {
            keyDuration = 60000;
         }
         cout << "Note memory time set to : " << keyDuration/1000 
              << " seconds" << endl;
         break;

      case '-':           // turn off computer keyboard note
         // turn off old note from computer's point of view
         computerMessage.time = t_time;
         computerMessage.p2() = 0;
         synth.insert(computerMessage);

         // turn off old note from synthesizer's point of view
         computer.off();
         break;
   }

}


void stick1trig(void) {
   generateVoices();
}

void stick2trig(void) { 
   generateVoices();
}

void b14plustrig(void) { }
void b15plustrig(void) { }
void b14minusuptrig(void) { }
void b14minusdowntrig(void) { }
void b15minusuptrig(void) { }
void b15minusdowntrig(void) { }



/*------------------ begining of assistant functions --------------------*/

//////////////////////////////
//
// checkBuffers -- look at the history buffers and determine if any
//     of the buffers needs to be emptied of any contents.
//

void checkBuffers(void) {

   // look at the volume buffers for old volumes:
   while ( (voltimes.getCount() > 0) && (voltimes[voltimes.getCount()-1] <
         t_time - volDuration) ) {
      volumes.extract();
      voltimes.extract();
   }

   // look at the duration buffers for old durations:
   while ( (durtimes.getCount() > 0) && (durtimes[durtimes.getCount()-1] <
         t_time - durDuration) ) {
      durations.extract();
      durtimes.extract();
   }

   // look at the key buffers for old keys:
   int oldkey;
   while ( (keytimes.getCount() > 0) && (keytimes[keytimes.getCount()-1] <
         t_time - keyDuration) ) {
      oldkey = keys.extract();
      keytimes.extract();
  
      performerPCHistory[oldkey%12]--;
      if (performerPCHistory[oldkey%12] < 0) {
         performerPCHistory[oldkey%12] = 0;
      }

   }

}



//////////////////////////////
//
// checkOffNotes -- check the voices array to see if any of the notes
//    need to be turned off.
//

void checkOffNotes(void) {
   for (int i=0; i<MAXVOICES; i++) {
      if (voiceState[i] && voiceOffTime[i] < t_time) {
         voice[i].off();
         voiceState[i] = 0;
      }
   }
}



//////////////////////////////
//
// countActiveVoices -- return the number of baton voices that
//   are currently on.
//

int countActiveVoices(void) {
   int output = 0;
   for (int i=0; i<MAXVOICES; i++) {
      if (voiceState[i]) {
         output++;
      }
   }
   
   return output;
}



//////////////////////////////
//
// displayVariables -- display the baton's control variables in
//    real-time on the computer monitor.
//

void displayVariables(void) {
   // move cursor to the beginning of the line
   cout << "\r";
   // erase previous line
   cout << 
"                                                                          ";
   cout << "\r";

   // print out each axis variable:
   cout << "den:";
   cout.width(6);
   cout << (int)(MAXVOICES*density);
   cout << "   reg:";
   cout.width(6);
   cout << (int)(3*pregister);
   cout << "   con:";
   cout.width(6);
   cout << cintervals[(int)(consonance*12)];
   cout << "   vel:";
   cout.width(6);
   cout << ((int)(velocity*100))/100.0;
   cout << "   art:";
   cout.width(6);
   cout << ((int)(articulation*100))/100.0;
   cout << "   rhy:";
   cout.width(6);
   cout << ((int)(rhythmType*100))/100.0;
   cout << flush;
}




//////////////////////////////
//
// generateVoices -- 
//

void generateVoices(void) {
   int voiceson = 0;
   int i;

   // count how many voices are on
   for (i=0; i<MAXVOICES; i++) {
      if (voiceState[i]) {
         voiceson++;
      }
   }

   // determine how many voices should be on.
   int voicesnow = (int)(MAXVOICES * density);

   if (voicesnow == voiceson) {
      return;
   }

   if (voicesnow < voiceson) {
      // need to erase voices
 
      int eraseindex; 
      for (i=0; i<voiceson-voicesnow; i++) {
         eraseindex = rand()%MAXVOICES;
         if (voiceState[eraseindex] == 1) {
            voiceState[eraseindex] = 0;
            voice[eraseindex].off();
         } else {
            int errorprotect = 0;
            while (voiceState[eraseindex] == 0 && errorprotect < MAXVOICES*2) {
               errorprotect++;
               eraseindex = (eraseindex + 1) % MAXVOICES;
               if (voiceState[eraseindex] == 1) {
                  voiceState[eraseindex] = 0;
                  voice[eraseindex].off();
               } 
            }
         }
      }

   } else {
      // need to add voices

      int totalnotes = 0;
      for (i=0; i<12; i++) {
         totalnotes += performerPCHistory[i];
      }
      if (totalnotes == 0) {
         return;
      }
         
      double noteDist[12];
      for (i=0; i<12; i++) {
         noteDist[i] = performerPCHistory[i]/(double)totalnotes;
      }

      double randomval;
      int j;
      for (i=0; i< voicesnow-voiceson; i++) {
         randomval = drand48();          // random number between 0.0 and 1.0
         double sum = 0.0;

         // choose a pitch class from the piano performer
         // you are more likely to choose a pitch which the pianist
         // has play alot.
         for (j=0; j<12; j++) {
            sum += noteDist[j]; 
            if (sum > randomval) {
               break;
            }
         }
         int choosenpc = j;

         // now that a pitch class from the pianist has been choosen
         // choose the output pitch class from the radio drum's 
         // consonance dissonance control
         int outputpc = (choosenpc + cintervals[(int)(consonance*12)]) % 12;

         // now that we have the output pitch class, assign a register 
         // to it.

         // choose a register, then look to find if there is a free register
         // associated with the chosen register
         int outputregister = (int)(pregister * 3);
         if (performerRegion[outputregister] != 0) {
            if (pregister < 0.5) {
               outputregister = (outputregister+1) % 3;
            } else {
               outputregister = (outputregister-1+30) % 3;
            }
         } 

         if (performerRegion[outputregister] != 0) {
            if (pregister < 0.5) {
               outputregister = (outputregister+1) % 3;
            } else {
               outputregister = (outputregister-1+30) % 3;
            }
         }

         if (performerRegion[outputregister] != 0) {
            if (pregister < 0.5) {
               outputregister = (outputregister+1) % 3;
            } else {
               outputregister = (outputregister-1+30) % 3;
            }
         }

         if (performerRegion[outputregister] != 0) {
            continue;
         }
            
         int octave = rand()%2 + outputregister * 2 + 3;

         int outputpitch = outputpc + octave * 12;
         int initialvolume = (int)(avgVol - VARIATION * avgVolRange + 
               VARIATION * 2 * avgVolRange * velocity);
         if (initialvolume < MINVEL) {
            initialvolume = MINVEL;
         } else if (initialvolume > MAXVEL) {
            initialvolume = MAXVEL;
         }
//         int oppositevol = 127 - initialvolume;
//         int outputvolume = (int)((oppositevol - initialvolume) * velocity + 
//               initialvolume);
int outputvolume = initialvolume;
         if (outputvolume < MINVEL) {
            outputvolume = MINVEL;
         } else if (outputvolume > MAXVEL) {
            outputvolume = MAXVEL;
         }
         
         // we now know the velocity and pitch of the output note.
         // now we need to calculate the duration and the voice slot
         // to place the note in.

         double outduration = avgDur - VARIATION * avgDurRange + 
               VARIATION * 2 * drand48() * avgDurRange;
       
         // adjust duration of notes according to baton control
         outduration += VARIATION * (avgDurRange * (articulation * 
            2 - 1));
         outduration *=  3.0 * rhythmType;
 
         // prevent very short notes, nothing less than 50 ms in duration
         if (outduration < 50) {
            outduration = 50.0;
         }
         if (avgDur < 50) {
            outduration = 150.0;
         }

         int outputduration = (int)outduration;

         // choose a voice slot to store the note.
         int assignment = (int)(drand48() * MAXVOICES);
         int errorprotect = 0;
         while (voiceState[assignment] != 0 && errorprotect < MAXVOICES*2) {
            errorprotect++;
            assignment = (assignment + 1) % MAXVOICES;
         }
         if (voiceState[assignment] == 0 && activeQ) {
            voiceState[assignment] = 1;
            voiceOnTime[assignment] = t_time;
            voiceOffTime[assignment] = t_time + outputduration;
            voice[assignment].play(outputpitch, outputvolume);
            if (displayOutput) {
               cout << "Note: " <<  outputpitch << "\tvolume = " 
                    << outputvolume << "\tduration = " << outputduration 
                    << endl;
            }
         }
      }
   }
}



//////////////////////////////
//
// midiLimit -- guarentee that the given number is in the range
//     from 0 to 127.
//

int midiLimit(int aNumber) {
   if (aNumber < 0) {
      return 0;
   } else if (aNumber > 127) {
      return 127;
   } else {
      return aNumber;
   }
}



//////////////////////////////
//
// processBaton -- update the program variables related to the radio
//      batons.
//

void processBaton(void) {
   // update the variable for keeping track of texture density:
   // density 0 = thick, density 1 = thin
   density = baton.x1p/127.0;             // baton.x1p == xt1

   // update the variable for keeping track of articulation
   // pregister 0 = low, pregister 1 = high
   pregister = baton.y1p/127.0;            // baton.y1p == yt1

   // update the variable for keeping track of the consonance:
   // consonance 0 = consonant, consonance 1 = dissonant
   // zt1 = baton.z1p
   if (zmax - zmin == 0) {
      consonance = 0.5;
   } else {
      consonance = 1.0 - (baton.z1p - zmin) * 1.0 / (zmax - zmin);
      if (consonance < 0.0) {
         consonance = 0.0;
      }
   }


   // update the variable for keeping track of velocity
   // velocity 0 = similar, velocity 1 = inverted
   velocity = baton.x2p/127.0;        // baton.x2p == xt2

   // update the variable for keeping track of articulation
   // articulation 0 = staccato, articulation 1 = tenuto, 0.5 = normal
   articulation = baton.y2p/127.0;        // baton.y2p == yt2

   // update the variable for keeping track of rhythm type:
   // rhythmType 0 = even, rhythmType 1 = random
   // zt2 = baton.z2p
   if (zmax - zmin == 0) {
      rhythmType = 0.5;
   } else {
      rhythmType = 1.0 - (baton.z2p - zmin) * 1.0 / (zmax - zmin);
   }
   if (rhythmType < 0.0) {
      rhythmType = 0.0;
   }
}



//////////////////////////////
//
// processKeyboard -- get notes from the piano and store them as needed
//    the the radio baton monitoring variables.
//

void processKeyboard(void) {
   MidiMessage message;
   int key;
   int vel;
   int command;
   
   while (synth.getNoteCount() > 0) {
      message = synth.extractNote();
      command = message.p0();
      key = message.p1();
      vel = message.p2();
 
      if (vel == 0 || (command & 0xf0 == 0x80)) {
         // note-off command section
         long duration = t_time - performerNotes[key];
         durations.insert(duration);
         durtimes.insert(t_time);

         performerNotes[key] = 0;
         performerPC[key%12]--;
         if (performerPC[key%12] < 0) {
            performerPC[key%12] = 0;
         }
      
      } else {   // note-on command
         performerNotes[key] = t_time;
         performerPC[key%12]++;
         performerPCHistory[key%12]++;
         keys.insert(key);
         keytimes.insert(t_time);
         volumes.insert(vel);
         voltimes.insert(t_time);

      } // end of the note-on command section

   }  // end of the while loop for processing notes from the performer

   // update the time that the performer last play a note on/off:
   lastPerformerTime = t_time;

   updatePerformRegions();
   updateDuration();
   updateVolume();
}



//////////////////////////////
//
// updateDuration -- recalculate the average duration and its possible
//       range.  Calculate average duration and the standard deviation 
//       of durations.
//

void updatePerformRegions(void) {
   // determine which region of the piano the performer is
   // playing on:
   performerRegion[0] = 0;
   performerRegion[1] = 0;
   performerRegion[2] = 0;
   for (int i=0; i<127; i++) {
      if (performerNotes[i] > 0) {
         if (i < BOUND1) {
            performerRegion[0] = 1;
         } else if (i < BOUND2) {
            performerRegion[1] = 1;
         } else {
            performerRegion[2] = 1;
         }
      }
   }

}



//////////////////////////////
//
// updateDuration -- recalculate the average duration and its possible
//       range.  Calculate average duration and the standard deviation 
//       of durations.
//

void updateDuration(void) {
   int i;
   double dursum = 0.0;
   for (i=0; i<durations.getCount(); i++) {
      dursum += durations[i];
   }
   if (durations.getCount() != 0) {
      avgDur = dursum / durations.getCount();
   } else {
      avgDur = 0.0;
   }
   // get the variance
   dursum = 0.0;
   for (i=0; i<durations.getCount(); i++) {
      dursum += (avgDur - durations[i]) * (avgDur - durations[i]);
   }
   avgDurRange = sqrt(dursum);
}



//////////////////////////////
//
// updateVolume -- recalculate the average volume and it possible
//       range.  Calculate average volume and the standard deviation 
//       of volumes
//
   
void updateVolume(void) {
   int i;
   double volsum = 0.0;
   for (i=0; i<volumes.getCount(); i++) {
      volsum += volumes[i];
   }
   if (volumes.getCount() != 0) {
      avgVol = volsum / volumes.getCount();
   } else {
      avgVol = 0.0;
   }
   // get the variance
   volsum = 0.0;
   for (i=0; i<volumes.getCount(); i++) {
      volsum += (avgVol - volumes[i]) * (avgVol - volumes[i]);
   }
   avgVolRange = sqrt(volsum);
}
        


            
   

/*------------------ end improvization algorithms -----------------------*/




// md5sum: c8bbb162e65d0138d1619ab7891fc58a seny.cpp [20050403]

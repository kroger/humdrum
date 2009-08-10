//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Oct 26 17:00:30 PST 1998
// Last Modified: Wed Oct 28, 1998
// Last Modified: Fri Jun 25 16:38:47 PDT 1999
// Filename:      ...sig/doc/examples/improv/synthImprov/position2/position2.cpp
// Syntax:        C++; synthImprov 2.0
//  
// Description: plays notes according to input data coming from NIDAQ card.
//     Sensor used to design this program was an ultrasonic sensor
//     part 134105 from Senix Corporation, 52 Maple St. Bristol. VT 05443.
//     Bought at Jameco in Redwood City CA, Fall 1998.
//     The sonar sensor output 0 volts when an object is 6 inches or less
//     positioned infront of the ultrasonic speaker.  The output voltage
//     rises linearly to 10 volts around the position 5 feet from the 
//     sensor.  Beyond the far range of the sensor, the voltage stays
//     at its maximum (9.9975 or so).  This program should work with
//     other types of sensors, but specific musical interface may be less
//     practical.  Also works well on Force Sensitive Resistors.
//

#include "synthImprov.h"    /* basic MIDI I/O interface with MIDI synthesizer */
#include "NidaqSensor.h"    /* interface with NIDAQ Card (Windows 95 only)    */

/*----------------- beginning of improvization algorithms ---------------*/

// global variables:
double      min = 2.4;       // miniumum value from NIDAQ, adapts
double      max = 2.5;       // maximum value from NIDAQ, adapts
int         keyno;           // MIDI keynumber to play
int         velocity;        // MIDI attack velocity
int         instrument = GM_VIBRAPHONE;  // instrument on MIDI synth
int         sustain = 0;     // sustain pedal (is initially off)
int         volume = 0;      // value for volume control
int         oldVolume = 0;   // previous value for volume control
int         display = 0;     // boolean for displaying data on screen
int         method = 0;      // which note generation method to use
int         methodCount = 4; // number of generation methods
int         minimumDuration = 10;  // 10 millisec is fastest tempo for neighb.
int         lastNoteTime = 0;// time of last note for method 2
int         volControl = 0;  // boolean for volume control mode
int         interval = 1;    // for certain note controller methods
int         channel = 0;     // NIDAQ channel to use
Voice       voice;           // for MIDI output (auto handles note off messages)
Nidaq       sensor;          // NIDAQ card interface for sensor
SigTimer    rhythmTick;      // for timing rhythm of voice
SigTimer    displayTimer;    // for displaying positions on the screen
int         lastVolTime =0;  // for volume continuous controller 
int         difference;      // static temp value
int         currentDuration; // static temp value
int         direction = 0;   // boolean for pitch range direction
#define STATE_COUNT 8        /* number of slots to save performance states */
#define STATE_VARS 9         /* number of variables to store for state */
int         state[STATE_COUNT][STATE_VARS] = {0};  // perf. state variables
int         stateQ = 0;      // true if save state is activated
int         stateA = 0;      // true if restore state is activated

// rhythmic ostinato variables
#define PATTERN_COUNT 3                       // number of rhythmic patterns
int patternSize[PATTERN_COUNT] = {1, 6, 6};   // array sizes of patterns
int patternIndex[PATTERN_COUNT] = {0,0,0};    // current index of patterns
int *pattern[PATTERN_COUNT];                  // the array of patterns
int pattern0[1] = {1};                        // constant 16th note pattern
int pattern1[6] = {3, 1, 2, 2, 1, 3};         // 3=dotted eighth, 2=eighth
int pattern2[6] = {2, -2, 2, -2, -2, 2};      // rests are negative
int curpat = 0;                               // pattern to currently play



// non-interface function declarations:
void determineMidiNotes(void);
void newNoteMethod1(void);
void newNoteMethod2(void);
void volumeMethod(void);
void saveState(int aState);
void restoreState(int aState);
void printState(int aState);


/*--------------------- maintenance algorithms --------------------------*/

//////////////////////////////
//
// description -- this function is called by the improv interface 
//     whenever a capital "D" is pressed on the computer keyboard.
//     Put a description of the program and how to use it here.
//

void description(void) {
   cout << '\n';
   printboxtop();
   pcl("POSITION2: Craig Stuart Sapp <craig@ccrma.stanford.edu> 26 Oct 1998");
   printintermediateline();
   psl(" NIDAQ device is hardwired to device #1. Initial data range is");
   psl(" 2.4 to 2.5, but range adapts.  So go through whole range of sensor");
   psl(" to set the software range.");
   psl(" Commands:");
   psl(" \"0\"-\"9\" = select NIDAQ analog input channel");
   psl(" \"d\" = toggle sensor display.   \"s\" = toggle sustain pedal");
   psl(" \"-\" = decrement instrument.    \"=\" = increment instrument number");
   psl(" \"m\" = control method select.   \"v\" = volume control toggle");
   psl(" \"r\" = select new rhythm.       \"x\" = reset the sensor range");
   psl(" \",\" = decrease interval.       \".\" = increase interval");
   psl(" \"t\" = slow tempo.              \"g\" = increase tempo");
   psl(" \"u\" = display current state.   \"z\" = reverse pitch range");
   psl(" \"q\" = save performance state.  \"]\" = restore preformance state");
   psl("Performance state storage keys:");
   pcl("\"i\"   \"o\"   \"p\"   \"[\"");
   pcl("   \"k\"   \"l\"   \";\"   \"\'\"");
   printboxbottom();
} 



//////////////////////////////
//
// initialization -- this function is called by the improv
//     intervace once at the start of the program.  Put items
//     here which need to be initialized at the beginning of 
//     the program.
//

void initialization(void) { 
   sensor.initialize(gargv);    // start CVIRTE stuff for NIDAQ Card
   sensor.setPollPeriod(1);     // check for new data every 1 millisecond   
   sensor.setBufferSize(8);     // buffer size of transfer frame
   sensor.setSrate(500);        // set NIDAQ sampling rate to X Hz
   cout << "starting data aquisition ... " << flush;
   sensor.start();              // start aquiring data from NIDAQ Card
   cout << "ready." << endl;
   voice.setPort(synth.getOutputPort());   // specify output port of voice
   channel = 0;                 // specify output channel of voice
   voice.pc(instrument);        // set the MIDI synthesizer tambre
   rhythmTick.setPeriod(100);   // minimum note duration every X milliseconds
   displayTimer.setPeriod(200); // display position every X milliseconds

   cout << "Using instrument number " << instrument << " (vibraphone)"
        << endl;

   // set the contents of the pattern array
   pattern[0] = pattern0;
   pattern[1] = pattern1;
   pattern[2] = pattern2;
}



//////////////////////////////
//
// finishup -- this function is called by the improv interface
//     whenever the function is exited.  Put items here which
//     need to be taken care of when the program is finished.
//

void finishup(void) { }


/*-------------------- main loop algorithms -----------------------------*/


//////////////////////////////
//
// mainloopalgorithms -- this function is called by the improv interface
//   continuously while the program is running.  The variable t_time 
//   which stores the current time is set just before this function is 
//   called and remains constant while in this functions.
//

void mainloopalgorithms(void) { 
   sensor.checkPoll();   // see if it is time to check for new data frame
   determineMidiNotes(); // check if it is time to play a MIDI note
   if (display && displayTimer.expired()) {
      displayTimer.reset();
      cout << "\rmin = " << min << "\tmax = " << max << "\tcurrent = "
           << sensor[channel][channel][0] << "\t\t" << flush;
   }
}



//////////////////////////////
//
// determineMidiNotes -- a user-specific function which is not part
//     of the improv interface.  This function determines the method
//     of note generation/control from incoming position data from
//     the sensor.
//

void determineMidiNotes(void) {
   switch (method) {
      case 0:   newNoteMethod1();   break;
      case 1:   newNoteMethod2();   break;
      case 3:   volumeMethod();     break;
   }
   if (volControl && method != 3) {
      volumeMethod();
   }
}



//////////////////////////////
//
// newNoteMethod2 -- plays a new note whenever sensor detects
//     a new note.  The global variable "interval" controls
//     the determination of a new note.  interval = 1 means that
//     the new note can be any intervale larger than a unison.
//     interval = 7 would mean that a new note would be triggered
//     whenever a interval size of a tritone (interval 6) is 
//     traversed.
//

void newNoteMethod2(void) {
   // if the duration of the previous note is long enough and
   // the sensor is not at its min or max value, then play a note
   // if the interval distance has been traversed since the last
   // note which was played.
   if (t_time - voice.getOnTime() > minimumDuration && 
        (sensor[channel][0] != min || sensor[channel][0] != max)) {

      // adjust the range of the sensor values if necessary:
      if (sensor[channel][0] > max)   max = sensor[channel][0];
      if (sensor[channel][0] < min)   min = sensor[channel][0];
 
      // keyno is the note to be played.  It is in the range from 0 to 127
      keyno = 127 - (int)((sensor[channel][0] - min) / (max - min) * 127);

      if (direction) {
         keyno = 127 - keyno;
      }
   
      // limit the range of keyno to valid range, in case max/min not correct
      if (keyno < 0)  keyno = 0;
      if (keyno > 127)  keyno = 127;

      // if the interval between the last note and the current position's
      // note is large enough, then play the new note.
      difference = voice.getKey() - keyno;
      if (difference < 0) {
         difference = -difference;
      }
      if (difference >= interval) {
         if (keyno < 2 || keyno > 125) { // turn off notes out of sensor range
            voice.off();
         } else {
            velocity = 120;
            voice.play(keyno, velocity);
         }
      }
   }
}



//////////////////////////////
//
//  volumeMethod -- interprets the sensor data as a volume control
//    which in MIDI standard is continuous controller #7 (0 offset)
//    which can take a value from 0 (no sound) to 127 (max sound).
//    Max sound is near the sonar detector, min sound is away from the
//    sonar detector.  If the sensor data suddenly jumps to maximum
//    voltage, then the loudness control is disabled to keep the 
//    current loudness level in effect.
//

void volumeMethod(void) {
   if (t_time - lastVolTime > 20) {   // if statement prevents MIDI saturation
      lastVolTime = t_time;

      // generate a volume level in the range from 0 to 127.
      volume = 127 - (int)((sensor[channel][0] - min) / (max - min) * 127);

      // disactivate volume control at extremes of range
      if (keyno <= 6)   return;
      if (keyno >= 126) return;  

      // this if statement isn't necessary but prevents sending out
      // duplicate volume messages which might tie up the MIDI cable.
      if (volume != oldVolume) {
         oldVolume = volume;
         voice.cont(7, volume);    
      }
   }
}



//////////////////////////////
//
// newNoteMethod1 -- plays notes in a rhythmic ostinato
//

void newNoteMethod1(void) {
   currentDuration = pattern[curpat][patternIndex[curpat]];
   if (rhythmTick.expired() >= abs(currentDuration)) {
      // time to play a new MIDI note
      rhythmTick.update(abs(currentDuration));

      // set the index of the next note duration
      patternIndex[curpat]++;
      patternIndex[curpat] %= patternSize[curpat];

      if (rhythmTick.expired() > 10) {
         // if this happens, then the timer is WAY behind, 
         // so just reset it.
         rhythmTick.reset();
      }

      // check for rests
      if (currentDuration < 0) {
         return;
      }

      // adjust the range of the sensor values if necessary:
      if (sensor[channel][0] > max)   max = sensor[channel][0];
      if (sensor[channel][0] < min)   min = sensor[channel][0];

      // create a MIDI key number in the range from 127 (near) to 0 (far)
      keyno = 127 - (int)((sensor[channel][0] - min) / (max - min) * 127);

      if (direction) {
         keyno = 127 - keyno;
      }

      // make sure there wasn't an invalid min or max value
      if (keyno < 0) keyno = 0;
      if (keyno > 127) keyno = 127;

      if (keyno < 2 || keyno > 125) {
         voice.off();
      } else {
         velocity = 120;
         voice.play(keyno, velocity);
      }
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

   // select a NIDAQ channel if the key is a number from 0 to 9:
   if (isdigit(key)) {
      cout << "\nresetting data aquisition ... " << flush;
      channel = key - '0';
      cout << "done." << endl;
      cout << "Channel " << channel << " selected" << endl;
      return;
   }

   switch (key) {
      case '[':               // performance state 4
         if (stateA) {
            printState(4);
         } else if (stateQ) {
            saveState(4);
         } else {
            restoreState(4);
         }
         stateA = stateQ = 0;
         break;
      case ';':               // performance state 7
         if (stateA) {
            printState(7);
         } else if (stateQ) {
            saveState(7);
         } else {
            restoreState(7);
         }
         stateA = stateQ = 0;
         break;
      case '\'':               // performance state 8
         if (stateA) {
            printState(8);
         } else if (stateQ) {
            saveState(8);
         } else {
            restoreState(8);
         }
         stateA = stateQ = 0;
         break;
      case '-':              // decrement the MIDI instrument
         instrument--;
         if (instrument < 0) {
            instrument = 0;
         }
         cout << "\n Instrument: " << instrument << endl;
         voice.pc(instrument);
         break;
      case '=':               // increment the MIDI instrument
         instrument++;
         if (instrument > 127) {
            instrument = 127;
         }
         cout << "\nInstrument: " << instrument << endl;
         voice.pc(instrument);
         break;
      case ',':              // decrement the method2 interval
         interval--;
         if (interval <= 0) {
            interval = 1;
         }
         cout << "\ninterval: " << interval << endl;
         break;
      case '.':               // increment the method2 interval
         interval++;
         if (interval > 24) {
            interval = 24;
         }
         cout << "\ninterval: " << interval << endl;
         break;
      case ']':               // set restore state flag
         if (stateQ != 0) {
            stateQ = 0;
         }
         stateA = 1;
         break;
      case 'd':               // toggle display of data on screen
         display = !display;
         break;
      case 'g':               // decrease sixteenth note duration by 10 ms
         if (rhythmTick.getPeriod() >= 20.0) {
            rhythmTick.setPeriod(rhythmTick.getPeriod() - 3.0);
         } 
         cout << "\nsixteenthnote duration set to " 
              << rhythmTick.getPeriod() << " milliseconds" << endl;
         break;
      case 'i':               // performance state 1 
         if (stateA) {
            printState(1);
         } else if (stateQ) {
            saveState(1);
         } else {
            restoreState(1);
         }
         stateA = stateQ = 0;
         break;
      case 'k':               // performance state 5
         if (stateA) {
            printState(5);
         } else if (stateQ) {
            saveState(5);
         } else {
            restoreState(5);
         }
         stateA = stateQ = 0;
         break;
      case 'l':               // performance state 6
         if (stateA) {
            printState(6);
         } else if (stateQ) {
            saveState(6);
         } else {
            restoreState(6);
         }
         stateA = stateQ = 0;
         break;
      case 'm':               // switch to a new note generation method
         method = (method + 1) % methodCount;
         cout << "\nUsing method " << method + 1;
         switch (method) {
            case 0:   cout << " (rhymic pattern) ";   break;
            case 1:   cout << " (neighbor) ";         break;
            case 2:   cout << " (volume only) ";      break;
            case 3:   cout << " (silence) ";          break; 
         }
         cout << endl;
         break;
      case 'o':               // performance state 2
         if (stateA) {
            printState(2);
         } else if (stateQ) {
            saveState(2);
         } else {
            restoreState(2);
         }
         stateA = stateQ = 0;
         break;
      case 'p':               // performance state 3
         if (stateA) {
            printState(3);
         } else if (stateQ) {
            saveState(3);
         } else {
            restoreState(3);
         }
         stateA = stateQ = 0;
         break;
      case 'q':               // set save state flag
         if (stateA != 0) {
            stateA = 0;
         }
         stateQ = 1;
         break;
      case 'r':              // select a new rhythmic pattern
         curpat = (curpat + 1) % PATTERN_COUNT;
         cout << "\nrhythmic pattern = " << curpat << endl;
         break; 
      case 's':              // toggle the sustain pedal
         sustain = !sustain;
         
         voice.sustain(sustain);    // sustain function takes a boolean


         if (sustain) {
            cout << "\nsustain pedal is ON" << endl;
         } else {
            cout << "\nsustain pedal is OFF" << endl;
         }
         break;
      case 't':              // increase sixteenth note duation by 10 ms.
         rhythmTick.setPeriod(rhythmTick.getPeriod() + 3.0);
         cout << "\nsixteenth note duration = " << rhythmTick.getPeriod() 
              << " milliseconds" << endl;
         break;
      case 'u':              // display performance variables
         cout << "\nsus = " << sustain 
              << "\tint = " << interval
              << "\tmet = " << method
              << "\trhy = " << curpat
              << "\tvol = " << volControl
              << "\tins = " << instrument
              << "\tdir = " << direction
              << "\ttem = " << (int)rhythmTick.getPeriod()
              << endl;
         break;
      case 'v':              // toggle volume control
         volControl = !volControl;
         if (volControl) {
            cout << "\nVolume control is ON" << endl;
         } else {
            cout << "\nVolume control is OFF" << endl;
         }
         break;
      case 'x':              // reset the min and max range
         min = 2.4;
         max = 2.5;
         break;
      case 'z':
         direction = !direction;
         break;
   }      
}


///////////////////////////
//
// saveState -- records performance state variables which can be restored
//     with the restoreState function to return to a particular
//     performance setup.  Format of state array is:
//          index 0 = validity of state (0 = bad, 1 = good)
//          index 1 = [sustain]
//          index 2 = [interval]
//          index 3 = [method]
//          index 4 = [rhythm]
//          index 5 = [volControl]
//          index 6 = [instrument]
//          index 7 = [direction]
//          index 8 = [rhythmTick.getPeriod()]
//

void saveState(int aState) {
   if (aState < 0 || aState >= STATE_COUNT) {
      return;   // invalid state
   }

   state[aState][0] = 1;   // state is a valid state which can be restored
   state[aState][1] = sustain;
   state[aState][2] = interval;
   state[aState][3] = method;
   state[aState][4] = curpat;
   state[aState][5] = volControl;
   state[aState][6] = instrument;
   state[aState][7] = direction;
   state[aState][8] = (int)rhythmTick.getPeriod();
}



//////////////////////////////
//
// restoreState -- restores a performance state that was previously stored
//     with the saveState function.
//

void restoreState(int aState) {
   if (aState < 0 || aState >= STATE_COUNT) {
      return;   // invalid state
   }

   if (state[aState][0] == 0) {
      return;   // also an invalid state
   }

   sustain = state[aState][1];
   voice.sustain(sustain);
   interval = state[aState][2];
   method = state[aState][3];
   curpat = state[aState][4];
   volControl = state[aState][5];
   instrument = state[aState][6];
   voice.pc(instrument);
   direction = state[aState][7];
   rhythmTick.setPeriod(state[aState][8]);
}




//////////////////////////////
//
// printState -- prints the values for a particular stored state
//

void printState(int aState) {
   if (aState < 0 || aState >= STATE_COUNT) {
      cout << "invalid state: " << aState << endl;
      return;
   }

   if (state[aState][0] == 0) {
      cout << "Empty state" << endl;
      return;
   }

  
   cout << "\nsus = " << state[aState][1] 
        << "\tint = " << state[aState][2] 
        << "\tmet = " << state[aState][3] 
        << "\trhy = " << state[aState][4] 
        << "\tvol = " << state[aState][5] 
        << "\tins = " << state[aState][6] 
        << "\tdir = " << state[aState][7] 
        << "\ttem = " << state[aState][8]
        << endl;
}



/*------------------ end improvization algorithms -----------------------*/


// md5sum: 8a1f8d223af3dd82b107fd0969fd5b0a position2.cpp [20050403]

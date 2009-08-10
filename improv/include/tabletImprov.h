//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu> 
// Creation Date: Tue Aug  5 21:34:26 PDT 2003
// Last Modified: Tue Aug  5 21:34:33 PDT 2003
// Filename:      ...sig/code/control/improv/tabletImprov.h
// Web Address:   http://sig.sapp.org/include/sig/tabletImprov.h
// Syntax:        C++
//
// Description:   The helper and main functions for tablet improv programs.
//

#ifndef _IMPROV_INTERFACE_INCLUDED
#define _IMPROV_INTERFACE_INCLUDED


// set up default locations for configuration files:
#ifndef VISUAL
   #define MIDI_DEF_FILE1 "midi.def"
   #define MIDI_DEF_FILE2 "../midi.def"
#else
   #define MIDI_DEF_FILE1 "c:\\improv\\data\\midi.def"
   #define MIDI_DEF_FILE2 ".\\midi.def"
#endif


#include "improv.h"
#include "Tablet.h"

#include <stdlib.h>
#include <string.h>

#ifndef OLDCPP
   #include <iostream>
   #include <iomanip>
   #include <fstream>
#else
   #include <iostream.h>
   #include <iomanip.h>
   #include <fstream.h>
#endif

// function declarations
int    checkKeyboard(void);
int    chooseSynthOutputPort(void);
void   echoKeysOff(void);
void   echoKeysOn(void);
void   finishup_automatic(void);
double getIdleEventPeriod(void);
void   initialization_automatic(void);
void   print_commands(void);
void   print_aux_commands(void);
int    readmidiconfig(void);
int    runImprovInterface(void);
void   setIdleEventPeriod(double aPeriod);
void   usage(const char* command);

void charsynth(int aKey);
void control_change(int channel, int controller, int value);
void note_off(int channel, int keynum);
void note_on(int channel, int keynum, int velocity);
void patch_change(int channel, int instrument);
void program_change(int channel, int instrument);


// user defined functions
void description(void);
void keyboardchar(int key);
void initialization(void);
void mainloopalgorithms(void); 
void finishup(void);

void pen1trigon(void);
void pen1trigoff(void);
void pen1button1on(void);
void pen1button1off(void);
void pen1button2on(void);
void pen1button2off(void);

void pen2trigon(void);
void pen2trigoff(void);
void pen2button1on(void);
void pen2button1off(void);
void pen2button2on(void);
void pen2button2off(void);



////////////////////
// global variables

Tablet      tablet;         // input from Wacom Intuos2 tablet
MidiOutput  synth;          // Midi Output for a synthesizer
long        t_time = 0;     // global time (in milliseconds) which remains
                            // fixed for one pass through mainloopalgorithms()

// other global variables the user might like to use but are not 
// part of the official tabletImprov environment:

SigTimer      mainTimer;         // Timer counting in milliseconds
KeyboardInput interfaceKeyboard; // for computer keyboard interface
Idler         eventIdler(1.0);   // to control CPU usage for multiprocessing
Options       options;           // for handling command-line options

// global variables which the users shouldn't be messing with:
char        recordFilename[1000] = {0};  // file to record tablet state

///////////////////////////////////////////////////////////////////////////


#ifndef SUPPRESS_MAIN_FUNCTION
   int main(int argc, char** argv) {
      options.setOptions(argc, argv);
      return runImprovInterface();
   }
#endif


int runImprovInterface(void) {
   int command;          // a keyboard single character command
   SigTimer displayTimer;// control the position/buffer display update period
   displayTimer.setPeriod(200);  // displaying buf data or position every 200 ms
   SigTimer keyboardTimer;       // for controlling the keyboard checking rate
   keyboardTimer.setPeriod(1);   // check the keyboard for new keys every 1 ms

   initialization_automatic();
   print_commands();   
   initialization();                     // user defined behavior

   while (1) {
      tablet.processIncomingMessages();
      t_time = mainTimer.getTime(); 

      mainloopalgorithms();               // user defined behavior

      if (keyboardTimer.expired()) {
         keyboardTimer.reset();
         command = checkKeyboard();
         switch (command) {
            case 'Q': 
               goto endmainwhile;
               break;
            default: ;
               // nothing
         } // end switch (command)

      } // end of if keyboardTimer.expired
 
      #ifndef VISUAL
         eventIdler.sleep();
      #endif

   } // end of while (1);

endmainwhile:
   finishup();                            // user defined behavior
   finishup_automatic();

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// checkKeyboard -- check to see if a key has been pressed on the keyboard.
//    If so, then interpret the keypress as a command.
//    Commands are listed in ASCII order.  Do not use the
//    characters: a-z, 0-9, or: ,./;'[]\=-`, control characters
//    or alt characters or the ESC key since these keys are
//    guarenteed to be available to the user for their own
//    use.
//

int checkKeyboard(void) {
   static KeyboardInput keyboard;

   if (!keyboard.hit()) return -1;

   char command;
   command = keyboard.getch();

   switch (command) {
      case 0x1b:                        // escape key
         command = 'Q';
         break;
   // case ' ': break;
   // case '!': break;
   // case '\"': break;
   // case '#': break;
   // case '$': break;
   // case '%': break;
   // case '&': break;
   // case '*': break;
   // case '(': break;
   // case '*': break;
   // case '\'': break;
   // case '-': break;
   // case '.': break;
   // case '/': break;
   // case '0': break;
   // case '1': break;
   // case '2': break;
   // case '3': break;
   // case '3': break;
   // case '4': break;
   // case '5': break;
   // case '6': break;
   // case '7': break;
   // case '8': break;
   // case '9': break;
   // case ':': break;
   // case ';': break;
   // case '<': break;
   // case '=': break;
   // case '>': break;
      case '?':                         // print main command menu
         print_commands();
         break;
   // case '@': break;
   // case 'A': break;
   // case 'B': break;
   // case 'C': break;
      case 'D':                         // print program description
         // the description function is defined by
         // the user.  Hopefully they used it!
         description();
         break;
   // case 'E': break;
      case 'F':                      // set filename for tablet state recording
         tablet.pause();
         cout << "Type a filename for recording tablet state variables: ";
         echoKeysOn();
         cin  >> recordFilename;
         echoKeysOff();
         tablet.unpause();
         break;
      case 'G':                         // change CPU speed variable
         {
         int aSpeed;
         cout << "Current speed is listed as: " 
              << mainTimer.getCpuSpeed()/1000000.0 << " MHz.\n";
         cout << "Enter the CPU speed of the computer in MHz: ";
         echoKeysOn();
         cin  >> aSpeed;
         echoKeysOff();
         mainTimer.setCpuSpeed(aSpeed * 1000000);
         }
         break;
   // case 'H': break;
   // case 'I': break;
   // case 'J': break;
      case 'K':                         // toggle record of tablet state
         if (tablet.recordingQ()) {
            cout << "Ending recording of tablet data." << endl;
            tablet.recordStateStop();
         } else {
            if (recordFilename[0] == '\0') {
               cout << "Start of tablet data recording to file: " 
                    << "drum.rec" << endl;
               tablet.recordStateStart("tablet.rec");
            } else {
               cout << "Start of tablet data recording to file: " 
                    << recordFilename << endl;
               tablet.recordStateStart(recordFilename);
            }
         }
         break;
   // case 'L': break;
      case 'M':                         // display more commands
         // this command displays less important commands
         print_aux_commands();
         break;
   // case 'N': break;
   // case 'O': break;
   // case 'P': break;
      case 'Q':                         // quit the program
         // don't do anything here, the calling function (main)
         // will handle this command.
         break;
   // case 'R': break;
      case 'S':                         // send note-offs to the synth
         cout << "Silencing synthesizer ...";
         synth.silence();
         cout << " done." << endl;
         break;
	  case 'T':                         // set MIDI out device for synth
         cout << "synthesizer MIDI output is currently set to device: "
              << synth.getPort() << endl;
         synth.setPort(chooseSynthOutputPort());
         synth.open();
         cout << "synthesizer MIDI output is now set to: "
              << synth.getPort() << endl;
         break;
   // case 'U': break;
   // case 'V': break;
   // case 'W': break;
      case 'X':                         // toggle MIDI output trace
         synth.toggleTrace();
         break;
   // case 'Y': break;
   // case 'Z': break;
   // case '[': break;
   // case '\': break;
   // case ']': break;
   // case '^': break;
   // case '_': break;
   // case '`': break;
   // case 'a': break;
   // case 'b': break;
   // case 'c': break;
   // case 'd': break;
   // case 'e': break;
   // case 'f': break;
   // case 'g': break;
   // case 'h': break;
   // case 'i': break;
   // case 'j': break;
   // case 'k': break;
   // case 'l': break;
   // case 'm': break;
   // case 'n': break;
   // case 'o': break;
   // case 'p': break;
   // case 'q': break;
   // case 's': break;
   // case 't': break;
   // case 'u': break;
   // case 'v': break;
   // case 'w': break;
   // case 'x': break;
   // case 'y': break;
   // case 'z': break;
   // case '{': break;
   // case '|': break;
   // case '}': break;
   // case '~': break;
      default:                          // unknown command
         // can't identify command, so it must be a user command.
         // give the character to the user command:
         keyboardchar(command);
   } // end of switch (command)

   return command;
}



//////////////////////////////
//
// chooseSynthOutputPort -- have the user select the 
//    MIDI output device to communicate with the synthesizer.
//

int chooseSynthOutputPort(void) {
   tablet.pause();
   int portCount = synth.getNumPorts();

   if (portCount <= 1) {
      tablet.unpause();
      return 0;
   }

   int choice = -100;

   cout << "Choose one of the following MIDI output ports "
        << "for synthesizer communication:" << endl;
   displayOutputPorts();

   while (choice < -1 || choice >= portCount) {
      cout << "Type port number choice here: ";
      echoKeysOn();
      cin  >> choice;
      echoKeysOff();
   }

   tablet.unpause();
   return choice;
}



//////////////////////////////
//
// echoKeysOff -- for compatibility with the Linux terminal.
//	calling this function will turn off local echo of keys
//	which might be turned on when getting user input.
//	Call this function just after getting the user to type in
//	some input from the keyboard which you want displayed on the
//	screen.
//

void echoKeysOff(void) {
   interfaceKeyboard.newset();
}



//////////////////////////////
//
// echoKeysOn -- for compatibility with the Linux terminal.
//	calling this function will turn on local echo of keys
//	which might be turned off when the program is running.
//	Call this function just before getting the user to type in
//	some input from the keyboard which you want displayed on the
//	screen.
//

void echoKeysOn(void) {
   interfaceKeyboard.oldset();
}



//////////////////////////////
//
// finishup_automatic -- place automatic configuration 
//    commands here.
//

void finishup_automatic(void) {
   cout << endl;
}

     

//////////////////////////////
//
// getIdleEventPeriod -- sleep time between each interation of the 
// 	main loop for use with Multi-processing systems.
//

double getIdleEventPeriod(void) {
   return eventIdler.getPeriod();
}



//////////////////////////////
//
// initialization_automatic -- place automatic initialization
//    commands here.
//

void initialization_automatic(void) {
   // define the defautl command-line options
   options.define("author=b");       // display author of interface
   options.define("version=b");      // display version of interface
   options.define("help=b");         // display usage synopsis
   options.define("ports=b");        // display MIDI I/O ports
   options.define("description=b");  // display the description message
   options.process(1);               // process options but don't
 				     // complain about undefined options (0)

   if (options.getBoolean("author")) {
      cout << "tabletImprov interface written by Craig Stuart Sapp,\n"
              "(craig@ccrma.stanford.edu), August 2003" << endl;
      cout << "The author of this particular implementation of tabletImprov\n"
              "may be found by using the --description option" << endl;
      exit(0);
   } else if (options.getBoolean("version")) {
      cout << "tabletImprov, version 1.0 (Tue Aug  5 21:45:17 PDT 2003)\n"
              "compiled: " << __DATE__ << endl;
      exit(0);
   } else if (options.getBoolean("help")) {
      usage(options.getCommand());
      exit(0);
   } else if (options.getBoolean("description")) {
      description();
      exit(0);
   } else if (options.getBoolean("ports")) {
      cout << endl;
      cout << "Number of Output Ports: " << synth.getNumPorts() << endl;
      displayOutputPorts();
      cout << endl;
      exit(0);
   }

   // choose the MIDI in and out ports
   if (readmidiconfig() == 0) {   
      tablet.pause();
      synth.setPort(chooseSynthOutputPort());
   }

   // open all communication ports:
   tablet.open();
   synth.open();

   // assign the behavior functions to the radio drum object:
   tablet.pen1trigon     = pen1trigon;
   tablet.pen1trigoff    = pen1trigoff;
   tablet.pen1button1on  = pen1button1on;
   tablet.pen1button1off = pen1button1off;
   tablet.pen1button2on  = pen1button2on;
   tablet.pen1button2off = pen1button2off;

   tablet.pen2trigon     = pen2trigon;
   tablet.pen2trigoff    = pen2trigoff;
   tablet.pen2button1on  = pen2button1on;
   tablet.pen2button1off = pen2button1off;
   tablet.pen2button2on  = pen2button2on;
   tablet.pen2button2off = pen2button2off;

   t_time = mainTimer.getTime();

   // set the idling rate for the event loop to 1 millisecond
   eventIdler.setSoftSleep(1.0);
}



//////////////////////////////
//
// print_commands -- print keyboard character commands
//

void print_commands(void) {
   cout << endl;
   printboxtop();
   pcl("UPPERCASE COMMANDS :: tabletImprov Interface");
   printintermediateline();
   psl("   D = print a description of this program given by the programmer");
   psl("   ? = print this command list         M = print more commands");
   psl("   P = position display toggle         S = silence synthesizer");
   psl("   Q = quit the program (or Esc)");

   printboxbottom();
}



void print_aux_commands(void) {
   cout << endl;
   printboxtop();
   pcl("Tablet Improv Interface version 5 August 2003 by");
   pcl("Craig Stuart Sapp  <craig@ccrma.stanford.edu>");
   printintermediateline();
   psl("   X = toggle MIDI out trace         Y = toggle MIDI in trace");
   psl("   G = chanGe CPU speed setting      T = set MIDI out port for synth");
   psl("   F = set tablet state recording filename");
   printboxbottom();
}



//////////////////////////////
//
// readmidiconfig -- returns true if it found the midiconfig file
//    which stores the input and output device numbers of the
//    computer for the programs.  
//
// The file format for the midi.def file is:
//   # comment line
//   synthin: 1
//   synthout: 0
//
// Labels are case sensitive, and can come in any order. any amount
// of space before label, and after label.  No space between text and
// colon.  Comment lines start with a '#' character with optional
// space preceding it.
//

int readmidiconfig(void) {
   int synthout;
   fstream infile1, infile2, infile;

   #ifndef OLDCPP
      infile1.open(MIDI_DEF_FILE1);
      infile2.open(MIDI_DEF_FILE2);
   #else
     infile1.open(MIDI_DEF_FILE1, ios::nocreate);
     infile2.open(MIDI_DEF_FILE2, ios::nocreate);
   #endif

   // had to do it this way for some buggy reason:
   if (infile1.is_open()) {
      cout << "Midi configuration read from \"" << MIDI_DEF_FILE1 << "\""
           << endl;
      infile1.close();
      #ifndef OLDCPP
         infile.open(MIDI_DEF_FILE1, ios::in);;
      #else 
         infile.open(MIDI_DEF_FILE1, ios::in | ios::nocreate);
      #endif
   } else if (infile2.is_open()) {
      cout << "Midi configuration read from \"" << MIDI_DEF_FILE2 << "\"" 
           << endl;
      infile2.close();
      #ifndef OLDCPP    
         infile.open(MIDI_DEF_FILE2, ios::in);
      #else
         infile.open(MIDI_DEF_FILE2, ios::in | ios::nocreate);
      #endif
   } else {
      return 0;
   }

   if (!infile.is_open()) {
      return 0;
   } 

   synthout = -1;

   char buffer[1024] = {0};
   char* token;
   infile.getline(buffer, 512, '\n');
   while (!infile.eof()) {
      token = strtok(buffer, " \t");
      if (token == NULL) {
         infile.getline(buffer, 512, '\n');
         continue;
      }

      if (strcmp(token, "synthout:") == 0) {
         token = strtok(NULL, " \t");
         if (token == NULL) {
            cout << "Warning: error in midi.def at synthout:" << endl;
         } else {
            sscanf(token, "%d", &synthout);
         }
      } 
      infile.getline(buffer, 512, '\n');
   }
   if (synthout < -1) {
      synthout = chooseSynthOutputPort();
   }

   synth.setPort(synthout);
   synth.open();

   cout << "Synthesizer output set to device " << synthout << " = "
      << synth.getName() << endl;

   return 1;
}



//////////////////////////////
//
// setIdleEventPeriod -- sleep time betwen each interation of the main loop
//

void setIdleEventPeriod(double aPeriod) {
   eventIdler.setPeriod(aPeriod);
}



//////////////////////////////
//
// usage -- how to run this program from the command-line
//

void usage(const char* command) {
   cout <<
   "\n"
   "tabletImprov 1.0 interface\n"
   "\n"
   "Usage: " << command << " [options]\n"
   "\n"
   "Options:\n"
   "   --author       = author of this tabletImprov interface\n"
   "   --version      = version of this tabletImprov interface\n"
   "   --description  = implementation author's description of the program\n"
   "   --help         = display this message\n"
   "   --ports        = display MIDI output ports and then exit\n"
   "   --options      = display all options, default values, and aliases\n"
   "\n"
   << endl;
}


///////////////////////////////////////////////////////////////////////////
//
// Convienence functions added for optional use by the user programmer:
//


//////////////////////////////
//
// charsynth -- plays the synthesizer with lowercase characters
//    on the computer's keyboard (bottom two rows).
// z=C4, x=D4, c=E4, v=F4, b=G4, n=A4, m=B4, ","=C5, etc.
//

void charsynth(int aKey) {
   static MidiMessage noteMessage;
   static int attack = 0;
   static int note = 0;

   synth.play(0, note, 0);

//   noteMessage.time = mainTimer.getTime();
//   noteMessage.command() = 0x90;
//   noteMessage.p1() = note;
//   noteMessage.p2() = 0;
//   synth.insert(noteMessage);

   switch (aKey) {
      case 'z':  note=C4;    break;
      case 'x':  note=D4;    break;
      case 'c':  note=E4;    break;
      case 'v':  note=F4;    break;
      case 'b':  note=G4;    break;
      case 'n':  note=A4;    break;
      case 'm':  note=B4;    break;
      case ',':  note=C5;    break;
      case '.':  note=D5;    break;
      case '/':  note=E5;    break;
      case 'a':  note=B3;    break;
      case 's':  note=Cs4;   break;
      case 'd':  note=Eb4;   break;
      case 'f':  note=0;     break;
      case 'g':  note=Fs4;   break;
      case 'h':  note=Gs4;   break;
      case 'j':  note=Bb4;   break;
      case 'k':  note=0;     break;
      case 'l':  note=Cs5;   break;
      case ';':  note=Eb5;   break;
      case '\'': note=Fs5;   break;
      default: return;
   }

   if (note < 0)  note = 0;
   else if (note > 127)  note = 127;

   attack = rand()%47 + 81;           // random int from 81 to 127
   synth.play(0,note,attack); 

//   noteMessage.time = mainTimer.getTime();
//   noteMessage.command() = 0x90;
//   noteMessage.p1() = note;
//   noteMessage.p2() = rand()%47 + 81;      // random int from 1 to 127
//   synth.insert(noteMessage);

}


#endif  /* _IMPROV_INTERFACE_INCLUDED */



// md5sum: 58f0a3e25c04989010eca3a1b73524c4 tabletImprov.h [20050403]

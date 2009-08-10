//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu> 
// Creation Date: Sun Jul 16 19:22:17 PDT 2000
// Last Modified: Sun Jul 16 19:22:23 PDT 2000
// Filename:      ...sig/code/control/improv/stickImprov.h
// Web Address:   http://sig.sapp.org/include/sig/stickImprov.h
// Syntax:        C++
//
// Description:   The helper and main functions for Adams Stick improv programs.
//

#ifndef _IMPROV_INTERFACE_INCLUDED
#define _IMPROV_INTERFACE_INCLUDED

#include "improv.h"
#include <stdlib.h>
#include <string.h>

#ifndef OLDCPP
   #include <iostream>
   #include <iomanip>
   #include <fstream>
   using namespace std;
#else
   #include <iostream.h>
   #include <iomanip.h>
   #include <fstream.h>
#endif

// set up default locations for configuration files:
#ifndef VISUAL
   #define MIDI_DEF_FILE1 "midi.def"
   #define MIDI_DEF_FILE2 "../midi.def"
#else
   #define MIDI_DEF_FILE1 "c:\\improv\\data\\midi.def"
   #define MIDI_DEF_FILE2 ".\\midi.def"
#endif

// function declarations
int    checkKeyboard(void);
int    chooseStickInputPort(void);
int    chooseStickOutputPort(void);
int    chooseSynthOutputPort(void);
void   echoKeysOff(void);
void   echoKeysOn(void);
void   finishup_automatic(void);
double getIdleEventPeriod(void);
void   displayPositions(void);
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
void stickresponse(void);
void fsr1ontrig(void);
void fsr1offtrig(void);
void fsr2ontrig(void);
void fsr2offtrig(void);
void fsr3ontrig(void);
void fsr3offtrig(void);


////////////////////
// global variables

AdamsStick  stick;          // MIDI I/O for the stick
MidiOutput  synth;          // Midi Output for a synthesizer
long        t_time = 0;     // global time (in milliseconds) which remains
                            // fixed for one pass through mainloopalgorithms()

// other global variables the user might like to use but are not 
// part of the official stickImprov environment:

SigTimer      mainTimer;         // Timer counting in milliseconds
KeyboardInput interfaceKeyboard; // for computer keyboard interface
Idler         eventIdler(1.0);   // to control CPU usage for multiprocessing
Options       options;           // for handling command-line options

///////////////////////////////////////////////////////////////////////////


#ifndef SUPPRESS_MAIN_FUNCTION
   int main(int argc, char** argv) {
      options.setOptions(argc, argv);
      return runImprovInterface();
   }
#endif


int runImprovInterface(void) {
   int report = 0;       // boolean to display position information
   int command;          // a keyboard single character command
   SigTimer displayTimer;// control the position/buffer display update period
   displayTimer.setPeriod(200);  // displaying buf data or position every 200 ms
   SigTimer keyboardTimer;       // for controlling the keyboard checking rate
   keyboardTimer.setPeriod(1);   // check the keyboard for new keys every 1 ms

   initialization_automatic();
   print_commands();   
   initialization();                     // user defined behavior

   while (1) {
      stick.processIncomingMessages();
      t_time = mainTimer.getTime(); 

      mainloopalgorithms();               // user defined behavior

      if (keyboardTimer.expired()) {
         keyboardTimer.reset();
         command = checkKeyboard();
         switch (command) {
            case 'Q': 
               goto endmainwhile;
            case 'P':                   // toggle display of positions
               report = !report;
               if (report) {
                  displayTimer.reset();
               } else {
                  cout << endl;
               }
               break;
            default: ;
               // nothing
         } // end switch (command)

         if (report && displayTimer.expired()) {
            displayTimer.reset();
            displayPositions();
         }

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
   // case ' ': break;
   // case '!': break;
   // case '\"': break;
   // case '#': break;
   // case '$': break;
   // case '%': break;
   // case '&': break;
   // case '(': break;
   // case ')': break;
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
      case 'A':                         // stick alive test
         cout << "Stick is ";
         cout.flush();
         if (stick.is_connected()) {
            cout << "alive." << endl;
         } else {
            cout << "dead." << endl;
         }
         break;
   // case 'B': break;
   // case 'C': break;
      case 'D':                         // print program description
         // the description function is defined by
         // the user.  Hopefully they used it!
         description();
         break;
   // case 'E': break;
   // case 'F': break;
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
      case 'I':                         // set input port for Radio Drum
         cout << "stick MIDI input is currently set to device: " 
              << stick.getInputPort() << endl;
         stick.pause();
         stick.setInputPort(chooseStickInputPort());
         stick.openInput();
         cout << "stick input is now set to: "
              << stick.getInputPort() << endl;
         break;
   // case 'J': break;
   // case 'K': break;
   // case 'L': break;
      case 'M':                         // display more commands
         // this command displays less important commands
         print_aux_commands();
         break;
   // case 'N': break;
      case 'O':                         // set MIDI out device for stick 
         cout << "stick MIDI output is currently set to device: "
              << stick.getOutputPort() << endl;
         stick.setOutputPort(chooseStickOutputPort());
         stick.openOutput();
         cout << "stick MIDI output is now set to: "
              << stick.getOutputPort() << endl;
         break;
      case 'P':                         // toggle stick and pot reporting
         // don't do anything here, the calling function (main)
         // will handle this command.
         break;
      case 'Q':                         // quit the program
         // don't do anything here, the calling function (main)
         // will handle this command.
         break;
      case 'R':                         // toggle position reporting
         stick.toggleMode();
         if (stick.getMode() == STICK_POLL_MODE) {
            cout << "Stick set to polling mode" << endl;
         } else if (stick.getMode() == STICK_STREAM_MODE) {
            cout << "Stick set to streaming mode" << endl;
         } else {
            cout << "Stick set to an unknown mode" << endl;
         }
         break;
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
         stick.toggleOutputTrace();
         if (stick.getOutputTrace()) {
            cout << "MIDI output trace is ON." << endl;
         } else {
            cout << "MIDI output trace is OFF." << endl;
         }
         synth.setTrace(stick.getOutputTrace());
         break;
      case 'Y':                         // toggle MIDI input trace
         stick.toggleInputTrace();
         if (stick.getInputTrace()) {
            cout << "MIDI input trace is ON." << endl;
         } else {
            cout << "MIDI input trace is OFF." << endl;
         }
         break;
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
// chooseStickInputPort -- have the user select the 
//    MIDI input device to communicate with the stick.
//

int chooseStickInputPort(void) {
   stick.pause();
   int portCount = stick.getNumInputPorts();

   if (portCount <= 1) {
      stick.unpause();
      return 0;
   }

   int choice = -100;

   cout << "Choose one of the following MIDI input ports "
        << "for Stick communication:" << endl;
   displayInputPorts();

   while (choice < 0 || choice >= portCount) {
      cout << "Type port number choice here: ";
      echoKeysOn();
      cin  >> choice;
      echoKeysOff();
   }

   stick.unpause();
   return choice;
}



//////////////////////////////
//
// chooseStickOutputPort -- have the user select the 
//    MIDI output device to communicate with the stick.
//

int chooseStickOutputPort(void) {
   stick.pause();
   int portCount = stick.getNumOutputPorts();

   if (portCount <= 1) {
      stick.unpause();
      return 0;
   }

   int choice = -100;

   cout << "Choose one of the following MIDI output ports "
        << "for Stick communication:" << endl;
   displayOutputPorts();

   while (choice < -1 || choice >= portCount) {
      cout << "Type port number choice here: ";
      echoKeysOn();
      cin  >> choice;
      echoKeysOff();
   }

   stick.unpause();
   return choice;
}



//////////////////////////////
//
// chooseSynthOutputPort -- have the user select the 
//    MIDI output device to communicate with the synthesizer.
//

int chooseSynthOutputPort(void) {
   stick.pause();
   int portCount = stick.getNumOutputPorts();

   if (portCount <= 1) {
      stick.unpause();
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

   stick.unpause();
   return choice;
}



//////////////////////////////
//
// displayPositions -- display the current positions of the
//   sticks, dials, etc.
//
 
void displayPositions(void) {
   cout << "\r";
   cout << "FSR1: pos=" 
        << setw(4) << (int)stick.s1p7() << " for="
        << setw(4) << (int)stick.s1f7() << ";   "
        << "FSR2: pos="
        << setw(4) << (int)stick.s2p7() << ", for="
        << setw(4) << (int)stick.s2f7() << ";   "
        << "FSR3: pos="
        << setw(4) << (int)stick.s3p7() << ", for="
        << setw(4) << (int)stick.s3f7();
   cout.flush();
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
   stick.setStreamMode();
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
   // define the default command-line options
   options.define("author=b");       // display author of interface
   options.define("version=b");      // display version of interface
   options.define("help=b");         // display usage synopsis
   options.define("ports=b");        // display MIDI I/O ports
   options.define("description=b");  // display the description message
   options.process(1);               // process options but don't
 				     // complain about undefined options (0)

   if (options.getBoolean("author")) {
      cout << "stickImprov interface written by Craig Stuart Sapp,\n"
              "(craig@ccrma.stanford.edu), July 2000" << endl;
      cout << "The author of this particular implementation of stickImprov\n"
              "may be found by using the --description option" << endl;
      exit(0);
   } else if (options.getBoolean("version")) {
      cout << "stickImprov, version 2.3 (Sun Jul 16 19:42:39 PDT 2000)\n"
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
      cout << "Number of Input Ports: " << stick.getNumInputPorts() << endl;
      displayInputPorts();
      cout << endl;
      cout << "Number of Output Ports: " << stick.getNumOutputPorts() << endl;
      displayOutputPorts();
      cout << endl;
      exit(0);
   }

   // choose the MIDI in and out ports
   if (readmidiconfig() == 0) {   
      stick.pause();
      stick.setInputPort(chooseStickInputPort());
      stick.setOutputPort(chooseStickOutputPort());
      synth.setPort(chooseSynthOutputPort());
   }

   // open all MIDI communication ports:
   stick.openInput();
   stick.openOutput();
   synth.open();

   // assign the behavior functions to the radio drum object:
   stick.response         = stickresponse;
   stick.fsr1ontrig       = fsr1ontrig;
   stick.fsr1offtrig      = fsr1offtrig;
   stick.fsr2ontrig       = fsr2ontrig;
   stick.fsr2offtrig      = fsr2offtrig;
   stick.fsr3ontrig       = fsr3ontrig;
   stick.fsr3offtrig      = fsr3offtrig;

   t_time = mainTimer.getTime();

   // set the idling rate for the event loop to 1 millisecond.
   // note that in Unix the minimum sleep period is 10 milliseconds.
   eventIdler.setSoftSleep(1.0);

   // determine if the stick is connected.  If so, set the 
   // data mode to streaming:
   if (stick.is_connected()) {
      stick.setStreamMode();
   }

}



//////////////////////////////
//
// print_commands -- print keyboard character commands
//

void print_commands(void) {
   cout << endl;
   printboxtop();
   pcl("UPPERCASE COMMANDS :: stickImprov Interface");
   printintermediateline();
   psl("   D = print a description of this program given by the programmer");
   psl("   ? = print this command list         M = print more commands");
   psl("   Q = quit the program                A = stick alive test");
   psl("   P = position display toggle         S = silence synthesizer");
   printboxbottom();
}



void print_aux_commands(void) {
   cout << endl;
   printboxtop();
   pcl("Stick Improv Interface version 16 July 2000 by");
   pcl("Craig Stuart Sapp  <craig@ccrma.stanford.edu>");
   printintermediateline();
   psl("   I = set MIDI in port for stick     G = chanGe CPU speed setting  ");
   psl("   O = set MIDI out port for stick    T = set MIDI out port for synth");
   psl("   X = toggle MIDI out trace          Y = toggle MIDI in trace");
   psl("   R = toggle reporting mode");
   printboxbottom();
}



//////////////////////////////
//
// readmidiconfig -- returns true if it found the midiconfig file
//    which stores the input and output device numbers of the
//    computer for the programs.  first number in file
//    is the stick input device, second number in the file is
//    the stick output device, and the third number in the file
//    is the synth output device. 
//
// Method changed 17 July 1999: The file format for the midi.def
// file is now like this:
//   # comment line
//   stickin: 0
//   stickout: 0
//   synthout: 1
// Labels are case sensitive, and can come in any order. any amount
// of space before label, and after label.  No space between text and
// colon.  Comment lines start with a '#' character with optional
// space preceding it.
//

int readmidiconfig(void) {
   int stickin, stickout, synthout;
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
         infile.open(MIDI_DEF_FILE1, ios::in);
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

   stickin = -1;
   stickout = -1;
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

      if (strcmp(token, "stickin:") == 0) {
         token = strtok(NULL, " \t");
         if (token == NULL) {
            cout << "Warning: error in midi.def at stickin:" << endl;
            stickin = 0;
         } else {
            sscanf(token, "%d", &stickin);
         }
      } else if (strcmp(token, "stickout:") == 0) {
         token = strtok(NULL, " \t");
         if (token == NULL) {
            cout << "Warning: error in midi.def at stickout:" << endl;
            stickin = 0;
         } else {
            sscanf(token, "%d", &stickout);
         }
      } else if (strcmp(token, "synthout:") == 0) {
         token = strtok(NULL, " \t");
         if (token == NULL) {
            cout << "Warning: error in midi.def at synthout:" << endl;
            stickin = 0;
         } else {
            sscanf(token, "%d", &synthout);
         }
      } 
      infile.getline(buffer, 512, '\n');
   }
   if (stickin < 0) {
      stickin = chooseStickInputPort();
   }
   if (stickout < 0) {
      stickout = chooseStickOutputPort();
   }
   if (synthout < 0) {
      stickout = chooseSynthOutputPort();
   }

   stick.pause();
   stick.setInputPort(stickin);
   stick.openInput();
   stick.unpause();

   stick.setOutputPort(stickout);
   stick.openOutput();

   synth.setPort(synthout);
   synth.open();

   cout << "Stick MIDI input set to device " << stickin << " = "
      << stick.MidiInput::getName() << endl;
   cout << "Stick MIDI output set to device " << stickout << " = "
      << stick.MidiOutput::getName() << endl;
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
   "stickImprov 2.3 interface\n"
   "\n"
   "Usage: " << command << " [options]\n"
   "\n"
   "Options:\n"
   "   --author       = author of this stickImprov interface\n"
   "   --version      = version of this synthImprov interface\n"
   "   --description  = implementation author's description of the program\n"
   "   --help         = display this message\n"
   "   --ports        = display MIDI input/output ports and then exit\n"
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




///////////////////////////////////////////////////////////////////////////
//
// C language MIDI interface functions
//

// The following define will convert a synthImprov program
// into a stickImprov program with empty stick functions.
// note that the synth variable in stickImprov programs does not
// have MIDI input capabilities, so if the synthImprov program
// uses MIDI input, then it cannot be made into a stickImprov program.
#define CONVERT_SYNTH_TO_STICK_IMPROV \
     void stickresponse(void) { } \
     void fsr1ontrig(void) { } \
     void fsr1offtrig(void) { } \
     void fsr2ontrig(void) { } \
     void fsr2offtrig(void) { } \
     void fsr3ontrig(void) { } \
     void fsr3offtrig(void) { }



//////////////////////////////
//
// control_change -- for compatibility with C version of stickImprov.
//	note channel offset from 1 rather than 0.
//

void control_change(int channel, int controller, int value) {
   synth.cont(channel - 1, controller, value);
}



//////////////////////////////
//
// note_off -- for compatibility with C version of stickImprov.
// 	note channel offset from 1 rather than 0.
//

void note_off(int channel, int keynum) {
   synth.play(channel - 1, keynum, 0);
}



//////////////////////////////
//
// note_on -- for compatibility with C version of stickImprov.
// 	note channel offset from 1 rather than 0.
//

void note_on(int channel, int keynum, int velocity) {
   synth.play(channel - 1, keynum, velocity);
}



//////////////////////////////
//
// patch_change -- for compatibility with C version of stickImprov.
//	note channel offset from 1 rather than 0.
//

void patch_change(int channel, int instrument) {
   synth.pc(channel - 1, instrument);
}



//////////////////////////////
//
// program_change -- for compatibility with C version of stickImprov.
//	note channel offset from 1 rather than 0.
//

void program_change(int channel, int instrument) {
   synth.pc(channel - 1, instrument);
}




#endif  /* _IMPROV_INTERFACE_INCLUDED */



// md5sum: 1cdda1fae7d20a0fb913d06a1251f9a5 stickImprov.h [20050403]

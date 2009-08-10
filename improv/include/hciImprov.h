//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Feb 11 23:19:44 GMT-0800 1998
// Last Modified: 14 Oct 1998
// Last Modified: Sat Sep 23 11:43:30 PDT 2000 (converted from synthImprov.h)
// Filename:      ...sig/code/control/improv/hciImprov.h
// Web Address:   http://improv.sapp.org/include/hciImprov.h
// Syntax:        C++
//
// Description:   Environment for MIDI sensor input I/O programming.
//

#ifndef _IMPROV_INTERFACE_INCLUDED
#define _IMPROV_INTERFACE_INCLUDED

#include "improv.h"

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

// function declarations
int    checkKeyboard(void);
int    chooseMidiInputPort(void);
int    chooseMidiOutputPort(void);
void   echoKeysOn(void);
void   echoKeysOff(void);
void   finishup_automatic(void);
double getIdleEventRate(void);
void   initialization_automatic(void);
void   print_commands(void);
void   print_aux_commands(void);
int    runImprovInterface(void);
void   setIdleEventRate(float aRate);
void   usage(const char* command);


// convienence functions for the improv programmer:
void  charsynth(int aKey);
void  octavekeyboard(int key);


// user supplied functions:
void initialization(void);
void description(void);
void keyboardchar(int key);
void mainloopalgorithms(void);
void mididata(int intime, int command, int p1, int p2);
void finishup(void);


////////////////////
//
// global variables for the hciImprov environment:
//

MidiIO midi;             // raw MIDI I/O for a MIDI synthesizer, etc.
SigTimer    mainTimer;   // Timer counting in milliseconds
long        t_time = 0;  // global time (in milliseconds) which remains
                         // fixed for one pass through mainloopalgorithms()

// global variables not officially in the synthImprov environment:

Options options;                 // for handling command-line options
KeyboardInput interfaceKeyboard; // for computer keyboard interface
Idler eventIdler(1.0);           // to control CPU usage for multiprocessing



///////////////////////////////////////////////////////////////////////////


#ifndef SUPPRESS_MAIN_FUNCTION
   int main(int argc, char** argv) {
      options.setOptions(argc, argv);
      return runImprovInterface();
   }
#endif

int runImprovInterface(void) {
   SigTimer keyboardTimer;       // for controlling the keyboard checking rate
   keyboardTimer.setPeriod(1);   // check the keyboard for new keys every 1 ms
   int command = 0;              // a key from the keyboard

   initialization_automatic();
   if (!options.optionsArg()) {
      print_commands();
   }
   initialization();             // user defined behavior
   options.process();            // process options checking for errors
                                 // and enabling --options option

   int mcount;   // prevention of stuck loop
   int intime;   // incoming time of a MIDI message in milliseconds
   int p0;       // MIDI command byte
   int p1;       // first parameter byte
   int p2;       // second parameter byte
  
   MidiMessage message;
   while (1) {                        // event loop
      mcount = 0;
      while(midi.getCount() > 0 && mcount < 15) {
         mcount++;
         message  = midi.extract();
         intime   = message.time;
         p0       = message.p0();
         p1       = message.p1();
         p2       = message.p2();
         mididata(intime, p0, p1, p2);
      }
      t_time = mainTimer.getTime(); 

      mainloopalgorithms();           // user defined behavior

      if (keyboardTimer.expired()) {
         keyboardTimer.reset();
         command = checkKeyboard();
         if (command == 'Q') {
            break;
         } 
            
      }

      #ifndef VISUAL
         eventIdler.sleep();
      #endif

   } // end while(1)

   finishup();                        // user defined behavior
   finishup_automatic();

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// checkKeyboard -- check to see if a key has been pressed on the keyboard.
//    If so, then interpret the keypress as a command and return the
//    keyboard character when done.  If the character is not used by
//    the main environment, then give the character to the user's function:
//    keyboardchar().
//
//    Commands below are listed in ASCII order.  Should not use here the
//    characters: a-z, 0-9, or: ,./;'[]\=-`, control characters,
//    or alt characters or the ESC key since these keys are
//    guarenteed to be available to the user for their own use.
//

int checkKeyboard(void) {
   if (!interfaceKeyboard.hit()) return -1;

   char command;
   command = interfaceKeyboard.getch();

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
   // case 'A': break;
   // case 'B': break;
      case 'C':                         // change CPU speed variable
         {
         int aSpeed;
         cout << "Current speed is listed as: " 
              << (double)mainTimer.getCpuSpeed()/1000000.0 << " MHz.\n";
         cout << "Enter the CPU speed of the computer in MHz: ";
         echoKeysOn();
         cin  >> aSpeed;
         echoKeysOff();
         mainTimer.setCpuSpeed(aSpeed * 1000000);
         }
         break;
      case 'D':                         // print description
         description(); // user defined function
         break;
   // case 'E': break;
   // case 'F': break;
   // case 'G': break; 
   // case 'H': break;
      case 'I':                         // set MIDI input
         cout << "MIDI input is currently set to port: " 
              << midi.getInputPort() << endl;
         midi.setInputPort(chooseMidiInputPort());
         midi.openInput();
         cout << "MIDI input is now set to port: "
              << midi.getInputPort() << endl;
         break;
   // case 'J': break;
   // case 'K': break;
   // case 'L': break;
      case 'M':                         // display more commands
         // this command displays less important commands
         print_aux_commands();
         break;
   // case 'N': break;
      case 'O':                         // set MIDI out device
         cout << "MIDI output is currently set to port: "
              << midi.getOutputPort() << endl;
         midi.setOutputPort(chooseMidiOutputPort());
         midi.openOutput();
         cout << "MIDI output is now set to port: "
              << midi.getOutputPort() << endl;
         break;
   // case 'P': break;
      case 'Q':                         // quit the program
         // don't do anything here, the calling function (main)
         // will handle this command.
         break;
   // case 'R': break;
      case 'S':                         // send note-offs to the synth
         cout << "Silencing synthesizer ...";
         midi.silence();
         cout << " done." << endl;
         break;
   // case 'T': break;
   // case 'U': break;
   // case 'V': break;
   // case 'W': break;
      case 'X':                         // toggle MIDI output trace
         midi.toggleOutputTrace();
         if (midi.getOutputTrace()) {
            cout << "MIDI output trace is ON." << endl;
         } else {
            cout << "MIDI output trace is OFF." << endl;
         }
         break;
      case 'Y':                         // toggle MIDI input trace
         midi.toggleInputTrace();
         if (midi.getInputTrace()) {
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
         // give the character to the user's keyboard function:
         keyboardchar(command);
   } // end of switch (command)

   return command;
}
   


//////////////////////////////
//
// chooseMidiInputDevice -- have the user select the
//    MIDI input device to communicate with the synthesizer
//

int chooseMidiInputPort(void) {
   midi.pause();
   int portCount = midi.MidiInPort::getNumPorts();

   if (portCount <= 1) {
      midi.unpause();
      return 0;
   }

   int choice = -100;

   cout << "Choose one of the following input ports"
        << " for MIDI communication" << endl;
   displayInputPorts();

   while (choice < 0 || choice >= portCount) {
      cout << "Type port number choice here: ";
      echoKeysOn();
      cin  >> choice;
      echoKeysOff();
   }

   midi.unpause();
   return choice;
}



//////////////////////////////
//
// chooseMidiOutputPort -- have the user select the 
//    MIDI output device to communicate with the synthesizer.
//

int chooseMidiOutputPort(void) {
   midi.pause();
   int portCount = midi.MidiOutPort::getNumPorts();

   if (portCount <= 1) {
      midi.unpause();
      return 0;
   }

   int choice = -100;

   cout << "Choose one of the following output ports "
        << "for MIDI communication:" << endl;
   displayOutputPorts();

   while (choice < -1 || choice >= portCount) {
      cout << "Type port number choice here: ";
      echoKeysOn();
      cin  >> choice;
      echoKeysOff();
   }

   midi.unpause();
   return choice;
}



//////////////////////////////
//
// echoKeysOff -- for compatibility with the Linux terminal.
//   calling this function will turn off local echo of keys
//   which might be turned on when getting user input.
//   call this function just after getting the user to type in
//   some input from the keyboard which you want displayed on the
//   screen.  
//

void echoKeysOff(void) {
   interfaceKeyboard.newset();
}



//////////////////////////////
//
// echoKeysOn -- for compatibility with the Linux terminal.
//   calling this function will turn on local echo of keys
//   which is turned of while the program is running.
//   call this function just before getting the user to type in
//   some input from the keyboard which you want displayed on the
//   screen.  Call echoKeysOff() when finished.
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
// getIdleEventRate -- sleep time between each iteration of the main
//      loop for use with Multi-processing systems.
//

double getIdleEventRate(void) {
   return eventIdler.getPeriod();
}



//////////////////////////////
//
// initialization_automatic -- place automatic initialization
//    commands here.
//

void initialization_automatic(void) {

   // define the default command-line options
   options.define("author=b");          // display author of interface
   options.define("version=b");         // display version of interface
   options.define("help=b");            // display usage synopsis
   options.define("ports=b");           // display MIDI I/O ports
   options.define("description=b");     // display the description message
   options.process(0, 1);               // process options but don't
                                        // complain about undefined options
   if (options.getBoolean("author")) {
      cout << "hciImprov interface written by Craig Stuart Sapp, "
              "craig@ccrma.stanford.edu, September 2000" << endl;
      cout << "The author of this particular implementation of synthImprov\n"
              "might be found by using the --description option" << endl;
      exit(0);
   } else if (options.getBoolean("version")) {
      cout << "hciImprov, version 2.3 (Sat Sep 23 11:54:45 PDT 2000)\n"
              "compiled: " << __DATE__ << endl;
      exit(0);
   } else if (options.getBoolean("help")) {
      usage(options.getCommand());
      exit(0);
   } else if (options.getBoolean("description")) {
      description();
      exit(0);
   } else if (options.getBoolean("ports")) {
      cout << "\nMIDI input ports: " << MidiInput::getNumPorts() << endl;
      for (int k=0; k<MidiInput::getNumPorts(); k++) {
         cout << "\t" << k << ": " << MidiInput::getName(k) << endl;
      }
      cout << endl;
      cout << "MIDI output ports: " << MidiOutput::getNumPorts() << endl;
      for (int m=0; m<MidiOutput::getNumPorts(); m++) {
         cout << "\t" << m << ": " << MidiOutput::getName(m) << endl;
      }
      cout << endl;
      exit(0);
   }

   // finish setup if --options is present on the command line.
   if (options.optionsArg()) {
      return;
   }

   // choose the MIDI out port for synthesizer
   midi.setOutputPort(chooseMidiOutputPort());
   midi.openOutput();

   // choose the MIDI in port for synthesizer
   midi.setInputPort(chooseMidiInputPort());
   midi.openInput();

   // enable MIDI input from the synthesizer (in case is starts paused)
   midi.unpause();

   t_time = mainTimer.getTime(); 

   // set the idling rate for the evet loop to 1 millisecond
   eventIdler.setSoftSleep(1.0);

}



//////////////////////////////
//
// print_commands -- print keyboard character commands
//

void print_commands(void) {
   cout << endl;
   printboxtop();
   printcenterline(":: UPPERCASE COMMANDS :: hciImprov Interface :: ");
   printintermediateline();
   printstringline(
"   D = print a description of this program given by the programmer         ");
   printstringline(
"   ? = print this command list         M = print more commands             ");
   printstringline(
"   Q = quit the program                S = silence synthesizer             ");
   printstringline(
"   X = toggle MIDI out trace           Y = toggle MIDI in trace           ");
   printstringline(
"   I = set MIDI in port                O = set MIDI out port              ");
   printboxbottom();
}


//////////////////////////////
//
// print_aux_commands -- display additional commands not in main menu.
//

void print_aux_commands(void) {
   cout << endl;
   printboxtop();
   printcenterline("Hci Improv Interface version 23 September 2000 by");
   printcenterline("Craig Stuart Sapp  <craig@ccrma.stanford.edu>");
   printintermediateline();
   printstringline(
"   C = set CPU speed (used for timer)                                     ");
   printboxbottom();
}



//////////////////////////////
//
// setIdleEventRate -- sleep time between each iteration of the main
//      loop
//

void setIdleEventRate(float aRate) {
   eventIdler.setPeriod(aRate);
}



//////////////////////////////
//
// usage -- how to run this program from the command-line.
//

void usage(const char* command) {
   cout << 
   "\n"
   "hciImprov 2.3 interface\n"
   "\n"
   "Usage: " << command << "[options]\n"
   "\n"
   "Options:\n"
   "   --author      = author of this hciImprov interface\n"
   "   --version     = version of this hciImprov interface\n"
   "   --description = description of this program, with any\n"
   "                   command-line options\n"
   "   --help        = display this message\n"
   "   --ports       = display MIDI input/output ports and then exit\n"
   "   --options     = display all options, default values, and aliases\n"
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
// This function is not called directly, but is here so that
// it can be called by the user's program if desired.
//

void charsynth(int aKey) {
   static MidiMessage noteMessage;
   static int attack = 0;
   static int note = 0;

   midi.play(0, note, 0);

   noteMessage.time = mainTimer.getTime();
   noteMessage.command() = 0x90;
   noteMessage.p1() = note;
   noteMessage.p2() = 0;
   midi.insert(noteMessage);

   switch (aKey) {
      case 'z':  note=C4;    break;
      case 'x':  note=D4;    break;
      case 'c':  note=E4;    break;
      case 'v':  note=F4;    break;
      case 'b':  note=F4;    break;
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

   attack = rand()%47 + 81;           // random int from 1 to 127
   midi.play(0,note,attack); 

   noteMessage.time = mainTimer.getTime();
   noteMessage.command() = 0x90;
   noteMessage.p1() = note;
   noteMessage.p2() = rand()%47 + 81;      // random int from 1 to 127
   midi.insert(noteMessage);

}



//////////////////////////////
//
// octavekeyboard -- plays notes on bottom two rows of keys on
//	computer keyboard.  Octave transpositions can be made by
//	typing the octave numbers 0 through 9.
//

void octavekeyboard(int key, int octave) {
   static MidiMessage noteMessage;
   static int attack = 0;
   static int note = 0;

   midi.play(0, note, 0);

   noteMessage.time = mainTimer.getTime();
   noteMessage.command() = 0x90;
   noteMessage.p1() = note;
   noteMessage.p2() = 0;
   midi.insert(noteMessage);

   switch (key) {
      case 'z': note = 12 * octave + 0;   break;   // C
      case 's': note = 12 * octave + 1;   break;   // C#
      case 'x': note = 12 * octave + 2;   break;   // D
      case 'd': note = 12 * octave + 3;   break;   // D#
      case 'c': note = 12 * octave + 4;   break;   // E
      case 'v': note = 12 * octave + 5;   break;   // F
      case 'g': note = 12 * octave + 6;   break;   // F#
      case 'b': note = 12 * octave + 7;   break;   // G
      case 'h': note = 12 * octave + 8;   break;   // G#
      case 'n': note = 12 * octave + 9;   break;   // A
      case 'j': note = 12 * octave + 10;  break;   // A#
      case 'm': note = 12 * octave + 11;  break;   // B
      case ',': note = 12 * octave + 12;  break;   // C
      default: return;
   }
   if (note < 0)  note = 0;
   else if (note > 127)  note = 127;

   attack = rand()%48 + 80; 
   midi.play(0, note, attack);

   noteMessage.time = mainTimer.getTime();
   noteMessage.command() = 0x90;
   noteMessage.p1() = note;
   noteMessage.p2() = attack;
   midi.insert(noteMessage);
}



#endif  /* _IMPROV_INTERFACE_INCLUDED */


// md5sum: 145a3d8af43b277e2657972a97e84a5d hciImprov.h [20050403]

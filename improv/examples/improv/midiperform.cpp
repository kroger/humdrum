//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu> 
// Creation Date: Sat Nov 27 16:15:50 PST 1999
// Last Modified: Mon Nov 29 14:07:19 PST 1999
// Last Modified: Tue Nov 30 17:05:16 PST 1999 (added MIDI input control)
// Filename:      ...sig/doc/examples/all/midiperform/midiperform.cpp
// Syntax:        C++
// 
// Description:   plays a MIDI file with basic impulse conducting controls.
//

#include "MidiPerform.h"
#include "MidiInput.h"
#include "MidiMessage.h"
#include "Options.h"
#include "KeyboardInput.h"
#include "Idler.h"

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

// function declarations:
int  checkKeyboard(void);
void checkOptions(Options& opts);
void example(void);
void keyboardCommand(int command);
void processMidiCommand(MidiMessage& message);
void usage(const char* command);

// Global variables:
SigTimer keyboardTimer;             // for polling the keyboard for keys
KeyboardInput keyboard;             // for computer keyboard controls
MidiPerform performance;            // performance interface
Idler eventIdler;                   // for multi-tasking courtesy

// command-line variables
int tempoMethod = TEMPO_METHOD_AUTOMATIC;   // -a -c -t options
int outport = 0;                            // -p option
int inport  = 0;                            // -p option
int maxamp  = 64;                           // -m option

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   Options options(argc, argv);
   checkOptions(options);

   keyboardTimer.setPeriod(10);
   int command = 0;

   MidiInput midiin;
   midiin.setPort(inport);
   midiin.open();
   MidiMessage midimessage;

   performance.read(options.getArg(1));
   performance.setPort(outport);
   performance.setMaxAmp(maxamp);
   performance.open();
   performance.setTempoMethod(tempoMethod);
   performance.play();
   while (command != 'Q') {
      while (midiin.getCount() > 0) {
         midimessage = midiin.extract();
         processMidiCommand(midimessage);
      }
      performance.xcheck();
      eventIdler.sleep();

      if (keyboardTimer.expired()) {
         keyboardTimer.reset();
         command = checkKeyboard();
         if (command == 'Q') {
            break;
         } else {
            keyboardCommand(command);
         }
      }

   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// checkKeyboard -- check the keyboard for a key hit and return it
//    if there was any, otherwise returns -1.
//

int checkKeyboard(void) {
   int output = -1;
   if (keyboard.hit()) {
      output = keyboard.getch();
   }
   return output;
}
   


//////////////////////////////
//
// checkOptions -- handle command-line options.
//

void checkOptions(Options& opts) {
   opts.define("a|auto|automatic=b");
   opts.define("c|const|constant=b");
   opts.define("t|tempo|tempo-average=i:1");
   opts.define("m|max|max-amplitude=i:64");
   opts.define("p|port|out-port=i:0");
   opts.define("i|inport|in-port=i:0");
   opts.define("1|z|channel-collapse=b");
   opts.define("author=b");
   opts.define("version=b");
   opts.define("example=b");
   opts.define("help=b");
   opts.process();              

   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Nov 1999" << endl;
      exit(0);
   }
   if (opts.getBoolean("version")) {
      cout << "midiplay version 1.0" << endl;
      cout << "compiled: " << __DATE__ << endl;
   }
   if (opts.getBoolean("help")) {
      usage(opts.getCommand());
      exit(0);
   }
   if (opts.getBoolean("example")) {
      example();
      exit(0);
   }               

   // can only have one output filename
   if (opts.getArgCount() != 1) {
      cout << "Error: need one input MIDI file for performance." << endl;
      usage(opts.getCommand());
      exit(1);
   } 

   // figure out the tempo performance method
   if (opts.getBoolean("automatic")) {
      tempoMethod = TEMPO_METHOD_AUTOMATIC;
   } else if (opts.getBoolean("constant")) {
      tempoMethod = TEMPO_METHOD_CONSTANT;
   } else {
      switch (opts.getInteger("tempo-average")) {
         case 1:  tempoMethod = TEMPO_METHOD_ONEBACK;   break;
         case 2:  tempoMethod = TEMPO_METHOD_TWOBACK;   break;
         case 3:  tempoMethod = TEMPO_METHOD_THREEBACK; break;
         case 4:  tempoMethod = TEMPO_METHOD_FOURBACK;  break;
         default: tempoMethod = TEMPO_METHOD_ONEBACK;   break;
      }
   }

   outport = opts.getInteger("out-port");
   inport = opts.getInteger("in-port");
   maxamp = opts.getInteger("max-amplitude");
   performance.channelCollapse(opts.getBoolean("channel-collapse"));
}



//////////////////////////////
//
// example -- gives example calls to the midiplay program.
//

void example(void) {
   cout <<
   "# textmidi examples:                                                     \n"
   "       midiplay midifile.mid                                             \n"
   << endl;
}
 

//////////////////////////////
//
// keyboardCommand -- process a computer keyboard command character.
//

void keyboardCommand(int command) {
   switch (command) {
      case ' ':
         performance.beat();
         break;
      case 'b':                   // print beat location in performance
         cout << "Current beat is: " << performance.getBeatLocation() << endl;
         break;
      case 'z':                   // toggle MIDI collapsing
         {
         int setting = performance.channelCollapse();
         setting = !setting;
         performance.channelCollapse(setting);
         if (setting) {
            cout << "All notes going to channel 1" << endl;
         } else {
            cout << "All channel settings are unmodified" << endl;
         }
         }
         break;
      case '[':                   // set performance volume lower
         {
         double amp = performance.getAmp();
         amp /= 1.15;
         performance.setAmp(amp);
         cout << "Amplitude scaling = " << performance.getAmp() << endl;
         }
         break;
      case ']':                   // set performance volume higher
         {
         double amp = performance.getAmp();
         amp *= 1.15;
         performance.setAmp(amp);
         cout << "Amplitude scaling = " << performance.getAmp() << endl;
         }
         break;
      case 't':                   // current tempo display
         cout << "The Current tempo is: " << performance.getTempo() << endl;
         break;
      case '-':                   // slow down the tempo
         {
         double tempo = performance.getTempo();
         tempo /= 1.03;
         performance.setTempo(tempo);
         cout << "The Tempo was set on the keyboard to : " << tempo << endl;
         }
         break;
      case '=':                   // speed up the tempo
         {
         double tempo = performance.getTempo();
         tempo *= 1.03;
         performance.setTempo(tempo);
         cout << "The Tempo was set on the keyboard to : " << tempo << endl;
         }
         break;
      case '0':                   // automatic tempo control
         performance.setTempoMethod(TEMPO_METHOD_AUTOMATIC);
         cout << "Automatic Tempo Setting at tempo = " 
              << performance.getTempo() << endl;
         break;
      case '9':                   // constant tempo control
         performance.setTempoMethod(TEMPO_METHOD_CONSTANT);
         cout << "Constant Tempo Contro at tempo = " 
              << performance.getTempo() << endl;
         break;
      case '1':                   // 1 beat history average tempo
         performance.setTempoMethod(TEMPO_METHOD_ONEBACK);
         cout << "One beat tempo history" << endl;
         break;
      case '2':                   // 2 beat history average tempo
         performance.setTempoMethod(TEMPO_METHOD_TWOBACK);
         cout << "Two beat tempo history" << endl;
         break;
      case '3':                   // 3 beat history average tempo
         performance.setTempoMethod(TEMPO_METHOD_THREEBACK);
         cout << "Three beat tempo history" << endl;
         break;
      case '4':                   // 4 beat history average tempo
         performance.setTempoMethod(TEMPO_METHOD_FOURBACK);
         cout << "Four beat tempo history" << endl;
         break;
   }
}



//////////////////////////////
//
// processMidiCommand -- how to run the textmidi program on the command line.
//

void processMidiCommand(MidiMessage& message) {
   if (message.p0() != 0x90 || message.p2() == 0) {
      return;
   }

   switch (message.p1()) {
      case 60:                     // Middle C = beat
         keyboardCommand(' ');
         break;
      case 61:                     // C# = amplitude control
         {
         double amp = performance.getAmp();
         amp = amp * message.p2() / 64;
         if (amp < 0) {
            amp = 0;
         } else if (amp > 127) {
            amp = 127;
         }
         performance.setAmp((int)amp);
         }
         break;
      case 71:                      // B  = 1 beat tempo follow
         keyboardCommand('1');
         break;
      case 72:                      // C  = 2 beat tempo follow
         keyboardCommand('2');
         break;
      case 73:                      // C# = 3 beat tempo follow
         keyboardCommand('3');
         break;
      case 74:                      // D  = 4 beat tempo follow
         keyboardCommand('4');
         break;
      case 79:                      // G  = constant tempo follow
         keyboardCommand('9');
         break;
      case 80:                      // G# = automatic
         keyboardCommand('0');
         break;
      case 62:                      // amplitude decrease
         keyboardCommand('[');
         break;
      case 63:                      // amplitude increase
         keyboardCommand(']');
         break;
      case 64:                      // tempo decrease
         keyboardCommand('-');
         break;
      case 65:                      // tempo increase
         keyboardCommand('=');
         break;
   }

}


//////////////////////////////
//
// usage -- how to run the textmidi program on the command line.
//

void usage(const char* command) {
   cout <<
   "                                                                         \n"
   "Plays a MIDI file.                                                       \n"
   "                                                                         \n"
   "Usage: " << command << " midifile                                        \n"
   "                                                                         \n"
   "Options:                                                                 \n"
   "   --options = list of all options, aliases and default values.          \n"
   "                                                                         \n"
   "                                                                         \n"
   << endl;               
}


// md5sum: fe33e4858d695ca9ebe7cb4c17c901f6 midiperform.cpp [20050403]

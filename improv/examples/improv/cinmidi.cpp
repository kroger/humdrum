//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: 22 Nov 1998
// Last Modified: Sun Jan 10 00:05:07 PST 1999
// Last Modified: Mon Jun 28 15:22:08 PDT 1999 (added sysex display capability)
// Last Modified: Fri Jul  2 18:10:43 PDT 1999 (added keyboard commands)
// Last Modified: Fri Nov  2 14:07:19 PST 2001 (added midi.open() before while)
// Last Modified: Sun Nov 20 02:09:20 PST 2005 (added cpu speed display)
// Filename:      ...sig/doc/examples/improv/improv/cinmidi.cpp
// Syntax:        C++; improv 2.2
//

#include "improv.h"
#include <ctype.h>
#include <string.h>

// Here is the syntax of a header comment:
#define HEADER_START ";; "

// abbreviated general MIDI instrument names for the patch change
// interpretation display.  General MIDI instruments are standardized
// by the numbers 0-127.  If your instruments are not General MIDI, or
// you are using a instrument bank other than bank 0, then the names
// do not have any meaning.  These names are modified from the source:
//   ftp://ftp.linpeople.org/pub/People/nathan/playmidi-2.4.tar.gz
//
const char *GMinstrument[128] = {
   "acpiano",   "britepno",  "synpiano",  "honkytonk", "epiano1",   "epiano2",
   "hrpschrd",  "clavinet",  "celeste",   "glocken",   "musicbox",  "vibes",
   "marimba",   "xylophon",  "tubebell",  "santur",    "homeorg",   "percorg",
   "rockorg",   "churchorg", "reedorg",   "accordn",   "harmonica", "concrtna",
   "nyguitar",  "acguitar",  "jazzgtr",   "cleangtr",  "mutegtr",   "odguitar",
   "distgtr",   "gtrharm",   "acbass",    "fngrbass",  "pickbass",  "fretless",
   "slapbas1",  "slapbas2",  "synbass1",  "synbass2",  "violin",    "viola",
   "cello",     "contraba",  "marcato",   "pizzcato",  "harp",      "timpani",
   "marcato",   "slowstr",   "synstr1",   "synstr2",   "choir",     "doo",
   "voices",    "orchhit",   "trumpet",   "trombone",  "tuba",      "mutetrum",
   "frenchorn", "hitbrass",  "synbras1",  "synbras2",  "sprnosax",  "altosax",
   "tenorsax",  "barisax",   "oboe",      "englhorn",  "bassoon",   "clarinet",
   "piccolo",   "flute",     "recorder",  "woodflut",  "bottle",    "shakazul",
   "whistle",   "ocarina",   "sqrwave",   "sawwave",   "calliope",  "chiflead",
   "charang",   "voxlead",   "lead5th",   "basslead",  "fantasia",  "warmpad",
   "polysyn",   "ghostie",   "bowglass",  "metalpad",  "halopad",   "sweeper",
   "aurora",    "soundtrk",  "crystal",   "atmosphr",  "freshair",  "unicorn",
   "sweeper",   "startrak",  "sitar",     "banjo",     "shamisen",  "koto",
   "kalimba",   "bagpipes",  "fiddle",    "shannai",   "carillon",  "agogo",
   "steeldrum", "woodblock", "taiko",     "toms",      "syntom",    "revcymb",
   "fx-fret",   "fx-blow",   "seashore",  "jungle",    "telephone", "helicptr",
   "applause",  "ringwhsl"
};


// global variables for command-line options:
Options   options;           // for command-line processing
int       absoluteQ = 0;     // for -a command-line option (time display)
int       interpretQ = 1;    // for -i command-line option (interpret input)
int       bytesQ = 1;        // no longer used, set to 1
int       style = 0;         // for -d, -x, and -2 command-line options
int       secondsQ = 0;      // for -s command-line option
int       keyboardQ = 1;     // for -k command-line option
int       fileQ = 0;         // for -o command-line option
int       suppressOffQ = 0;  // for -n command-line option
int       filter[8] = {0};   // for -f command-line option
int       cfilter[16] = {0}; // for -c command-line option
fstream   outputfile;        // for -o command-line option

#define   MAX_KEY_BUFF (1024)
char      inputBuffer[MAX_KEY_BUFF+1] = {0}; // for keyboard input buffer
int       bufferIndex = 0;                   // for keyboard input buffer


// function declarations
void        checkOptions             (Options& opts);
void        displayMessage           (ostream& out, MidiMessage message, 
		                      int style);
void        displayHeader            (ostream& out);
void        examineInputForCommand   (const char* inputBuffer);
void        example                  (void);
const char* getGmInst                (int number);
char*       getKey                   (int keynum);
void        interpret                (ostream& out, MidiMessage message);
void        interpretSysex           (ostream& out, MidiMessage message);
float       makePitchBend            (int lsb, int msb);
void        printbyte                (ostream& out, int value, int location, 
                                      int style);
void        usage                    (const char* command);
void        userCommandSend          (const char* arguments);

MidiIO midi;                 // MIDI I/O interface
SigTimer sigtimer;           // timing control


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   options.setOptions(argc, argv);
   checkOptions(options);


   displayHeader(cout);
   if (fileQ) {
      displayHeader(outputfile);
   }

   KeyboardInput keyboard;     // for typing comments into output file
   char keych;                 // character from keyboard
   MidiMessage message;
   int lastTime = -1;

   midi.open();
   while (1) {
      while (midi.getCount() > 0) {
         message = midi.extract();

         // filter any specified message types
         if (suppressOffQ && ((message.p0() & 0xf0) == 0x90) &&
               (message.p2() == 0)) {      
            continue;
         } else if (filter[(message.p0() >> 4) - 8]) {
            continue;
         } else if (cfilter[message.p0() & 0x0f]) {
            continue;
         }

         // adjust message time to delta time if necessary
         if (!absoluteQ) {
            if (lastTime == -1) {
               lastTime = message.time;
               message.time = 0;
            } else {
               int temp = message.time;
               message.time = message.time - lastTime;
               lastTime = temp;
            }
         }

         displayMessage(cout, message, style);
         if (fileQ) {
            displayMessage(outputfile, message, style);
         }
      }

      if (keyboardQ && keyboard.hit()) {
         keych = keyboard.getch();
         switch (keych) {
            case 27:                   // escape key 
               if (fileQ && bufferIndex != 0 && bufferIndex < MAX_KEY_BUFF) {
                  inputBuffer[bufferIndex] = '\0';
                  outputfile << inputBuffer;
               }
               keyboard.deinitialize();
               exit(0);      
               break;
            case 0x08:                 // backspace key
            case 0x7f:                 // delete key
               if (bufferIndex > 0) {
                  cout << "\b \b" << flush;
                  bufferIndex--;
               }
               break;
            case 0x0a:                 // enter key only
               #ifdef VISUAL
                  break;
               #endif
            case 13:                   // line feed
               cout << endl;
               if (bufferIndex < MAX_KEY_BUFF) {
                  inputBuffer[bufferIndex] = '\0';
                  if (fileQ) {
                     outputfile << inputBuffer << '\n';
                  }
                  examineInputForCommand(inputBuffer);
               }
               bufferIndex = 0;
               break;
            case 0x0c:                 // ^L key (redraw input)
               cout << endl;
               if (bufferIndex < MAX_KEY_BUFF) {
                  inputBuffer[bufferIndex] = '\0';
                  cout << inputBuffer << flush;
               }
               break;
            default:                   // normal key
               cout << keych << flush;
               if (bufferIndex < MAX_KEY_BUFF) {
                  inputBuffer[bufferIndex++] = keych;
               } else { 
                  // buffer is WAY to long: kill it
                  bufferIndex = 0;
               }
         }
      }

      millisleep(1);   // sleep for 1 millisec for multi-tasking courtesy

   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// checkOptions -- process the command-line options
//

void checkOptions(Options& opts) {  // options are:
   opts.define("a|absolute=b");     //    boolean for absoute time display
   opts.define("i|interpret=b");    //    boolean for interpreting messages
   opts.define("p|port|inport=i:0");//    MIDI input port selection
   opts.define("t|outport=i:0");    //    MIDI output port selection
   opts.define("l|list=b");         //    boolean for displaying possible inputs
   opts.define("d|dec|decimal=b");  //    boolean for printing bytes as decimal
   opts.define("x|hex|hexadecimal=b"); // boolean for printing bytes as hex
   opts.define("2|bin|binary=b");   //    boolean for printing bytes as binary
   opts.define("o|output=s");       //    for duplicating stdout to file
   opts.define("n|nooffs=b");       //    boolean for suppressing note-offs
   opts.define("f|filter=s:");      //    for filtering out various commands
   opts.define("c|chan-filter=s:"); //    for filtering out various channels
   opts.define("u|user=s");         //    for displaying in header info
   opts.define("s|seconds=b");      //    for displaying time info in seconds
   opts.define("k|keyboard=b");     //    for handling computer keyboard
   opts.define("cpu=d:0.0");        //    for setting the CPU speed
   opts.define("author=b");
   opts.define("version=b");
   opts.define("example=b");
   opts.define("h|help=b");
   opts.process();

   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
              "craig@ccrma.stanford.edu, November 1998" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << "cinmidi, version 1.2 (2 Jul 1999)\n"
              "compiled: " << __DATE__ << endl;
      exit(0);
   } else if (opts.getBoolean("help")) {
      usage(opts.getCommand());
      exit(0);
   } else if (opts.getBoolean("example")) {
      example();
      exit(0);
   }


   if (opts.getBoolean("cpu")) {
      sigtimer.setCpuSpeed((int64bits)(opts.getDouble("cpu") * 1000000.0));
   }

   if (opts.getBoolean("list")) {
      // display the possible MIDI inputs to use with -p option
      cout << "\nMidi Input ports: " << MidiInput::getNumPorts() << endl;
      for (int i=0; i<MidiInput::getNumPorts(); i++) {
         cout << "\t" << i << ": " << MidiInput::getName(i) << '\n';
      }
      cout << endl;
      exit(0);
   }


   if (opts.getBoolean("absolute")) {
      absoluteQ = 1;
   }
   if (opts.getBoolean("interpret")) {
      interpretQ = 0;
   }
   if (opts.getBoolean("decimal")) {
      style = 10;
   } else if (opts.getBoolean("hexadecimal")) {
      style = 16;
   } else if (opts.getBoolean("binary")) {
      style = 2;
   } else {
      style = 0;
   }
   if (style != 0) {
      bytesQ = 1;
   }

   if (opts.getBoolean("nooffs")) {
      suppressOffQ = 1;
   }
   if (opts.getBoolean("seconds")) {
      secondsQ = 1;
   }
   if (opts.getBoolean("keyboard")) {
      keyboardQ = 0;
   }
   
   const char *filterString = opts.getString("filter");
   char current;
   for (int i=0; i<(int)strlen(filterString); i++) {
      current = filterString[i];
      switch (tolower(current)) {
         case '8':   filter[0] = 1;   break;
         case '9':   filter[1] = 1;   break;
         default:
            if (!isdigit(current) && isxdigit(current)) {
               if (current-'a'+2 >= 8 || current-'a'+2 < 0) {
                  cerr << "Error in filter string" << endl;
                  exit(1);
               } else {
                  filter[current - 'a' + 2] = 1;
               }
            }
      }
   }
   
   const char *cfilterString = opts.getString("chan-filter");
   char stringc [2] = {0};
   int cindex;
   for (int j=0; j<(int)strlen(cfilterString); j++) {
      if (isxdigit(cfilterString[j])) {
         stringc[0] = cfilterString[j];
         cindex = strtol(stringc, NULL, 16);
         if (cindex < 0 || cindex > 15) {
            cout << "Error in channel string: " << cfilterString << endl;
            exit(1);
         } else {
            cfilter[cindex] = 1;
         }
      }
         
   }

   if (opts.getBoolean("output")) {
      fileQ = 1;
      outputfile.open(opts.getString("output"), ios::out);
      if (!outputfile.is_open()) {
         cout << "Error: cannot open file: "<< opts.getString("output") << endl;
         exit(1);
      }
   }

   midi.setInputPort(opts.getInteger("inport"));
   midi.setOutputPort(opts.getInteger("outport"));
   midi.openInput();
   midi.openOutput();

   if (opts.getArgCount() != 0) {
      usage(opts.getCommand());
      exit(1);
   }
}

  
//////////////////////////////
//
// displayHeader -- display data formatting information
//

void displayHeader(ostream& out) {
   out << HEADER_START << "\n";
   if (options.getBoolean("user")) {
      out << HEADER_START << "Recorded by:    ";
      out << options.getString("user") << endl;
   }
   out << HEADER_START << "Style:          ";
   switch (style) {
      case 10:   out << "decimal\n";       break;
      case 16:   out << "hexadecimal\n";   break;
      case  2:   out << "binary\n";        break;
      default:   out << "default\n"; 
   }
   out << HEADER_START << "Timing:         ";
   if (absoluteQ) {
      out << "absolute";
      if (secondsQ) {
         out << " seconds\n";
      } else {
         out << " milliseconds\n";
      }
      out << HEADER_START << "Message format: received-time, command-byte,"
          " parameter-byte(s)\n";
   } else {
      out << "delta";
      if (secondsQ) {
         out << " seconds\n";
      } else {
         out << " milliseconds\n";
      }
      out << HEADER_START << "Message format: delta-time, MIDI command-byte, "
          "MIDI parameter-byte(s)\n";
   }
   out << HEADER_START << "Format:         asciimidi 1.0\n";
   out << HEADER_START << "Command-line:   " 
       << options.getCommandLine() << '\n';
   out << HEADER_START << "Input Port:     " << options.getInteger("inport")
       << ": " << MidiInput::getName(options.getInteger("inport")) << '\n';
   out << HEADER_START << "Cpu Speed:      " 
       << (double)sigtimer.getCpuSpeed() / 1000000.0 << " MHz" << '\n';
   out << HEADER_START << '\n';
   out << endl;
}



//////////////////////////////
//
// displayMessage -- displays the MIDI input message in the style
//	specified on the command-line
//

void displayMessage(ostream& out, MidiMessage message, int style) {
   if (secondsQ) {
      out << dec << message.time / 1000.0 << "\t";
   } else {
      out << dec << message.time << "\t";
   }
   
   if (bytesQ) {
      printbyte(out, (int)message[0], 0, style);
      if (message.p0() == 0xf0) {
         int byteCount = midi.getSysexSize((int)message.p1());
         uchar* sysexmessage = midi.getSysex((int)message.p1());
         for (int k=1; k<byteCount-1; k++) {
            out << " ";
            printbyte(out, (int)sysexmessage[k], k, style);
         } 
         out << " ";
         printbyte(out, (int)sysexmessage[byteCount-1], 0, style);
      } else {  // not a system exclusive
         int parameterCount = message.getParameterCount();
         for (int i=0; i<parameterCount; i++) {
            out << " ";
            printbyte(out, (int)message[i+1], i+1, style);
         }
      }
   }

   if (bytesQ && interpretQ && (style == 2)) {
      if (message.getParameterCount() <= 1) {
         out << '\t';
      }
      out << "\t ; ";
   } else if (bytesQ && interpretQ) {
      if (message.getParameterCount() <= 1) {
         out << '\t';
      }
      out << "\t\t; ";
   }

   if (interpretQ) {
      interpret(out, message);
   }

   out << endl;
}



//////////////////////////////
//
// examineInputForCommand -- Look for a command embedded in user
//      computer keyboard input.
//

void examineInputForCommand(const char* string) {
   if (strncmp(HEADER_START, string, 3) != 0) {
      // not a possible command string
      return;
   }

   int length = strlen(string);
   int i;
   for (i=0; i<length+1; i++) {
      if (string[i] == ':') {
         goto command_found;
      }
   }
   return;

   // some commands are:
   // ";; Send:  bytes = send these MIDI bytes on MIDI output
   // ";; Filter On: x or xx
   // ";; Filter Off: x or xx

   command_found:

   char* buffer = new char[length + 1];
   char* arguments = new char[length + 1];
   strcpy(buffer, string);
   
   char* command = strtok(buffer, ";: \t");
   int j = 0;
   while (command[j] != '\0') {
      command[j] = tolower(command[j]);
      j++;
   }

   if (strcmp(command, "send") == 0) {
      strncpy(arguments, strtok(NULL, ""), length);
      userCommandSend(arguments);
   } 

   delete [] buffer;
   delete [] arguments;
}



//////////////////////////////
//
// example -- shows various command-line option calls to program.
//

void example(void) {
   cout << 
   "# display MIDI input data in binary form:\n"
   "cinmidi -2\n"
   "\n"
   "# don't display note-off messages:\n"
   "cinmidi -n\n"
   "\n"
   "# don't display pitchbend data:\n"
   "cinmidi -fe\n"
   "\n"
   "# display only pitchbend data:\n"
   "cinmidi -f 89abcdf\n"
   "\n"
   "# display only note-on messages:\n" 
   "cinmidi -n -f 8abcdef\n"
   "\n"
   << endl;
}



//////////////////////////////
//
// getGmInst -- the General MIDI instrument name.
//

const char* getGmInst(int number) {
   return GMinstrument[number % 0x80];
}



//////////////////////////////
//
// getKey -- return the key name of the specified MIDI key number.
//

char* getKey(int keynum) {
   static char name[16] = {0};
   static char temp[16] = {0};
   int octave = keynum / 12 - 1;

   switch (keynum % 12) {
      case  0:   strcpy(name, "C");    break;
      case  1:   strcpy(name, "C#");   break;
      case  2:   strcpy(name, "D");    break;
      case  3:   strcpy(name, "Eb");   break;
      case  4:   strcpy(name, "E");    break;
      case  5:   strcpy(name, "F");    break;
      case  6:   strcpy(name, "F#");   break;
      case  7:   strcpy(name, "G");    break;
      case  8:   strcpy(name, "Ab");   break;
      case  9:   strcpy(name, "A");    break;
      case 10:   strcpy(name, "Bb");   break;
      case 11:   strcpy(name, "B");    break;
      default:   strcpy(name, "");
   }

   sprintf(temp, "%d", octave);
   strcat(name, temp);

   return name;
}



//////////////////////////////
//
// interpret -- convert a MIDI message to English.
//

void interpret(ostream& out, MidiMessage message) {
   out << dec;      // set output to decimal notation
 
   switch (message.p0() & 0xf0) {
      case 0x80:                                           // NOTE OFF
         out << "NOTEOFF chan: " << (int)(message.p0() & 0x0f) + 1
             << " key:" << getKey(message.p1()) << " vel: "
             << (int)message.p2();
         break;
      case 0x90:                                           // NOTE ON
         out << "NOTE chan:" << (int)(message.p0() & 0x0f) + 1
             << " key:" << getKey(message.p1())
             << " vel:" << (int)message.p2();
         if (message.p2() == 0) {
            out << " OFF";
         }
         break;
      case 0xa0:                                           // AFTERTOUCH
         out << "AFTERTOUCH chan:" << (int)(message.p0() & 0x0f) + 1
             << " key:" << getKey(message.p1())
             << " val:" << (int)message.p2();
         break;
      case 0xb0:
         out << "CONTROL chan:" << (int)(message.p0() & 0x0f) + 1
             << " #:" << (int)message.p1()
             << " val:" << (int)message.p2();
         switch (message.p1()) {
            case  7:  out << " (volume)";   break;
            case 64:  out << " (sustain)";  break;
         }
         break;
      case 0xc0:                                           // PATCH CHANGE
         out << "PATCH chan:" << (int)(message.p0() & 0x0f) + 1
             << " inst:" << (int)message.p1()
             << " (" << getGmInst(message.p1()) << ")";
         break;
      case 0xd0:                                           // CHANNEL PRESSURE
         out << "PRESSURE chan:" << (int)(message.p0() & 0x0f) + 1
             << " val:" << (int)message.p1();
         break;
      case 0xe0:                                           // PITCH BEND
         out << "PITCHBEND chan:" << (int)(message.p0() & 0x0f) + 1
             << " val:" << makePitchBend(message.p1(), message.p2());
         break;
      case 0xf0:
         switch (message.p0()) {
            // system common messages
            case 0xf0: out << "SYSEX"; interpretSysex(out, message); break;
            case 0xf1: out << "MIDI Time Code Quarter Frame";        break;
            case 0xf2: out << "Song Position Pointer";               break;
            case 0xf3: out << "Song Select";                         break;
            case 0xf4: out << "Undefined sys com msg";               break;
            case 0xf5: out << "Undefined sys com msg";               break;
            case 0xf6: out << "Tune Request";                        break;
            // system real time messages
            case 0xf8: out << "Timing Clock";                        break;
            case 0xf9: out << "Undefined sys realtime msg";          break;
            case 0xfa: out << "Start";                               break;
            case 0xfb: out << "Continue";                            break;
            case 0xfc: out << "Stop";                                break;
            case 0xfd: out << "Undefined";                           break;
            case 0xfe: out << "Active Sensing";                      break;
            case 0xff: out << "System Reset";                        break;
         }
         break;
   }
}
         


//////////////////////////////
//
// interpretSysex -- try to identify the system exclusive message
//

void interpretSysex(ostream& out, MidiMessage message) {
   if (message.p0() != 0xf0) {
      cout << "Error: interpretSysex needed a sysex message"
              " but got: " << hex << (int)message.p0() << dec << endl;
      exit(1);
   }

   uchar* sysex = midi.getSysex(message.p1());
   int length = midi.getSysexSize(message.p1());


   // check for Currently defined Universal System Exclusive Messages
   // first the non-real time messages where sysex[1] == 0x7e
   if (length >= 5 && sysex[1] == 0x7e) {
      out << ": UNIV";

      if (sysex[3] == 0x01) {
         out << ": Sample Dump Header";
         return;
      } else if (sysex[3] == 0x02) { 
         out << ": Sample Data Packet";
         return;
      } else if (sysex[3] == 0x03) {
         out << ": Sample Dump Request";
         return;
      } else if (sysex[3] == 0x04) {
         out << ": MIDI Time Code";
         switch (sysex[4]) {
            case 0x00: out << ": Special";                         break;
            case 0x01: out << ": Punch In Points";                 break;
            case 0x02: out << ": Punch Out Points";                break;
            case 0x03: out << ": Delete Punch In Point";           break;
            case 0x04: out << ": Delete Punch Out Point";          break;
            case 0x05: out << ": Event Start Point";               break;
            case 0x06: out << ": Event Stop Point";                break;
            case 0x07: out << ": Event Start Points + add. info";  break;
            case 0x08: out << ": Event Stop Points + add. info";   break;
            case 0x09: out << ": Delete Event Start Point";        break;
            case 0x0a: out << ": Delete Event Stop Point";         break;
            case 0x0b: out << ": Cue Points";                      break;
            case 0x0c: out << ": Cue Points with add. info";       break;
            case 0x0d: out << ": Delete Cue Point";                break;
            case 0x0e: out << ": Event Name in add. info";         break;
         }
         return;
      } else if (sysex[3] == 0x05) {
         out << ": Sample Dump Extensions";
         switch (sysex[4]) {
            case 0x01: out << ": Multiple Loop Points";   break;
            case 0x02: out << ": Loop Points Request";    break;
         }
         return;
      } else if (sysex[3] == 0x06) {
         out << ": General Information";
         switch (sysex[4]) {
            case 0x01: out << ": Identity Request";       break;
            case 0x02: out << ": Identity Reply";         break;
         }
         return;
      } else if (sysex[3] == 0x07) {
         out << ": File Dump";
         switch (sysex[4]) {
            case 0x01: out << ": Header";       break;
            case 0x02: out << ": Data Packet";  break;
            case 0x03: out << ": Request";      break;
         }
         return;
      } else if (sysex[3] == 0x08) {
         out << ": MIDI Tuning Standard";
         switch (sysex[4]) {
            case 0x01: out << ": Bulk Dump Request";   break;
            case 0x02: out << ": Bulk Dump Reply";     break;
         }
         return;
      } else if (sysex[3] == 0x09) {
         out << ": General MIDI";
         switch (sysex[4]) {
            case 0x01: out << ": On";     break;
            case 0x02: out << ": Off";    break;
         }
         return;
      } else if (sysex[3] == 0x7b) {
         out << ": End Of File";
         return;
      } else if (sysex[3] == 0x7c) {
         out << ": Wait";
         return;
      } else if (sysex[3] == 0x7d) {
         out << ": Cancel";
         return;
      } else if (sysex[3] == 0x7e) {
         out << ": NAK (bad transfer)";
         return;
      } else if (sysex[3] == 0x7f) {
         out << ": ACK (good transfer)";
         return;
      }
   }

       
    // check for Currently defined Universal System Exclusive Messages
    // now the non-real time messages where sysex[1] == 0x7f
   if (length >= 5 && sysex[1] == 0x7f) {
      out << ": UNIV";

      if (sysex[3] == 0x01) {
         out << ": MIDI Time Code";
         switch (sysex[4]) {
            case 0x01: out << ": Full Message";   break;
            case 0x02: out << ": User Bits";      break;
         }
         return;
      } else if (sysex[3] == 0x02) {
         out << ": MIDI Show Control";
         switch (sysex[4]) {
            case 0x00: out << ": Extensions";     break;
            default:   out << ": Commands";       break;
         }
         return;
      } else if (sysex[3] == 0x03) {
         out << ": Notation Info";
         switch (sysex[4]) {
            case 0x01: out << ": Bar Number";            break;
            case 0x02: out << ": Time Sig. (Immediate)"; break;
            case 0x42: out << ": Time Sig. (Delayed)";   break;
         }
         return;
      } else if (sysex[3] == 0x04) {
         out << ": Device Control";
         switch (sysex[4]) {
            case 0x01: out << ": Master Volume";    break;
            case 0x02: out << ": Master Balance";   break;
         }
         return;
      } else if (sysex[3] == 0x05) {
         out << ": Real Time MTC Cueing";        
         switch (sysex[4]) {
            case 0x00: out << ": Special";              break;
            case 0x01: out << ": Punch In Points";      break;
            case 0x02: out << ": Punch Out Points";     break;
            case 0x03: out << ": Reserved";             break;
            case 0x04: out << ": Reserved";             break;
            case 0x05: out << ": Event Start Points";   break;
            case 0x06: out << ": Event Stop Points";    break;
            case 0x07: out << ": Event Start Points + add. info"; break;
            case 0x08: out << ": Event Stop Points + add. info";  break;
            case 0x09: out << ": Reserved";             break;
            case 0x0a: out << ": Reserved";             break;
            case 0x0b: out << ": Cue Points";           break;
            case 0x0c: out << ": Cue Points with add. info"; break;
            case 0x0d: out << ": Reserved";             break;
            case 0x0e: out << ": Event Name in add. info"; break;
         }
         return;
      } else if (sysex[3] == 0x06) {
         out << ": MIDI Maching Control Commands";
         return;
      } else if (sysex[3] == 0x07) {
         out << ": MIDI Maching Control Response";
         return;
      } else if (sysex[3] == 0x08) {
         out << ": MIDI Tuning Standard";
         switch (sysex[4]) {
            case 0x02: out << ": Note Change";
         }
         return;
      }
   }

   // If gotten to this point, the sysex starts with a
   // manufacturer's ID number.

   if (sysex[1] == 0x7f) {
      out << ": MANF=non-commercial";
      return;
   } 
   if (sysex[1] >= 0x01 && sysex[1] <= 0x1f) {
      out << ": MANF=American1";
      return;
   }
   if (sysex[1] >= 0x20 && sysex[1] <= 0x3f) {
      out << ": MANF=European1";
      return;
   }
   if (sysex[1] >= 0x40 && sysex[1] <= 0x5f) {
      out << ": MANF=Japanese1";
      return;
   }
   if (sysex[1] >= 0x60 && sysex[1] <= 0x7c) {
      out << ": MANF=other1";
      return;
   }

   // must be a three byte sysex if get here
   if (sysex[1] == 0) {
      if (sysex[2] >= 0x20 && sysex[2] <= 0x3f) {
         out << ": MANF=European3";
         return;
      }
      if (sysex[2] >= 0x40 && sysex[2] <= 0x5f) {
         out << ": MANF=Japanese3";
         return;
      }
      if (sysex[2] >= 0x60 && sysex[2] <= 0x7F) {
         out << ": MANF=Other3";
         return;
      }
      else {
         out << ": MANF=American3";
         return;
      }
   }
}



//////////////////////////////
//
// makePitchBend -- create a number in range -1 to 1.
//

float makePitchBend(int lsb, int msb) {
   int value = (msb << 7) | lsb;
   float output = (float)((2.0 * value / 0x3fff) - 1.0);
   if ((output < 0.0001) && (output > -0.0001)) {
      return 0.0;
   } else {
      output = (float)(((int)(output * 1000.0))/1000.0);
      return output;
   }
}



//////////////////////////////
//
// printbyte -- display a byte according to specified style and location.
//

void printbyte(ostream& out, int value, int location, int style) {
   int i;
   switch (style) {
      case 10: 
         if (value < 100) {
            out << ' ';
         } 
         if (value < 10) {
            out << ' ';
         }
         out << dec << value;
         break;
      case 16:
         if (value < 0x10) {
            out << ' ';
         }
         out << hex << value;
         break;
      case  2:
         for (i=0; i<8; i++) {
            if (value & (1 << (7-i))) {
               out << '1';
            } else {
               out << '0';
            }
         }
         break;
      default:
         if (location == 0) {
            out << "0x";
            printbyte(out, value, 0, 16);
         } else {
            printbyte(out, value, location, 10);
         }
   }
}


   
//////////////////////////////
//
// usage -- how to run this program from the command-line.
//

void usage(const char* command) {
   cout << 
   "\n"
   "Display/Record MIDI input data.\n"
   "\n"
   "Usage: " << command << " [-a][-i][-x|-d|-2] [-o output][-p port]\n"
   "\n"
   "Options:\n"
   "   -a = display MIDI timing in absolute time instead of delta time\n"
   "   -i = don't display interpretation of MIDI message as comments\n"
   "   -d = display MIDI bytes in decimal form\n"
   "   -x = display MIDI bytes in hexadecimal form\n"
   "   -2 = display MIDI bytes in binary form\n"
   "   -n = do not display note-off messages\n"
   "   -f string = filter out specified MIDI commands (e.g. \"bc\" will\n"
   "        filter out control change and patch change messages)\n"
   "   -c string = filter out specified MIDI channels, offset zero\n"
   "        (e.g., \"09AF\", filters channels 1, 10, 11, and 16)\n"
   "   -p port = use the specified MIDI input port\n"
   "   -l = list MIDI input ports and then exit\n"
   "   -o filename = send display to file as well as to screen\n"
   "   -u string = specify a user name for the header information\n"
   "   -s = display time values in seconds\n"
   "   -k = disable keyboard input\n"
   "   --options = list all options, default values, and aliases\n"
   "\n"
   << endl;
}



//////////////////////////////
//
//  userCommandSend -- send some midi data out the MIDI out port.
//     currently only allows decimal data.
//

void userCommandSend(const char* midibytes) {
   char* buffer = new char[strlen(midibytes)+1];
   strcpy(buffer, midibytes);
   Array<uchar> midiinfo(0);
   uchar inputdata;

   char* data = strtok(buffer, " \t");
   int number;
   while (data != NULL && isdigit(data[0])) {
      sscanf(data, "%d", &number);
      inputdata = (uchar)number;
      midiinfo.append(inputdata);
      data = strtok(NULL, " \t");
   }

   midi.rawsend(midiinfo.getBase(), midiinfo.getSize());
}
   


// md5sum: 1a0047a203cd3948d5ec60e8c0766584 cinmidi.cpp [20090626]

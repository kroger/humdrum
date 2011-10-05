//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu May  6 17:15:41 PDT 2000
// Last Modified: Thu Jun 29 00:11:48 PDT 2000 updated to museinfo 1.1
// Last Modified: Thu Jun 29 00:11:48 PDT 2000 added shorten options
// Last Modified: Wed May 27 16:19:29 EDT 2009 added more keyboard commands
// Last Modified: Thu Jun  4 10:24:13 EDT 2009 added colorizing
// Last Modified: Fri Jun 12 18:25:00 PDT 2009 added muting/hiding/marking
// Last Modified: Thu Mar 24 04:22:03 PDT 2011 fixes for 64-bit compiling
// Filename:      ...sig/doc/examples/all/hplay/hplay.cpp
// Syntax:        C++ 
//
// Description:   Play **kern entries in a Humdrum file through MIDI.
//

#include "synthImprov.h"
#include "HumdrumFile.h"
#include "Convert.h"
#include <string.h>

// include string stream class which can have various names:
#ifndef OLDCPP
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
#else
   #ifdef VISUAL
      #include <strstrea.h>
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif

/*----------------- beginning of improvization algorithms ---------------*/

// colorization messages:
#define COLOR_INVALID       -1
#define COLOR_RESET          0
#define COLOR_INIT           1
#define COLOR_BARLINE        2
#define COLOR_CLEAR_SCREEN   3
#define COLOR_MUTE           4
#define COLOR_OVERFILL       5
#define COLOR_INTERPRETATION 6
#define COLOR_TANDEM         7
#define COLOR_MARKS          8
#define COLOR_COMMENT        9

double tempo = 120.0;     // current metronome tempo
SigTimer timer;           // for playing the humdrum file
EventBuffer eventBuffer;  // for storing note offs when playing notes

HumdrumFile data;         // humdrum file to play
int linenum       = 0;    // for keeping track of current line in file
double tempoScale = 1.0;  // for global adjustment of tempo
int velocity      = 64;   // default velocity to play MIDI notes
int echoTextQ     = 1;    // boolean for displaying input file
int fileNumber    = 1;    // current file number being played
int pauseQ        = 0;    // pause or unpause the music.
int colorQ        = 0;    // colorize the display
int hideQ         = 0;    // hide/show non-kern spines
int noteonlyQ     = 0;    // hide/show stems and beaming in **kern spines
int colormode     = 'b';  // w = white background; b = black background
int tabsize       = 12;   // used with 't' and 'T' real-time commands
int transpose     = 0;    // used with 'l' and 'L' real-time commands
Array<int> trackmute;     // used to mute/unmute tracks
Array<int> markers;       // storage for markers created with space bar
int markerindex   =  0;   // place to store first marker

int mine          = 30;   // used with the -m option
int shortenQ      =  0;   // used with the -s option
int shortenamount = 30;   // used with the -s option

// non-synthImprov function declarations:

void     checkOptions            (void);
void     inputNewFile            (void);
void     playdata                (HumdrumFile& data, int& linenum, 
                                  SigTimer& timer);
void     printInputLine          (HumdrumFile& infile, int line);
void     processNotes            (HumdrumRecord& record);
int      getMeasureLine          (HumdrumFile& data, int number);
ostream& colormessage            (ostream& out, int messagetype, int mode, 
                                  int status);
ostream& tabPrintLine            (ostream& out, HumdrumRecord& record, 
                                  int tabsize, int sMessage = COLOR_INVALID);
int      getKernTrack            (int number, HumdrumFile& infile);
int      goBackMeasures          (HumdrumFile& data, int startline, int target);
int      goForwardMeasures       (HumdrumFile& data, int startline, int target);
void     printExclusiveInterpLine(int linenum, HumdrumFile& infile);
int      getKernNoteOnlyLen      (char* buffer, const char* ptr);
void     printMarkLocation       (HumdrumFile& infile, int line, int mindex);
void     printMarkerInfo         (ostream& out, int mindex, int line, 
                                  HumdrumFile& infile, int style);
void     printAllMarkers         (ostream& out, Array<int>& markers, 
                                  HumdrumFile& infile);


/*--------------------- maintenance algorithms --------------------------*/


//////////////////////////////
//
// description -- This function is called with the user presses 
//      the letter 'D' on the computer keyboard.
//

void description(void) { 
   printboxtop();      
   psl(" HUMPLAY -- by Craig Stuart Sapp, 4 May 2000");
   psl(" _ = lower volume   + = raise volume   s = silence current notes");
   psl(" , = slow down      . = speed up       p = toggle MIDI output");
   psl(" < = slow down more > = speed up more  space = toggle text output");
   printboxbottom();
}


//////////////////////////////
//
// initialization -- This function is called at the start of the program.
//

void initialization(void) { 
   checkOptions();
   timer.setPeriod(500); 
   timer.reset();
   eventIdler.setPeriod(0);
   eventBuffer.setPollPeriod(10);
   eventBuffer.setPort(synth.getOutputPort());
   if (colorQ) {
      colormessage(cout, COLOR_INIT, colormode, colorQ);
      colormessage(cout, COLOR_CLEAR_SCREEN, colormode, colorQ);
      print_commands();
      sleep(1);
   }
   trackmute.setSize(1000); // maximum track in humdrum file assumed to be 1000
                            // space 1000-10 is used for a special purpose.
   trackmute.allowGrowth(0);
   trackmute.setAll(0);

   markers.setSize(1001);
   markers.setAll(0);
   markers.allowGrowth(0);
   markerindex = 0;
}


//////////////////////////////
//
// finishup -- This function is called just before exiting the program.
//

void finishup(void) { 
   eventBuffer.off();
   colormessage(cout, COLOR_RESET, colormode, colorQ);
}

/*-------------------- main loop algorithms -----------------------------*/

//////////////////////////////
//
// mainloopalgorithms -- This function is called continuously while
//    the programming is running.  It checks to determine whether another
//    line of the Humdrum file is ready to be played.
//

void mainloopalgorithms(void) { 
   eventBuffer.checkPoll();
   if (pauseQ) {
      return;
   }
   if (timer.expired()) {
      playdata(data, linenum, timer);
      if (linenum >= data.getNumLines()) {
         inputNewFile();
      }
   }
}

/*-------------------- triggered algorithms -----------------------------*/

//////////////////////////////
//
// keyboardchar -- When the user presses a key on the computer keyboard,
//    it is sent to this program.  Certain capital letter keys are 
//    used by the interface (such as "?", "M", "D", "X", "Y", "G"), and 
//    will not be sent to this function.
//

void keyboardchar(int key) { 
   static int lastkeytime = 0;
   static int number      = 0;

   if (t_time - lastkeytime > 5000) {
      // reset the number value if more than 5 seconds has elapsed
      // since the last key press.
      number = 0;
   }
   lastkeytime = t_time;

   if (isdigit(key)) {
      number = number * 10 + (key - '0');
      return;
   }
   switch (key) {
   // case 'a': break;
      case 'b':               // set color mode to black
         colorQ = 1;          // turn on colorization automatically
         colormode = 'b';
         colormessage(cout, COLOR_INIT, colormode, colorQ);
         cout << "!! CHANGING TO BLACK BACKGROUND" << endl;
         break;
      case 'c':               // toggle colorization
         colorQ = !colorQ;
         if (colorQ) {
            colormessage(cout, COLOR_INIT, colormode, colorQ);
            cout << "!! COLORIZATION TURNED ON" << endl;
         } else {
            colormessage(cout, COLOR_RESET, colormode, !colorQ);
            cout << "!! COLORIZATION TURNED OFF" << endl;
         }
         break;
   // case 'd': break;
      case 'e':               // print exclusive interpretation info for spines
         printExclusiveInterpLine(linenum, data);
         break;
   // case 'f': break;
   // case 'g': break;

      case 'h':               // hide/unhide non-kern spine
         hideQ = !hideQ;
         if (hideQ) {
            cout << "!! Hiding non-kern spines" << endl;
         } else {
            cout << "!! Showing all spines" << endl;
         }
         break;
   // case 'i': break;
   // case 'j': break;
   // case 'k': break;
      case 'l':               // transpose up specified number of semitones
         if (number < 100) {
            transpose = number;
            cout << "!! Transposing " << transpose << " steps up" << endl;
         }
         break;
      case 'L':               // transpose down specified number of semitones
         if (number < 100) {
            transpose = -number;
            cout << "!! Transposing " << -transpose << " steps down" << endl;
         }
         break;
      case 'm':               // mute or unmute all tracks
         if (number == 0) {
            trackmute.setAll(!trackmute[trackmute.getSize()-1]);
            if (trackmute[0]) {
               cout << "!! All spines are muted" << endl;
            } else {
               cout << "!! All spines are unmuted" << endl;
            }
         } else {
            int tracknum = getKernTrack(number, data); 
            trackmute[tracknum] = !trackmute[tracknum];
            if (trackmute[tracknum]) {
               cout << "!! **kern spine " << number << " is muted" << endl;
            } else {
               cout << "!! **kern spine " << number << " is unmuted" << endl;
            }
         }
         break;
         break;
      case 'n':               // toggle display of note only (supression
                              // of beam and stem display
         noteonlyQ = !noteonlyQ;
         if (noteonlyQ) {
            cout << "!! Notes only: supressing beams and stems in **kern data" 
                 << endl;
         } else {
            cout << "!! Displaying *kern data unmodified" << endl;
         }
         break;
      case 'o':               // set the tempo to a particular value
         if (number > 20 && number < 601) {
            cout << "!! TEMPO SET TO " << number << endl;
            tempo = number;
            tempoScale = 1.0;
         } else if (number == 0) {
            cout << "!! Current tempo: " << tempo * tempoScale << endl;
         }
         break;
      case 'p':               // toggle music pausing
         eventBuffer.off();
         timer.reset();
         pauseQ = !pauseQ;
         if (pauseQ) {
            cout << "!! Paused" << endl;
         }
         break;
      case 'q':               // toggle display of file while playing
         echoTextQ = !echoTextQ;
         if (echoTextQ) {
            cout << "!! FILE DISPLAY TURNED ON" << endl;
         } else {
            cout << "!! FILE DISPLAY TURNED OFF" << endl; 
         }
         break;
      case 'r':               // return to a marker
         if (number == 0) {
            linenum = markers[0];
            cout << "!! Going to line " << linenum << endl;
            eventBuffer.off();
            timer.reset();
         } else if (number < markers.getSize()) {
            linenum = markers[number];
            cout << "!! Going to line " << linenum << endl;
            eventBuffer.off();
            timer.reset();
         }
         break;
      case 'R':               // Print a list of all markers
         printAllMarkers(cout, markers, data);
         break;
      case 's':    // silence notes
         eventBuffer.off();
         break;
      case 't':    // increase tab size
         tabsize++;
         // cout << "!! tabsize = " << tabsize << endl;
         break;
      case 'T':    // decrease tab size
         tabsize--;
         if (tabsize < 3) {
            tabsize = 3;
         }
         // cout << "!! tabsize = " << tabsize << endl;
         break;
   // case 'u': break;
   // case 'v': break;
      case 'w':               // set color mode to white
         colorQ = 1;          // turn on colorization automatically
         colormode = 'w';
         colormessage(cout, COLOR_INIT, colormode, colorQ);
         cout << "!! CHANGING TO WHITE BACKGROUND" << endl;
         break;
      case 'x':               // clear the screen
         colormessage(cout, COLOR_CLEAR_SCREEN, colormode, 1);
         printInputLine(data, linenum-1);
         break;
   // case 'y': break;
   // case 'z': break;

      case ' ':               // mark the measure/beat/line of the music
         if ((number != 0) && (number < markers.getSize())) {
            markerindex = number;
         } else {
            markerindex++;
            if (markerindex > markers.getSize()-1) {
               markerindex = 1;
            }
         }
         printMarkLocation(data, linenum == 0 ? 0 : linenum-1, markerindex);
         break;
      case ',':    // slow down tempo 
         tempoScale *= 0.97;
         cout << "!! TEMPO SET TO " << (int)(tempo * tempoScale) << endl;
         break;
      case '<':
         tempoScale *= 0.93; 
         cout << "!! TEMPO SET TO " << (int)(tempo * tempoScale) << endl;
         break;
      case '.':    // speed up tempo 
         tempoScale *= 1.03;
         cout << "!! TEMPO SET TO " << (int)(tempo * tempoScale) << endl;
         break;
      case '>':
         tempoScale *= 1.07;
         cout << "!! TEMPO SET TO " << (int)(tempo * tempoScale) << endl;
         break;
      case '=': 
         {
            int newline = 0;
            if (number == 0) {
               newline = 0;
            } else {
               newline = getMeasureLine(data, number);
            }
            if (newline >= 0) {
               cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" 
                    << " =" << number
                    << endl;
               linenum = newline;
               eventBuffer.off();
               timer.reset();
            }
         }
         break;
      case '(': 
         {
            int newline = goBackMeasures(data, linenum, number);
            cout << "!! back " << number << " measure" 
		 << (number==1? "":"s") << endl;
            linenum = newline;
            eventBuffer.off();
            timer.reset();
         }
         break;
      case ')': 
         {
            int newline = goForwardMeasures(data, linenum, number);
            cout << "!! forward " << number << " measure" 
                 << (number==1? "":"s") << endl;
            linenum = newline;
            eventBuffer.off();
            timer.reset();
         }
         break;
      case '+':    // louder
         velocity++;
         if (velocity > 127) {
            velocity = 127;
         }
         cout << "!! velocity = " << velocity << endl;
         break;

      case '_':    // sofer
         velocity--;
         if (velocity < 1) {
            velocity = 1;
         }
         cout << "!! velocity = " << velocity << endl;
         break;

      case '^':    // go to the start of the file
         linenum = 0;
         cout << "!! Going to start of file" << endl;
         break;
   }

   if (!isdigit(key)) {
      number = 0;
   }
}



/*------------------ end improvization algorithms -----------------------*/


//////////////////////////////
//
// printAllMarkers --
//

void printAllMarkers(ostream& out, Array<int>& markers, HumdrumFile& infile) {
   int i;
   colormessage(cout, COLOR_MARKS, colormode, colorQ);
   out << "**mark\t**line\t**abeat\t**bar\t**beat\n";
   for (i=1; i<markers.getSize(); i++) {
      if (markers[i] > 0) {
         printMarkerInfo(out, i, markers[i], infile, 0);
         out << "\n";
      }
   }
   out << "*-\t*-\t*-\t*-\t*-" << endl;
   colormessage(cout, COLOR_INIT, colormode, colorQ);
}



//////////////////////////////
//
// printMarkerInfo --
//  style = 0 : print without text labels.
//  style = 1 : print with text labels.
//

void printMarkerInfo(ostream& out, int mindex, int line, HumdrumFile& infile, 
      int style) {
   double absbeat = infile.getAbsBeat(line);
   double measure = -10;
   double beat = infile[line].getBeat();
 
   int i;
   int counter = 0;
   for (i=line; i>=0; i--) {
      if (data[i].isMeasure()) {
         counter++;
         if (sscanf(data[i][0], "=%lf", &measure)) {
            break;
         }
         if (counter >= 2) {
            // don't search more than two measures backwards
            break;
         }
      }
   }

   if (style == 0) {
      out << mindex << "\t" << line+1 << "\t" << absbeat;
      out << "\t" << measure << "\t" << beat;
   } else { 
      out << "Mark:"         << mindex 
           << "\tLine:"       << line+1
           << "\tAbsbeat:"    << absbeat;
      if (measure > -10) {
         out << "\tMeasure:" << measure;
      }
      if (beat >= 0) {
         out << "\tBeat:"    << beat;
      }
   }

}



//////////////////////////////
//
// printMarkLocation -- print where you are in the file.
//

void printMarkLocation(HumdrumFile& infile, int line, int mindex) {
   if (line < 0 || line > infile.getNumLines()-1) {
      return;
   }
   colormessage(cout, COLOR_COMMENT, colormode, colorQ);
   cout << "!! ";
   colormessage(cout, COLOR_MARKS, colormode, colorQ);
   printMarkerInfo(cout, mindex, line, infile, 1);
   colormessage(cout, COLOR_INIT, colormode, colorQ);
   cout << endl;

   markers[mindex] = line;
   markers[0]      = line;
}



//////////////////////////////
//
// printExclusiveInterpLine --
//

void printExclusiveInterpLine(int linenum, HumdrumFile& infile) {
   int dataline = -1;
   int i;
   for (i=0; i<data.getNumLines(); i++) { 
      if (infile[i].hasSpines()) {
         dataline = i;
         break;
      }
   }
   if (dataline < 0) {
      return;
   }

   SSTREAM tempstream;
   for (i=0; i<infile[dataline].getFieldCount(); i++) {
      tempstream << infile[dataline].getExInterp(i);
      if (i < infile[dataline].getFieldCount()-1) {
         tempstream << "\t";
      }
   }
   tempstream << "\n";
   for (i=0; i<infile[dataline].getFieldCount(); i++) {
      tempstream << "*-";
      if (i < infile[dataline].getFieldCount()-1) {
         tempstream << "\t";
      }
   }
   tempstream << ends;

   HumdrumFile dummyfile;
   dummyfile.read(tempstream);
   printInputLine(dummyfile, 0);
}



//////////////////////////////
//
// goBackMeasures --
//

int goBackMeasures(HumdrumFile& data, int startline, int target) {
   if (target < 0) {
      target = 1; 
   }
   target++; // need to go back past the current measure barline as well.
   int i;
   int counter = 0;
   for (i=startline; i>=0; i--) {
      if (data[i].isMeasure()) {
         counter++;
         if (counter > target) {
            break;
         }
      }
   }
   
   return i;
}



//////////////////////////////
//
// goForwardMeasures --
//

int goForwardMeasures(HumdrumFile& data, int startline, int target) {
   if (target < 0) {
      target = 1; 
   }
   if (startline > data.getNumLines()-1) {
      return data.getNumLines()-1;
   }
   int i;
   int counter = 0;
   for (i=startline; i<data.getNumLines(); i++) {
      if (data[i].isMeasure()) {
         counter++;
         if (counter > target) {
            break;
         }
      }
   }
   
   return i;
}



//////////////////////////////
//
// getMeasureLine -- return the line number on which the measure
//     number is found.  Return -1 if measure is not found.
//

int getMeasureLine(HumdrumFile& data, int number) {
   int i;
   int testnum = -1;
   for (i=0; i<data.getNumLines(); i++) {
      if (data[i].isMeasure()) {
         if (sscanf(data[i][0], "=%d", &testnum) == 1) {
            if (number == testnum) {
               return i;
            }
         }
      }
   }

   return -1;
}



//////////////////////////////
//
// getKernTrack -- Returns the track number of the nth **kern spine.
//

int getKernTrack(int number, HumdrumFile& infile) {
   int counter = 0;
   int i;

   for (i=1; i<=infile.getMaxTracks(); i++) {
      if (strcmp("**kern", infile.getTrackExInterp(i)) == 0) {
         counter++;
      }
      if (counter == number) {
         return i;
      }
   }

   // Give some high-numbered spine which is not likely
   // to be used (but is valid based on the size of trackmute).
   return trackmute.getSize()-10;  
}



//////////////////////////////
//
// checkOptions --  process command-line options and setup the
//   humdrum data to play.
//

void checkOptions(void) {
   options.define("t|tempo=d:120.0", "Base tempo");
   options.define("e|tempo-scale=d:1.0", "Tempo scaling factor");
   options.define("c|color|colour=s:b", "colorize the display");
   options.define("p|pause=d:1.0", "pause time in seconds between files");
   options.define("q|quiet=b", "Turn off data echoing while playing");
   options.define("v|velocity=i:64", "Default MIDI key velocity");
   options.define("m|min=i:30",      "minimum millisecond duration of notes");
   options.define("s|shorten=i:30",  "shortening millisecond value for note durations");
   options.process();

   velocity = options.getInteger("velocity");
   tempoScale = options.getDouble("tempo-scale");
   tempo = options.getDouble("tempo");
   if (options.getBoolean("quiet")) {
      echoTextQ = 0;
   }

   if (options.getArgCount() < 1) {
      data.read(cin);
   } else {
      inputNewFile();
   }

   mine = options.getInteger("min");
   if (mine < 0) {
      mine = 0;
   }
   shortenQ = !options.getBoolean("shorten");
   shortenamount = options.getInteger("shorten");

   colorQ = !options.getBoolean("color");
   colormode = options.getString("color")[0];
   if ((colormode != 'b') && (colormode != 'w')) {
      colormode = 'w';
   }
}



//////////////////////////////
//
// inputNewFile -- load in a new Humdrum file.
//

void inputNewFile(void) {
   data.clear();
   linenum = 0;    

   int count = options.getArgCount();
   if (fileNumber > count) {
      finishup();
      exit(0);
   }

   data.read(options.getArg(fileNumber));
   data.analyzeRhythm("4");

   if (fileNumber > 1) {
      millisleep((float)(1000 * options.getDouble("pause")));
   }
   fileNumber++;
}



//////////////////////////////
//
// playdata -- play the next line of the humdrum file, update the 
//     line number and the time for the next events to be read 
//     from the file.
//

void playdata(HumdrumFile& data, int& linenum, SigTimer& timer) {
   double duration = 0;     // duration of the current line;
   int type = data[linenum].getType();

   while (linenum < data.getNumLines() && duration == 0.0) {
      duration = data[linenum].getDuration();   

      if (type == E_humrec_data) {
         processNotes(data[linenum]);
      } else if (type == E_humrec_interpretation) {
         if (strncmp(data[linenum][0], "*MM", 3) == 0) {
            tempo = atoi(&data[linenum][0][3]);
         }
      }
      if (echoTextQ) {
         printInputLine(data, linenum);
      }
      if (duration > 0.0) {
         timer.setPeriod(60000 / tempo / tempoScale * duration);
         timer.reset();
      }
      linenum++;
      if (linenum < data.getNumLines()) {
         type = data[linenum].getType();
      }
   }
}



//////////////////////////////
//
// printInputLine -- print the current line of the file,
//  omitting the duration field at the end of the line
//

void printInputLine(HumdrumFile& infile, int line) {
   if ((line < 0) || (line >= infile.getNumLines())) {
      return;
   }
   HumdrumRecord& record = infile[line];
   if (record.isMeasure()) {
      colormessage(cout, COLOR_BARLINE, colormode, colorQ);
      tabPrintLine(cout, record, tabsize, COLOR_BARLINE);
      colormessage(cout, COLOR_INIT, colormode, colorQ);
      cout << endl;
   } else if (record.isTandem()) {
      colormessage(cout, COLOR_TANDEM, colormode, colorQ);
      tabPrintLine(cout, record, tabsize, COLOR_TANDEM);
      colormessage(cout, COLOR_INIT, colormode, colorQ);
      cout << endl;
   } else if (record.isInterpretation()) {
      colormessage(cout, COLOR_INTERPRETATION, colormode, colorQ);
      tabPrintLine(cout, record, tabsize, COLOR_INTERPRETATION);
      colormessage(cout, COLOR_INIT, colormode, colorQ);
      cout << endl;
   } else {
      tabPrintLine(cout, record, tabsize);
      cout << endl;
   }
}



//////////////////////////////
//
// tabPrintLine -- print a line with tabs changed to spaces
// default value sMessage = COLOR_INVALID
//

ostream& tabPrintLine(ostream& out, HumdrumRecord& record, int tabsize, int sMessage) {
   int i, j;
   int len;
   int track;
   int suppressColor = record.isMeasure() || record.isInterpretation();

   // count the **kern spines (used with hideQ varaible):
   int kerncount = 0;
   int lastkern = -1;
   for (i=0; i<record.getFieldCount(); i++) {
      if (strcmp(record.getExInterp(i), "**kern") == 0) {
         kerncount++;
         lastkern = i;
      }
   }

   const char *ptr = NULL;
   char buffer[1024] = {0};
   for (i=0; i<record.getFieldCount()-1; i++) {
      if (hideQ && (strcmp(record.getExInterp(i), "**kern") != 0)) {
         continue;
      }
      if (hideQ && (lastkern == i)) {
         // print the last kern spine outside of the loop below
         break;
      }
      track = record.getPrimaryTrack(i);
      ptr = record[i];
      if (noteonlyQ && strcmp(record.getExInterp(i), "**kern") == 0) {
         len = getKernNoteOnlyLen(buffer, ptr);
         ptr = buffer;
      } else {
         len = strlen(ptr);
      }

      if (len < tabsize) {
         if (!suppressColor && trackmute[track]) {
            colormessage(cout, COLOR_MUTE, colormode, colorQ);
            out << ptr;
            colormessage(cout, COLOR_INIT, colormode, colorQ);
         } else {
            out << ptr;
         }
         for (j=len; j<tabsize; j++) {
            out << ' ';
         }
      } else {
         if (!suppressColor && trackmute[track]) {
            colormessage(cout, COLOR_MUTE, colormode, colorQ);
         }
         for (j=0; j<tabsize - 1; j++) {
            out << ptr[j];
         }
         if (!trackmute[track]) {
            colormessage(cout, COLOR_OVERFILL, colormode, colorQ);
         }
         out << '|';
         if (suppressColor) {
               colormessage(cout, sMessage, colormode, colorQ);
         } else {
            if (!trackmute[track]) {
               colormessage(cout, COLOR_INIT, colormode, colorQ);
            }
   
            if (!suppressColor && trackmute[track]) {
               colormessage(cout, COLOR_INIT, colormode, colorQ);
            }
         }
      }
   }

   int lastindex = 0;
   if (hideQ && (lastkern >= 0)) {
      track = record.getPrimaryTrack(lastkern);
      lastindex = lastkern; 
   } else {
      lastindex = record.getFieldCount()-1;
      track = record.getPrimaryTrack(lastindex);
   }

   ptr = record[lastindex];
   if (noteonlyQ && strcmp(record.getExInterp(lastindex), "**kern") == 0) {
      len = getKernNoteOnlyLen(buffer, ptr);
      ptr = buffer;
   } else {
      len = strlen(ptr);
   }
   
   if ((track > 0) && !suppressColor && trackmute[track]) {
      colormessage(cout, COLOR_MUTE, colormode, colorQ);
   }
   out << ptr;
   if ((track > 0) && !suppressColor && trackmute[track]) {
      colormessage(cout, COLOR_INIT, colormode, colorQ);
   }
   out << flush;

   return out;
}



//////////////////////////////
//
// getKernNoteOnlyLen -- return the length of the **kern data excluding beaming
//   and steming information.
//

int getKernNoteOnlyLen(char* buffer, const char* ptr) {
   buffer[0] = '\0';
   int counter = 0;
   int len = strlen(ptr);
   int i;
   for (i=0; i<len; i++) {
      switch (ptr[i]) {
         case 'k': case 'K': case 'L': case 'J':   // beaming characters
         case '/': case '\\':                      // stem direction characters
            break;
         default:
            buffer[counter++] = ptr[i];
            buffer[counter] = '\0';
      }
   }

   buffer[counter] = '\0';
   return counter;
}



//////////////////////////////
//
// processNotes -- play all kern notes in the current record and
//    return the shortest note duration.  Don't play notes
//    if they are in a track which is muted.
//

void processNotes(HumdrumRecord& record) {
   NoteEvent note;
   int pitch = 0;
   double duration = 0.0;
   int staccatoQ = 0;
   int accentQ = 0;
   int sforzandoQ = 0;
   int i, j;
   int notecount = 0;
   char buffer[128] = {0};
   for (i=0; i<record.getFieldCount(); i++) {
      if ((record.getPrimaryTrack(i) < trackmute.getSize()) 
            && trackmute[record.getPrimaryTrack(i)]) {
         continue;
      }
      if (record.getExInterpNum(i) == E_KERN_EXINT) {
         notecount = record.getTokenCount(i);
         if (strcmp(record[i], ".") == 0) {
            continue;
         }
         for (j=0; j<notecount; j++) {
            record.getToken(buffer, i, j);
            if (strchr(buffer, '[')) {
               // total tied note durations
               duration = data.getTiedDuration(linenum, i, j);
            } else {
               duration = Convert::kernToDuration(buffer);
            }
            pitch = Convert::kernToMidiNoteNumber(buffer); 
            // skip rests
            if (pitch < 0) {
               continue;
            }
            pitch += transpose;
            // don't play note which is transposed too extremely
            if (pitch < 0)   { continue; }
            if (pitch > 127) { continue; }

            // skip tied notes
            if (strchr(buffer, '_') || strchr(buffer, ']')) {
               continue;
            }

            accentQ    = strchr(buffer, '^')  == NULL? 0 : 1;
            sforzandoQ = strchr(buffer, 'z')  == NULL? 0 : 1;
            staccatoQ  = strchr(buffer, '\'') == NULL? 0 : 1;
            note.setChannel(0);
            note.setKey(pitch);
            note.setOnTime(t_time);
            duration = duration * 60000 / tempo / tempoScale;
            if (shortenQ) {
               duration -= shortenamount;
               if (duration < mine) {
                  duration = mine;
               }
            }
            note.setDur((int)duration);
            if (staccatoQ) {
               note.setDur((int)(0.5 * note.getDur()));
            }
            note.setVelocity(velocity);
            if (accentQ) {
               note.setVelocity((int)(note.getVelocity() * 1.3));
            }
            if (sforzandoQ) {
               note.setVelocity((int)(note.getVelocity() * 1.5));
            }
 
            note.activate();
            note.action(eventBuffer);
            eventBuffer.insert(note);
         }
      }
   }
}



//////////////////////////////
//
// colormessage --
//

ostream& colormessage(ostream& out, int messagetype, int mode, int status) {
   if (messagetype == COLOR_INVALID) {
      return out;
   }

   #define ANSI_RESET          "\033[0m"   /* go back to the original colors */
   #define ANSI_UNDERLINE_ON   "\033[4m"
   #define ANSI_UNDERLINE_OFF  "\033[24m"
   #define ANSI_CLEAR_SCREEN   "\033[2J\033[H" /* and move to the top */

   #define ANSI_BLACK          "\033[37;40m"  /* black background; white text */
   #define ANSI_WHITE          "\033[30;107m" /* white background; black text */

   #define ANSI_BLACK_TX       "\033[30m"
   #define ANSI_RED_TX         "\033[31m"
   #define ANSI_GREEN_TX       "\033[32m"
   #define ANSI_YELLOW_TX      "\033[33m"
   #define ANSI_BLUE_TX        "\033[34m"
   #define ANSI_MAGENTA_TX     "\033[35m"
   #define ANSI_CYAN_TX        "\033[36m"
   #define ANSI_WHITE_TX       "\033[37m"

   #define ANSI_HI_BLACK_TX    "\033[90m"
   #define ANSI_HI_RED_TX      "\033[91m"
   #define ANSI_HI_GREEN_TX    "\033[92m"
   #define ANSI_HI_YELLOW_TX   "\033[93m"
   #define ANSI_HI_BLUE_TX     "\033[94m"
   #define ANSI_HI_MAGENTA_TX  "\033[95m"
   #define ANSI_HI_CYAN_TX     "\033[96m"
   #define ANSI_HI_WHITE_TX    "\033[97m"

   #define ANSI_GRAY_BG        "\033[47m"

   if (status == 0) {
      return out;   // don't do any coloring
   }

   if (mode == 'b') {

      switch (messagetype) {
         case COLOR_RESET:         out << ANSI_RESET;         break;
         case COLOR_INIT:          out << ANSI_BLACK;         break;
         case COLOR_BARLINE:       out << ANSI_YELLOW_TX;     break;
         case COLOR_CLEAR_SCREEN:  out << ANSI_CLEAR_SCREEN;  break;
         case COLOR_MUTE:          out << ANSI_BLUE_TX;       break;
         case COLOR_OVERFILL:      out << ANSI_HI_BLUE_TX;    break; 
         case COLOR_INTERPRETATION:out << ANSI_HI_RED_TX;     break; 
         case COLOR_TANDEM:        out << ANSI_MAGENTA_TX;    break; 
         case COLOR_MARKS:         out << ANSI_CYAN_TX;       break; 
         case COLOR_COMMENT:       out << ANSI_GREEN_TX;      break; 
         default: return colormessage(out, COLOR_INIT, mode, status);
      }

   } else if (mode == 'w') {

      switch (messagetype) {
         case COLOR_RESET:         out << ANSI_RESET;         break;
         case COLOR_INIT:          out << ANSI_WHITE;         break;
         case COLOR_BARLINE:       out << ANSI_GRAY_BG;       break;
         case COLOR_CLEAR_SCREEN:  out << ANSI_CLEAR_SCREEN;  break;
         case COLOR_MUTE:          out << ANSI_YELLOW_TX;     break;
         case COLOR_OVERFILL:      out << ANSI_HI_CYAN_TX;    break; 
         case COLOR_INTERPRETATION:out << ANSI_HI_RED_TX;     break; 
         case COLOR_TANDEM:        out << ANSI_MAGENTA_TX;    break; 
         case COLOR_MARKS:         out << ANSI_CYAN_TX;       break; 
         case COLOR_COMMENT:       out << ANSI_GREEN_TX;      break; 
         default: return colormessage(out, COLOR_INIT, mode, status);
      }

   }

   out << flush;
   return out;
}


// md5sum: cea522f0a889b56a2e92d7b301ffc6a6 humplay.cpp [20090615]

//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Apr 28 04:46:58 PDT 2001
// Last Modified: Sat Apr 28 04:47:03 PDT 2001
// Filename:      ...sig/examples/all/kern2notelist.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/kern2notelist.cpp
// Syntax:        C++; museinfo
//
// Description:   Converts kern data into the format used in 
//                Director Musices, a program that genrates
// 		  computer-based performances of musical scores.
// 

#include "humdrum.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>


// function declarations
void   checkOptions             (Options& opts, int argc, char* argv[]);
void   example                  (void);
void   usage                    (const char* command);

void convertToDM(HumdrumFile& infile);
void processTrack(HumdrumFile& infile, int track);
void convertKernNoteToDM(HumdrumFile& infile, int line, int spine, int track);
void printNote(int base40);
void printFractionDuration(double duration);
int  parseInterpretation(HumdrumFile& infile, int line, int track);
void addExtraStuff(void);
void printPitchClass(int note);
void getNoteArray(Array<int>& notes, HumdrumFile& infile, int line, int spine);

// user interface variables
Options      options;            // database for command-line arguments

// other variables:
int key = -1;      // for storing the key  (base 40 number)
int nameinit = 0;
char name[128] = {0};
int bar = 0;       // bar number
int mode = -1;     // for storing the mode (0 = major, 1 = minor)
float metronome = -1.0;  // 
int metertop = -1;  // 
int meterbottom = -1; //
int start = -1;
int terminus = -1;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   HumdrumFile infile;

   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   Array<double> absbeat;
   Array<int> pitch;
   Array<int> testset;
   Array<double> duration;
   Array<double> level;
   Array<double> coef;

   for (int i=0; i<numinputs || i==0; i++) {
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
      } else {
         infile.read(options.getArg(i+1));
      }
      infile.analyzeRhythm();
      convertToDM(infile);
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// convertToDM -- convert kern spines into Director musices format.
//

void convertToDM(HumdrumFile& infile) {
   for (int i=infile.getMaxTracks()-1; i>= 0; i--) {
      // set global state variables to uninitialized
      key = -1;
      nameinit = 0;
      name[0] = '\0';
      mode = -1;
      metronome = -1;
      metertop = -1;
      meterbottom = -1;
      bar = 0;

      processTrack(infile, i+1);
   }
}



//////////////////////////////
//
// processTrack -- print out the specified track
//

void processTrack(HumdrumFile& infile, int track) {
   int status;
   int i, j;
   for (i=0; i<infile.getNumLines(); i++) {
      if (start > i && track != infile.getMaxTracks()) {
         // skip over global comments after the first time:
         i = start;
      }
      if (terminus < i && terminus != -1 && track != 1) {
         // skip over global comments after the first time:
         break;
      }
      switch (infile[i].getType()) {
         case E_humrec_data:
            for (j=0; j<infile[i].getFieldCount(); j++) {
               if (infile[i].getPrimaryTrack(j) == track) {
                  convertKernNoteToDM(infile, i, j, track);
                  break;  // don't process second part of split track
               }
            }
            break;
         case E_humrec_bibliography:
            cout << ";;;" << (char*)&infile[i][0][3] << "\n";
            break;
         case E_humrec_global_comment:
            cout << ";;" << (char*)&infile[i][0][2] << "\n";
            break;
         case E_humrec_data_comment:
            for (j=0; j<infile[i].getFieldCount(); j++) {
               if (infile[i].getPrimaryTrack(j) == track) {
                  cout << ";" << (char*)&infile[i][j][1] << "\n";
                  break;  // don't process second part of split track
               }
            }
            break;
         case E_humrec_data_measure:
            break;
         case E_humrec_interpretation:
            status = parseInterpretation(infile, i, track);
            if (status == 0) {
               return;
            }
            if (status == 1) {
               cout << "mono-track\n";     // print the start marker of a track
            }
         default: ;
      }
   }
}



//////////////////////////////
//
// parseInterpretation --
//

int parseInterpretation(HumdrumFile& infile, int line, int track) {
   int i;
   int status = 2;
   int length = 0;
   if ((terminus == -1) && (strcmp(infile[line][0], "*-") == 0)) {
      terminus = line;
   }
   for (i=0; i<infile[line].getFieldCount(); i++) {
      if (infile[line].getPrimaryTrack(i) == track) {
         length = strlen(infile[line][i]);
         if (strncmp(infile[line][i], "**", 2) == 0) {
            if (start == -1) {
               start = line;
            }
            if (strcmp(infile[line][i], "**kern") != 0) {
               // don't translate non-musical data.
               return 2;
            } else {
               status = 1;
            }
         } else if (infile[line][i][0] == '*' && 
               infile[line][i][length-1] == ':') {
            key = Convert::kernToBase40(infile[line][i]);
            if (isupper(infile[line][i][1])) {
               mode = 0;
            } else if (islower(infile[line][i][1])) {
               mode = 1;
            } else {
               mode = -1;
            }
         } else if (strncmp(infile[line][i], "*I", 2) == 0) {
            if (islower(infile[line][i][2])) {
               strcpy(name, &infile[line][i][2]);
            }
         } else if (strncmp(infile[line][i], "*MM", 3) == 0) {
            sscanf(infile[line][i], "*MM%f", &metronome);
         } else if (strncmp(infile[line][i], "*M", 2) == 0) {
            sscanf(infile[line][i], "*M%d/%d", &metertop, &meterbottom);
         }
      }
   }
   return status;
}



//////////////////////////////
//
// convertKernNoteToDM --
//

void convertKernNoteToDM(HumdrumFile& infile, int line, int spine, int track) {
   const char* element = infile[line][spine];
   double duration;
   Array<int> notes;
   if (strcmp(element, ".") == 0) {
      return;
   }
   if (infile[line].getExInterpNum(spine) != E_KERN_EXINT) {
      return;
   }
   if (nameinit == 0 && name[0] == '\0') {
      cout << " :trackname \"track " << track << "\"\n";
      nameinit = 1;
   } else if (nameinit == 0) {
      cout << " :trackname \"" << name << "\"\n";
      nameinit = 1;
   }
   cout << " (";
   if (infile[line].getBeat() == 1.0) {
      bar++;
      cout << "bar " << bar << " ";
   }
   duration = Convert::kernToDuration(infile[line][spine]);
   if (strchr(element, 'r') != NULL) {
      cout << "n (nil ";
      printFractionDuration(duration);
      cout << ") rest t";
      addExtraStuff();
      cout << ")\n";
      return;
   }
   getNoteArray(notes, infile, line, spine);
   int notecount = notes.getSize();
   cout << "n (";
   if (notecount > 1) {
      cout << "(";
   } 
   for (int i=0; i<notecount; i++){
      printNote(notes[i]);
      if (i < notecount - 1) {
         cout << " ";
      }
   }
   if (notecount > 1) {
      cout << ")";
   } 
   cout << " ";
   printFractionDuration(duration);
   cout << ")";
   if (strchr(infile[line][spine], '[') != NULL) {
      cout << " tie t";
   } else if (strchr(infile[line][spine], '_') != NULL) {
      cout << " tie t";
   }
   addExtraStuff();
   cout << ")\n";
}



//////////////////////////////
//
// getNoteArray --
//

void getNoteArray(Array<int>& notes, HumdrumFile& infile, int line, int spine){
   int notecount = infile[line].getTokenCount(spine);
   char buffer[128] = {0};
   notes.setSize(0);
   int note;
   notes.allowGrowth(1);
   for (int i=0; i<notecount; i++) {
      infile[line].getToken(buffer, spine, i, 128);
      note = Convert::kernToBase40(buffer);
      notes.append(note);
   }
   notes.allowGrowth(0);

}


//////////////////////////////
//
// addExtraStuff --
//

void addExtraStuff(void) {
   if (key != -1) {
      cout << " key \"";
      printPitchClass(key);
      cout << "\"";
   }
   if (mode == 0) {
      cout << " modus \"maj\"";
   } else if (mode == 1) {
      cout << " modus \"min\"";
   }

   if (metronome > 0) {
      cout << " mm " << metronome;
   }

   if (metertop != -1) {
      cout << " meter (" << metertop << " " << meterbottom << ")";
   }

   metertop = -1;
   meterbottom = -1;
   mode = -1;
   metronome = -1;
   key = -1;
}



//////////////////////////////
//
// printFractionDuration --
//

void printFractionDuration(double duration) {
   duration = duration/4.0;
   double tolerance = 0.001;
   double frac = duration - (int)duration;
   int whole = (int)(duration - frac);
   if (fabs(whole - duration) < tolerance) {
      cout << whole;
      return;
   }
   int top = 1;
   int bottom;
   while (top<257) {
      for (bottom=2; bottom<257; bottom++) {
         if (fabs((double)top/bottom - frac) < tolerance) {
            cout << top + whole * bottom;
            cout << "/";
            cout << bottom;
            return;
         }
      }
      top++;
   }
}



//////////////////////////////
//
// printPitchClass --
//

void printPitchClass(int base40) {
   switch ((base40 % 40) - 2) {	
      case -2:  cout << "Cbb";	break; case -1:  cout << "Cb";	break;
      case 0:   cout << "C";	break; case 1:   cout << "C#";	break;
      case 2:   cout << "C##";	break; case 3:   cout << "X";	break;
      case 4:   cout << "Dbb";	break; case 5:   cout << "Db";	break;
      case 6:   cout << "D";	break; case 7:   cout << "D#";	break;
      case 8:   cout << "D##";	break; case 9:   cout << "X";	break;
      case 10:  cout << "Ebb";	break; case 11:  cout << "Eb";	break;
      case 12:  cout << "E";	break; case 13:  cout << "E#";	break;
      case 14:  cout << "E##";	break; case 15:  cout << "Fbb";	break;
      case 16:  cout << "Fb";	break; case 17:  cout << "F";	break;
      case 18:  cout << "F#";	break; case 19:  cout << "F##";	break;
      case 20:  cout << "X";	break; case 21:  cout << "Gbb";	break;
      case 22:  cout << "Gb";	break; case 23:  cout << "G";	break;
      case 24:  cout << "G#";	break; case 25:  cout << "G##";	break;
      case 26:  cout << "X";	break; case 27:  cout << "Abb";	break;
      case 28:  cout << "Ab";	break; case 29:  cout << "A";	break;
      case 30:  cout << "A#";	break; case 31:  cout << "A##";	break;
      case 32:  cout << "X";	break; case 33:  cout << "Bbb";	break;
      case 34:  cout << "Bb";	break; case 35:  cout << "B";	break;
      case 36:  cout << "B#";	break; case 37:  cout << "B##";	break;
      default: cout << "X";
   }
}



//////////////////////////////
//
// printNote --
//

void printNote(int base40) {
   cout << "\"";
   printPitchClass(base40);
   cout << base40/40;   // the octave number
   cout << "\"";
}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("debug=b",           "trace input parsing");   
   opts.define("author=b",          "author of the program");   
   opts.define("version=b",         "compilation information"); 
   opts.define("example=b",         "example usage"); 
   opts.define("h|help=b",          "short description"); 
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, April 2001" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 28 April 2001" << endl;
      cout << "compiled: " << __DATE__ << endl;
      cout << MUSEINFO_VERSION << endl;
      exit(0);
   } else if (opts.getBoolean("help")) {
      usage(opts.getCommand());
      exit(0);
   } else if (opts.getBoolean("example")) {
      example();
      exit(0);
   }
}



//////////////////////////////
//
// example -- example usage of the kern2dm program
//

void example(void) {
   cout <<
   "                                                                        \n"
   << endl;
}



//////////////////////////////
//
// usage -- gives the usage statement for the quality program
//

void usage(const char* command) {
   cout <<
   "                                                                        \n"
   << endl;
}


// md5sum: de24ce4347d8c7467a806db14105547b kern2dm.cpp [20050403]

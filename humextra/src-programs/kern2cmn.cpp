//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri May  4 17:04:43 PDT 2001
// Last Modified: Fri May  4 17:04:48 PDT 2001
// Filename:      ...sig/examples/all/kern2cmn.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/kern2cmn.cpp
// Syntax:        C++; museinfo
// Reference:     http://www-ccrma.stanford.edu/CCRMA/Software/cmn/cmn-manual/cmn.html
//
// Description:   Converts kern data into the format used in CMN, 
//                a LISP program that genrates PostScript musical scores.
//
// Note: Not completed
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

void convertToCMN(HumdrumFile& infile);
void processTrack(HumdrumFile& infile, int track);
void convertKernNoteToDM(HumdrumFile& infile, int line, int spine, int track);
void printNote(int base40);
void printRest(double duration);
void printCMNDuration(double duration);
int  parseInterpretation(HumdrumFile& infile, int line, int track);
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
      cout << "(cmn\n";
      convertToCMN(infile);
      cout << ")\n";
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// convertToCMN -- convert kern spines into CMN format.
//

void convertToCMN(HumdrumFile& infile) {
   for (int i=infile.getMaxTracks()-1; i>= 0; i--) {
      // set global state variables to uninitialized
      key = -1;
      nameinit = 0;
      name[0] = '\0';
      mode = -1;
      metronome = -1;
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
   if (strcmp(infile[line][0], "*-") == 0) {
      cout << "\n";
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
               cout << " " << (char)tolower(infile[line][i][1]) << "-major\n";
            } else if (islower(infile[line][i][1])) {
               cout << " " << infile[line][i][1] << "-minor\n";
            } 
         } else if (strncmp(infile[line][i], "*I", 2) == 0) {
            if (islower(infile[line][i][2])) {
               strcpy(name, &infile[line][i][2]);
            }
         } else if (strncmp(infile[line][i], "*MM", 3) == 0) {
            sscanf(infile[line][i], "*MM%f", &metronome);
         } else if (strncmp(infile[line][i], "*M", 2) == 0) {
            int metertop;
            int meterbottom;
            sscanf(infile[line][i], "*M%d/%d", &metertop, &meterbottom);
            cout << " (meter " << metertop << " " << meterbottom << ")\n";
         } else if (strcmp(infile[line][i], "*clefG2") == 0) {
            cout << " treble\n";
         } else if (strcmp(infile[line][i], "*clefF4") == 0) {
            cout << " bass\n";
         } 
         if (strcmp(infile[line][i], "**kern") == 0) {
            cout << " staff\n";
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
   if (infile[line].getBeat() == 1.0) {
      bar++;
      cout << "\n (bar " << bar << ") ";
   }
   duration = Convert::kernToDuration(infile[line][spine]);
   if (strchr(element, 'r') != NULL) {
      cout << " ";
      printRest(duration);
      return;
   }
   getNoteArray(notes, infile, line, spine);
   int notecount = notes.getSize();
   cout << " (";
   for (int i=0; i<notecount; i++){
      printNote(notes[i]);
      if (i < notecount - 1) {
         cout << " ";
      }
   }
   cout << " ";
   printCMNDuration(duration);
   if (strchr(infile[line][spine], '[') != NULL) {
      cout << " begin-tie";
   } else if (strchr(infile[line][spine], ']') != NULL) {
      cout << " end-tie";
   } else if (strchr(infile[line][spine], '_') != NULL) {
   }
   cout << ")";
}


//////////////////////////////
//
// printRest --
//

void printRest(double duration) {
   if (fabs(duration - 4.0) < 0.001) {
      cout << "whole-rest";
   } else if (fabs(duration - 2.0) < 0.001) {
      cout << "half-rest";
   } else if (fabs(duration - 1.0) < 0.001) {
      cout << "quarter-rest";
   } else if (fabs(duration - 0.5) < 0.001) {
      cout << "eighth-rest";
   } else if (fabs(duration - 0.25) < 0.001) {
      cout << "sixteenth-rest";
   } else if (fabs(duration - 0.125) < 0.001) {
      cout << "thirty-second-rest";
   }
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
// printCMNDuration --
//

void printCMNDuration(double duration) {
   char buffer[128] = {0};
   Convert::durationToKernRhythm(buffer, duration);
   if (strcmp(buffer, "8") == 0) {
      cout << "e";
   } else if (strcmp(buffer, "8.") == 0) {
      cout << "e.";
   } else if (strcmp(buffer, "16") == 0) {
      cout << "s";
   } else if (strcmp(buffer, "16.") == 0) {
      cout << "s.";
   } else if (strcmp(buffer, "4") == 0) {
      cout << "q";
   } else if (strcmp(buffer, "4.") == 0) {
      cout << "q.";
   } else if (strcmp(buffer, "2") == 0) {
      cout << "h";
   } else if (strcmp(buffer, "2.") == 0) {
      cout << "h.";
   } else if (strcmp(buffer, "1") == 0) {
      cout << "w";
   } else if (strcmp(buffer, "1.") == 0) {
      cout << "w.";
   }
   
}



//////////////////////////////
//
// printPitchClass --
//

void printPitchClass(int base40) {
   switch ((base40 % 40) - 2) {	
      case -2:  cout << "cff";	break; case -1:  cout << "cf";	break;
      case 0:   cout << "c";	break; case 1:   cout << "cs";	break;
      case 2:   cout << "css";	break; case 3:   cout << "x";	break;
      case 4:   cout << "dff";	break; case 5:   cout << "df";	break;
      case 6:   cout << "d";	break; case 7:   cout << "ds";	break;
      case 8:   cout << "dss";	break; case 9:   cout << "x";	break;
      case 10:  cout << "eff";	break; case 11:  cout << "ef";	break;
      case 12:  cout << "e";	break; case 13:  cout << "es";	break;
      case 14:  cout << "ess";	break; case 15:  cout << "fff";	break;
      case 16:  cout << "ff";	break; case 17:  cout << "f";	break;
      case 18:  cout << "fs";	break; case 19:  cout << "fss";	break;
      case 20:  cout << "x";	break; case 21:  cout << "gff";	break;
      case 22:  cout << "gf";	break; case 23:  cout << "g";	break;
      case 24:  cout << "gs";	break; case 25:  cout << "gss";	break;
      case 26:  cout << "x";	break; case 27:  cout << "aff";	break;
      case 28:  cout << "af";	break; case 29:  cout << "a";	break;
      case 30:  cout << "as";	break; case 31:  cout << "ass";	break;
      case 32:  cout << "x";	break; case 33:  cout << "bff";	break;
      case 34:  cout << "bf";	break; case 35:  cout << "b";	break;
      case 36:  cout << "bs";	break; case 37:  cout << "bss";	break;
      default: cout << "x";
   }
}



//////////////////////////////
//
// printNote --
//

void printNote(int base40) {
   printPitchClass(base40);
   cout << base40/40;   // the octave number
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


// md5sum: 7cd8a48eb583b3469648b762ea66e627 kern2cmn.cpp [20050403]

//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Oct 25 20:55:49 PDT 2004
// Last Modified: Mon Oct 25 20:55:53 PDT 2004
// Filename:      ...sig/examples/all/ottava.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/ottava.cpp
// Syntax:        C++; museinfo
//
// Description:   Convert ottava sections between sounding and score forms.
// 
// *8va = found in sounding score which indicates that the following
//        music is printed one octave lower than it sounds when printed.
// *8va/V = found in visual score which indicates that the following
//        music is one octave lower than actual sounding pitch.
// *8ba, *8ba/V = used for ottava basso indications.
// *15ma, *15ma/V = used for quintessima (two octaves up) indications.
// *15ba, *15ba/V = used for quintessima basso (two octaves down) indications.
//
// Signifiers cancelled by adding an "X" in front of the prevailing
// ottava marker: *X8va, *X8va/V, *X8ba, *X8ba/V, *X15ma, *X15ma/V, 
// *X15ba, *X15ba/V.
//

#include "humdrum.h"

#ifndef OLDCPP
   using namespace std;
#endif

///////////////////////////////////////////////////////////////////////////

#define TOSOUND   1
#define TOVISUAL -1

// function declarations
void      checkOptions       (Options& opts, int argc, char* argv[]);
void      example            (void);
void      usage              (const char* command);
void      processFile        (HumdrumFile& infile);
void      checkLineForOttavas(HumdrumFile& infile, int index, 
                              Array<int>& states);
void      printDataLine      (HumdrumFile& infile, int line, 
                              Array<int>& octavestate, int direction);
void      printNoteData      (HumdrumRecord& dataline, int index, 
                              int transpose);
void      printTandemInterpretation(HumdrumFile& infile, int line, 
                              int direction);

// global variables
Options   options;            // database for command-line arguments
int       direction = 0;      // used with -v (+1) and -s (-1) arguments


///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   HumdrumFile infile;

   // process the command-line options
   checkOptions(options, argc, argv);

   const char* filename;
   infile.clear();
   // if no command-line arguments read data file from standard input
   int numinputs = options.getArgCount();
   if (numinputs < 1) {
      infile.read(cin);
   } else {
      filename = options.getArg(1);
      infile.read(options.getArg(1));
   }
   processFile(infile);

}

///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile) {
   Array<int> octavestate;
   int maxtracks = infile.getMaxTracks();
   octavestate.setSize(maxtracks);
   octavestate.allowGrowth(0);
   octavestate.setAll(0);

   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() == E_humrec_interpretation) {
         checkLineForOttavas(infile, i, octavestate);
      } 

      if (infile[i].getType() == E_humrec_data) {
         printDataLine(infile, i, octavestate, direction);
      } else if (infile[i].getType() == E_humrec_interpretation) {
         printTandemInterpretation(infile, i, direction);
      } else {
         cout << infile[i];
      }
      cout << "\n";
   }
}



//////////////////////////////
//
// printTandemInterpretation --
//

void printTandemInterpretation(HumdrumFile& infile, int line, int direction) {
   int i;
   for (i=0; i<infile[line].getFieldCount(); i++) {
      if (strcmp(infile[line].getExInterp(i), "**kern") == 0) {
         if (direction == TOSOUND) {

            if (strcmp(infile[line][i], "*8va/V") == 0) {
               cout << "*8va";
            } else if (strcmp(infile[line][i], "*X8va/V") == 0) {
               cout << "*X8va";
            } else if (strcmp(infile[line][i], "*8ba/V") == 0) {
               cout << "*8ba";
            } else if (strcmp(infile[line][i], "*X8ba/V") == 0) {
               cout << "*X8ba";
            } else if (strcmp(infile[line][i], "*15ma/V") == 0) {
               cout << "*15ma";
            } else if (strcmp(infile[line][i], "*X15ma/V") == 0) {
               cout << "*X15ma";
            } else if (strcmp(infile[line][i], "*15ba/V") == 0) {
               cout << "*15ba";
            } else if (strcmp(infile[line][i], "*X15ba/V") == 0) {
               cout << "*X15ba";
            } else {
               cout << infile[line][i];
            }

         } else if (direction == TOVISUAL) {

            if (strcmp(infile[line][i], "*8va") == 0) {
               cout << "*8va/V";
            } else if (strcmp(infile[line][i], "*X8va") == 0) {
               cout << "*X8va/V";
            } else if (strcmp(infile[line][i], "*8ba") == 0) {
               cout << "*8ba/V";
            } else if (strcmp(infile[line][i], "*X8ba") == 0) {
               cout << "*X8ba/V";
            } else if (strcmp(infile[line][i], "*15ma") == 0) {
               cout << "*15ma/V";
            } else if (strcmp(infile[line][i], "*X15ma") == 0) {
               cout << "*X15ma/V";
            } else if (strcmp(infile[line][i], "*15ba") == 0) {
               cout << "*15ba/V";
            } else if (strcmp(infile[line][i], "*X15ba") == 0) {
               cout << "*X15ba/V";
            } else {
               cout << infile[line][i];
            }

         }
      } else {
         cout << infile[line][i];
      }

      if (i < infile[line].getFieldCount()-1) {
         cout << "\t";
      }
   }


}



//////////////////////////////
//
// printDataLine --
//

void printDataLine(HumdrumFile& infile, int line, Array<int>& octavestate,
      int direction) {
   int i;
   int ptrack = 0;
   int transpose = 0;
   for (i=0; i<infile[line].getFieldCount(); i++) {
      if (strcmp(infile[line][i], ".") == 0) {
         cout << ".";
      } else if (strcmp(infile[line].getExInterp(i), "**kern") == 0) {
         ptrack = infile[line].getPrimaryTrack(i) - 1;
         transpose = octavestate[ptrack] * direction;
         printNoteData(infile[line], i, transpose);
      } else {
         cout << infile[line][i];
      }

      if (i < infile[line].getFieldCount() - 1) {
         cout << "\t";
      }

   }

}



//////////////////////////////
//
// printNoteData --
//

void printNoteData(HumdrumRecord& dataline, int index, int transpose) {
   if (transpose == 0) {
      cout << dataline[index];
      return;
   }
 
   int tokencount = dataline.getTokenCount(index);
   int i;
   int j;
   int pitch = 0;
   int printQ = 0;
   int slen;
   char buffer[256] = {0};
   char newpitch[32] = {0};
   for (i=0; i<tokencount; i++) {
      dataline.getToken(buffer, index, i);
      if (strchr(buffer, 'r') != NULL) {
         cout << buffer;
      } else {
         pitch = Convert::kernToBase40(buffer);
         pitch = pitch + transpose * 40;
         slen = strlen(buffer);
         printQ = 0;
         for (j=0; j<slen; j++) {
            if (toupper(buffer[j]) == 'A' || 
                toupper(buffer[j]) == 'B' || 
                toupper(buffer[j]) == 'C' || 
                toupper(buffer[j]) == 'D' || 
                toupper(buffer[j]) == 'E' || 
                toupper(buffer[j]) == 'F' || 
                toupper(buffer[j]) == 'G' || 
                buffer[j] == '#' || 
                buffer[j] == '-'  ) {
                if (printQ == 0) {
                   Convert::base40ToKern(newpitch, pitch);
                   cout << newpitch;
                   printQ = 1;
                }
             } else {
                cout << buffer[j];
             }
         }
      }

      if (i < tokencount-1)  {
         cout << ' ';
      }
   }
}



//////////////////////////////
//
// checkLineForOttavas --
//  *8va = up one octave
//  *8ba = down one octave
//  *15ma, *15va = up two octaves
//  *15ba = down two octaves
//

void checkLineForOttavas(HumdrumFile& infile, int index, Array<int>& states) {
   int j;
   for (j=0; j<infile[index].getFieldCount(); j++) {
      if (direction == TOSOUND) {

         if (strcmp(infile[index][j], "*8va/V") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = +1;
         } else if (strcmp(infile[index][j], "*X8va/V") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = 0;
         } else if (strcmp(infile[index][j], "*8ba/V") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = -1;
         } else if (strcmp(infile[index][j], "*X8ba/V") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = 0;
         } else if (strcmp(infile[index][j], "*15ma/V") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = +2;
         } else if (strcmp(infile[index][j], "*X15ma/V") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = 0;
         } else if (strcmp(infile[index][j], "*15va/V") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = +2;
         } else if (strcmp(infile[index][j], "*X15va/V") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = 0;
         } else if (strcmp(infile[index][j], "*15ba/V") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = -2;
         } else if (strcmp(infile[index][j], "*X15ba/V") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = 0;
         }

      } else if (direction == TOVISUAL) {

         if (strcmp(infile[index][j], "*8va") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = +1;
         } else if (strcmp(infile[index][j], "*X8va") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = 0;
         } else if (strcmp(infile[index][j], "*8ba") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = -1;
         } else if (strcmp(infile[index][j], "*X8ba") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = 0;
         } else if (strcmp(infile[index][j], "*15ma") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = +2;
         } else if (strcmp(infile[index][j], "*X15ma") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = 0;
         } else if (strcmp(infile[index][j], "*15va") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = +2;
         } else if (strcmp(infile[index][j], "*X15va") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = 0;
         } else if (strcmp(infile[index][j], "*15ba") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = -2;
         } else if (strcmp(infile[index][j], "*X15ba") == 0) {
            states[infile[index].getPrimaryTrack(j)-1] = 0;
         }

      }
   }


}




//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("p|print|v|visual=b", "convert to printed visual score format");
   opts.define("s|sound=b", "convert to sounding score format");
   opts.define("debug=b");           // determine bad input line num
   opts.define("author=b");          // author of program
   opts.define("version=b");         // compilation info
   opts.define("example=b");         // example usages
   opts.define("h|help=b");          // short description
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Oct 2004" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 25 Oct 2004" << endl;
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

   if (opts.getBoolean("sound")) {
      direction = +1;
   } else if (opts.getBoolean("visual")) {
      direction = -1;
   }

   if (direction == 0) {
      cout << "Error: specify -v to convert to visual score or -s ";
      cout << "for sounding score\n";
      exit(1);
   }
}



//////////////////////////////
//
// example -- example usage of the program
//

void example(void) {
   cout <<
   "                                                                         \n"
   << endl;
}



//////////////////////////////
//
// usage -- gives the usage statement for the program
//

void usage(const char* command) {
   cout <<
   "                                                                         \n"
   << endl;
}



// md5sum: c4481c6932f208dde76d06b4c1d8ae0e ottava.cpp [20110215]

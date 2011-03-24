//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 31 04:21:10 PDT 2010
// Last Modified: Sun Oct 31 04:21:17 PDT 2010
// Filename:      ...sig/examples/all/motive.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/motive.cpp
// Syntax:        C++; museinfo
//
// Description: Tool for motive identification in monophonic or single-voice
// 		polyphonic scores.
// 

#include "humdrum.h"

#ifndef OLDCPP
   using namespace std;
#endif
   
#define REST    -1000
#define INVALID -7000

class Coord {
   public: 
      int row;
      int col;
      Coord(void) { row = col = -1; }
};

// function declarations
void      checkOptions       (Options& opts, int argc, char* argv[]);
void      example            (void);
void      usage              (const char* command);
void      doDiatonicIntervalAnalysis(HumdrumFile& infile);
void      getDiatonicIntervals(Array<Array<int> >& intervals, 
                               Array<Array<Coord> >& coords, 
                               HumdrumFile& infile);
void      printDiatonicIntervals(Array<Array<int> >& intervals);
void      printDiatonicData(HumdrumFile& infile, Array<Array<int> >& intervals, 
                               Array<Array<Coord> >& coords);
void      printDiatonicLine   (HumdrumFile& infile, int i, 
                               Array<Array<int> >& intervals, 
                               const char* strang);


// global variables
Options   options;             // database for command-line arguments
int       debugQ = 0;          // used with --debug option
int       zeroQ  = 0;          // used with -z option
int       restQ  = 0;          // used with -r option
int       outputonlyQ = 0;     // used with -I option
int       printDiatonicIntervalsQ  = 1; // used with -D option


///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   HumdrumFile infile;

   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   for (int i=0; i<numinputs || i==0; i++) {
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
      } else {
         infile.read(options.getArg(i+1));
      }
      infile.analyzeRhythm();

      doDiatonicIntervalAnalysis(infile);
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// doDiatonicIntervalAnalysis --
//


void doDiatonicIntervalAnalysis(HumdrumFile& infile) {
   Array<Array<int> > intervals;
   Array<Array<Coord> > coords;
   getDiatonicIntervals(intervals, coords, infile);
   // printDiatonicIntervals(intervals);
   if (printDiatonicIntervalsQ) {
      printDiatonicData(infile, intervals, coords);
      exit(0);
   }
}


//////////////////////////////
//
// printDiatonicData --
//

void printDiatonicData(HumdrumFile& infile, Array<Array<int> >& intervals, 
      Array<Array<Coord> >& coords) {

   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
         case E_humrec_data_comment:
            printDiatonicLine(infile, i, intervals, "!");
            break;
         case E_humrec_data_kern_measure:
            printDiatonicLine(infile, i, intervals, infile[i][0]);
            break;
         case E_humrec_interpretation:
            if (strncmp(infile[i][0], "**", 2) == 0) {
               printDiatonicLine(infile, i, intervals, "**dint");
            } else if (strcmp(infile[i][0], "*-") == 0) {
               printDiatonicLine(infile, i, intervals, "*-");
            }
            break;
         case E_humrec_data:
            printDiatonicLine(infile, i, intervals, ".");
            break;
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
         default:
            cout << infile[i] << endl;
      }
   }
}



//////////////////////////////
//
// printDiatonicLine --
//

void printDiatonicLine(HumdrumFile& infile, int i, 
      Array<Array<int> >& intervals, const char* strang) {
   int j;
   int tcounter = 0;
   int track;
   int track2;
   int value;
   for (j=0; j<infile[i].getFieldCount(); j++) {
      if (!outputonlyQ) {
         cout << infile[i][j];
      }
      if (strcmp(infile[i].getExInterp(j), "**kern") == 0) {
         track = infile[i].getPrimaryTrack(j);
         track2 = -10000;
         if (j < infile[i].getFieldCount()-1) {
            track2 = infile[i].getPrimaryTrack(j+1);
         }
         if (track != track2) {
            if (outputonlyQ && tcounter) {
               cout << "\t";
            } else if (!outputonlyQ) {
               cout << "\t";
            }
            tcounter++;
            if (intervals[i][j] >= REST) {
               value = intervals[i][j];
               if (restQ && (value == REST)) {
                  cout << "r";
               } else {
                  if (!zeroQ) {
                     if (value > 0) {
                        value++;
                     } else if (value < 0) {
                        value--;
                     }
                  }
                  if (value > 0) {
                     cout << "+";
                  }
                  cout << value;
               }
            } else {
               if ((strcmp(strang, "*") == 0) && 
                   (strncmp(infile[i][j], "*staff", strlen("*staff")) == 0)) {
                  cout << infile[i][j];
               } else {
                  cout << strang;
               }
            }
         }
      }
      if (!outputonlyQ) {
         if (j < infile[i].getFieldCount()-1) {
            cout << "\t";
         }
      }
   }
   cout << endl;
}



//////////////////////////////
//
// printDiatonicIntervals --
//

void printDiatonicIntervals(Array<Array<int> >& intervals) {
   int i, j;
   for (i=0; i<intervals.getSize(); i++) {
      for (j=0; j<intervals[i].getSize(); j++) {
         cout << intervals[i][j] << "\n";
      }
      if (i < intervals.getSize()-1) {
         cout << "\n";
      }
   }   

}



//////////////////////////////
//
// getDiatonicIntervals -- ignoring rests for now.
//

void getDiatonicIntervals(Array<Array<int> >& intervals, 
      Array<Array<Coord> >& coords, HumdrumFile& infile) {
   int maxtrack = infile.getMaxTracks();
   int track;
   int pitch;
   Coord coord;
   intervals.setSize(infile.getNumLines());
   coords.setSize(maxtrack);
   int i, j;
   // pre-allocate storage space for data:
   for (i=0; i<maxtrack; i++) {
      coords[i].setSize(infile.getNumLines());
      coords[i].setSize(0);
   }
   for (i=0; i<infile.getNumLines(); i++) {
      intervals[i].setSize(infile[i].getFieldCount());
      intervals[i].allowGrowth(0);
      intervals[i].setAll(INVALID);
   }

   for (i=0; i<infile.getNumLines(); i++) {
      if (debugQ) {
         cout << "Processing input line " << i + 1 << "..." << endl;
      }
      if (!infile[i].isData()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strcmp(infile[i].getExInterp(j), "**kern") != 0) {
            continue;
         }
         track = infile[i].getPrimaryTrack(j)-1;
         if (strchr(infile[i][j], 'r') != NULL) {
            if (restQ) {
               pitch = REST;
               intervals[i][j] = pitch;
               coord.row = i;
               coord.col = j;
               coords[track].append(coord);
            }
            continue;
         }
         if (strcmp(infile[i][j], ".") == 0) {
            continue;
         }
         if (strchr(infile[i][j], '_') != NULL) {
            // middle tie note
            continue;
         }
         if (strchr(infile[i][j], ']') != NULL) {
            // ending tied note
            continue;
         }
         track = infile[i].getPrimaryTrack(j) - 1;
         pitch = Convert::kernToBase40(infile[i][j]);
         pitch = pitch/40 * 7 + Convert::base40ToDiatonic(pitch) % 7;
         coord.row = i;
         coord.col = j;
         intervals[i][j] = pitch;
         coords[track].append(coord);
      }
   }

   if (debugQ) {
      cout << "FINISHED EXTRACTING DATA FROM FILE" << endl;
   }

   Array<Array<Coord> >& c = coords;
   int lasti, lastj;

   // convert absolute diatonic pitch into diatonic interval.
   if (restQ) {
      for (i=0; i<c.getSize(); i++) {
         for (j=0; j<c[i].getSize()-1; j++) {
            if (intervals[c[i][j].row][c[i][j].col] == REST) {
               continue;
            }
            if (intervals[c[i][j+1].row][c[i][j+1].col] == REST) {
               intervals[c[i][j].row][c[i][j].col] = INVALID;
               continue;
            }
            intervals[c[i][j].row][c[i][j].col] = 
                  intervals[c[i][j+1].row][c[i][j+1].col] 
                  - intervals[c[i][j].row][c[i][j].col];
         }
         lasti = c[i][c[i].getSize()-1].row;
         lastj = c[i][c[i].getSize()-1].col;
         intervals[lasti][lastj] = INVALID;
         c[i].setSize(c[i].getSize()-1);
      }
   } else {
      for (i=0; i<c.getSize(); i++) {
         for (j=0; j<c[i].getSize()-1; j++) {
            intervals[c[i][j].row][c[i][j].col] = 
                  intervals[c[i][j+1].row][c[i][j+1].col] 
                  - intervals[c[i][j].row][c[i][j].col];
         }
         lasti = c[i][c[i].getSize()-1].row;
         lastj = c[i][c[i].getSize()-1].col;
         intervals[lasti][lastj] = INVALID;
         c[i].setSize(c[i].getSize()-1);
      }
   }

   int value;
   int writepos = 0;
   if (restQ) {
      // remove invalid intervals from list
      for (i=0; i<c.getSize(); i++) {
         if (c[i].getSize() == 0) {
            continue;
         }
         writepos = 0;
         for (j=0; j<c[i].getSize(); j++) {
            value = intervals[c[i][j].row][c[i][j].col];
            if (value == INVALID) {
               continue;
            } else {
               c[i][writepos].row = c[i][j].row;
               c[i][writepos].col = c[i][j].col;
               writepos++;
            }
         }
         c[i].setSize(writepos);
      }
   }

   if (debugQ) {
      cout << "FINISHED CREATING INTERVALS" << endl;
   }
}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("I=b", "display output without echoing input data");  
   opts.define("z|zero=b", "display diatonic intervals zero-offset");  
   opts.define("r|rest=b", "do not cross rests");  

   opts.define("debug=b");              // determine bad input line num
   opts.define("author=b");             // author of program
   opts.define("version=b");            // compilation info
   opts.define("example=b");            // example usages
   opts.define("h|help=b");             // short description
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Oct 2010" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 31 Oct 2010" << endl;
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

   debugQ = opts.getBoolean("debug");
   zeroQ  = opts.getBoolean("zero");
   restQ  = opts.getBoolean("rest");
   outputonlyQ = opts.getBoolean("I");

}



//////////////////////////////
//
// example -- example usage of the quality program
//

void example(void) {
   cout <<
   "                                                                         \n"
   "# example usage of the meter program.                                    \n"
   "# analyze a Bach chorale for meter position:                             \n"
   "     meter chor217.krn                                                   \n"
   "                                                                         \n"
   "# display the metrical location spine with original data:                \n"
   "     meter -a chor217.krn                                                \n"
   "                                                                         \n"
   << endl;
}



//////////////////////////////
//
// usage -- gives the usage statement for the meter program
//

void usage(const char* command) {
   cout <<
   "                                                                         \n"
   "                                                                         \n"
   << endl;
}



// md5sum: 915382eb3b12ff8e7cc66b07c64fd8c5 motive.cpp [20101226]

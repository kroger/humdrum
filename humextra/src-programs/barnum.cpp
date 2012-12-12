//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Sep  9 21:30:46 PDT 2004
// Last Modified: Thu Sep  9 21:30:48 PDT 2004
// Filename:      ...sig/examples/all/barnum.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/barnum.cpp
// Syntax:        C++; museinfo
//
// Description:   Number, renumber or remove measure numbers from Humdrum files.
//

#include "humdrum.h"

#include <string.h>
#include <ctype.h>

#ifndef OLDCPP
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #ifdef VISUAL
      #include <strstrea.h>     /* for windows 95 */
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif
   

// function declarations
void      checkOptions       (Options& opts, int argc, char* argv[]);
void      example            (void);
void      usage              (const char* command);
void      processFile        (HumdrumFile& file);
void      removeBarNumbers   (HumdrumFile& infile);
void      printWithoutBarNumbers(HumdrumRecord& humrecord);
void      printWithBarNumbers(HumdrumRecord& humrecord, int measurenum);
void      printSingleBarNumber(const char* string, int measurenum);

// global variables
Options   options;            // database for command-line arguments
int       removeQ  = 0;       // used with -r option
int       startnum = 1;       // used with -s option
int       allQ     = 0;       // used with -a option
int       debugQ   = 0;       // used with --debug option


///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   HumdrumFile infile, outfile;

   // process the command-line options
   checkOptions(options, argc, argv);

   // if no command-line arguments read data file from standard input
   int numinputs = options.getArgCount();
   if (numinputs < 1) {
      infile.read(cin);
   } else {
      infile.read(options.getArg(1));
   }
   
   if (removeQ) {
      removeBarNumbers(infile);
   } else{
      processFile(infile);
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


///////////////////////////////
//
// removeBarNumbers -- You guessed it.
//

void removeBarNumbers(HumdrumFile& infile) {
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() != E_humrec_data_measure) {
         cout << infile[i] << "\n";
         continue;
      }
      printWithoutBarNumbers(infile[i]);
   }
}



//////////////////////////////
//
// printWithoutBarNumbers --
//

void printWithoutBarNumbers(HumdrumRecord& humrecord) {
   int i;
   int j;
   int length;

   for (i=0; i<humrecord.getFieldCount(); i++) {
      if (humrecord[i][0] != '=') {
         cout << humrecord[i];
      } else {
         length = strlen(humrecord[i]);
         for (j=0; j<length; j++) {
            if (!isdigit(humrecord[i][j])) {
               cout << humrecord[i][j];
            }
         }
      }
      if (i < humrecord.getFieldCount()-1) {
         cout << "\t";
      }
   }
   cout << "\n";
}






//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile) {
   infile.analyzeRhythm("4");

   Array<int>    measureline;   // line number in the file where measure occur
   Array<double> measurebeats;  // duration of measure
   Array<double> timesigbeats;  // duration according to timesignature
   Array<int>    control;       // control = numbered measure
   Array<int>    measurenums;   // output measure numbers

   measureline.setSize(infile.getNumLines());
   measureline.setSize(0);
   measurebeats.setSize(infile.getNumLines());
   measurebeats.setSize(0);
   timesigbeats.setSize(infile.getNumLines());
   timesigbeats.setSize(0);

   int i;
   double timesigdur = 0.0;
   double timetop = 4;
   double timebot = 1;
   double value   = 1;
   double lastvalue = 1;
   for (i=0; i<infile.getNumLines(); i++) {
      if (debugQ) {
         cout << "LINE " << i+1 << "\t" << infile[i] << endl;
      }
      if (infile[i].getType() == E_humrec_interpretation) {
         if ((strncmp(infile[i][0], "*M", 2) == 0) 
               && (strchr(infile[i][0], '/') != NULL)) {
            timetop = Convert::kernTimeSignatureTop(infile[i][0]);
            timebot = Convert::kernTimeSignatureBottomToDuration(infile[i][0]);
            timesigdur = timetop * timebot;
            // fix last timesigbeats value
            if (timesigbeats.getSize() > 0) {
               timesigbeats[timesigbeats.getSize()-1] = timesigdur;
               measurebeats[measurebeats.getSize()-1] = lastvalue * timebot;
            }
         }
      } else if (infile[i].getType() == E_humrec_data_measure) {
         measureline.append(i);
         lastvalue = infile[i].getBeat();
         // shouldn't use timebot (now analyzing rhythm by "4")
         // value = lastvalue * timebot;
         value = lastvalue;
         measurebeats.append(value);
         timesigbeats.append(timesigdur);
      }
   }

   if (measurebeats.getSize() == 0) {
      // no barlines, nothing to do...
      cout << infile;
      return;
   }

   // Identify controlling/non-controlling barlines
   // at each measure line determine one of three cases:
   // 
   // (1) all ok -- the summation of durations in the measure
   //     matches the current time sign
   // (2) a partial measure -- the measure durations do not
   //     add up to the time signature, but the measure is
   //     at the start/end of a musical section such as the
   //     beginning of a piece, end of a piece, or between
   //     repeat bar dividing a full measure.
   // (3) the sum of the durations does not match the 
   //     time signature because the durations are incorrectly
   //     given.
   //

   control.setSize(measureline.getSize());
   measurenums.setSize(measureline.getSize());
   control.setAll(-1);
   measurenums.setAll(-1);

   // if the time signature and the number of beats in a measure
   // agree, then the bar is worth numbering:
   for (i=0; i<control.getSize(); i++) {
      if (measurebeats[i] == timesigbeats[i]) {
         control[i] = 1;
      }
   }

   // determine first bar (which is marked with a negative value)
   // if there is a pickup bar
   if (measurebeats[0] < 0) {
      if (-measurebeats[0] == timesigbeats[0]) {
         control[0] = 1;
      }
   }

   // Check for intermediate barlines which split one measure
   for (i=control.getSize()-2; i>=0; i--) {
      if ((control[i] == 1) || (control[i+1] == 1)) {
         continue;
      }
      if (timesigbeats[i] != timesigbeats[i+1]) {
         continue;
      }
      if ((measurebeats[i]+measurebeats[i+1]) == timesigbeats[i]) {
         control[i] = 1;
         control[i+1] = 0;
      }
   }

   // if two (or more) non-controlling bars occur in a row, then
   // make them controlling:
   for (i=0; i<control.getSize()-1; i++) {
      if ((control[i] < 1) && (control[i+1] < 1)) {
         while ((i < control.getSize()) && (control[i] < 1)) {
            control[i++] = 1;
         }
      }
   }

   // if a measure contains no beats, then it is not a controlling barline
   for (i=0; i<control.getSize(); i++) {
      if (measurebeats[i] == 0) {
         control[i] = 0;
      }
   }

   // if the first bar is not a pickup measure, but there is no
   // starting measure, then subtract one from the starting barline
   // count;
   int offset = 0;
   int dataq = 0;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isData()) {
         dataq = 1;
         continue;
      }
      if (infile[i].getType() == E_humrec_data_measure) {
         if ((measurebeats[0] > 0) && dataq) {
            offset = 1;
         }
         break;
      }
   }

   if (allQ) {
      control.setAll(1);
      offset = 0;
   }

   // assign the measure numbers;
   int mnum = startnum + offset;
   for (i=0; i<measurenums.getSize(); i++) {
      if (control[i] == 1) {
         measurenums[i] = mnum++;
      }
   }

   if (debugQ) {
      cout << "cont\tnum\tbeats" << endl;
      for (i=0; i<control.getSize(); i++) {
         cout << control[i] << "\t" << measurenums[i] << "\t"
              << measurebeats[i] << "\t" << timesigbeats[i] << endl;
      }
   }


   // ready to print the new barline numbers
   int mindex = 0;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() != E_humrec_data_measure) {
         cout << infile[i] << "\n";
         continue;
      }

      if (measurenums[mindex] < 0) {
         printWithoutBarNumbers(infile[i]);
      } else {
         printWithBarNumbers(infile[i], measurenums[mindex]);
      }

      mindex++;
   }

}



//////////////////////////////
//
// printWithBarNumbers --
//

void printWithBarNumbers(HumdrumRecord& humrecord, int measurenum) {
   int i;
   for (i=0; i<humrecord.getFieldCount(); i++) {
       printSingleBarNumber(humrecord[i], measurenum);
       if (i < humrecord.getFieldCount() -1) {
          cout << "\t";
       }
   }
   cout << "\n";
}


//////////////////////////////
//
// printSingleBarNumber --
//

void printSingleBarNumber(const char* string, int measurenum) {
   int length = strlen(string);
   int i;
   for (i=0; i<length; i++) {
      if (string[i] == '=' && string[i+1] != '=') {
         cout << string[i] << measurenum;
      } else if (!isdigit(string[i])) {
         cout << string[i];
      }
   }
}




//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

  
void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("r|remove=b", "Remove barlines from the file");             
   opts.define("s|start=i:1", "starting barline number");             
   opts.define("a|all=b",     "print numbers on all barlines");

   opts.define("debug=b");                // print debug info
   opts.define("author=b");               // author of program
   opts.define("version=b");              // compilation info
   opts.define("example=b");              // example usages
   opts.define("h|help=b");               // short description
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Sep 2004" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 10 Sep 2004" << endl;
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

   removeQ  = opts.getBoolean("remove");
   startnum = opts.getInteger("start");
   debugQ   = opts.getBoolean("debug");
   allQ     = opts.getBoolean("all");

}
  


//////////////////////////////
//
// example -- example usage of the quality program
//

void example(void) {
   cout <<
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
   << endl;
}



// md5sum: dc8a875e92aded1fc1ae7353f83977fa barnum.cpp [20050403]

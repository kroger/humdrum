//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Mar 23 19:44:07 PDT 2011
// Last Modified: Wed Mar 23 19:44:10 PDT 2011
// Filename:      ...sig/examples/all/onset.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/onset.cpp
// Syntax:        C++; museinfo
//
// Description:   Analyzes **kern data for onset data.
//

#include "humdrum.h"

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif

// function declarations
void        checkOptions          (Options& opts, int argc, char* argv[]);
void        example               (void);
void        usage                 (const char* command);
void        processData           (HumdrumFile& infile, 
                                   RationalNumber& sampledur);
void        printBrief            (Array<int> positions, Array<int> mpositions, 
                                   HumdrumFile& infile, 
                                   RationalNumber& sampledur);
void        printScore            (Array<int> positions, Array<int> mpositions, 
                                   HumdrumFile& infile, 
                                   RationalNumber& sampledur);

// global variables
Options        options;           // database for command-line arguments
RationalNumber Sampledur;         // used with -t option
int            timebaseQ = 0;     // used with -t option
int            briefQ = 0;        // used with -b option
int            csvQ   = 1;        // used with -C option

///////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[]) {
   HumdrumFile infile, outfile;

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

      infile.analyzeRhythm("4");
      if (timebaseQ == 0) {
         Sampledur = infile.getMinTimeBaseR();
         Sampledur.invert();
      }
      processData(infile, Sampledur);
       
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// processData -- 
//

void processData(HumdrumFile& infile, RationalNumber& sampledur) {
   RationalNumber scoreduration = infile.getTotalDurationR();
   Array<int> positions;
   RationalNumber psize = scoreduration / sampledur;
   RationalNumber position;
   positions.setSize(int(psize.getFloat()+1));
   positions.setAll(0);
   Array<int> mpositions;  // location of measures
   mpositions.setSize(positions.getSize());
   mpositions.setAll(0);
   char buffer[1024] = {0};

   int pindex = 0;      // position index
   int rcount = 0;      // number of attacks on the line
   int acount = 0;      // number of attacks on the line
   int tcount = 0;      // for keeping track of chords
   int lastindex = 0;   // for suppressing double rest markers

   int i, j, k;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isMeasure()) {
         if (strchr(infile[i][0], '-') != NULL) {
            // don't mark invisible measures
            continue;
         }
         position = infile[i].getAbsBeatR() / sampledur;
         if (position.getDenominator() == 1) {
            pindex = position.getNumerator();
         } else {
            pindex = int(position.getFloat());
         }
         mpositions[pindex] = 1;
      }
      if (!infile[i].isData()) {
         continue;
      } 

      if (infile[i].getDuration() == 0.0) {
         // ignore grace notes.
         continue;
      }

      position = infile[i].getAbsBeatR() / sampledur;
      if (position.getDenominator() == 1) {
         pindex = position.getNumerator();
      } else {
         // not at the requested rhythmic sampling position in the file.
         continue;
      }
      rcount = 0;
      acount = 0;
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (!infile[i].isExInterp(j, "**kern")) {
            continue;
         }         
         if (strcmp(infile[i][j], ".") == 0) {
            // ignore sustained items
            continue;
         }
         if (strchr(infile[i][j], 'r') != NULL) {
            rcount++;
            continue;
         }
         tcount = infile[i].getTokenCount(j);
         for (k=0; k<tcount; k++) {
            infile[i].getToken(buffer, j, k);
            if (strchr(buffer, '_') != NULL) {
               // ignore the middle notes of ties
               continue;
            }
            if (strchr(buffer, ']') != NULL) {
               // ignore the ending notes of ties
               continue;
            }
            acount++;  // found a note onset
         }

      }
      if (acount > 0) {
         positions[pindex] = acount;
         lastindex = pindex;
      } else {
         // print the rest marker only if the last marker was not a rest
         if (positions[lastindex] >= 0) {
            positions[pindex] = -rcount;
         }
      }
   }

   if (briefQ) {
      printBrief(positions, mpositions, infile, sampledur);
   } else {
      printScore(positions, mpositions, infile, sampledur);
   }
}



//////////////////////////////
//
// printScore --
//

void printScore(Array<int> positions, Array<int> mpositions, 
      HumdrumFile& infile, RationalNumber& sampledur) {

   RationalNumber rpos;
   int position;
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      cout << infile[i];
      if (infile[i].isBibliographic()) {
         cout << "\n";
         continue;
      }
      if (infile[i].isGlobalComment()) {
         cout << "\n";
         continue;
      }
      if (infile[i].isLocalComment()) {
         cout << "\t!\n";
         continue;
      } 
      if (infile[i].isMeasure()) {
         cout << "\t" << infile[i][0] << "\n";
         continue;
      } 
      if (infile[i].isInterpretation()) {
         if (strncmp(infile[i][0], "**", 2) == 0) {
            cout << "\t**onset\n";
            continue;
         }
         if (strcmp(infile[i][0], "*-") == 0) {
            cout << "\t*-\n";
            continue;
         }
         cout << "\t*\n";
         continue;
      } 
      if (!infile[i].isData()) {
         cout << "\n";
      }

      cout << "\t";
      rpos = infile[i].getAbsBeatR() / sampledur;
      if (rpos.getDenominator() == 1) {
         position = int(rpos.getFloat());
         cout << positions[position];
      } else {
         cout << 0;
      }
      cout << "\n";
   }
}



//////////////////////////////
//
// printBrief --
//

void printBrief(Array<int> positions, Array<int> mpositions, 
      HumdrumFile& infile, RationalNumber& sampledur) {
   RationalNumber maxR = infile.getTotalDurationR() / sampledur;
   int max = int(maxR.getFloat());
   int i;

   for (i=0; i<max; i++) {
      if (mpositions[i]) {
         cout << "\n";
      }
      if (positions[i]) {
         cout << 1;
      } else {
         cout << 0;
      }
      if (csvQ && (i < mpositions.getSize()-1) && !mpositions[i+1]) {
         cout << ", ";
      }
   }
   cout << "\n";
}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("t|timebase=s:2", "rhythm at which to sample onsets");
   opts.define("b|brief=b",      "print brief form of output data");

   opts.define("author=b",     "author of program");
   opts.define("version=b",    "compilation info");
   opts.define("example=b",    "example usages");
   opts.define("h|help=b",     "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Mar 2011" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 23 Mar 2011" << endl;
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

   briefQ = opts.getBoolean("brief");
   
   timebaseQ = opts.getBoolean("timebase");

   if (strchr(opts.getString("timebase"), '/') != NULL) {
      int top = 1;
      int bottom = 4;
      sscanf(opts.getString("timebase"), "%d/%d", &bottom, &top);
      Sampledur = top;
      Sampledur /= bottom;
      Sampledur *= 4; // convert from whole note units to quarter notes
   } else {
      Sampledur = Convert::kernToDurationR(opts.getString("timebase"));
   }

}



//////////////////////////////
//
// example -- example usage of the sonority program
//

void example(void) {
   cout <<
   "                                                                         \n"
   << endl;
}



//////////////////////////////
//
// usage -- gives the usage statement for the sonority program
//

void usage(const char* command) {
   cout <<
   "                                                                         \n"
   << endl;
}



// md5sum: fba4c547c330f2cc4e940fca35cc1790 onset.cpp [20110325]

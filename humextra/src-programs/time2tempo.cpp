//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Mar 18 22:08:55 PST 2005
// Last Modified: Sat Jun  3 18:46:57 PDT 2006 (changed tempo interp direction)
// Last Modified: Sat Jun  3 18:46:57 PDT 2006 (added fill option)
// Filename:      ...sig/examples/all/time2tempo.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/time2tempo.cpp
// Syntax:        C++; museinfo
//
// Description:   
// 

#include "humdrum.h"
#include <string.h>
#include <stdio.h>

// function declarations
void   checkOptions             (Options& opts, int argc, char* argv[]);
void   example                  (void);
void   printAnalysis            (HumdrumFile& infile, Array<double>& timings,
                                 Array<double>& tempo);
void   getTimings               (HumdrumFile& infile, Array<double>& timings,
                                 Array<double>& tempo);
void   usage                    (const char* command);

// global variables
Options      options;            // database for command-line arguments
int          appendQ    = 0;     // used with -a option
int          prependQ   = 0;     // used with -p option
int          fillQ      = 0;     // used with -f option
int          roundQ     = 0;     // used with -r option
int          debugQ     = 0;     // used with --debug option

///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   HumdrumFile    infile;
   Array<double>  timings;
   Array<double>  tempo;

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

      getTimings(infile, timings, tempo);
      printAnalysis(infile, timings, tempo);
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// getTimings -- Get an array of the times for each line in the
//     input file from a **time spine.  Currently only look for
//     the first **time spine in the file.
// 

void getTimings(HumdrumFile& infile, Array<double>& timings,
      Array<double>& tempo) {
   infile.analyzeRhythm("4");

   int i;
   int j;

   timings.setSize(infile.getNumLines());
   tempo.setSize(infile.getNumLines());

   timings.setAll(-1.0);
   tempo.setAll(-1.0);

   double atime;
   int secondsQ = 0;

   // get the timing values from the **time spine.
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() == E_humrec_interpretation) {
         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (strcmp(infile[i].getExInterp(j), "**time") == 0) {
               if (strcmp(infile[i][j], "*u=sec") == 0) {
                  // measuring time in seconds.
                  secondsQ = 1;
               } else if (strcmp(infile[i][j], "*u=msec") == 0) {
                  secondsQ = 0;
               }
               break;
            }
         }
      }
      if (infile[i].getType() != E_humrec_data) {
         continue; 
      }

      for (j=0; j<infile[i].getFieldCount(); j++) {
         // just looking for the first **time spine for now...
         if (strcmp(infile[i].getExInterp(j), "**time") != 0) {
            continue;
         }
         if (strcmp(infile[i][j], ".") == 0) {
            timings[i] = -1.0;
         } else {
            if (sscanf(infile[i][j], "%lf", &atime) == 1) {
               timings[i] = atime;
               if (secondsQ) {
                  // convert seconds measurements into milliseconds
                  timings[i] *= 1000.0;
               }
            } else {
               timings[i] = -1.0;
            }
         }
         break;
      }
   }

   // the times have been extracted from the Humdrum data, so now
   // convert them into tempo markings.  

   int lastindex = 0;
   double beatdur;
   double timedur;
   double atempo;
   
   for (i=0; i<infile.getNumLines(); i++) {
      if (timings[i] < 0.0) {
         continue;
      }
      beatdur = infile[i].getAbsBeat() - infile[lastindex].getAbsBeat();
      timedur = (timings[i] - timings[lastindex])/1000.0;
      atempo  = 60.0 * beatdur / timedur;
      tempo[lastindex] = atempo;
      if (roundQ) {
         tempo[lastindex] = (int)(tempo[lastindex] + 0.5);
      }
      lastindex = i;
   }

   // set all zero tempos to be the same as the previous
   // tempo marking.
   if (fillQ) {
      for (i=1; i<infile.getNumLines(); i++) {
         if (tempo[i] <= 0.0) {
            tempo[i] = tempo[i-1];
         }
      }
   }

}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("a|append=b",   "append analysis to input data");   
   opts.define("p|prepend=b",  "prepend analysis to input data");   
   opts.define("r|round=b",    "round output tempos to nearest integer");   
   opts.define("f|fill=b",     "fill tempo spine removing null tokens");

   opts.define("debug=b",   "trace input parsing");   
   opts.define("author=b",  "author of the program");   
   opts.define("version=b", "compilation information"); 
   opts.define("example=b", "example usage"); 
   opts.define("h|help=b",  "short description"); 
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, March 2004" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 4 June 2006" << endl;
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

   debugQ   = opts.getBoolean("debug");
   appendQ  = opts.getBoolean("append");
   roundQ   = opts.getBoolean("round");
   fillQ    = opts.getBoolean("fill");
   prependQ = opts.getBoolean("prepend");
   if (appendQ) {   // exclusive options
      prependQ = 0;
   }
}



//////////////////////////////
//
// example -- example usage of the addtime program
//

void example(void) {
   cout <<
   "                                                                        \n"
   << endl;
}



//////////////////////////////
//
// printAnalysis -- 
//

void printAnalysis(HumdrumFile& infile, Array<double>& timings,
      Array<double>& tempo) {
   int tempomark = 1;   // disabled for now
   double lasttempo = -1.0;
   int i;
   int m;
   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
      case E_humrec_global_comment:
      case E_humrec_bibliography:
      case E_humrec_none:
      case E_humrec_empty:
         cout << infile[i].getLine() << "\n";
         break;
      case E_humrec_data:
         if (appendQ) {
            cout << infile[i].getLine() << "\t";
         }
         if (tempomark == 0) {
            tempomark = 1;
            cout << "*MM" << tempo[i] << "\t";
            for (m=0; m<infile[i].getFieldCount(); m++) {
               cout << "*";
               if (m < infile[i].getFieldCount() - 1) {
                  cout << "\t";
               }
            }
            cout << "\n";
         }
         if (!fillQ && tempo[i] <= 0.0) {
            cout << ".";
	 } else if (!fillQ && tempo[i] == lasttempo) {
            cout << ".";
         } else {
            cout << tempo[i];
            lasttempo = tempo[i];
         }
         if (prependQ) {
            cout << "\t" << infile[i].getLine();
         }
         cout << "\n";
         break;
      case E_humrec_data_comment:
         if (appendQ) {
            cout << infile[i].getLine() << "\t";
         }
         if (infile[i].equalFieldsQ("**kern")) {
            cout << infile[i][0];
         } else {
            cout << "!";
         }
         if (prependQ) {
            cout << "\t" << infile[i].getLine();
         }
         cout << "\n";
         break;
      case E_humrec_data_measure:
         if (appendQ) {
            cout << infile[i].getLine() << "\t";
         }
         if (infile[i].equalFieldsQ("**kern")) {
            cout << infile[i][0];
         } else {
            cout << "=";
         }
         if (prependQ) {
            cout << "\t" << infile[i].getLine();
         }
         cout << "\n";
         break;
      case E_humrec_data_interpretation:
         if (appendQ) {
            cout << infile[i].getLine() << "\t";
         }
         if (strncmp(infile[i][0], "**", 2) == 0) {
            cout << "**tempo";
         } else if (strcmp(infile[i][0], "*-") == 0) {
            cout << "*-";
         } else if (strncmp(infile[i][0], "*MM", 3) == 0) {
            tempomark = 1;
            cout << infile[i][0];
         } else {
            cout << "*";
         }
         if (prependQ) {
            cout << "\t" << infile[i].getLine();
         }
         cout << "\n";
         break;
      }

   }
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


// md5sum: 3bc6af6a088a036d00da4efc0f00977f time2tempo.cpp [20060701]

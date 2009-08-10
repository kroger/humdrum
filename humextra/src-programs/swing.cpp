//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Apr 15 20:05:01 PDT 2002
// Last Modified: Mon Apr 15 20:05:04 PDT 2002
// Filename:      ...sig/examples/all/addtime.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/addtime.cpp
// Syntax:        C++; museinfo
//
// Description:   Add a spine indicating the time of performance for
//                a given line of music.
// 

#include "humdrum.h"

#include <string.h>
#include <stdio.h>


// function declarations
void   checkOptions             (Options& opts, int argc, char* argv[]);
void   example                  (void);
void   printAnalysis            (HumdrumFile& infile, Array<double>& timings,
                                 Array<double>& tempo);
void   analyzeTiming            (HumdrumFile& infile, Array<double>& timings,
                                 Array<double>& tempo);
void   usage                    (const char* command);
void   generateSwing            (HumdrumFile& infile, Array<double>& timings, 
                                 double amount);

// global variables
Options      options;            // database for command-line arguments
int          debugQ     = 0;     // used with the --debug option
double       offset     = 0.0;   // used with the -o option
int          style      = 's';   // used with the -m option
double       dtempo     = 60.0;  // used with the -t option
double       percentage = 60.0;  // used with the -p option

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

      analyzeTiming(infile, timings, tempo);
      generateSwing(infile, timings, percentage);
      printAnalysis(infile, timings, tempo);
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// generateSwing -- generate the swing on eighth notes.
//

void generateSwing(HumdrumFile& infile, Array<double>& timings, double amount) {
   int i;
   double fraction = amount / 100.0;

   Array<int> data;
   Array<int> nextdata;
   Array<int> lastdata;
   data.setSize(timings.getSize());
   nextdata.setSize(timings.getSize());
   lastdata.setSize(timings.getSize());

   data.setAll(0);
   // nextdata.setAll(-1);
   nextdata.setAll(0);
   lastdata.setAll(0);

   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isData()) {
         data[i] = 1;
      }
   }

   int last = 0;
   for (i=0; i<data.getSize(); i++) {
      if (data[i]) {
         lastdata[i] = last;
         last = i;
      }
   }

   int next = 0;
   for (i=data.getSize()-1; i>= 0; i--) {
      if (data[i]) {
         nextdata[i] = next;
         next = i;
      }
   }

   Array<int> rhylev;
   infile.analyzeMetricLevel(rhylev);

   // switch sign for now so that eighth note level is -1.
   for (i=0; i<rhylev.getSize(); i++) {
      rhylev[i] = -rhylev[i];
   }

   double newtime;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isData()) {
         continue;
      }
      if (rhylev[i] == -1) {
         // if we are on an eighth note metric level
         if (rhylev[nextdata[i]] >= 0 && rhylev[lastdata[i]] >= 0) {
            // if the eighth note is surrounded by beat levels
            // then adjust the timing of the note
            newtime = timings[nextdata[i]] - timings[lastdata[i]];
            newtime = newtime * fraction;
            newtime = newtime + timings[lastdata[i]];
  
            // fix the ending case later:
            if (newtime >= timings[lastdata[i]]) {
               timings[i] = newtime;
            }
         }
      }
   }

}



//////////////////////////////
//
// analyzeTiming -- determing the absolute time position of each
//     line in the file.
// 

void analyzeTiming(HumdrumFile& infile, Array<double>& timings,
      Array<double>& tempo) {
   infile.analyzeRhythm("4");

   timings.setSize(infile.getNumLines());
   timings.setAll(0.0);
   tempo.setSize(infile.getNumLines());
   tempo.setAll(dtempo);
   tempo[0] = dtempo;
   double currtempo = dtempo;
   double input = 0.0;
   int count;

   int i;
   for (i=1; i<infile.getNumLines(); i++) {
      // check for new tempo...
      if (strncmp(infile[i][0], "*MM", 3) == 0) {
         count = sscanf(infile[i][0], "*MM%lf", &input);
         if (count == 1) {
            currtempo = input;
            if (i > 0) {
               tempo[i-1] = currtempo;
            }
            tempo[i] = currtempo;
         }
      }

      tempo[i] = currtempo;
      timings[i] = timings[i-1] + (infile[i].getAbsBeat() - 
            infile[i-1].getAbsBeat()) * 60.0/currtempo;

   }

}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("o|offset=d:0.0",   "starting time offset");   
   opts.define("s|seconds=b",      "use seconds as the time unit");   
   opts.define("t|tempo=d:60.0",   "default tempo to use if none in file");   
   opts.define("p|percent=d:60.0", "swing percentage 0=previous; 100=next");   

   opts.define("debug=b",   "trace input parsing");   
   opts.define("author=b",  "author of the program");   
   opts.define("version=b", "compilation information"); 
   opts.define("example=b", "example usage"); 
   opts.define("h|help=b",  "short description"); 
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, April 2002" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 15 April 2002" << endl;
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
   offset = opts.getDouble("offset");
   if (opts.getBoolean("seconds")) {
      style = 's';
   } else {
      style = 'm';
   }
   dtempo = opts.getDouble("tempo");

   // don't let people set the tempo to slow or negative
   if (dtempo < 1.0) {
      dtempo = 60.0;
   }
   percentage= opts.getDouble("percent");
   if (percentage < 0.0) {
      percentage = 0.0;
   }
   if (percentage > 100.0) {
      percentage = 100.0;
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
   int tempomark = 0;
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
         if (style == 's') {
            cout << timings[i] + offset << "\t";
         } else {
            cout << (int)((timings[i] + offset + 0.0005)* 1000) << "\t";
         }
         cout << infile[i].getLine() << "\n";
         break;
      case E_humrec_data_comment:
         if (infile[i].equalFieldsQ("**kern")) {
            cout << infile[i][0] << "\t";
            cout << infile[i].getLine() << "\n";
         } else {
            cout << "!\t" << infile[i].getLine() << "\n";
         }
         break;
      case E_humrec_data_measure:
         if (infile[i].equalFieldsQ("**kern")) {
            cout << infile[i][0] << "\t";
            cout << infile[i].getLine() << "\n";
         } else {
            cout << "=\t" << infile[i].getLine() << "\n";
         }
         break;
      case E_humrec_data_interpretation:
         if (strncmp(infile[i][0], "**", 2) == 0) {
            cout << "**time" << "\t";
            cout << infile[i].getLine() << "\n";
            cout << "*u=";
            switch (style) {
               case 's': cout << "sec";  break;
               case 'm': cout << "msec"; break;
               default : cout << "msec"; break;
            }
            cout << "\t";
            for (m=0; m<infile[i].getFieldCount(); m++) {
               cout << "*";
               if (m < infile[i].getFieldCount() - 1) {
                  cout << "\t";
               }
            }
            cout << "\n";
         } else if (strcmp(infile[i][0], "*-") == 0) {
            cout << "*-\t" << infile[i].getLine() << "\n";
         } else if (strncmp(infile[i][0], "*MM", 3) == 0) {
            tempomark = 1;
            cout << infile[i][0] << "\t" << infile[i].getLine() << "\n";
         } else if (infile[i].equalFieldsQ("**kern")) {
            if ((strcmp(infile[i][0], "*^") != 0) &&
                (strcmp(infile[i][0], "*+") != 0) &&
                (strcmp(infile[i][0], "*x") != 0) &&
                (strcmp(infile[i][0], "*v") != 0)) {
               cout << infile[i][0] << "\t";
               cout << infile[i].getLine() << "\n";
            } else {
               cout << "*\t";
               cout << infile[i].getLine() << "\n";
            }
         } else {
            cout << "*\t" << infile[i].getLine() << "\n";
         }
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


// md5sum: c8e491cf1559b3acf94b56b7be734365 swing.cpp [20050403]

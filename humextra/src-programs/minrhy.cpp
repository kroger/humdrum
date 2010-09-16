//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jun  9 14:48:41 PDT 2001
// Last Modified: Sun Jun 20 13:35:41 PDT 2010
// Filename:      ...museinfo/examples/all/minrhy.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/minrhy.cpp
// Syntax:        C++; museinfo
//
// Description:   calculates the minimum timebase which is the least common
//                multiple of all rhythms in the file.
//

#include "humdrum.h"

// function declarations
void      checkOptions         (Options& opts, int argc, char* argv[]);
void      example              (void);
void      usage                (const char* command);
int       findlcm              (Array<int>& list);
int       GCD                  (int a, int b);
void      insertRhythm         (Array<int>& allrhythms, int value);


// global variables
Options   options;             // database for command-line arguments
int       listQ = 0;           // used with -l option


///////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv) {
   checkOptions(options, argc, argv);

   HumdrumFile hfile;
   Array<int> timebase;
   timebase.setSize(argc-1);
   timebase.setAll(0);
   timebase.allowGrowth(0);
   Array<int> rhythms;
   Array<int> allrhythms;
   allrhythms.setSize(100);
   allrhythms.setSize(0);
   allrhythms.setGrowth(1000);

   int i, j;
   // figure out the number of input files to process
   int numinputs = options.getArgCount();
   
   // can't handle standard input yet
   for (i=1; i<=numinputs; i++) {
      hfile.clear();
      hfile.read(options.getArg(i));
      hfile.analyzeRhythm();
      if (numinputs > 1) {
         cout << argv[i] << ":\t";
      }
      if (listQ) {
         hfile.getRhythms(rhythms);
         for (j=0; j<rhythms.getSize(); j++) {
            cout << rhythms[j];
            if (j<rhythms.getSize()-1) {
               cout << " ";
            }
            insertRhythm(allrhythms, rhythms[j]);
         }
	 cout << "\n";
      } else {
         cout << hfile.getMinTimeBase() << "\n";
      }
      timebase[i-1] = hfile.getMinTimeBase();
   }

   if (numinputs > 1) {
      if (listQ) {
         cout << "all:\t";
         for (j=0; j<allrhythms.getSize(); j++) {
            cout << allrhythms[j];
            if (j < allrhythms.getSize()-1) {
               cout << " ";
            }
         }
	 cout << "\n";
      } else {
         cout << "all:\t" << findlcm(timebase) << endl;
      }
   }

}



///////////////////////////////////////////////////////////////////////////



//////////////////////////////
//
// insertRhythm --
//

void insertRhythm(Array<int>& allrhythms, int value) {
   int i;
   for (i=0; i<allrhythms.getSize(); i++) {
      if (value == allrhythms[i]) {
         return;
      }
   }
   allrhythms.append(value);
}



//////////////////////////////
//
// findlcm -- find the least common multiple between rhythms
//

int findlcm(Array<int>& rhythms) {
   if (rhythms.getSize() == 0) {
      return 0;
   }
   int output = rhythms[0];
   for (int i=1; i<rhythms.getSize(); i++) {
      output = output * rhythms[i] / GCD(output, rhythms[i]);
   }

   return output;
}

 

//////////////////////////////
//
// GCD -- greatest common divisor
//
 
int GCD(int a, int b) {
   if (b == 0) {
      return a;
   }
   int z = a % b;
   a = b;
   b = z;
   return GCD(a, b);
}    



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("l|list=b", "list time units used in file");  

   opts.define("debug=b");              // determine bad input line num
   opts.define("author=b");             // author of program
   opts.define("version=b");            // compilation info
   opts.define("example=b");            // example usages
   opts.define("h|help=b");             // short description
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, June 2001" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 21 June 2010" << endl;
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

   if (opts.getArgCount() < 1) {
      cout << "Usage: " << opts.getCommand() << " input-kern-file" << endl;
      exit(1);
   }

   listQ = opts.getBoolean("list");
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


// md5sum: 4ea5bed2925442c538d76e286e5de3db minrhy.cpp [20100623]

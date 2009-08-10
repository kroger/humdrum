//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Oct 23 19:44:36 PDT 2000
// Last Modified: Mon Oct 23 19:44:38 PDT 2000
// Filename:      ...sig/examples/all/scordur.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/scordur.cpp
// Syntax:        C++; museinfo
//
// Description: measures the total length of a humdrum file in terms of
//     metrical beats.  If more than one data set in input file, then displays
//     the total beat duration of each set.
//

#include "humdrum.h"

#include <string.h>

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif


// function declarations
void      checkOptions       (Options& opts, int argc, char* argv[]);
void      displayResults     (HumdrumFile& hfile, int count, 
                              const char* filename);
void      example            (void);
void      usage              (const char* command);

// global variables:
Options   options;            // database for command-line arguments
int       numinputs;          // the total number of input files

//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   HumdrumFile hfile;

   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   numinputs = options.getArgCount();
 
   const char* filename = "";

   for (int i=0; i<numinputs || i==0; i++) {
      hfile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         filename = "";
         hfile.read(cin);
      } else {
         filename = options.getArg(i+1);
         hfile.read(filename);
      }
     
      hfile.analyzeRhythm();
      displayResults(hfile, numinputs, filename);
   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("author=b");               // author of program
   opts.define("version=b");              // compilation info
   opts.define("example=b");              // example usages
   opts.define("h|help=b");               // short description
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, October 2000" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 23 October 2000" << endl;
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
// displayResults -- display the total rhythmic duration(s) 
//     of the file.
//

void displayResults(HumdrumFile& hfile, int count, const char* filename) {
   for (int i=0; i<hfile.getNumLines(); i++) {
      if ((hfile[i].getType() == E_humrec_interpretation) &&
          (strcmp(hfile[i][0], "*-") == 0)) {
         if (count > 1) {
            cout << filename << ":\t";
         }
         cout << hfile.getAbsBeat(i) << "\n";
      }
   }
}



//////////////////////////////
//
// example -- example usage of the quality program
//

void example(void) {
   cout <<
   "                                                                         \n"
   "# example usage of the scordur program.                                  \n"
   "     scordur chor217.krn                                                 \n"
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
   "Measures the total length of a humdrum file in terms of                  \n"
   "metrical beats.  If more than one data set in input file, then displays  \n"
   "the total beat duration of each set.                                     \n"
   "                                                                         \n"
   "Usage: " << command << " [input1 [input2 ...]]                           \n"
   "                                                                         \n"
   "Options:                                                                 \n"
   "   --options = list of all options, aliases and default values           \n"
   "                                                                         \n"
   << endl;
}



// md5sum: 655580d46fd07c70dd921d54482e3fa6 scordur.cpp [20060502]

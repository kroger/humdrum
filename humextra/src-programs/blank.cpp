//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 25 16:56:33 PDT 2003
// Last Modified: Mon Aug 25 16:56:37 PDT 2003
// Filename:      ...sig/examples/all/blank.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/blank.cpp
// Syntax:        C++; museinfo
//
// Description:   Create a blank spine with the same number of lines
//                as the input file.
//

#include "humdrum.h"

#include <string.h>
#include <ctype.h>

// function declarations
void      checkOptions       (Options& opts, int argc, char* argv[]);
void      example            (void);
void      usage              (const char* command);
void      printOutput        (HumdrumFile& file);

// global variables
Options   options;            // database for command-line arguments
int       appendQ = 0;        // used with -a option

///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   HumdrumFile infile, outfile;

   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   for (int i=0; i<numinputs || i==0; i++) {
      infile.clear();
      outfile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
      } else {
         infile.read(options.getArg(i+1));
      }

      printOutput(infile);
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// printOutput --
//

void printOutput(HumdrumFile& file) {
   for (int i=0; i<file.getNumLines(); i++) {
      switch (file[i].getType()) {
         case E_humrec_data_comment:
            if (appendQ) {
               cout << file[i] << "\t" << "!" << "\n";
            } 
            break;
         case E_humrec_data_kern_measure:
            if (appendQ) {
               cout << file[i] << "\t";
            } 
            cout << file[i][0] << "\n";
            break;
         case E_humrec_interpretation:
            if (appendQ) {
               cout << file[i] << "\t";
            }
            if (strncmp(file[i][0], "**", 2) == 0) {
               cout << "**blank" << "\n";
            } else if (strcmp(file[i][0], "*-") == 0) {
               cout << "*-" << "\n";
            } else {
               if (appendQ) {
                  cout << "*\n";
               }
            }
            break;
         case E_humrec_data:
            if (appendQ) {
               cout << file[i] << "\t";
            } 
            cout << "." << "\n";
            break;
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
         default:
            cout << file[i] << "\n";
            break;
      }
   }

}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("a|assemble=b");           // assemble analysis with input

   opts.define("debug=b");                // determine bad input line num
   opts.define("author=b");               // author of program
   opts.define("version=b");              // compilation info
   opts.define("example=b");              // example usages
   opts.define("h|help=b");               // short description
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Aug 2003" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: Aug 2003" << endl;
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

   appendQ = opts.getBoolean("assemble");
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
// usage -- gives the usage statement for the meter program
//

void usage(const char* command) {
   cout <<
   "                                                                         \n"
   << endl;
}



// md5sum: 975d5ce516a5027155ca5be2d87067af blank.cpp [20050403]

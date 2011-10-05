//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Sep 17 23:28:09 PDT 2011
// Last Modified: Sat Sep 17 23:28:15 PDT 2011
// Filename:      ...sig/examples/all/pullout.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/pullout.cpp
// Syntax:        C++; museinfo
//
// Description: Extract movements from a multi-movement Humdrum file.
//

#include "humdrum.h"

#include <string.h>
#include <ctype.h>

#ifndef OLDCPP
   #include <iostream>
   #include <fstream>
   using namespace std;
#else
   #include <iostream.h>
   #include <fstream.h>
#endif
   

// function declarations
void      checkOptions       (Options& opts, int argc, char* argv[]);
void      example            (void);
void      usage              (const char* command);
int       countSegments      (istream& stream, int precount = 0, int nth = 0);

// global variables
Options   options;             // database for command-line arguments
int       countQ = 0;          // used with -c option
int       nth    = 0;          // used with -n option
int       rangeQ = 0;          // used with -r option


///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   ifstream input;
   istream* stream;
   int count = 0;
   int ncount = 0;
   int i;
   for (i=0; i<numinputs || i==0; i++) {
      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         stream = &cin;
      } else {
         input.open(options.getArgument(i+1));
         stream = &input;
      }

      if (countQ || rangeQ) {
         count += countSegments(*stream);
      } else if (nth) {
         ncount += countSegments(*stream, ncount, nth);
         if (ncount >= nth) {
            break;
         }
      }
 
   }

   if (countQ) {
      cout << count << endl;
   } else if (rangeQ) {
      if (count >= 1) { 
         for (i=1; i<=count; i++) {
            cout << i; 
            if (i < count) {
               cout << ' ';
            }
         }
         cout << "\n";
      }
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////




//////////////////////////////
//
// countSegments --
//

int countSegments(istream& stream, int precount, int nth) {
   char ch;
   int  output    = 0;
   char lastlast  = 0;
   char last      = 0;
   int  charcount = 0;
   int  printQ    = 0;
   int  state     = 0;  // 0 = out of data, 1 = in data
   while (!stream.eof()) {
      if (state == 0) {
         // out of data, waiting for an exclusive interpretation
         if (((lastlast == 0x0a) || (charcount < 3)) && (last == '*') 
               && (ch == '*')) {
            state = 1;
            if ((nth > 0) && (precount+output == nth-1)) {
               cout << "**";
               printQ =1;
            }
         }
      } else {
         if ((lastlast == 0x0a) && (last == '*') && (ch == '-')) {
            // should also check that the next character
            // is either a newline, a tab or an end of file.
            output++;
            state = 0;
            if (printQ) {
               // while not end of line or file, print characters
               // then print an end of line.
               stream.get(ch);
               while ((!stream.eof()) && (ch != 0x0a) && (ch != 0x0d)) {
                  cout << ch;
                  stream.get(ch);
               }
               cout << endl;
               printQ = 0;
               // force a break in the main loop:
               return 999999999;
            }
         }
      }
      lastlast = last;
      last = ch;
      stream.get(ch);
      if (printQ) {
         cout << ch;
      }
      if (ch == 0x0d) {
         ch = 0x0a;
      }
   }
   return output;
}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("c|count=b", "count number of segments");
   opts.define("r|range=b", "print a range for the count");
   opts.define("n|nth=i:0", "print a particular segment");

   opts.define("debug=b");              // determine bad input line num
   opts.define("author=b");             // author of program
   opts.define("version=b");            // compilation info
   opts.define("example=b");            // example usages
   opts.define("h|help=b");             // short description
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Sept 2011" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 18 Sept 2011" << endl;
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

   countQ = opts.getBoolean("count");
   rangeQ = opts.getBoolean("range");
   nth    = opts.getInteger("nth");
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



// md5sum: 2e1571a60880800f2828e78de4b3df8f pullout.cpp [20111004]

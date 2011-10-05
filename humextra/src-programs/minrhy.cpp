//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jun  9 14:48:41 PDT 2001
// Last Modified: Sun Jun 20 13:35:41 PDT 2010 added rational rhythms
// Last Modified: Wed Jan 26 18:14:20 PST 2011 added --debug
// Last Modified: Tue Apr 12 12:04:23 PDT 2011 fixed findlcm for rational rhys.
// Filename:      ...museinfo/examples/all/minrhy.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/minrhy.cpp
// Syntax:        C++; museinfo
//
// Description:   calculates the minimum timebase which is the least common
//                multiple of all rhythms in the file.
//
// Todo: some things to clean up in rhythmic representation due to conversions
// from ints to RationalNumber class for rhythms/durations...

#include "humdrum.h"

// function declarations
void      checkOptions         (Options& opts, int argc, char* argv[]);
void      example              (void);
void      usage                (const char* command);
int       findlcm              (Array<int>& list);
int       findlcmR             (Array<RationalNumber>& list);
int       GCD                  (int a, int b);
void      insertRhythm         (Array<RationalNumber>& allrhythms, 
                                 RationalNumber value);
void      sortArray            (Array<RationalNumber>& rhythms);
int       ratcomp              (const void* a, const void* b);

template<class type>
void uniqArray(Array<type>& array);

// global variables
Options   options;             // database for command-line arguments
int       listQ = 0;           // used with -l option
int       debugQ = 0;          // used with --debug


///////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv) {
   checkOptions(options, argc, argv);

   HumdrumFile hfile;
   Array<RationalNumber> timebase;
   RationalNumber zeroR(0, 1);
   timebase.setSize(argc-1);
   timebase.setAll(zeroR);
   timebase.allowGrowth(0);
   Array<RationalNumber> rhythms;
   Array<RationalNumber> allrhythms;
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
         sortArray(rhythms);
         uniqArray(rhythms);
         for (j=0; j<rhythms.getSize(); j++) {
            cout << rhythms[j];
            if (j<rhythms.getSize()-1) {
               cout << " ";
            }
            insertRhythm(allrhythms, rhythms[j]);
         }
	 cout << "\n";
      } else {
         cout << hfile.getMinTimeBaseR() << "\n";
      }
      timebase[i-1] = hfile.getMinTimeBaseR();
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
         cout << "all:\t" << findlcmR(timebase) << endl;
      }
   }

}



//////////////////////////////
//
// sortRatArray --
//

void sortArray(Array<RationalNumber>& rhythms) {
   qsort(rhythms.getBase(), rhythms.getSize(), sizeof(RationalNumber), ratcomp);
}



//////////////////////////////
//
// uniqArray -- filter out adjacent duplicate elements.
//

template<class type>
void uniqArray(Array<type>& array) {
   Array<type> newarray(array.getSize());
   newarray.setSize(0);
   if (array.getSize() <= 1) {
      // nothing to do
      return;
   }
   newarray.append(array[0]);
   int i;
   for (i=1; i<array.getSize(); i++) {
      if (array[i] ==  array[i-1]) {
         continue;
      }
      newarray.append(array[i]);
   }

   array = newarray;
}



//////////////////////////////
//
// ratcomp -- compare two rational numbers for ordering
//

int ratcomp(const void* a, const void* b) {
   if (*((RationalNumber*)a) < *((RationalNumber*)b)) {
      return -1;
   } else if (*((RationalNumber*)a) > *((RationalNumber*)b)) {
      return 1;
   } else {
      return 0;
   }
}



///////////////////////////////////////////////////////////////////////////



//////////////////////////////
//
// insertRhythm --
//

void insertRhythm(Array<RationalNumber>& allrhythms, RationalNumber value) {
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
// findlcmR -- find the least common multiple between rhythms
//     rational number version.
//

int findlcmR(Array<RationalNumber>& rhythms) {
   if (rhythms.getSize() == 0) {
      return 0;
   }

   Array<int> plain;
   plain.setSize(rhythms.getSize());
   int i;
   for (i=0; i<rhythms.getSize(); i++) {
      plain[i] = rhythms[i].getNumerator() * rhythms[i].getDenominator();
   }
   return findlcm(plain);
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
   int output = GCD(a, b);
   if (debugQ) {  
      cout << "GCD of " << a << " and " << b << " is " << output << endl;
   }
   return output;
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

   listQ  = opts.getBoolean("list");
   debugQ = opts.getBoolean("debug");
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


// md5sum: 2e2081f3c9c269c9e012127aba61ec02 minrhy.cpp [20111004]

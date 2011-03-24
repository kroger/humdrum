//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jan  7 17:32:52 PST 2011
// Last Modified: Fri Jan  7 19:33:31 PST 2011 (added -o and -a options)
// Filename:      ...sig/examples/all/rscale.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/rscale.cpp
// Syntax:        C++; museinfo
//
// Description:   Scale the duration of all **kern rhythms by a constant
//                factor.
//
//

#include "PerlRegularExpression.h"
#include "humdrum.h"
#include <math.h>

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
void      printOutput        (HumdrumFile& infile, RationalNumber& rnum);
void      printKernToken     (HumdrumFile& infile, int row, int col, 
                              RationalNumber& factor);
void      printSingleKernSubtoken(const char* buff, RationalNumber& factor);
void      printSpecialRational(RationalNumber& value);
void      processTimeSignature(HumdrumFile& infile, int row, 
                              RationalNumber& factor);
void      handleBibliographic(HumdrumFile& infile, int row, 
                              RationalNumber& num);
void      getOriginalFactor  (HumdrumFile& infile, RationalNumber& factor);
void      getAlternateFactor (HumdrumFile& infile, RationalNumber& factor);

// global variables
Options   options;             // database for command-line arguments
int       debugQ     = 0;      // used with --debug
RationalNumber factor;         // used with -f option
int       meterQ     = 1;      // used with -M option
int       FoundRef   = 0;      // used with !!!rscale: reference record
int       originalQ  = 0;      // used with --original
int       alternateQ = 0;      // used with --alternate
int       longQ      = 0;      // used with -r option

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
      // analyze the input file according to command-line options
      infile.analyzeRhythm("4");
 
      if (originalQ) {
         getOriginalFactor(infile, factor);
      } else if (alternateQ) {
         getAlternateFactor(infile, factor);
      }

      printOutput(infile, factor);
   }

   if ((!originalQ) && (!FoundRef) && (factor != 1)) {
      cout << "!!!rscale: " << factor << endl;
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// printOutput -- 
//

void printOutput(HumdrumFile& infile, RationalNumber& rnum) {
   int i, j;
   
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isBibliographic()) {
         handleBibliographic(infile, i, rnum);
         continue;
      }
      if (!infile[i].isData()) {
         if (meterQ && infile[i].isInterpretation()) {
            processTimeSignature(infile, i, rnum); 
         } else {
            cout << infile[i] << "\n";
         }
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (!(infile[i].isExInterp(j, "**kern") || 
               infile[i].isExInterp(j, "**recip"))) {
            cout << infile[i][j];
            if (j < infile[i].getFieldCount() - 1) {
               cout << "\t";
            }
            continue;
         }
         printKernToken(infile, i, j, rnum);
         if (j < infile[i].getFieldCount() - 1) {
            cout << "\t";
         }
      }
      cout << "\n";
   }
}



//////////////////////////////
//
// getOriginalFactor -- return the inverse of the value in the ref record:
//    !!!rscale: 1/2
// Would return "2" in the above case.
//

void getOriginalFactor(HumdrumFile& infile, RationalNumber& factor) {
   int i;
   PerlRegularExpression pre;
   RationalNumber newvalue;
   int top;
   int bot;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) {
         continue;
      }
      if (!pre.search(infile[i][0], "^!!!rscale\\s*:\\s*(\\d+)(/?)(\\d*)", "")) {
         continue;
      }

      top = atoi(pre.getSubmatch(1));
      bot = 1;
      if (strlen(pre.getSubmatch(2)) != 0) {
         if (strlen(pre.getSubmatch(3)) != 0) {
            bot = atoi(pre.getSubmatch());
         }
      }
      if (top == 0) { top = 1; }
      if (bot == 0) { bot = 1; }
      newvalue.setValue(bot, top);
      factor = newvalue;
      return;
   }

}



///////////////////////////////
//
// getAlternateFactor -- return the factor which will produce the preferred
//     alternate rhythmic values.
//

void getAlternateFactor(HumdrumFile& infile, RationalNumber& factor) {
   PerlRegularExpression pre;
   int i;
   int top;
   int bot;
   RationalNumber value;

   RationalNumber orig(1,1);
   getOriginalFactor(infile, orig);

   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) {
         continue;
      }
      if (!pre.search(infile[i][0], "^!!!rscale-alt\\s*:\\s*(\\d+)(/?)(\\d*)", "")) {
         continue;
      }

      top = atoi(pre.getSubmatch(1));
      bot = 1;
      if (strlen(pre.getSubmatch(2)) != 0) {
         if (strlen(pre.getSubmatch(3)) != 0) {
            bot = atoi(pre.getSubmatch());
         }
      }
      if (top == 0) { top = 1; }
      if (bot == 0) { bot = 1; }
      value.setValue(top, bot);
      factor = value * orig;
      return;
   }

}



//////////////////////////////
//
// handleBibliographic -- print reference records.  Scan for a reference
//     record in the form:
//        !!!rscale: 1/2
//     which is the scaling factor applied to the file already.
//     Modify this value by the factor.  Suppresss the record if
//     the output scaling is 1/1.
//

void handleBibliographic(HumdrumFile& infile, int row, RationalNumber& num) {
   char buffer[1024] = {0};
   infile[row].getBibKey(buffer, 1000);
   if (strcmp(buffer, "rscale") != 0) {
      cout << infile[row] << endl;
      return;
   }

   RationalNumber onum;
   PerlRegularExpression pre;
   if (!pre.search(infile[row][0], "!!!rscale([^\\d]*)(\\d+)(/?)(\\d*)(.*)", "")) {
      cout << infile[row] << endl;
      return;
   }

   int top = atoi(pre.getSubmatch(2));
   int bot = 1;
   if (strlen(pre.getSubmatch(3)) != 0) {
      if (strlen(pre.getSubmatch(4)) != 0) {   
         bot = atoi(pre.getSubmatch());
      }
   }
   if (bot == 0) {
      bot = 1;
   }
   onum.setValue(top, bot);
   onum *= num;
   FoundRef = 1;
   if (onum == 1) {
      // don't print !!!rscale: entry if scaling is 1.
      return;
   }

   if (originalQ) {
      // original rhythmic values are being generated, don't print ref rec.
      return;
   }
   
   cout << "!!!rscale";
   cout << pre.getSubmatch(1);
   cout << onum;
   cout << pre.getSubmatch(5);
   cout << "\n";

}



//////////////////////////////
//
// processTimeSignature -- look for time signatures, and alter the
//     bottom of the time signature by the scaling factor.
//

void processTimeSignature(HumdrumFile& infile, int row, 
      RationalNumber& factor) {
   int j;
   RationalNumber beat;
   int mtop;
   int top;
   int bot;
   PerlRegularExpression pre;
   for (j=0; j<infile[row].getFieldCount(); j++) {
      if (pre.search(infile[row][j], "^\\*tb(\\d+)(%?)(\\d*)$", "")) {
         // process a timebase interpretation
         top = atoi(pre.getSubmatch(1));
         bot = 1;
         if (strlen(pre.getSubmatch(2)) != 0) {
            if (strlen(pre.getSubmatch(3)) != 0) {
               bot = atoi(pre.getSubmatch());
               if (bot == 0) {
                  bot = 1;
               }
            }
         }
         beat.setValue(top, bot);
         beat /= factor;
         cout << "*tb";
         printSpecialRational(beat);
         if (j < infile[row].getFieldCount() - 1) {
            cout << "\t";
         }
         continue;
      }

      if ((!meterQ) || (!pre.search(infile[row][j], "^\\*M(\\d+)/(\\d+)(%?)(\\d*)(.*)", ""))) {
         cout << infile[row][j];
         if (j < infile[row].getFieldCount() - 1) {
            cout << "\t";
         }
         continue;
      }

      mtop = atoi(pre.getSubmatch(1));
      top = atoi(pre.getSubmatch(2));
      bot = 1;
      if (strlen(pre.getSubmatch(3)) != 0) {
         if (strlen(pre.getSubmatch(4)) != 0) {
            bot = atoi(pre.getSubmatch());
            if (bot == 0) {
               bot = 1;
            }
         }
      }
      beat.setValue(top, bot);
      beat /= factor;
      cout << "*M";
      cout << mtop;
      cout << "/";
      printSpecialRational(beat);
      cout << pre.getSubmatch(5);
      if (j < infile[row].getFieldCount() - 1) {
         cout << "\t";
      }
   }
   cout << "\n";
}



/////////////////////////////
//
// printKernToken --
//

void printKernToken(HumdrumFile& infile, int row, int col, 
      RationalNumber& factor) {
   int k;
   static char buffer[1024] = {0};

   if (strcmp(infile[row][col], ".") == 0) {
      // print null tokens and return
      cout << infile[row][col];
      return;
   }
   if (strchr(infile[row][col], 'q') != NULL) {
      // print notes/chords which have grace notes (chord rhythms 
      // should not mix and always be equal).
      cout << infile[row][col];
      return;
   }

   PerlRegularExpression pre;
   int tcount = infile[row].getTokenCount(col);
   for (k=0; k<tcount; k++) {
      infile[row].getToken(buffer, col, k, 1000);
      printSingleKernSubtoken(buffer, factor);
      if (k < tcount - 1) {
         cout << " ";
      }
   }
}



//////////////////////////////
//
// printSingleKernSubtoken --
//

void printSingleKernSubtoken(const char* buff, RationalNumber& factor) {
   PerlRegularExpression pre;
   RationalNumber value;

   if (pre.search(buff, "([^\\d]*)(\\d+)(%?)(\\d*)(.*)", "")) {
      int top = atoi(pre.getSubmatch(2));
      int bot;
      if (strlen(pre.getSubmatch(4)) != 0) {
         bot = atoi(pre.getSubmatch());
         if (bot == 0) {
            bot = 1;
         }
      } else {
         bot = 1; 
      }
      if (top == 0) {
         // handle cases where breve=0; long=00; maxima=000
         int tcount = 0;
         int i;
         int len = strlen(buff);
         for (i=0; i<len; i++) {
            if (buff[i] == '0') {
               tcount++;
            }
         }
         top = 1;
         bot = int(pow(2.0,tcount));
      }
      value.setValue(top, bot);
      value /= factor;           // factor is duration so inverse

      // print stuff before numeric part of rhythm
      cout << pre.getSubmatch(1);

      printSpecialRational(value);

      // print stuff after numeric part of rhythm
      cout << pre.getSubmatch(5);
   } else {
      cout << buff;
   }
}



//////////////////////////////
//
// printSpecialRational -- print rational number rhythm, but use 0 
//    for 1/2 (breve), 00 for 1/4 (long), or 000 for 1/8 (maxima).
//

void printSpecialRational(RationalNumber& value) {
   static RationalNumber half(1,2);      // breve
   static RationalNumber quarter(1,4);   // long
   static RationalNumber eighth(1,8);    // maxima

   if (longQ) {
      // don't print 0 for breve, 00 for long or 000 for maxima.
      cout << value.getNumerator();
      if (value.getDenominator() != 1) {
         cout << '%' << value.getDenominator();
      }

   } else {

      if (value == half) {               // breve alternate
         cout << "0";
      } else if (value == quarter) {     // long alternate
         cout << "00";
      } else if (value == eighth) {      // maxima alternate
         cout << "000";
      } else {
         cout << value.getNumerator();
         if (value.getDenominator() != 1) {
            cout << '%' << value.getDenominator();
         }
      }

   }

}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("d|f|factor=s:1/1", "Factor to multiply durations by");
   opts.define("M|T|nometer=b", "Do not alter time signatures");
   opts.define("o|original=b", "Revert to original rhythms");
   opts.define("a|alternate=b", "Change to alternate rhythm set");
   opts.define("r|long=b", "Do not use short form for breve,long,maxima");

   opts.define("debug=b");              // determine bad input line num
   opts.define("author=b");             // author of program
   opts.define("version=b");            // compilation info
   opts.define("example=b");            // example usages
   opts.define("h|help=b");             // short description
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Jan 2011" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 6 Jan 2011" << endl;
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

   PerlRegularExpression pre;
   if (pre.search(opts.getString("factor"), "(\\d+)\\/?(\\d*)", "")) {
      int top = 1;
      int bot = 1;
      top = atoi(pre.getSubmatch(1));
      if (strlen(pre.getSubmatch(2)) != 0) {
         bot = atoi(pre.getSubmatch());
         if (bot == 0) {
            bot = 1;
         }
      } else {
         bot = 1;
      }
      factor.setValue(top, bot);
   } else {
      factor.setValue(1,1);
      cerr << "Scaling factor set to " << factor << endl;
   }

   debugQ = opts.getBoolean("debug");

   if (debugQ) {
      cerr << "Scaling factor set to " << factor << endl;
   }

   meterQ      = !opts.getBoolean("nometer");
   originalQ   =  opts.getBoolean("original");
   alternateQ  =  opts.getBoolean("alternate");
   longQ       =  opts.getBoolean("long");
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



// md5sum: 4935a10d529f7ff8a83b1db087dc9bb6 rscale.cpp [20110318]

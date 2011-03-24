//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Mar 31 21:51:34 PST 2005
// Last Modified: Fri Apr  1 15:12:06 PST 2005
// Filename:      ...sig/examples/all/range.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/range.cpp
// Syntax:        C++; museinfo
//
// Description:   Calculate pitch histograms in a score of **kern data.
// 

#include "humdrum.h"

#include <string.h>
#include <math.h>

// function declarations
void   checkOptions             (Options& opts, int argc, char* argv[]);
void   example                  (void);
void   usage                    (const char* command);
void   generateAnalysis         (HumdrumFile& infile, 
                                 Array<double>& rdata);
void   printAnalysis            (Array<double>& rdata);
void   printPercentile          (Array<double>& midibins, double percentile);
double countNotesInRange        (Array<double>& midibins, int low, int high);
void   getRange                 (int& rangeL, int& rangeH, 
                                 const char* rangestring);

// global variables
Options      options;            // database for command-line arguments
int          durationQ  = 0;     // used with -d option
int          debugQ     = 0;     // used with --debug option
int          percentileQ= 0;     // used with -p option
int          addfractionQ = 0;   // used with -f option
double       percentile = 0.0;   // used with -p option
int          rangeQ     = 0;     // used with -r option
int          rangeL     = 0;     // used with -r option
int          rangeH     = 0;     // used with -r option
int          printQ     = 0;     // used with --print option

///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   HumdrumFile infile;

   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   Array<double> values;
   values.setSize(0);

   Array<double> midibins;
   midibins.setSize(128);
   midibins.setAll(0);
   midibins.allowGrowth(0);

   for (int i=0; i<numinputs || i==0; i++) {
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
      } else {
         infile.read(options.getArg(i+1));
      }

      generateAnalysis(infile, midibins);
   }

   printAnalysis(midibins);

   return 0;
}


///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// printAnalysis --
//

void printAnalysis(Array<double>& midibins) {
   if (percentileQ) {
      printPercentile(midibins, percentile);
      return;
   }  else if (rangeQ) {
      double notesinrange = countNotesInRange(midibins, rangeL, rangeH);
      cout << notesinrange << endl;
      return;
   }

   // print the pitch histogram

   double fracL = 0.0;
   double fracH = 0.0;
   double fracA = 0.0;
   double sum = midibins.sum();
   double runningtotal = 0.0;
   cout << "**keyno\t**kern\t**count";
   if (addfractionQ) {
      cout << "\t**fracL";
      cout << "\t**fracA";
      cout << "\t**fracH";
   }
   cout << "\n";

   char buffer[1024] = {0};
   int i;
   for (i=0; i<midibins.getSize(); i++) {
      if (midibins[i] <= 0.0) {
         continue;
      }      
      cout << i << "\t";
      cout << Convert::base12ToKern(buffer, i);
      cout << "\t";
      cout << midibins[i];
      fracL = runningtotal/sum;
      runningtotal += midibins[i];
      fracH = runningtotal/sum;
      fracA = (fracH + fracL)/2.0;
      fracL = (int)(fracL * 10000.0 + 0.5)/10000.0;
      fracH = (int)(fracH * 10000.0 + 0.5)/10000.0;
      fracA = (int)(fracA * 10000.0 + 0.5)/10000.0;
      if (addfractionQ) {
         cout << "\t" << fracL;
         cout << "\t" << fracA;
         cout << "\t" << fracH;
      }
      cout << "\n";
   }
   cout << "*-\t*-\t*-";
   if (addfractionQ) {
      cout << "\t*-";
      cout << "\t*-";
      cout << "\t*-";
   }
   cout << "\n";

}


//////////////////////////////
//
// countNotesInRange --
//

double countNotesInRange(Array<double>& midibins, int low, int high) {
   int i;
   double sum = 0;
   for (i=low; i<=high; i++) {
      sum += midibins[i];
   }
   return sum;
}




//////////////////////////////
//
// printPercentile --
//

void printPercentile(Array<double>& midibins, double percentile) {
   double sum = midibins.sum();
   double runningtotal = 0.0;
   int i;
   for (i=0; i<midibins.getSize(); i++) {
      if (midibins[i] <= 0) {
         continue;
      }
      runningtotal += midibins[i] / sum;
      if (runningtotal >= percentile) {
         cout << i << endl;
         return;
      }
   }

   cout << "unknown" << endl;
}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("c|range|count=s:60-71", "count notes in a particular range");
   opts.define("d|duration=b",      "weight pitches by duration");   
   opts.define("f|fraction=b",      "display histogram fractions");   
   opts.define("p|percentile=d:0.0","display the xth percentile pitch");   
   opts.define("print=b",           "count printed notes rather than sounding");

   opts.define("debug=b",       "trace input parsing");   
   opts.define("author=b",      "author of the program");   
   opts.define("version=b",     "compilation information"); 
   opts.define("example=b",     "example usage"); 
   opts.define("h|help=b",      "short description"); 
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Mar 2005" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 31 Mar 2005" << endl;
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

   debugQ       = opts.getBoolean("debug");
   printQ       = opts.getBoolean("print");
   durationQ    = opts.getBoolean("duration");
   percentileQ  = opts.getBoolean("percentile");
   rangeQ       = opts.getBoolean("range");
   getRange(rangeL, rangeH, opts.getString("range"));
   addfractionQ = opts.getBoolean("fraction");
   percentile   = opts.getDouble("percentile");

   // the percentile is a fraction from 0.0 to 1.0.
   // if the percentile is above 1.0, then it is assumed
   // to be a percentage, in which case the value will be
   // divided by 100 to get it in the range from 0 to 1.
   if (percentile > 1) {
      percentile = percentile / 100.0;
   }

}



//////////////////////////////
//
// getRange --
//

void getRange(int& rangeL, int& rangeH, const char* rangestring) {
   rangeL = -1; rangeH = -1;
   if (rangestring == NULL) {
      return;
   }
   if (rangestring[0] == '\0') {
      return;
   }
   int length = strlen(rangestring);
   char* buffer = new char[length+1];
   strcpy(buffer, rangestring);
   char* ptr;
   if (isdigit(buffer[0])) {
      ptr = strtok(buffer, " \t\n:-");
      sscanf(ptr, "%d", &rangeL);
      ptr = strtok(NULL, " \t\n:-");
      if (ptr != NULL) {
         sscanf(ptr, "%d", &rangeH);
      }
   } else {
      ptr = strtok(buffer, " :");
      if (ptr != NULL) {
         rangeL = Convert::kernToMidiNoteNumber(ptr);
         ptr = strtok(NULL, " :");
         if (ptr != NULL) {
            rangeH = Convert::kernToMidiNoteNumber(ptr);
         }
      }
   }

   if (rangeH < 0) {
      rangeH = rangeL;
   }

   if (rangeL <   0) { rangeL =   0; }
   if (rangeH <   0) { rangeH =   0; }
   if (rangeL > 127) { rangeL = 127; }
   if (rangeH > 127) { rangeH = 127; }
   if (rangeL > rangeH) {
      int temp = rangeL;
      rangeL = rangeH;
      rangeH = temp;
   }

}



//////////////////////////////
//
// example -- example usage of the maxent program
//

void example(void) {
   cout <<
   "                                                                        \n"
   << endl;
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



//////////////////////////////
//
// generateAnalysis --
//

void generateAnalysis(HumdrumFile& infile, Array<double>& midibins) {
   int i, j, k;

   char buffer[1024] = {0};
   int tokencount;
   int keynum;
   double duration;

   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() != E_humrec_data) {
         continue;
      } 
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strcmp(infile[i].getExInterp(j), "**kern") != 0) {
            continue;
         }
         if (strcmp(infile[i][j], ".") == 0) {  // ignore null tokens
            continue;
         }
         tokencount = infile[i].getTokenCount(j);
         for (k=0; k<tokencount; k++) {
            infile[i].getToken(buffer, j, k);         
            if (strcmp(buffer, ".") == 0) {    // ignore strange null tokens
               continue;
            }
            if (strchr(buffer, 'r') != NULL) { // ignore rests
               continue;
            }
            if (!printQ && !durationQ) {
               // filter out middle and ending tie notes if counting
               // by sounding pitch
               if (strchr(buffer, '_') != NULL) {
                  continue;
               }
               if (strchr(buffer, ']') != NULL) {
                  continue;
               }
            }
            keynum = Convert::kernToMidiNoteNumber(buffer);
            if (keynum > 127) {
               cout << "ERROR: Funny pitch: " << keynum 
                    << " = " << buffer << endl;
            } else if (durationQ) {
               duration = Convert::kernToDuration(buffer);
               midibins[keynum] += duration;
            } else {
               midibins[keynum] += 1.0;
            }
         }
      }
   }
}



// md5sum: e591f5492fe210cef04d4fd0e1536ff7 prange.cpp [20101222]

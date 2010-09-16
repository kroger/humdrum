//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Feb  3 15:49:07 PST 2010
// Last Modified: Wed Feb  3 15:49:10 PST 2010
// Filename:      ...sig/examples/all/matphrase.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/matphrase.cpp
// Syntax:        C++; museinfo
//
// Description:   Extract monophonic phrase information for analysis in matlab.
//

#include <math.h>
#include "humdrum.h"

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif


// function declarations:
void    usage             (const char* command);
void    example           (void);
void    checkOptions      (Options& opts, int argc, char* argv[]);
void    processFile       (int count, const char* filename);
int     processPhrase     (HumdrumFile& infile, const char* filename, 
                           int scount, int pcount, int line);
void    printRegion       (HumdrumFile& infile, const char* defaultvalue);

// User interface variables:
Options   options;
const char* defaultregion = "NONE";   // used with -r option


//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   // process the command-line options
   checkOptions(options, argc, argv);

   int i;
   for (i=0; i<options.getArgCount(); i++) {
      processFile(i+1, options.getArg(i+1));
   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////




//////////////////////////////
//
// processFile --
// 

void processFile(int scount, const char* filename) {
   HumdrumFile infile;
   infile.read(filename);
   infile.analyzeRhythm("4");

   int i, j;
   int pcount = 0;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isData()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strcmp(infile[i].getExInterp(j), "**kern") != 0) {
            continue;
         }
         i = processPhrase(infile, filename, scount, ++pcount, i);
	 if (i <= 0) {
            return;
         }
	 break;
      }

   }


}



//////////////////////////////
//
// processPhrase --
//

int processPhrase(HumdrumFile& infile, const char* filename, int scount, 
      int pcount, int line) {

   int minrhy = infile.getMinTimeBase();

   Array<int> pitches;
   Array<double> durations;
   pitches.setSize(100);
   durations.setSize(100);
   pitches.setSize(0);
   durations.setSize(0);

   int pit;
   double dur;

   int i, j;
   int finished = 0;
   for (i=line; i<infile.getNumLines(); i++) {
      if (!infile[i].isData()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strcmp(infile[i].getExInterp(j), "**kern") != 0) {
            continue;
         }
         if (strchr(infile[i][j], '}') != NULL) {
            finished = i;
         }
         if (strcmp(infile[i][j], ".") == 0) {
            break;
         }
         if (strchr(infile[i][j], '_') != NULL) {
            // ignore middles of ties
            break;
         }
         if (strchr(infile[i][j], ']') != NULL) {
            // ignore ends of ties
            break;
         }
         if (strchr(infile[i][j], 'r') != NULL) {
            // handle rests here...
            break;
         }
         pit = Convert::kernToMidiNoteNumber(infile[i][j]) - 60 + 1;
         pitches.append(pit);
         dur = infile.getTiedDuration(i, j);
         durations.append(dur);

         break;
      }

      if (finished) {
         break;
      }
   }

   cout << "songnumber: " << scount << "\n";
   cout << "lastphrase: " << pcount << "\n";
   cout << "source: "     << filename << "\n";
   cout << "region: ";
   printRegion(infile, defaultregion);
   cout << "\n";

   cout << "dur: [";
   for (i=0; i<durations.getSize(); i++) {
      cout << minrhy * durations[i] / 4;
      if (i < durations.getSize()-1) {
         cout << " ";
      }
   }
   cout << "]\n";

   cout << "pit: [";
   for (i=0; i<pitches.getSize(); i++) {
      cout << pitches[i];
      if (i < pitches.getSize()-1) {
         cout << " ";
      }
   }
   cout << "]\n";

   // add a blank line between phrases
   cout << endl;
   

   return finished;
}



//////////////////////////////
//
// printRegion -- 
//

void printRegion(HumdrumFile& infile, const char* defaultvalue) {
   int i;
   char buffer[1024] = {0};
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) {
         continue;
      }
      infile[i].getBibKey(buffer, 900);
      if (strcmp(buffer, "ARE") == 0) {
         cout << infile[i].getBibValue(buffer, 900);
         return;
      }
   }
   
   cout << defaultvalue;
}




//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("r|region=s:NONE", "Regional source of melody");

   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Feb 2010" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 3 Feb 2010" << endl;
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
   
   defaultregion = opts.getString("region");
}



//////////////////////////////
//
// example --
//

void example(void) {


}



//////////////////////////////
//
// usage --
//

void usage(const char* command) {

}



// md5sum: 484e8c85ae16df6939459364e59522bd matphrase.cpp [20100430]

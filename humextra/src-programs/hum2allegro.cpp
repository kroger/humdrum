//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Oct 22 16:33:21 PDT 2008
// Last Modified: Wed Oct 22 16:33:25 PDT 2008
// Filename:      ...sig/examples/all/hum2allegro.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/hum2allegro.cpp
// Syntax:        C++; museinfo
//
// Description:   Converts a Humdrum file into Allegro data.
//

#include "humdrum.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif

#ifndef VISUAL
   #include <time.h>
#endif

#define TOLERANCE 0.00001

// function declarations:
void      checkOptions          (Options& opts, int argc, char** argv);
void      example               (void);
void      usage                 (const char* command);
void      convertToAllegro      (ostream& out, HumdrumFile& infile);
void      printKernTokenData    (ostream& out, HumdrumFile& infile, int row, 
                                 int col, int& outputcounter,
                                 int& currentbeat, int measurenum);

// User interface variables:
Options   options;
int       beatQ = 0;            // used with -b option
double    beatduration = 1.0;   // used with -b option

//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   checkOptions(options, argc, argv);  // process the command-line options
   HumdrumFile infile(options.getArg(1));
   infile.analyzeRhythm("4");
   convertToAllegro(std::cout, infile);
   return 0;
}

//////////////////////////////////////////////////////////////////////////


///////////////////////////////
//
// convertToAllegro --
//

void convertToAllegro(ostream& out, HumdrumFile& infile) {
   int i, j;
   int measurenum = -1;
   int outputcounter = 0;
   int currentbeat = -100;

   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() == E_humrec_data_measure) {
         sscanf(infile[i][0], "=%d", &measurenum);
      }    
      if (infile[i].getType() != E_humrec_data) {
         continue;
      }
	       
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (infile[i].getExInterpNum(j) != E_KERN_EXINT) {
            continue;
         }
         printKernTokenData(out, infile, i, j, outputcounter, currentbeat,
               measurenum);
      }
   }
}



///////////////////////////////
//
// printKernTokenData --
//

void printKernTokenData(ostream& out, HumdrumFile& infile, int row, int col,
      int& outputcounter, int& currentbeat, int measurenum) {
   if (strcmp(infile[row][col], ".") == 0) {
      // ignore null tokens
      return;
   }
   if (strchr(infile[row][col], 'r') != NULL) {
      // ignore rests
      return;
   }

   double startbeat = infile[row].getAbsBeat();
   double duration;

   double beatnum = startbeat / beatduration;
   double fraction = beatnum - (int)beatnum;

   char buffer[1024] = {0};
   int  track = infile[row].getPrimaryTrack(col);
   int  count = infile[row].getTokenCount(col);
   int  midinote;
   int  i;
   int  tempbeat;

   for (i=0; i<count; i++) {
      infile[row].getToken(buffer, col, i);
      if (strchr(buffer, '_') != NULL) {
         // ignore middles of tied note groups
         continue;
      }
      if (strchr(buffer, ']') != NULL) {
         // ignore ends of tied note groups
         continue;
      }
      if (strchr(buffer, '[') != NULL) {
         duration = infile.getTiedDuration(row, col, i);
      } else {
         duration = Convert::kernToDuration(buffer);
      }
	        
      outputcounter++;
      if (!beatQ) {
         out << outputcounter;
         out << " " << track;
         midinote = Convert::kernToMidiNoteNumber(buffer);
         out << " " << midinote;
         out << " " << 64;
         out << " " << int(startbeat * 1000.0 + 0.5)/1000.0;
         out << " " << int(duration * 1000.0 + 0.5)/1000.0;
         out << endl;
      } else {
         if ((fraction < TOLERANCE) || (1-fraction < TOLERANCE)) {
            tempbeat = int(beatnum + 0.5);
            if (tempbeat != currentbeat) {
               currentbeat = tempbeat;
               out << outputcounter;
               out << " " << currentbeat;
               out << " " << measurenum;
	       out << " " << (infile.getBeat(row)-1) / beatduration + 1;
               out << endl;
            }
         }
      }
   }
}



///////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("b|beat=s:4", "extract list of notes which occur at beat times");

   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("h|help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Oct 2008" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 22 Oct 2008" << endl;
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
   
   beatQ = opts.getBoolean("beat");
   beatduration = Convert::kernToDuration(opts.getString("beat"));
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



// md5sum: 8f7e2ed4ebe7da117e0dff81cd339459 hum2allegro.cpp [20081027]

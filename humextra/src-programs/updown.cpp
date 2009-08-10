//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jan 29 16:16:33 PST 2009
// Last Modified: Thu Jan 29 16:16:38 PST 2009
// Filename:      ...sig/examples/all/updown.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/updown.cpp
// Syntax:        C++; museinfo
//
// Description:   Identifies when there are m notes which rise in
//                in pitch, followed by n notes falling in pitch.
//
// four types of outputs can be given:
//   -u   = specifies the number up expected ascending pitches
//   -d   = specifies the number up expected descending pitches
//   -i   = ignore repeated notes
//

#include "humdrum.h"

// function declarations
void      checkOptions       (Options& opts, int argc, char* argv[]);
void      example            (void);
void      usage              (const char* command);

void    generateData       (HumdrumFile& infile, Array<int>& pitches,
                            Array<int>& lines, Array<int>& measures, 
			    int ignorerepeat);
void    analyzeData        (Array<int>& matchstart, Array<int>& pitches, 
                            int upcount, int downcount);
void    printAnalysis      (Array<int>& matchstart, HumdrumFile& infile, 
                            Array<int>& pitches, Array<int>& lines, 
                            Array<int>& measures, int upcount, 
                            int downcount);
int     getFirstKernColumn (HumdrumFile& infile, int index);
void    printMatchSequence (Array<int>& pitches, int startindex, int count);

// global variables
Options options;           // database for command-line arguments
int     debugQ       = 0;  // set with --debug
int     risecount    = 5;  // set with -u option
int     fallcount    = 5;  // set with -d option
int     ignorerepeat = 0;  // set with -i option


///////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[]) {
   HumdrumFile infile;

   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   Array<int> pitches;     // list of pitches in melody
   Array<int> lines;       // list of lines in input file for pitches
   Array<int> measures;    // list of measures for pitches
   Array<int> matchstart;  // list of measures for pitches

   for (int i=0; i<numinputs || i==0; i++) {
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
      } else {
         infile.read(options.getArg(i+1));
      }

      generateData(infile, pitches, lines, measures, ignorerepeat);
      analyzeData(matchstart, pitches, risecount, fallcount);
      printAnalysis(matchstart, infile, pitches, lines, measures, 
            risecount, fallcount);
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// generateData -- extract musical data from Humdrum File.
//

void generateData(HumdrumFile& infile, Array<int>& pitches,
      Array<int>& lines, Array<int>& measures, int ignorerepeat) {

   // pre-allocate maximum array sizes:
   pitches.setSize(infile.getNumLines());
   lines.setSize(infile.getNumLines());
   measures.setSize(infile.getNumLines());

   pitches.setSize(0);
   lines.setSize(0);
   measures.setSize(0);

   int currmeas  = 0;
   int apitch    = 0;
   int col       = 0;

   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {

         case E_humrec_data_kern_measure:
            if (sscanf(infile[i][0], "=%d", &currmeas) != 1) {
               currmeas++;
            }
            break;

         case E_humrec_data:
            col = getFirstKernColumn(infile, i);
            if (strcmp(infile[i][col], ".") == 0) {
               // ignore null tokens
               continue;
            } else if (strchr(infile[i][col], 'r') != NULL) {
               // ignore rests
               continue;
            }
	    if (strchr(infile[i][col], ' ') != NULL) {
               cout << "WARNING: chord found on line " << i+1 
                    << "(" << infile[i][col] 
                    << "): only using first note." << endl;
            }

            apitch = Convert::kernToMidiNoteNumber(infile[i][col]);

            if (ignorerepeat && (pitches[pitches.getSize()-1] == apitch)) {
               continue;
            }
            pitches.append(apitch);
            lines.append(i);
            measures.append(currmeas);

            if (debugQ) {
               cout << apitch << "\t" << i << "\t" << currmeas << endl;
            }

            break;

         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
         case E_humrec_data_comment:
         case E_humrec_interpretation:
         default:
            break;
      }
   }
}



//////////////////////////////
//
// getFirstKernColumn -- return the index of the first
//    **kern data column on the row.
//

int getFirstKernColumn(HumdrumFile& infile, int index) {
   int j;
   for (j=0; j<infile[index].getFieldCount(); j++) {
      if (strcmp(infile[index].getExInterp(j), "**kern") == 0) {
         return j;
      }
   }
   // did not find any **kern data on the line.
   return -1;
}



//////////////////////////////
//
// analyzeData -- location required number of ascending and descending
//     pitches in the melody.
//

void analyzeData(Array<int>& matchstart, Array<int>& pitches, 
      int upcount, int downcount) {

   matchstart.setSize(pitches.getSize());
   matchstart.setSize(0);

   Array<int> intervals;
   intervals.setSize(pitches.getSize()-1);

   int i, j;
   for (i=0; i<intervals.getSize(); i++) {
      intervals[i] = pitches[i+1] - pitches[i];
      if (debugQ) {
         cout << "INTERVAL: " << intervals[i] << endl;
      }
   }
   
   int success = 0;
   int sum = upcount + downcount;
   for (i=0; i<intervals.getSize()-sum; i++) {
      success = 1;
      for (j=0; j<upcount; j++) {
         if (intervals[i+j] <= 0) {
            success = 0;
            break;
         } 
		  
      }
      if (success != 0) {
         for (j=0; j<downcount; j++) {
            if (intervals[i+j+upcount] >= 0) {
               success = 0;
               break;
            }
         }
      }
      if (success) {
         matchstart.append(i);
         if (debugQ) {
            cout << "MATCH at index " << i << endl;
         }
      }
      
   }
}



//////////////////////////////
//
// printAnalysis -- print the Analysis results.
//

void printAnalysis(Array<int>& matchstart, HumdrumFile& infile, 
      Array<int>& pitches, Array<int>& lines, Array<int>& measures, 
      int upcount, int downcount) {

   // analyze the input file according to command-line options
   infile.analyzeRhythm();


   int i;
   for (i=0; i<matchstart.getSize(); i++) {
      cout << measures[matchstart[i]] << ":";
      cout << infile[lines[matchstart[i]]].getBeat() << " ";
      printMatchSequence(pitches, matchstart[i], upcount+downcount+1);
      cout << endl;
   } 

}



//////////////////////////////
//
// printMatchSequence --
//

void printMatchSequence(Array<int>& pitches, int startindex, int count) {
   int i;
   char buffer[1024] = {0};
   cout << "[";
   for (i=0; i<count; i++) {
      cout << Convert::base12ToKern(buffer, pitches[startindex+i]);
      if (i < count-1) {
         cout << ", ";
      }
   }
   cout << "]";

}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("i|ignore-repeated-notes=b", "ignore repeated notes"); 
   opts.define("u|up-count=i:5", "number of notes going upwards"); 
   opts.define("d|down-count=i:5", "number of notes going downwards"); 
   opts.define("debug=b", "pritn debugging statements"); 

   opts.define("author=b");             // author of program
   opts.define("version=b");            // compilation info
   opts.define("example=b");            // example usages
   opts.define("h|help=b");             // short description
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, 29 Jan 2008" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 29 Jan 2008" << endl;
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
   risecount    = opts.getInteger("up-count") - 1;
   fallcount    = opts.getInteger("down-count") - 1;
   ignorerepeat = opts.getBoolean("ignore-repeated-notes");
   debugQ       = opts.getBoolean("debug");
}
  


//////////////////////////////
//
// example -- example usage of the quality program
//

void example(void) {
   // add here
}



//////////////////////////////
//
// usage -- gives the usage statement for the meter program
//

void usage(const char* command) {
  // add here
}


// md5sum: fedf5927e269a1ee1eb1b1e084e104e9 updown.cpp [20090323]

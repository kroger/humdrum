//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jan 12 14:12:54 PST 2005
// Last Modified: Wed Jan 12 15:09:07 PST 2005
// Last Modified: Tue Apr  7 21:05:20 PDT 2009 (added -F option)
// Filename:      ...sig/examples/all/scaletype.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/scaletype.cpp
// Syntax:        C++; museinfo
//
// Description:   Identifies **kern data as pentatonic, sexatonic, 
//                or heptatonic.
// 

#include "humdrum.h"

// function declarations
void   checkOptions             (Options& opts, int argc, char* argv[]);
void   example                  (void);
void   usage                    (const char* command);
void   analyzeFile              (HumdrumFile& infile, Array<int>& pc12,
                                 Array<int>& pc40);
void   printAnalysis            (HumdrumFile& infile, Array<int>& pc12, 
                                 Array<int>& pc40, const char* filename);
int    countPitchClasses        (Array<int>& pc12);
void   printPitches             (Array<int>& pc40);

// global variables
Options      options;            // database for command-line arguments
int          debugQ     = 0;     // used with the --debug option
int          shortQ     = 0;     // used with the -s option
int          pitchesQ   = 0;     // used with the -p option
int          suppressQ  = 0;     // used with -F option


///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   HumdrumFile infile;

   // process the command-line options
   checkOptions(options, argc, argv);

   int numinputs = options.getArgCount();
   const char* filename = "";
   int i;

   Array<int> pc12;  // twelve-tone pitch-class note histogram
   Array<int> pc40;  // base-40 enharmonic pitch-class note histogram

   pc12.setSize(12);
   pc12.allowGrowth(0);
   pc12.setAll(0);

   pc40.setSize(40);
   pc40.allowGrowth(0);
   pc40.setAll(0);


   if (numinputs < 1) {
      // if no command-line arguments read data file from standard input
      infile.read(cin);
      analyzeFile(infile, pc12, pc40);
      if (suppressQ) {
         filename = "";
      }
      printAnalysis(infile, pc12, pc40, filename);
   } else if (options.getArgCount() == 1) {
      infile.read(options.getArg(1));
      if (suppressQ) {
         filename = "";
      }
      analyzeFile(infile, pc12, pc40);
      printAnalysis(infile, pc12, pc40, filename);
   } else {
      for (i=0; i<options.getArgCount(); i++) {
         filename = options.getArg(i+1);
         infile.read(filename);
         if (suppressQ) {
            filename = "";
         }
         analyzeFile(infile, pc12, pc40);
         printAnalysis(infile, pc12, pc40, filename);
      }
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// printAnalysis --
//

void printAnalysis(HumdrumFile& infile, Array<int>& pc12, 
      Array<int>& pc40, const char* filename) {

   int setcount = countPitchClasses(pc12);

   if (filename[0] != '\0') {
      if (shortQ && (strrchr(filename, '/') != NULL)) { 
         cout << strrchr(filename, '/')+1 << ":\t";
      } else {
         cout << filename << ":\t";
      }
   }
   if (setcount < 5)       { cout << "toofew";     }
   else if (setcount == 5) { cout << "pentatonic"; }
   else if (setcount == 6) { cout << "hexatonic";  }
   else if (setcount == 7) { cout << "heptatonic"; }
   else if (setcount > 7)  { cout << "chromatic";  }
   else                    { cout << "error";      }

   if (pitchesQ) {
      cout << "\t";
      printPitches(pc40);
   }

   cout << endl;
}



///////////////////////////////
//
// printPitches --
//

void printPitches(Array<int>& pc40) {
   int i;
   char buffer[128] = {0};
   int upc = countPitchClasses(pc40);
   int count = 0;

   for (i=0; i<pc40.getSize(); i++) {
      if (pc40[i] > 0) {
         cout << Convert::base40ToKern(buffer, i + 120);
         count++;
         if (count < upc) {
            cout << " ";
         }
      }
   }
}



//////////////////////////////
//
// countPitchClasses --
//

int countPitchClasses(Array<int>& pc12) {
   int i;
   int sum = 0;
   for (i=0; i<pc12.getSize(); i++) {
      if (pc12[i] > 0) {
         sum++;
      }
   }

   return sum;
}



//////////////////////////////
//
// analyzeFile -- count the twelve-tone pitch classes in all **kern spines.
//

void analyzeFile(HumdrumFile& infile, Array<int>& pc12, Array<int>& pc40) {
   pc12.setAll(0);
   pc40.setAll(0);

   char buffer[1024] = {0};
   int i, j, k;
   int tokencount = 0;
   int notenum;

   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() != E_humrec_data) {
         // ignore non-data lines in the humdrum file
         continue;
      }
      
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (infile[i].getExInterpNum(j) != E_KERN_EXINT) {
            // ignore non-**kern spine data
            continue;
         }
         if (strcmp(infile[i][j], ".") == 0) {
            // ignore null tokens
            continue;
         }
         tokencount = infile[i].getTokenCount(j);
         for (k=0; k<tokencount; k++) {
            infile[i].getToken(buffer, j, k);
            if (strchr(buffer, 'r') != NULL) {
               // ignore rests
               continue;
            }
            notenum = Convert::kernToMidiNoteNumber(buffer) % 12;
            pc12[notenum]++;
            notenum = Convert::kernToBase40(buffer) % 40;
            pc40[notenum]++;
         }
      }
   }

}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("p|pitches=b",   "display used pitches classes");   
   opts.define("D|no-directory=b", "do not display directory in filename");   
   opts.define("F|no-filename=b",  "do not display filename");   

   opts.define("debug=b",       "trace input parsing");   
   opts.define("author=b",      "author of the program");   
   opts.define("version=b",     "compilation information"); 
   opts.define("example=b",     "example usage"); 
   opts.define("h|help=b",      "short description"); 
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Jan 2005" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 12 Jan 2005" << endl;
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

   debugQ   = opts.getBoolean("debug");
   shortQ   = opts.getBoolean("no-directory");
   pitchesQ = opts.getBoolean("pitches");
   suppressQ = opts.getBoolean("no-filename");

}



//////////////////////////////
//
// example -- example usage of the program
//

void example(void) {
   cout <<
   "                                                                        \n"
   << endl;
}



//////////////////////////////
//
// usage -- gives the usage statement for the program
//

void usage(const char* command) {
   cout <<
   "                                                                        \n"
   << endl;
}


// md5sum: d8db7c428a8274f9ffba82ae03efd376 scaletype.cpp [20090419]

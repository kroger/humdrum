//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 22 15:33:41 PDT 2000
// Last Modified: Sun May 26 19:39:01 PDT 2002 (mostly finished)
// Last Modified: Tue Mar 16 05:53:19 PST 2010 (added *M meter description)
// Filename:      ...sig/examples/all/beat.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/beat.cpp
// Syntax:        C++; museinfo
//
// Description:   Generates metrical location data for **kern entries
//                Test functions for the built-in rhythmic analysis
//                in the HumdrumFile class.  Should give the same
//                output as the beat program.
//
// There are two cases when an incomplete measure needs to
//    be counted backwards.  These cases will be handled by
//    the beat program:
//    (1) an initial pickup beat
//    (2) a repeat sign breaks a true measure
//
// There is a bug that needs fixing:
//   The *M2/2 interpretations are erased (at least with the -s option)
//
// four types of outputs can be given:
//   -s   = sum the number of beats in a measure
//   -b   = display the beat (default if no other output type given)
//   -d   = duration
//   -t   = running total beat/duration
//

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
void      printOutput        (HumdrumFile& file);
double    getPickupDuration  (HumdrumFile& file);

// global variables
Options   options;             // database for command-line arguments
int       appendQ  = 0;        // used with -a option
int       prependQ = 0;        // used with -p option
int       durQ     = 0;        // used with -d option
int       absQ     = 0;        // used with -t option
int       beatQ    = 0;        // used with -b option
int       sumQ     = 0;        // used with -s option
int       zero     = 0;        // zero offset instead of 1 for first beat


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
      // analyze the input file according to command-line options
      infile.analyzeRhythm();

      printOutput(infile);
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// getPickupDuration --
//

double getPickupDuration(HumdrumFile& file) {
   int i;
   for (i=0; i<file.getNumLines(); i++) {
      if (!file[i].isMeasure()) {
         continue;
      }
      return file[i].getAbsBeat();
   }

   return -1;
}



//////////////////////////////
//
// printOutput --
//

void printOutput(HumdrumFile& file) {
   int lastmeasureline = -1;
   int pickupstate = 0;

   for (int i=0; i<file.getNumLines(); i++) {
      switch (file[i].getType()) {
         /*case E_humrec_data_comment:
            if (appendQ) {
               cout << file[i] << "\t" << "!" << "\n";
            }  else if (prependQ) {
               cout << "!\t" << file[i] << "\n";
            } else {
               cout << file[i] << "\n";
            }
            break;
	    */

         case E_humrec_data_kern_measure:
            if (prependQ) {
               cout << file[i][0] << "\t";
               cout << file[i] << "\n";
            } else if (appendQ) {
               cout << file[i] << "\t";
               cout << file[i][0] << "\n";
            } else {
               cout << file[i][0] << "\n";
            }
            lastmeasureline = i;
            break;

         case E_humrec_interpretation:
            if (appendQ) {
               cout << file[i] << "\t";
            } 

            if (strncmp(file[i][0], "**", 2) == 0) {
               if (absQ) {
                  cout << "**absbeat";
               } else if (durQ) {
                  cout << "**dur";
               } else {
                  cout << "**beat";
               }
            } else if (strcmp(file[i][0], "*-") == 0) {
               cout << "*-";
            } else {
               if ((strncmp(file[i][0], "*M", 2) == 0) && 
                  (strchr(file[i][0], '/') != NULL)) {
	          cout << file[i][0];	     
               } else if (appendQ || prependQ) {
                  cout << "*";
               } else {
                  cout << "*";
               }
            }

            if (prependQ) {
               cout << "\t" << file[i];
            }

            cout << "\n";

            break;

         case E_humrec_data:
            if (appendQ) {
               cout << file[i] << "\t";
            } 
            if (file[i][0][0] == '=') {
               pickupstate++;
            }

            if (durQ) {
               cout << file[i].getDuration();
            } else if (absQ) {
               cout << file[i].getAbsBeat();
            } else if (sumQ) {
               if (lastmeasureline > 0) {
                  cout << fabs(file[lastmeasureline].getBeat());
                  pickupstate++;
                  lastmeasureline = -1;
               } else if (pickupstate < 1) {
                  cout << getPickupDuration(file);
                  pickupstate++;
                  lastmeasureline = -1;
               } else {
                  if (appendQ || prependQ) {
                     cout << ".";
                  } else {
                     cout << ".";
                  }
               }
            } else if (beatQ) {
               cout << file[i].getBeat();
            }
            
            if (prependQ) {
               cout << "\t" << file[i];
            }

            cout << "\n";

            break;
         case E_humrec_local_comment:
            if (appendQ) {
               cout << file[i] << "\t";
            }
            cout << "!";
            if (prependQ) {
               cout << "\t" << file[i];
            }
            cout << "\n";

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
   opts.define("a|append=b");           // append analysis to input
   opts.define("p|prepend=b");          // prepend analysis to input
   opts.define("base|timebase=s:");     // rhythmic unit of one beat
   opts.define("b|beat=b");             // display beat of note in measure
   opts.define("abs|absbeat=b");        // display absbeat of note in piece
   opts.define("t|total-beat=b");       // absoute beat location
   opts.define("d|dur|duration=b");     // display rhymic duration of records
   opts.define("s|sum=b");              // sum the duration of each measure
   opts.define("z|zero|zero-offset=b"); // first beat is represented by a 0
   opts.define("n|nulls|keep-nulls=b"); // doesn't remove nulls with -s option
   opts.define("debug=b");              // determine bad input line num
   opts.define("author=b");             // author of program
   opts.define("version=b");            // compilation info
   opts.define("example=b");            // example usages
   opts.define("h|help=b");             // short description
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Oct 2000" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 28 May 2002" << endl;
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

   if (options.getBoolean("zero")) {
      zero = 1;
   }
   appendQ  = opts.getBoolean("append");
   prependQ = opts.getBoolean("prepend");
   durQ     = opts.getBoolean("duration");
   absQ     = opts.getBoolean("total-beat");
   beatQ    = opts.getBoolean("beat");
   sumQ     = opts.getBoolean("sum");
  
   // display beat information if no other output option is given.
   if (!(absQ || durQ)) {
      beatQ = 1;
   }
}
  


//////////////////////////////
//
// example -- example usage of the quality program
//

void example(void) {
   cout <<
   "                                                                         \n"
   "# example usage of the meter program.                                    \n"
   "# analyze a Bach chorale for meter position:                             \n"
   "     meter chor217.krn                                                   \n"
   "                                                                         \n"
   "# display the metrical location spine with original data:                \n"
   "     meter -a chor217.krn                                                \n"
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
   "Analyzes **kern data and generates a rhythmic analysis which gives       \n"
   "the beat location of **kern data records in the measure.  Currently,     \n"
   "input spines cannot split or join.                                       \n"
   "                                                                         \n"
   "Usage: " << command << " [-a][-b base-rhythm][-s|-d][input1 [input2 ...]]\n"
   "                                                                         \n"
   "Options:                                                                 \n"
   "   -a = assemble the analysis spine with the input data.                 \n"
   "   -b = set the base rhythm for analysis to specified kern rhythm value. \n"
   "   -d = gives the duration of each kern record in beat measurements.     \n"
   "   -s = sum the beat count in each measure.                              \n"
   "   --options = list of all options, aliases and default values           \n"
   "                                                                         \n"
   << endl;
}



// md5sum: fba6176a0fa2e484b840086f942d07e6 beat.cpp [20090427]

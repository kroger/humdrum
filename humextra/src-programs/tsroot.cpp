//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Nov 25 00:40:56 PST 2004
// Last Modified: Tue Apr 14 13:41:35 PDT 2009 (fixed interpretations when
//						not appending to original data)
// Last Modified: Wed Jun 24 15:41:09 PDT 2009 (updated for GCC 4.4)
//
// Filename:      ...sig/examples/all/tsroot.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/tsroot.cpp
// Syntax:        C++; museinfo
//
// Description:   Analyze the chord root analysis from Temperley &
//                Sleator Melisma Music Analysis <i>harmony</i> program
//                (2003 version) and optionally append to original **kern data.
//
// Requires the following ancillary programs:
//	(1) meter -- Melisma meter program which adds Beat information to
//	    note data.
//      (2) harmony -- Melisma root analysis program which adds chord 
//	    analysis data to meter program output.
// 	(3) kern2melisma -- museinfo program which converts **kern data into
//          melisma Note data.
// 	(4) harmony2humdrum -- PERL program which converts Melisma harmony
//  	    program output into a temporary Humdrum data file.
//
// For roman-numeral analysis, two more programs are required:
//      (5) key -- Melisma key analysis program
//      (6) key2humdrum -- PERL program which converts Melisma Roman Numeral
//          analysis into a temprary Humdrum data file.
//
// programs are expected to be in two directories:
// 
// meldir = set with --meldir option.  This is the location of the 
//          melisma music analyzer command-line programs (meter, harmony, key).
// midir  = set with --midir option.  This is the location of museinfo programs.
//
// key parameter file contents for roman numeral analysis:
//
//   verbosity=2                      <-- required for segment times
//   default_profile_value = 1.5
//   npc_or_tpc_profile=1
//   scoring_mode = 1
//   segment_beat_level=3
//   beat_printout_level=2
//   romnums=1                        <-- required for roman numeral analysis
//   romnum_type=0
//   running=0
//

#include "humdrum.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "stdio.h"

#ifndef OLDCPP
   #include <fstream>
   using namespace std;
#else
   #include <fstream.h>
#endif

///////////////////////////////////////////////////////////////////////////


// function declarations
void      checkOptions       (Options& opts, int argc, char* argv[]);
void      example            (void);
void      usage              (const char* command);
void      processFile        (const char* filename, HumdrumFile& infile);
void      analyzeTiming      (HumdrumFile& infile, Array<double>& timings,
                              Array<double>& tempo);
void      getAnalysisTimes   (Array<int>& analysistimes, 
                              HumdrumFile& rootanalysis);
void      printAnalysis      (HumdrumFile& rootanalysis, 
                              HumdrumFile& romananalysis,
                              HumdrumFile& infile);
int       getClosestRootLine (Array<int>& analysistimes, int targettime);
void      getDataLineTimings (Array<int>& linetimes, HumdrumFile& infile);
const char* getMatchRoot     (int matchline, HumdrumFile& hfile);
void      createParameterFile(const char* parametertmpname);
void      getRomanAnalysisTimes(Array<int>& romantimes, 
                              HumdrumFile& romananalysis);
void      getRomanAnalysisLines(Array<int>& romanlines, 
                              Array<int>& romantimes, 
                              Array<int>& linetimes, 
                              HumdrumFile& romananalysis);
const char* getRomanData     (HumdrumFile& romananalysis, 
                              Array<int>& romanlines, int target);
void      printRomanKey      (HumdrumFile& romananalysis, 
                              Array<int>& romanlines, int target, 
                              HumdrumRecord& aRecord);

double    dtempo = 120.0;

// global variables
Options   options;            // database for command-line arguments
int       debugQ   = 0;       // used with --debug option
int       appendQ  = 0;       // used with -a option
int       prependQ = 0;       // used with -p option
int       verboseQ = 0;       // used with -v option
int       harmonyQ = 0;       // used with -h option
const char* tmpdir = "/tmp";  // used with --tmpdir option
const char* meldir = "/usr/ccarh/melisma/bin";  // used with --meldir option
const char* midir  = "/var/www/websites/museinfo/bin"; // used with --midir


///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   #ifndef VISUAL
      srand48(time(NULL)*12345);   // seed rand num generator with current time
   #else
      srand(time(NULL)*12345);
   #endif

   HumdrumFile infile;

   // process the command-line options
   checkOptions(options, argc, argv);

   const char* filename;
   infile.clear();
   // if no command-line arguments read data file from standard input
   int numinputs = options.getArgCount();
   if (numinputs < 1) {
      cout << "Error: you must supply at least one input fileme" << endl;
      exit(1);
   } else {
      filename = options.getArg(1);
      infile.read(options.getArg(1));
   }
   processFile(filename, infile);

}

///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// processFile --
//

void processFile(const char* filename, HumdrumFile& infile) {
   // first convert the file to melisma format

   if (verboseQ) {
      cout << "The filename is: " << filename << endl;
   }
   char buffer[4096] = {0};
   const char *ptr = NULL;
   ptr = strrchr(filename, '/');
   if (ptr != NULL) {
      strcpy(buffer, ptr+1);
   } else {
      strcpy(buffer, filename);
   }

   if (verboseQ) {
      cout << "The filename base is: " << buffer << endl;
   }

   #ifndef VISUAL
      int randval = lrand48();
   #else
      int randval = rand();
   #endif 
   if (verboseQ) {
      cout << "Random value is: " << randval << endl;
      cout << "Creating temporary Melisma file: " << endl;
      cout << "\t" << tmpdir << "/" << buffer << "." << randval << endl;
   }
   char commandbuffer[20000] = {0};
   char tmpname[1024] = {0};   // output from the harmony2humdrum command
   char tmpname2[1024] = {0};  // output from the key2humdrum command
   char parametertmpname[1024] = {0};  // key parameter file 
   char harmonytmpname[1024] = {0};

   sprintf(tmpname, "%s/%s.%d", tmpdir, buffer, randval);
   sprintf(harmonytmpname, "%s/%s-%s.%d", tmpdir, buffer, "harmony", randval);
   sprintf(tmpname2, "%s/%s-%s.%d", tmpdir, buffer, "roman", randval);
   sprintf(parametertmpname, "%s/%s-%s.%d", tmpdir, buffer, "parameters", randval);

   if (harmonyQ) {
      createParameterFile(parametertmpname);
      
      sprintf(commandbuffer, 
            "%s/kern2melisma %s | egrep -v '(Reference|Comment|Info)' | %s/meter | %s/harmony > %s; %s/harmony2humdrum %s > %s; %s/key -p %s %s | %s/key2humdrum > %s",
	    midir, filename, meldir, meldir, harmonytmpname, meldir, 
            harmonytmpname, tmpname,
            meldir, parametertmpname, harmonytmpname, meldir, tmpname2);
   } else {
      sprintf(commandbuffer, 
            "%s/kern2melisma %s | egrep -v '(Reference|Comment|Info)' | %s/meter | %s/harmony | %s/harmony2humdrum > %s",
	    midir, filename, meldir, meldir, meldir, tmpname);
   }

   if (verboseQ) {
      cout << "COMMAND: " << commandbuffer << endl;
   }
   int status = system(commandbuffer);
   if (status < 0) {
      cout << "ERROR: command not successful:\n" << commandbuffer << endl;
      exit(1);
   }


   // read in the newly created Humdrum file:
   HumdrumFile rootanalysis;
   rootanalysis.read(tmpname);

   HumdrumFile romananalysis;
   if (harmonyQ) {
      romananalysis.read(tmpname2);
   }

   printAnalysis(rootanalysis, romananalysis, infile);

   // delete temporary files
   if (harmonyQ) {
      sprintf(commandbuffer, "rm -f %s", tmpname);
      if (system(commandbuffer) == -1) {
         exit(-1);
      }
      sprintf(commandbuffer, "rm -f %s", tmpname2);
      if (system(commandbuffer) == -1) {
         exit(-1);
      }
      sprintf(commandbuffer, "rm -f %s", harmonytmpname);
      if (system(commandbuffer) == -1) {
         exit(-1);
      }
      sprintf(commandbuffer, "rm -f %s", parametertmpname);
      if (system(commandbuffer) == -1) {
         exit(-1);
      }
   } else {
      sprintf(commandbuffer, "rm -f %s", tmpname);
      if (system(commandbuffer) == -1) {
         exit(-1);
      }
   }
}



//////////////////////////////
//
// createParameterFile --
//

void createParameterFile(const char* parametertmpname) {
   ofstream pfile(parametertmpname);

   if (!pfile.is_open()) {
      cout << "Error: cannot write parameter file\n";
      exit(1);
   }

   pfile << 
      "verbosity=2\n"
      "default_profile_value = 1.5\n"
      "npc_or_tpc_profile=1\n"
      "scoring_mode = 1\n"
      "segment_beat_level=3\n"
      "beat_printout_level=2\n"
      "romnums=1\n"
      "romnum_type=0\n"
      "running=0\n"
   ;
   pfile.close();

}



//////////////////////////////
//
// printAnalysis --  match Melismla Music Analyzer output back to Humdrum data.
//

void printAnalysis(HumdrumFile& rootanalysis, HumdrumFile& romananalysis, 
      HumdrumFile& infile) {
   const char* romandatum = ".";
   Array<int> analysistimes;
   getAnalysisTimes (analysistimes, rootanalysis);
   Array<int> linetimes;
   getDataLineTimings(linetimes, infile);

   Array<int> romantimes;
   Array<int> romanlines;
   if (harmonyQ) {
      getRomanAnalysisTimes(romantimes, romananalysis);
      getRomanAnalysisLines(romanlines, romantimes, linetimes, romananalysis);
   }

   const char* matchroot;
   int matchline;
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
         case E_humrec_data_comment:

            if (prependQ) {
               if (harmonyQ) cout << "!\t";
               cout << "!\t";
            }
            if (prependQ || appendQ) {
               cout << infile[i];
            }
            if (appendQ) {
               if (harmonyQ) cout << "\t!";
               cout << "\t!";
            }
            cout << endl;

            break;
         case E_humrec_data_kern_measure:

            if (prependQ) {
               if (harmonyQ) cout << infile[i][0] << "\t";
               cout << infile[i][0] << "\t";
            }
            if (prependQ || appendQ) {
               cout << infile[i];
            } else {
               cout << infile[i][0];
               if (harmonyQ) {
                  cout << "\t" << infile[i][0];
               }
            }
            if (appendQ) {
               if (harmonyQ) cout << "\t" << infile[i][0];
               cout << "\t" << infile[i][0];
            }
            cout << endl;

            break;
         case E_humrec_interpretation:

            if (prependQ) {
               if (strcmp(infile[i][0], "*-") == 0) {
                  if (harmonyQ) cout << "*-\t";
                  cout << "*-\t";
               } else if (strncmp(infile[i][0], "**", 2) == 0) {
                  if (harmonyQ) cout << "**tshrm\t";
                  cout << "**tsroot\t";
               } else if (infile[i].equalFieldsQ()) {

                  if ((!infile[i].isSpineManipulator(0)) &&
                        (strchr(infile[i][0], ':') == NULL) ) {
                     if (harmonyQ) cout << infile[i][0] << "\t";
                     cout << infile[i][0] << "\t";
                  } else {
                     if (harmonyQ) cout << "*\t";
                     cout << "*\t";
                  }

               } else {
                  if (harmonyQ) cout << "*\t";
                  cout << "*\t";
               }
            }
            if (prependQ || appendQ) {
               cout << infile[i];
            } else {
               if (strncmp(infile[i][0], "**", 2) == 0) {
                  if (harmonyQ) {
                     cout << "**tsharm\t";
                  }
                  cout << "**tsroot";
                } else if (strcmp(infile[i][0], "*-") == 0) {
                   if (harmonyQ) {
                      cout << "*-\t";
                   }
                   cout << "*-";
                } else if (infile[i].equalFieldsQ()) {
                   if (!infile[i].isSpineManipulator(0)) {
                      if (harmonyQ) cout << infile[i][0] << "\t";
                      cout << infile[i][0];
                   } else {
                      if (harmonyQ) cout << "*\t";
                      cout << "*";
                   }
                } else {
                      if (harmonyQ) cout << "*\t";
                      cout << "*";
                }
            }
            if (appendQ) {
               if (strcmp(infile[i][0], "*-") == 0) {
                  if (harmonyQ) cout << "\t*-";
                  cout << "\t*-";
               } else if (strncmp(infile[i][0], "**", 2) == 0) {
                  if (harmonyQ) cout << "**tshrm\t";
                  cout << "**tsroot\t";
               } else if (infile[i].equalFieldsQ()) {
                  if ((!infile[i].isSpineManipulator(0)) &&
                        (strchr(infile[i][0], ':') == NULL) ) {
                     if (harmonyQ) cout << "\t" << infile[i][0];
                     cout << "\t" << infile[i][0];
                  } else {
                     if (harmonyQ) cout << "\t*";
                     cout << "\t*";
                  }
               } else {
                  if (harmonyQ) cout << "\t*";
                  cout << "\t*";
               }
            }
            cout << endl;

            break;
         case E_humrec_data:
            if (harmonyQ) {
               printRomanKey(romananalysis, romanlines, i, infile[i]);
            }
            // matchline = linetimes[i];
// temporary fix for 12/8 meter analysis problem:
//            matchline = getClosestRootLine(analysistimes, linetimes[i]*2);

            matchline = getClosestRootLine(analysistimes, linetimes[i]);
            matchroot = getMatchRoot(matchline, rootanalysis);
            if (harmonyQ) {
               romandatum = getRomanData(romananalysis, romanlines, i);
            }

            if (prependQ) {
               if (harmonyQ) cout << romandatum << "\t";
               cout << matchroot;
               cout << "\t";
            }
            if (prependQ || appendQ) {
               cout << infile[i];
            } else {
               if (harmonyQ) cout << romandatum << "\t";
               cout << matchroot;
            }
            if (appendQ) {
               if (harmonyQ) cout << "\t" << romandatum;
               cout << "\t";
               cout << matchroot;
            }
            cout << endl;
            
            break;
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
         default:
            cout << infile[i] << endl;
      }
   }


   if (harmonyQ && debugQ) {
      cout << romananalysis;
      cout << endl;
   }

}



//////////////////////////////
//
// printRomanKey --
//

void printRomanKey(HumdrumFile& romananalysis, Array<int>& romanlines, 
   int target, HumdrumRecord& aRecord) {

   int i;
   int j;
   for (i=0; i<romanlines.getSize(); i++) {
      if (romananalysis[i].getType() != E_humrec_interpretation) {
         continue;
      }
      if (target < romanlines[i]) {
         return;
      }
      if (target > romanlines[i]) {
         continue;
      }

      // target == romanlines[i]: assume first column contains data

      if (strchr(romananalysis[i][0], ':') == NULL) {
         continue;
      }

      // found a key interpretation which should be printed

      if (prependQ) {
         cout << romananalysis[i][0] << "\t";
         cout << romananalysis[i][0] << "\t";
      }

      if (prependQ || appendQ) {
         for (j=0; j<aRecord.getFieldCount(); j++) {
            cout << "*";
            if (j < aRecord.getFieldCount() - 1) {
               cout << "\t";
            } 
         }
      } else {
         if (harmonyQ) {
            cout << romananalysis[i][0] << "\t";
         }
         cout << romananalysis[i][0];

      }

      if (appendQ) {
         cout << "\t" << romananalysis[i][0];
         cout << "\t" << romananalysis[i][0];
      }

      cout << "\n";

      romanlines[i] = -1;  // make sure the that key will not be printed again

      break;
   }

}

//////////////////////////////
//
// getRomanData --
//

const char* getRomanData(HumdrumFile& romananalysis, Array<int>& romanlines, 
   int target) {

   int i;
   for (i=0; i<romanlines.getSize(); i++) {
      if (romananalysis[i].getType() != E_humrec_data) {
         continue;
      }
      if (target < romanlines[i]) {
         return ".";
      }
      if (target > romanlines[i]) {
         continue;
      }

      // target == romanlines[i]: assume first column contains data

      if (strcmp(romananalysis[i][0], "|") == 0) {
         continue;
      }

      return romananalysis[i][0];

   }

   // most likely some sort of error if got to here
   return ".";
}




//////////////////////////////
//
// getRomanAnalysisLines --
//

void getRomanAnalysisLines(Array<int>& romanlines, Array<int>& romantimes, 
   Array<int>& linetimes, HumdrumFile& romananalysis) {

   romanlines.setSize(romantimes.getSize());
   romanlines.setAll(-1);

   int i;
   for (i=0; i<romantimes.getSize(); i++) {
      romanlines[i] = getClosestRootLine(linetimes, romantimes[i]);
   }

   if (debugQ) {
      for (i=0; i<romananalysis.getNumLines(); i++) {
         cout << romananalysis[i] << "\t" << romantimes[i]
                                  << "\t" << romanlines[i] << "\n";
      }
   }
}



//////////////////////////////
//
// getRomanAnalysisTimes --
//

void getRomanAnalysisTimes(Array<int>& romantimes, HumdrumFile& romananalysis) {

   romantimes.setSize(romananalysis.getNumLines());
   romantimes.setAll(-1);
   int i, j;
   long timeval;
   for (i=0; i<romananalysis.getNumLines(); i++) {
      if (romananalysis[i].getType() != E_humrec_data) {
         continue;
      }
      for (j=0; j<romananalysis[i].getFieldCount(); j++) {
         if (strcmp(romananalysis[i].getExInterp(j), "**time") != 0) {
            continue;
         }
         if (strcmp(romananalysis[i][j], ".") == 0) {
            romantimes[i] = -100;
         } else {
            timeval = strtol(romananalysis[i][j], (char**)NULL, 10);
            romantimes[i] = timeval;
         }
      }
   }

   int k;
   int count = 0;
   int countother = 0;
   int endingtime = 0;
   double increment = 0.0;
   for (i=0; i<romantimes.getSize(); i++) {
      if (romantimes[i] >= 0) {
         count = 0;
         countother = 0;
         endingtime = -1;
         j = i+1;
         while (j < romantimes.getSize()) {
            if (romantimes[j] < -10) {
               count++;
            } else if (romantimes[j] < 0) {
               countother++;
            } else {
               endingtime = romantimes[j];
               break;
            }
            j++;
         }
         if (count > 0) {
            increment = 1.0*(endingtime - romantimes[i])/count;
         } else {
            increment = 0.0;
         }

         count = 0;
         k = i+1;
         while (k<romantimes.getSize()) {
            if (romantimes[k] < -10) {
               romantimes[k] = (int)(romantimes[i] + increment * count + 0.5);
               count++;
            } else if (romantimes[k] >= 0) {
               break;
            }
            k++;
         }
         i = j-1;
      }
   }

   for (i=romantimes.getSize()-2; i>=0; i--) {
      if (romantimes[i] == -1) {
         romantimes[i] = romantimes[i+1];
      }
   }

   for (i=1; i<romantimes.getSize(); i++) {
      if (romantimes[i] == -1) {
         romantimes[i] = romantimes[i-1];
      }
   }

   // for (i=0; i<romananalysis.getNumLines(); i++) {
   //    cout << romananalysis[i] << "\t" << romantimes[i] << "\n";
   // }

}



//////////////////////////////
//
// getMatchRoot -- return the **tsroot spine info on the given line;
//

const char* getMatchRoot(int matchline, HumdrumFile& hfile) {
   int i;

   if (hfile.getNumLines() <= matchline) {
      return "X";
   } else if (matchline < 0) {
      return "r";
   } else if (hfile[matchline].getType() != E_humrec_data) {
      return "Z";
   }

   for (i=0; i<hfile[i].getFieldCount(); i++) {
      if (strcmp("**tsroot", hfile[matchline].getExInterp(i)) == 0) {
         return hfile[matchline][i];
      }
   }

   // **tsroot data not found
   return "W";
}



//////////////////////////////
//
// getClosestRootLine -- get the nearst root analysis line number
//    given the input time (which is rounded to the nearest 5 ms).
//    Returns -1 if there is no nearest time value.
//

int getClosestRootLine(Array<int>& analysistimes, int targettime) {
   int i;
   int diff = 0;
   int mindiff = 100000;
   int mindiffline = -1;
   for (i=0; i<analysistimes.getSize(); i++) {
      diff = targettime - analysistimes[i];
      if (diff < 0) {
         diff = -diff;
      }
      if (diff < 5) {
         return i;
      }
      if (diff < mindiff) {
         mindiff = diff;
         mindiffline = i;
      }
   }
  
   if (mindiff < 20) {
      return mindiffline;
   } else {
      return -mindiff;
   }

}



//////////////////////////////
//
// getAnalysisTimes -- get the analysis times from the data
//

void getAnalysisTimes(Array<int>& analysistimes, HumdrumFile& rootanalysis) {
   int i;
   analysistimes.setSize(rootanalysis.getNumLines());
   analysistimes.allowGrowth(0);
   analysistimes.setAll(-10000);
   int j;
   int datavalue = 0;
   for (i=0; i<rootanalysis.getNumLines(); i++) {
      if (rootanalysis[i].getType() == E_humrec_data) {
         for (j=0; j<rootanalysis[i].getFieldCount(); j++) {
            if (strcmp("**time", rootanalysis[i].getExInterp(j)) == 0) {
               // found a **time data record on the line, so store the data
               datavalue = atoi(rootanalysis[i][j]);
               analysistimes[i] = datavalue;
            }
         }
      }
   }

}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("r|roman=b",     "add roman numeral analysis as well");
   opts.define("v|verbose=b",   "display intermediate command");
   opts.define("a|append=b",    "append analysis to right side of input data");
   opts.define("p|prepend=b",   "prepend analysis to left side of input data");
   opts.define("tmpdir=s:/tmp", "temporary directory for intermediate output");
   opts.define("meldir=s:/usr/ccarh/melisma/bin", "melisma command dir.");
   opts.define("midir=s:/var/www/websites/museinfo/bin", "museinfo command dir.");
   opts.define("debug=b");           // determine bad input line num
   opts.define("author=b");          // author of program
   opts.define("version=b");         // compilation info
   opts.define("example=b");         // example usages
   opts.define("h|help=b");          // short description
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Nov 2004" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 23 Nov 2004" << endl;
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
   tmpdir   = opts.getString("tmpdir");
   meldir   = opts.getString("meldir");
   midir    = opts.getString("midir");
   appendQ  = opts.getBoolean("append");
   prependQ = opts.getBoolean("prepend");
   harmonyQ = opts.getBoolean("roman");
   if (prependQ) {
      appendQ = 0;
   }

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
// usage -- gives the usage statement for the program
//

void usage(const char* command) {
   cout <<
   "                                                                         \n"
   << endl;
}



//////////////////////////////
//
// analyzeTiming -- determing the absolute time position of each
//     line in the file. (Borrowed from the addtime program).
// 

void analyzeTiming(HumdrumFile& infile, Array<double>& timings,
      Array<double>& tempo) {
   infile.analyzeRhythm("4");

   timings.setSize(infile.getNumLines());
   timings.setAll(0.0);
   tempo.setSize(infile.getNumLines());
   tempo.setAll(dtempo);
   tempo[0] = dtempo;
   double currtempo = dtempo;
   double input = 0.0;
   int count;

   int i;
   for (i=1; i<infile.getNumLines(); i++) {
      // check for new tempo...
      if (strncmp(infile[i][0], "*MM", 3) == 0) {
         count = sscanf(infile[i][0], "*MM%lf", &input);
         if (count == 1) {
            currtempo = input;
            if (i > 0) {
               tempo[i-1] = currtempo;
            }
            tempo[i] = currtempo;
         }
      }

      tempo[i] = currtempo;
      timings[i] = timings[i-1] + (infile[i].getAbsBeat() - 
            infile[i-1].getAbsBeat()) * 60.0/currtempo;

   }

}



//////////////////////////////
//
// getDataLineTimings --  borrowed and modified from addtime program.
//

void getDataLineTimings(Array<int>& linetimes, HumdrumFile& infile) {
   Array<double>  timings;
   Array<double>  tempo;
   analyzeTiming(infile, timings, tempo);


   linetimes.setSize(infile.getNumLines());
   linetimes.allowGrowth(0);
   linetimes.setAll(-5000);

   int i;
   int offset = 0;
   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
         case E_humrec_data:
            linetimes[i]  = (int)((timings[i] + offset + 0.0005) * 1000);
            break;
         case E_humrec_global_comment:
         case E_humrec_bibliography:
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_data_comment:
         case E_humrec_data_measure:
         case E_humrec_data_interpretation:
         default:
            break;
      }

   }
}
// md5sum: eba16e614e3343bbd91df5abe111f924 tsroot.cpp [20090626]

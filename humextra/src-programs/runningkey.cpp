//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun May 13 14:15:43 PDT 2001
// Last Modified: Sun May 13 14:15:46 PDT 2001
// Last Modified: Sun Apr 24 12:48:19 PDT 2005
// Filename:      ...sig/examples/all/keycordl.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/runningkey.cpp
// Syntax:        C++; museinfo
//
// Description:   Key correlation measurements using the Krumhansl-Schmuckler
//		  key-finding profile.
// 
//		  By default, the comparison key profiles are
//		  the Krumhansl probe-tone profiles.  If you
//		  specify the --aa option, the program will use the
//		  Aarden 2003 intermediate form key profiles.
//		  And -s will use a simple key profile which is
//		  1 if it is a scale degree and 0 if it is a non-scale
//		  degree.
// 

#include "humdrum.h"
#include <math.h>
#include <ctype.h>

// function declarations
void   checkOptions             (Options& opts, int argc, char* argv[]);
void   example                  (void);
void   printAnalysis            (int bestkey, Array<double>& scores,
                                 Array<double>& durhist);
void   usage                    (const char* command);
void   readWeights              (const char* filename);
int    analyzeKeyRawCorrelation (double* scores, double* distribution, 
                                 int* pitch, double* durations, int size, 
                                 int rhythmQ, double* majorKey, 
                                 double* minorKey);
int    analyzeKeyEuclidean      (double* scores, double* distribution, 
                                 int* pitch, double* durations, int size, 
                                 int rhythmQ, double* majorKey, 
                                 double* minorKey);
void   normalizeData            (double* data, int asize);
void   adjustData               (double* data, int asize, double mean, 
                                 double sd);
double getStandardDeviation     (double mean, double* data, int asize);
double getMean                  (double* data, int asize);
void   equalizeData             (double* data, int asize, double summation);

// user interface variables
Options      options;           // database for command-line arguments
int          frequencyQ = 0;    // used with -f option
int          allQ       = 0;    // used with -a option
int          rhythmQ    = 1;    // used with -q option
int          binaryQ    = 0;    // used with -b option
int          rawQ       = 0;    // used with --raw option
int          normalizeQ = 0;    // used with -n option
int          euclideanQ = 0;    // used with -e option
int          mmaQ       = 0;    // used with -F option

double* majorKey;
double* minorKey;

// page 35 of Krumhansl: Cognitive foundations of musical pitch
double majorKeyKrumhansl[12] = {
   6.35,    // C
   2.23,    // C#
   3.48,    // D
   2.33,    // D#
   4.38,    // E
   4.09,    // F
   2.52,    // F#
   5.19,    // G
   2.39,    // G#
   3.66,    // A
   2.29,    // A#
   2.88};   // B

double minorKeyKrumhansl[12] = {
   6.33,    // C
   2.68,    // C#
   3.52,    // D
   5.38,    // D#
   2.60,    // E
   3.53,    // F
   2.54,    // F#
   4.75,    // G
   3.98,    // G#
   2.69,    // A
   3.34,    // A#
   3.17};   // B

// page 85 of Temperley: Music and Probability
double majorKeyKostkaPayne[12] = {
   0.748,    // C
   0.060,    // C#
   0.488,    // D
   0.082,    // D#
   0.670,    // E
   0.460,    // F
   0.096,    // F#
   0.715,    // G
   0.104,    // G#
   0.366,    // A
   0.057,    // A#
   0.400};   // B

double minorKeyKostkaPayne[12] = {
   0.712,    // C
   0.084,    // C#
   0.474,    // D
   0.618,    // D#
   0.049,    // E
   0.460,    // F
   0.105,    // F#
   0.747,    // G
   0.404,    // G#
   0.067,    // A
   0.133,    // A#
   0.330};   // B


// from Aarden's dissertation, also displayed graphically in
// Huron: Sweet Anticipation
double majorKeyAarden[12] = {
   17.7661,      // C
    0.145624,    // C#
   14.9265,      // D
    0.160186,    // D#
   19.8049,      // E
   11.3587,      // F
    0.291248,    // F#
   22.062,       // G
    0.145624,    // G#
    8.15494,     // A
    0.232998,    // A#
    4.95122};    // B

double minorKeyAarden[12] = {
   18.2648,      // C
    0.737619,    // C#
   14.0499,      // D
   16.8599,      // D#
    0.702494,    // E
   14.4362,      // F
    0.702494,    // F#
   18.6161,      // G
    4.56621,     // G#
    1.93186,     // A
    7.37619,     // A#
    1.75623   }; // B

// from Bellman's CMMR 2005 paper
double majorKeyBellman[12] = {
   16.80,	// C
    0.86,	// C#
   12.95,	// D
    1.41,	// D#
   13.49,	// E
   11.93,	// F
    1.25,	// F#
   20.28,	// G
    1.80,	// G#
    8.04,	// A
    0.62,	// A#
   10.57 };	// B

double minorKeyBellman[12] = {
   18.16,	// C
    0.69,	// C#
   12.99,	// D
   13.34,	// D#
    1.07,	// E
   11.15,	// F
    1.38,	// F#
   21.07,	// G
    7.49,	// G#
    1.53,	// A
    0.92,	// A#
   10.21 };	// B

// Made up by Craig Sapp (see ICMPC10 paper)
double majorKeySimple[12] = {
   2.0,    // C
   0.0,    // C#
   1.0,    // D
   0.0,    // D#
   1.0,    // E
   1.0,    // F
   0.0,    // F#
   2.0,    // G
   0.0,    // G#
   1.0,    // A
   0.0,    // A#
   1.0};   // B

double minorKeySimple[12] = {
   2.0,    // C
   0.0,    // C#
   1.0,    // D
   1.0,    // D#
   0.0,    // E
   1.0,    // F
   0.0,    // F#
   2.0,    // G
   1.0,    // G#
   0.0,    // A
   1.0,    // A#
   0.0};   // B
   

double majorKeyUser[12] = {0};
double minorKeyUser[12] = {0};

///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   HumdrumFile infile;

   majorKey = majorKeyKrumhansl;
   minorKey = minorKeyKrumhansl;

   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   Array<double> absbeat;
   Array<int>    pitch;
   Array<double> duration;
   Array<double> level;
   Array<double> distribution(12);
   Array<double> scores(24);
  
   int bestkey = 0;
   int i, j;
   for (i=0; i<numinputs || i==0; i++) {
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
      } else {
         infile.read(options.getArg(i+1));
      }

      infile.getNoteArray(absbeat, pitch, duration, level);
      for (j=0; j<pitch.getSize(); j++) {
         pitch[j] = Convert::base40ToMidiNoteNumber(pitch[j]);
      }
      if (rawQ) {
	 if (normalizeQ) { 
            normalizeData(majorKey, 12);
            normalizeData(minorKey, 12);
         }
         bestkey = analyzeKeyRawCorrelation(scores.getBase(), 
               distribution.getBase(), pitch.getBase(), duration.getBase(),
               pitch.getSize(), rhythmQ, majorKey, minorKey);
      } else if (euclideanQ) {
         equalizeData(majorKey, 12, 1.0);
         equalizeData(minorKey, 12, 1.0);
         bestkey = analyzeKeyEuclidean(scores.getBase(), 
               distribution.getBase(), pitch.getBase(), duration.getBase(),
               pitch.getSize(), rhythmQ, majorKey, minorKey);
      } else {
         bestkey = analyzeKeyKS2(scores.getBase(), distribution.getBase(),
               pitch.getBase(), duration.getBase(), pitch.getSize(), rhythmQ,
                     majorKey, minorKey);
      }
      printAnalysis(bestkey, scores, distribution);
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// printAnalysis -- 
//

void printAnalysis(int bestkey, Array<double>& scores, Array<double>& durhist) {
   char buffer[64] = {0};

   if (mmaQ) {
      cout << "{";
      cout << durhist[8]  << ", ";   // G#
      cout << durhist[3]  << ", ";   // D#
      cout << durhist[10] << ", ";   // A#
      cout << durhist[5]  << ", ";   // F
      cout << durhist[0]  << ", ";   // C
      cout << durhist[7]  << ", ";   // G
      cout << durhist[2]  << ", ";   // D
      cout << durhist[9]  << ", ";   // A
      cout << durhist[4]  << ", ";   // E
      cout << durhist[11] << ", ";   // B
      cout << durhist[6]  << ", ";   // F#
      cout << durhist[1]  << "";   // C#
      cout << "};" << endl;
      return;
   } 

   if (bestkey < 12) {
      cout << "The best key is: " 
           << Convert::base12ToKern(buffer, bestkey+12*4)
           << " Major" << "\n";
   } else {
      cout << "The best key is: " 
           << Convert::base12ToKern(buffer, bestkey+12*3)
           << " Minor" << "\n";
   }
   int i;

   if (allQ) {
      for (i=0; i<12; i++) {
         cout << "Major[" << i << "] = " << scores[i] << "\t\t\t"
              << "Minor[" << i << "] = " << scores[i+12] << "\n";
      }
   }

   if (frequencyQ) {
      for (i=0; i<12; i++) {
         cout << "Pitch[" << i << "] = " << durhist[i] << "\n";
      }
   }
}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("a|all=b",             "show all scores");   
   opts.define("Aarden|aarden|aa=b",  "use Aarden profile");   
   opts.define("Bellman|bellman|bb=b","use Bellman profile");   
   opts.define("Temperley|temperley|kp|=b","use Kostka-Payne profile");   
   opts.define("raw=b",               "use raw correlation");
   opts.define("n|normalize=b",       "normalize raw correlation input data");
   opts.define("e|euclidean=b",       "euclidean keyfinding method");
   opts.define("s|simple=b",          "do simple profile");   
   opts.define("D|no-duration=b",     "ignore duration of notes in input");   
   opts.define("f|frequency|freq=b",  "show pitch frequencies");   
   opts.define("F|Freq=b",            "pitch frequencies MMA by fifths");   
   opts.define("w|weights=s:",        "weighting factor file");

   opts.define("debug=b",       "trace input parsing");   
   opts.define("author=b",      "author of the program");   
   opts.define("version=b",     "compilation information"); 
   opts.define("example=b",     "example usage"); 
   opts.define("h|help=b",      "short description"); 
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, May 2001" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 24 April 2005" << endl;
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

   allQ        =  opts.getBoolean("all");
   rhythmQ     = !opts.getBoolean("no-duration");
   frequencyQ  =  opts.getBoolean("frequency");
   if (opts.getBoolean("F")) {
      frequencyQ  =  1;
      mmaQ     = 1;
   }
   rawQ        =  opts.getBoolean("raw");
   normalizeQ  =  opts.getBoolean("normalize");
   euclideanQ  =  opts.getBoolean("euclidean");

   if (opts.getBoolean("simple")) {
      majorKey = majorKeySimple;
      minorKey = minorKeySimple;
   } else if (opts.getBoolean("Aarden")) {
      majorKey = majorKeyAarden;
      minorKey = minorKeyAarden;
   } else if (opts.getBoolean("Bellman")) {
      majorKey = majorKeyBellman;
      minorKey = minorKeyBellman;
   } else if (opts.getBoolean("Temperley")) {
      majorKey = majorKeyKostkaPayne;
      minorKey = minorKeyKostkaPayne;
   } else if (opts.getBoolean("weights")) {
      readWeights(opts.getString("weights"));
      majorKey = majorKeyUser;
      minorKey = minorKeyUser;
   } else {
      majorKey = majorKeyKrumhansl;
      minorKey = minorKeyKrumhansl;
   }
}



//////////////////////////////
//
// readWeights --
//

void readWeights(const char* filename) {

   int i;
   int j;
   int key;
   double value;

   HumdrumFile wfile;
   wfile.read(filename);
   for (i=0; i<wfile.getNumLines(); i++) {
      if (wfile[i].getType() != E_humrec_data) {
         continue;
      }
      key = -1;
      value = -1000000.0;
      for (j=0; j<wfile[i].getFieldCount(); j++) {
         if (strcmp(wfile[i].getExInterp(j), "**kern") == 0) {
            key = Convert::kernToMidiNoteNumber(wfile[i][j]) % 12;
            if (islower(wfile[i][j][0])) {
               key += 12;
            }
         } else if (strcmp(wfile[i].getExInterp(j), "**weight") == 0) {
            sscanf(wfile[i][j], "%lf", &value);
         }
      }
      if ((key >= 0) && (key < 24) && (value != -1000000.0)) {
         if (key < 12) {
            majorKeyUser[key] = value;
         } else {
            minorKeyUser[key-12] = value;
         }
      }
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
// analyzeKeyEuclidean --
//

int analyzeKeyEuclidean (double* scores, double* distribution, 
      int* pitch, double* durations, int size, int rhythmQ, double* majorKey, 
      double* minorKey) {

   int i, j;
   int histogram[12] = {0};

   for (i=0; i<24; i++) {
      scores[i] = 0.0;
   }
   if (size == 0) {
      return -1;  // return -1 if no data to analyze
   }

   for (i=0; i<12; i++) {
      distribution[i] = 0.0;
   }

   // generate a histogram of pitches
   for (i=0; i<size; i++) {
      histogram[pitch[i]%12]++;
      if (rhythmQ) {
         distribution[pitch[i]%12] += durations[i]/4.0;  // dur in whole notes
      } else {
         distribution[pitch[i]%12] += 0.25;
      }
   }
   equalizeData(distribution, 12, 1.0);

   double maj_temp;
   double min_temp;
   int subscript;
     
   double* r_major = scores;
   double* r_minor = scores + 12;
   double value_maj;
   double value_min;

   for (i=0; i<12; i++) {
      maj_temp = min_temp = 0;
   
      // Examine all pitches for each key,
      for (j=0; j<12; j++) {
         subscript = (i+j)%12;

         value_maj = majorKey[j] - distribution[subscript];
         value_min = minorKey[j] - distribution[subscript];
         maj_temp += (value_maj * value_maj);
         min_temp += (value_min * value_min);
      }
      
      if (maj_temp <= 0.0) {
         r_major[i] = 0.0;
      } else {
         r_major[i] = sqrt(maj_temp);
      }

      if (min_temp <= 0.0) {
         r_minor[i] = 0.0;
      } else {
         r_minor[i] = sqrt(min_temp);
      }
   }

   // now figure out which key is the largest.
   int bestkey = 0;
   for (i=1; i<24; i++) {
      if (scores[i] < scores[bestkey]) {  // searching for minimum score
         bestkey = i;
      }
   }

   return bestkey;
}



//////////////////////////////
//
// analyzeKeyKS -- apply the Krumhansl-Schmuckler
//   key-finding algorithm.  Return value is the base-12 most likely
//   key.  scores 0-11 are the major keys starting on C major. 12-23 are
//   the minor keys starting on C minor.  input array scores must have
//   a size of 24 or greater.  input array pitch must have a size of "size".
//

int analyzeKeyRawCorrelation(double* scores, double* distribution, int* pitch, 
      double* durations, int size, int rhythmQ, double* majorKey,
      double* minorKey) {
   int i, j;
   int histogram[12] = {0};

   for (i=0; i<24; i++) {
      scores[i] = 0.0;
   }
   if (size == 0) {
      return -1;  // return -1 if no data to analyze
   }

   for (i=0; i<12; i++) {
      distribution[i] = 0.0;
   }

   // generate a histogram of pitches
   for (i=0; i<size; i++) {
      histogram[pitch[i]%12]++;
      if (rhythmQ) {
         distribution[pitch[i]%12] += durations[i]/4.0;  // dur in whole notes
      } else {
         distribution[pitch[i]%12] += 0.25;
      }
   }
   if (normalizeQ) {
      normalizeData(distribution, 12);
   }

   double maj_temp;
   double min_temp;
   int subscript;
     
   double* r_major = scores;
   double* r_minor = scores + 12;

   for (i=0; i<12; i++) {
      maj_temp = min_temp = 0;
   
      // Examine all pitches for each key,
      for (j=0; j<12; j++) {
         subscript = (i+j)%12;
         maj_temp += (majorKey[j] * distribution[subscript]);
         min_temp += (minorKey[j] * distribution[subscript]);
      }
      
      if (maj_temp <= 0.0) {
         r_major[i] = 0.0;
      } else {
         r_major[i] = maj_temp;
      }

      if (min_temp <= 0.0) {
         r_minor[i] = 0.0;
      } else {
         r_minor[i] = min_temp;
      }
   }

   // now figure out which key is the largest.
   int bestkey = 0;
   for (i=1; i<24; i++) {
      if (scores[i] > scores[bestkey]) {
         bestkey = i;
      }
   }

   return bestkey;
}



//////////////////////////////
//
// normalizeData --
//

void normalizeData(double* data, int asize) {
   double mean = getMean(data, asize);
   double sd =  getStandardDeviation(mean, data, asize);
   adjustData(data, 12, mean, sd);
}



//////////////////////////////
//
// adjustData -- apply mean shift and standard deviation scaling.
//

void adjustData(double* data, int asize, double mean, double sd) {
   int i;
   for (i=0; i<asize; i++) {
      data[i] = (data[i] - mean) / sd;
   }
}



//////////////////////////////
//
// getMean --
//

double getMean(double* data, int asize) {
   if (asize <= 0) {
      return 0.0;
   }

   int i;
   double sum = 0.0;
   for (i=0; i<asize; i++) {
      sum += data[i];
   }

   return sum / asize;
}



//////////////////////////////
//
// getStandardDeviation --
//

double getStandardDeviation(double mean, double* data, int asize) {
   double sum = 0.0;
   double value;
   int i;
   for (i=0; i<asize; i++) {
      value = data[i] - mean;
      sum += value * value;
   }

   return sqrt(sum / asize);
}



//////////////////////////////
//
// equalizeData -- make the max value equal to summation (assuming that
//    all values are non-negative.
//

void equalizeData(double* data, int asize, double summation) {
   double max = 0.0;
   int i;
   for (i=0; i<asize; i++) {
      if (max < data[i]) {
         max = data[i];
      }
   }

   for (i=0; i<asize; i++) {
      data[i] = data[i] / max * summation;
   }
}



//////////////////////////////
//
// equalizeData2 -- make the sum of the sequence equal to summation
//

void equalizeData2(double* data, int asize, double summation) {
   double sum = 0.0;
   int i;
   for (i=0; i<asize; i++) {
      sum += fabs(data[i]);
   }

   for (i=0; i<asize; i++) {
      data[i] = data[i] * summation / sum;
   }
}


// md5sum: 9df4276c4c37b5b431df00e4be811bc0 runningkey.cpp [20090323]

//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun May 13 14:15:43 PDT 2001
// Last Modified: Sun May 13 14:15:46 PDT 2001
// Last Modified: Sun Apr 24 12:48:19 PDT 2005
// Last Modified: Thu May 28 22:30:54 PDT 2009 added continuous analysis
// Last Modified: Mon Mar  7 14:58:02 PST 2011 added --name option
// Last Modified: Mon Sep 10 15:43:07 PDT 2012 added enharmonic key labeling
// Filename:      ...sig/examples/all/keycordl.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/keycor.cpp
// Syntax:        C++; museinfo
//
// Description:   Key correlation measurements using the Krumhansl-Schmuckler
//		  key-finding algorithm. (and also the Gabura algorithm).
// 
// 

#include "humdrum.h"
#include <math.h>
#include <ctype.h>

// function declarations
void   checkOptions             (Options& opts, int argc, char* argv[]);
void   example                  (void);
void   printAnalysis            (int bestkey, Array<double>& scores,
                                 Array<double>& durhist, const char* filename,
                                 Array<int>& b40hist);
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
void   analyzeContinuously      (HumdrumFile& infile, int windowsize,
                                 double stepsize, double* majorKey,
                                 double* minorKey);
void   loadHistograms           (Array<Array<double> >& histograms,
                                 HumdrumFile& infile, int segments);
void   addToHistogramDouble     (Array<Array<double> >& histogram, int pc, 
                                 double start, double dur, double tdur, 
				 int segments);
void   createHistogram          (Array<double>& pitchhist, int start, 
                                 int count, Array<Array<double> >& segments);
void   identifyKeyDouble        (Array<double>& histogram, 
                                 Array<double>& correlations,
                                 double* majorKey, double* minorKey);
void   printBestKey             (int keynumber);
double pearsonCorrelation       (int size, double* x, double* y);
void   printCorrelation         (double value, int style);
void   printHistogramTotals     (Array<Array<double> >& segments);
double getConfidence            (Array<double>& cors, int best);
void   getLocations             (Array<double>& measures, HumdrumFile& infile, 
                                 int segments);
void   getBase40Histogram       (Array<int>& base40, HumdrumFile& infile);
int    identifyBranchCut        (int base12, Array<int>& base40);

// user interface variables
Options      options;           // database for command-line arguments
int          frequencyQ  = 0;    // used with -f option
int          allQ        = 0;    // used with -a option
int          rhythmQ     = 1;    // used with -q option
int          binaryQ     = 0;    // used with -b option
int          rawQ        = 0;    // used with --raw option
int          normalizeQ  = 0;    // used with -n option
int          euclideanQ  = 0;    // used with -e option
int          mmaQ        = 0;    // used with -F option
double       stepsize    = 1.0;  // used with --step option
int          windowsize  = 32;   // used with --window option
int          continuousQ = 0;    // used with -c option
int          roundQ      = 1;    // used with -R option
int          debugQ      = 0;    // used with --debug option
int          nameQ       = 0;    // used with --name option

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

   majorKey = majorKeyBellman;
   minorKey = minorKeyBellman;

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
   const char* filename = "";

   Array<int> b40hist;
  
   int bestkey = 0;
   int i, j;
   for (i=0; i<numinputs || i==0; i++) {
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
         filename = "";
      } else {
         infile.read(options.getArg(i+1));
         filename = options.getArg(i+1);
      }

      if (continuousQ) {
         analyzeContinuously(infile, windowsize, stepsize, majorKey, minorKey);
         continue;
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
      getBase40Histogram(b40hist, infile);
      printAnalysis(bestkey, scores, distribution, filename, b40hist);
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// analyzeContinuously --
//

void analyzeContinuously(HumdrumFile& infile, int windowsize,
      double stepsize, double* majorKey, double* minorKey) {

   Array<Array<double> > segments;
   infile.analyzeRhythm("4");
   int segmentCount = int(infile.getTotalDuration() / stepsize + 0.5);

   if (segmentCount < windowsize) {
      cout << "Not enough data for requested analysis" << endl;
      return;
   }

   segments.setSize(segmentCount);
   segments.allowGrowth(0);
   int i;
   for (i=0; i<segments.getSize(); i++) {
      segments[i].setSize(12);
      segments[i].allowGrowth(0);
      segments[i].setAll(0);
   }
	    
   loadHistograms(segments, infile, segmentCount);

   if (debugQ) {
      printHistogramTotals(segments);
   }

   Array<Array<double> > pitchhist; 
   Array<Array<double> > correlations;

   pitchhist.setSize(segmentCount-windowsize);
   correlations.setSize(segmentCount-windowsize);
   pitchhist.allowGrowth(0);
   correlations.allowGrowth(0);
   for (i=0; i<segmentCount-windowsize; i++) {
      pitchhist[i].setSize(13);  //last spot for best key
      pitchhist[i].allowGrowth(0);
      correlations[i].setSize(24);
      correlations[i].allowGrowth(0);
   }

   for (i=0; i<segmentCount - windowsize; i++) {
      createHistogram(pitchhist[i], i, windowsize, segments);
      identifyKeyDouble(pitchhist[i], correlations[i], majorKey, minorKey);
   }


   Array<double> measures;
   getLocations(measures, infile, segmentCount);

   cout << "**key\t**rval\t**conf\t**start\t**mid\t**end\n";
   for (i=0; i<pitchhist.getSize(); i++) {
      printBestKey(int(pitchhist[i][12] + 0.5));
      cout << "\t";
      printCorrelation(correlations[i][int(pitchhist[i][12]+ 0.1)], roundQ);
      cout << "\t";
      cout << getConfidence(correlations[i], int(pitchhist[i][12]+0.1));
      cout << "\t";
      cout << "=" << measures[i];
      cout << "\t";
      cout << "=" << int((measures[i] + measures[i+windowsize])/2+0.49);
      cout << "\t";
      cout << "=" << measures[i+windowsize];
      cout << endl;
   }
   cout << "*-\t*-\t*-\t*-\t*-\t*-\n";

}



//////////////////////////////
//
// getLocations -- return the measure number in which a segment
//    starts.
//

void getLocations(Array<double>& measures, HumdrumFile& infile, int segments) {
   infile.analyzeRhythm("4");
   double totaldur = infile.getTotalDuration();

   measures.setSize(segments);
   measures.allowGrowth(0);
   measures.setAll(-1);

   int barnum = 0;
   int index;

   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isMeasure()) {
         continue;
      }
      if (sscanf(infile[i][0], "=%d", &barnum) == 1) {
         index = int(infile[i].getAbsBeat() / totaldur * segments);
         measures[index] = barnum;
      }
   }

   for (i=1; i<segments; i++) {
      if (measures[i] < 0) {
         measures[i] = measures[i-1];
      } 
   }

   index = -1;
   for (i=0; i<segments; i++) {
      if (measures[i] != -1) {
         index = i - 1;
         break;
      }
   }

   if (index < 0) {
      return;
   }
   if (index >= segments) {
      return;
   }

   for (i=index; i>=0; i--) {
      measures[i] = measures[index+1] - 1;
   }

}



//////////////////////////////
//
// printHistogramTotals --
//

double getConfidence(Array<double>& cors, int best) {
   int i;
   int start = 1;
   int secondbest = 0;
   if (best == 0) {
      secondbest = 1;
      start = 2;
   }
   for (i=start; i<24; i++) {
      if (best == i) {
         continue;
      }
      if (cors[i] > cors[secondbest]) {
         secondbest = i;
      }
   }

   double output = (cors[best] - cors[secondbest]) * 300;
   output = int(output + 0.5);
   if (output > 100.0) {
      output = 100.0;
   }
	     
   return output;
}



//////////////////////////////
//
// printHistogramTotals --
//

void printHistogramTotals(Array<Array<double> >& segments) {
   Array<double> sums(12);
   sums.allowGrowth(0);
   sums.setAll(0);
   int i, j;
   for (i=0; i<segments.getSize(); i++) {
      for (j=0; j<12; j++) {
         sums[j] += segments[i][j];
      }
   }

   cout << "!! C  = " << sums[0]  << endl;
   cout << "!! C# = " << sums[1]  << endl;
   cout << "!! D  = " << sums[2]  << endl;
   cout << "!! D# = " << sums[3]  << endl;
   cout << "!! E  = " << sums[4]  << endl;
   cout << "!! F  = " << sums[5]  << endl;
   cout << "!! F# = " << sums[6]  << endl;
   cout << "!! G  = " << sums[7]  << endl;
   cout << "!! G# = " << sums[8]  << endl;
   cout << "!! A  = " << sums[9]  << endl;
   cout << "!! A# = " << sums[10] << endl;
   cout << "!! B  = " << sums[11] << endl;
}



//////////////////////////////
//
// printCorrelation --
//

void printCorrelation(double value, int style) {
   if (style == 0) {
      cout << value;
      return;
   }
   if (value == 1.0) {
      cout << "1.000"; 
      return;
   } else if (value == 0.0) {
      cout << "0.000";
      return;
   } else if (value == -1.0) {
      cout << "-1.000";
      return;
   }
   int newvalue = int(value * 1000.0 + 0.5);
   int digit1 = newvalue % 10;
   newvalue = newvalue / 10;
   int digit2 = newvalue % 10;
   newvalue = newvalue / 10;
   int digit3 = newvalue % 10;
   newvalue = newvalue / 10;
   cout << "0." << digit3 << digit2 << digit1;
}



//////////////////////////////
//
// pearsonCorrelation --
//

double pearsonCorrelation(int size, double* x, double* y) {

   double sumx  = 0.0;
   double sumy  = 0.0;
   double sumco = 0.0;
   double meanx = x[0];
   double meany = y[0];
   double sweep;
   double deltax;
   double deltay;

   int i;
   for (i=2; i<=size; i++) {
      sweep = (i-1.0) / i;
      deltax = x[i-1] - meanx;
      deltay = y[i-1] - meany;
      sumx  += deltax * deltax * sweep;
      sumy  += deltay * deltay * sweep;
      sumco += deltax * deltay * sweep;
      meanx += deltax / i;
      meany += deltay / i;
   }

   double popsdx = sqrt(sumx / size);
   double popsdy = sqrt(sumy / size);
   double covxy  = sumco / size;

   return covxy / (popsdx * popsdy);
}



//////////////////////////////
//
// printBestKey --
//

void printBestKey(int keynumber) {
   switch (keynumber) {
      // major keys
      case 0:	cout << "C";	return;
      case 1:	cout << "D-";	return;
      case 2:	cout << "D";	return;
      case 3:	cout << "E-";	return;
      case 4:	cout << "E";	return;
      case 5:	cout << "F";	return;
      case 6:	cout << "F#";	return;
      case 7:	cout << "G";	return;
      case 8:	cout << "A-";	return;
      case 9:	cout << "A";	return;
      case 10:	cout << "B-";	return;
      case 11:	cout << "B";	return;

      // minor keys
      case 12:	cout << "c";	return;
      case 13:	cout << "c#";	return;
      case 14:	cout << "d";	return;
      case 15:	cout << "e-";	return;
      case 16:	cout << "e";	return;
      case 17:	cout << "f";	return;
      case 18:	cout << "f#";	return;
      case 19:	cout << "g";	return;
      case 20:	cout << "g#";	return;
      case 21:	cout << "a";	return;
      case 22:	cout << "b-";	return;
      case 23:	cout << "b";	return;
   }
}



//////////////////////////////
//
// createHistogram --
//

void createHistogram(Array<double>& pitchhist, int start, int count, 
   Array<Array<double> >& segments) {

   pitchhist.setAll(0);
   int i, j;
   for (i=0; i<count; i++) {
      for (j=0; j<12; j++) {
         pitchhist[j] += segments[i+start][j];
      }
   }
}



////////////////////////////////////////
//
// identifyKeyDouble --
//

void identifyKeyDouble(Array<double>& histogram, Array<double>& correlations,
   double* majorKey, double* minorKey) {
   int i;

   double h[24];
   for (i=0; i<12; i++) {
      h[i] = histogram[i];
      h[i+12] = h[i];
   }

   double testsum = 0.0;
   for (i=0; i<12; i++) {
      testsum += h[i];
      correlations[i]    = pearsonCorrelation(12, majorKey, h+i);
      correlations[i+12] = pearsonCorrelation(12, minorKey, h+i);
   }

   if (testsum == 0.0) {
      histogram[12] = 24;  // empty histogram, so going to display black
      return;
   }

   // find max value
   int besti = 0;
   double bestsum = correlations[0];
   for (i=1; i<24; i++) {
      if (correlations[i] > bestsum) {
         besti = i;
         bestsum = correlations[i];
      }
   }

   histogram[12] = besti;
}



//////////////////////////////
//
// loadHistograms --
//

void loadHistograms(Array<Array<double> >& histograms,
   HumdrumFile& infile, int segments) {

   infile.analyzeRhythm("4");
   double totalduration = infile.getTotalDuration();

   double duration;
   int i;
   int j;
   int k;
   char buffer[10000] = {0};
   int pitch;
   double start;
   int tokencount;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() !=  E_humrec_data) {
         continue;
      }
      start = infile[i].getAbsBeat();
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strcmp(infile[i].getExInterp(j), "**kern") != 0) {
            continue;
         }
         if (strcmp(infile[i][j], ".") == 0) {
            continue; // ignore null tokens
         }
         tokencount = infile[i].getTokenCount(j);
         for (k=0; k<tokencount; k++) {
            infile[i].getToken(buffer, j, k);
            if (strcmp(buffer, ".") == 0) {
               continue;  // ignore illegal inline null tokens
            }
            pitch = Convert::kernToMidiNoteNumber(buffer);
            if (pitch < 0) {
               continue;  // ignore rests or strange objects
            }
            pitch = pitch % 12;  // convert to chromatic pitch-class
            duration = Convert::kernToDuration(buffer);
            if (duration <= 0.0) {
               continue;   // ignore grace notes and strange objects
            }
            
            addToHistogramDouble(histograms, pitch,
                  start, duration, totalduration, segments);
         }
      }
   }
}



//////////////////////////////
//
// addToHistogramDouble -- fill the histogram in the right spots.
//

void addToHistogramDouble(Array<Array<double> >& histogram, int pc, 
      double start, double dur, double tdur, int segments) {

   pc = pc % 12;

   double startseg = start / tdur * segments;
   double startfrac = startseg - (int)startseg;

   double segdur = dur / tdur * segments;

   if (segdur <= 1.0 - startfrac) {
      histogram[(int)startseg][pc] += segdur;
      return;
   } else if (1.0 - startfrac > 0.0) {
      histogram[(int)startseg][pc] += (1.0 - startfrac);
      segdur -= (1.0 - startfrac);
   }

   int i = (int)(startseg + 1);
   while (segdur > 0.0 && i < histogram.getSize()) {
      if (segdur < 1.0) {
         histogram[i][pc] += segdur;
         segdur = 0.0;
      } else {
         histogram[i][pc] += 1.0;
         segdur -= 1.0;
      }
      i++;
   }
}



//////////////////////////////
//
// printAnalysis -- 
//

void printAnalysis(int bestkey, Array<double>& scores, Array<double>& durhist,
      const char* filename, Array<int>& b40hist) {
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

   int best40;

   if (bestkey < 12) {
      if (nameQ) {
         cout << filename << ":\t";
      } else {
         cout << "The best key is: ";
      }
      best40 = identifyBranchCut(bestkey, b40hist);
      // cout << Convert::base12ToKern(buffer, bestkey+12*4)
      cout << Convert::base40ToKern(buffer, best40+40*3)
           << " Major" << "\n";
   } else {
      if (nameQ) {
         cout << filename << ":\t";
      } else {
         cout << "The best key is: ";
      }
      best40 = identifyBranchCut(bestkey, b40hist);
      // cout << Convert::base12ToKern(buffer, bestkey+12*3)
      cout << Convert::base40ToKern(buffer, best40+40*3)
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
   opts.define("Aarden|aarden|aa=b",  "use Aarden profiles");   
   opts.define("Bellman|bellman|bb=b","use Bellman profiles");   
   opts.define("Krumhansl|k|kk=b",    "use Krumhansl-Kessler profiles");   
   opts.define("Temperley|temperley|kp|=b","use Kostka-Payne profiles");   
   opts.define("G|gabura|raw=b",      "use raw correlation");
   opts.define("n|normalize=b",       "normalize raw correlation input data");
   opts.define("e|euclidean=b",       "euclidean keyfinding method");
   opts.define("s|simple=b",          "do simple profile");   
   opts.define("D|no-duration=b",     "ignore duration of notes in input");   
   opts.define("f|frequency|freq=b",  "show pitch frequencies");   
   opts.define("F|Freq=b",            "pitch frequencies MMA by fifths");   
   opts.define("w|weights=s:",        "weighting factor file");
   opts.define("step=d:1.0",          "step size for continuous analysis");
   opts.define("window=i:32",         "window size for continuous analysis");
   opts.define("c|continuous=b",      "continuous analysis");
   opts.define("R|no-round=b",        "don't round correlation values");
   opts.define("name=b",              "print filenames");

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
      cout << argv[0] << ", version: 26 May 2009" << endl;
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
   continuousQ =  opts.getBoolean("continuous");
   windowsize  =  opts.getInteger("window");
   stepsize    =  opts.getDouble("step");
   roundQ      = !opts.getBoolean("no-round");
   debugQ      =  opts.getBoolean("debug");
   nameQ       =  opts.getBoolean("name");

   if (opts.getBoolean("Krumhansl")) {
      majorKey = majorKeyKrumhansl;
      minorKey = minorKeyKrumhansl;
   } else if (opts.getBoolean("Aarden")) {
      majorKey = majorKeyAarden;
      minorKey = minorKeyAarden;
   } else if (opts.getBoolean("simple")) {
      majorKey = majorKeySimple;
      minorKey = minorKeySimple;
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
      // default weightings set at start of main().
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
// analyzeKeyRawCorrelation -- apply the Krumhansl-Schmuckler
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



//////////////////////////////
//
// getBase40Histogram -- Get the enharmonic pitch-class histogram for the
//    music.  Counts note attacks only not weighted by durations, but 
//    difference between duration weighting and not duration weighting 
//    should be negligible.
//

void getBase40Histogram(Array<int>& base40, HumdrumFile& infile) {
   base40.setSize(40);
   base40.allowGrowth(0);
   base40.setAll(0);
   int tcount;
   int i, j, k;
   int b40;
   char buffer[1024] = {0};
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isData()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (!infile[i].isExInterp(j, "**kern")) {
            continue;
         }
         if (strcmp(infile[i][j], ".") == 0) {
            // ignore null records.
            continue;
         }
         if (strchr(infile[i][j], 'r') != NULL) {
            // ignore rests
            continue;
         }
         tcount = infile[i].getTokenCount(j);
         for (k=0; k<tcount; k++) {
            infile[i].getToken(buffer, j, k);
            if (strchr(buffer, ']') != NULL) {
               continue;
            }
            if (strchr(buffer, '_') != NULL) {
               continue;
            }
            b40 = Convert::kernToBase40(buffer);
            if (b40 <0) {
               continue;
            }
            base40[b40%40]++;
         }
      }
   }
}



//////////////////////////////
//
// identifyBranchCut -- distinguish between G# major and A-flat minor 
//     for example.  Currently only checking for enharmonic differences
//     between black-key notes (i.e., cannot distinguish between 
//     C major and B-sharp major, although the function could be
//     generalized to do so).  Returns a base-40 pitch class which
//     can represent diatonic pitches with up to 2 chromatic step 
//     alterations.
//

int identifyBranchCut(int base12, Array<int>& base40) {
   int tval       = Convert::base12ToBase40(base12+5*12);
   int accidental = Convert::base40ToAccidental(tval);
   int diatonic   = Convert::base40ToDiatonic(tval) % 7;

   if (accidental == 0) {
      return tval % 40;
   }

   int sdia, fdia;
   
   if (accidental == -1) {
      sdia = (diatonic+6)%7;
      fdia = diatonic;
   } else if (accidental == +1) {
      sdia = diatonic;
      fdia = (diatonic+1)%7;
   } else {
      // something strange happened
      return tval % 40;
   }
  
   int sb40, fb40;
   sb40 = (Convert::base7ToBase40(sdia+4*7) + 1)%40;
   fb40 = (Convert::base7ToBase40(fdia+4*7) - 1)%40;

   int sscore, fscore;
   sscore = base40[sb40] + base40[(sb40+23)%40];
   fscore = base40[fb40] + base40[(fb40+23)%40];

   if (sscore > fscore) {
      return sb40;
   } else if (fscore > sscore) {
      return fb40;
   }

   // some strange condition where scores are equivalent.
   return tval%40;
}



// md5sum: 5c1f12dfa25f697ca76f2908c8e2942d keycor.cpp [20121016]

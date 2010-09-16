//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 22 15:33:41 PDT 2000
// Last Modified: Sun May 26 19:39:01 PDT 2002 (mostly finished)
// Last Modified: Tue Mar 16 05:53:19 PST 2010 (added *M meter description)
// Last Modified: Wed Apr 21 14:31:44 PDT 2010 (added search feature)
// Last Modified: Wed May 19 15:30:49 PDT 2010 (added tick & rational values)
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
// five types of outputs can be given:
//   -s   = sum the number of beats in a measure
//        = display the beat (default if no other output type given)
//   -d   = duration
//   -c   = cumulative running total beat/duration
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
void      printOutput        (HumdrumFile& file, 
		              Array<RationalNumber>& Bfeatures, 
                              Array<int>& Blines, 
			      Array<RationalNumber>& Dfeatures, 
			      Array<int>& Dlines, Array<int>& tickanalysis);
RationalNumber getPickupDuration (HumdrumFile& file);
void      fillSearchString   (Array<double>& searcher, const char* string);
void      printSearchResults (HumdrumFile& infile, 
		              Array<RationalNumber>& Bfeatures,
                              Array<int>& Blines, 
			      Array<RationalNumber>& Dfeatures,
                              Array<int>& Dlines);
void      printSearchResultsFinal(Array<int>& linematch, HumdrumFile& infile,
                              Array<RationalNumber>& Bfeatures, 
			      Array<int>& Blines, 
                              Array<RationalNumber>& Dfeatures, 
			      Array<int>& Dloines);
void      doBeatSearch       (Array<int>& results, HumdrumFile& infile, 
                              Array<double> search, 
			      Array<RationalNumber>& Bfeatures,
                              Array<int>& Blines);
void      doDurSearch        (Array<int>& results, HumdrumFile& infile, 
                              Array<double> search, 
			      Array<RationalNumber>& Dfeatures,
                              Array<int>& Dlines);
void      doDurSearch        (Array<int>& results, HumdrumFile& infile, 
                              Array<double> search, Array<double>& Dfeatures, 
			      Array<int>& Dlines);
void      mergeResults       (Array<int>& output, Array<int>& input1, 
                              Array<int>& input2);
void      printSequence      (Array<double>& pattern);
void      printSequence      (Array<RationalNumber>& pattern);
void      fillMeasureInfo    (HumdrumFile& infile, Array<double>& measures);
void      doComparison       (Array<int>& results, Array<int>& line, 
                              Array<double>& search, Array<double>& data,
                              HumdrumFile& infile);
int       checkForWildcard   (Array<double>& sequence);
void      extractBeatFeatures(HumdrumFile& infile, Array<int>& line,
                              Array<RationalNumber>& data);
void      extractDurFeatures (HumdrumFile& infile, Array<int>& line,
                              Array<RationalNumber>& data);
void      printSequence      (Array<double>& features, Array<int>& lines, 
                              Array<double>& search, int startline);
void      printSequence      (Array<RationalNumber>& features, 
		              Array<int>& lines, Array<RationalNumber>& search,
			      int startline);
void      printSequence      (Array<RationalNumber>& features, 
		              Array<int>& lines, Array<double>& search, 
			      int startline);
void      printMatchesWithData(Array<int>& linematch, HumdrumFile& infile);
void      fillAttackArray    (HumdrumFile& infile, Array<int>& attacks);
int       getCountForLine    (HumdrumFile& infile, int line);
int       doTickAnalysis     (Array<int>& tickanalysis, HumdrumFile& infile);
RationalNumber getDurationOfFirstMeasure(HumdrumFile& file);

// global variables
Options   options;             // database for command-line arguments
int       appendQ  = 0;        // used with -a option
int       prependQ = 0;        // used with -p option
int       durQ     = 0;        // used with -d option
int       absQ     = 0;        // used with -t option
int       beatQ    = 0;        // used with -b option
int       sumQ     = 0;        // used with -s option
int       zeroQ    = 0;        // zero offset instead of 1 for first beat
int       nullQ    = 0;        // used with -n option
Array<double> Bsearch;         // used with -B option
Array<double> Dsearch;         // used with -D option
double    Rvalue   = -1.0;     // used with -R option
double    Tolerance = 0.001;   // used for rounding
int       Attack   = 1;        // used with -A option
Array<int> Attacks;            // used with -A option
int       tickQ    = 0;        // used with -t option
int       rationalQ= 0;        // used with -r option
int       tpwQ     = 0;        // used with --tpw option
int       tpqQ     = 0;        // used with --tpq option
const char* beatbase = "4";    // used with --beatsize option
int       uQ       = 0;        // used for -f and -u interactions

///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   HumdrumFile infile;

   Array<RationalNumber> Bfeatures; // used to extract beat data from input
   Array<RationalNumber> Dfeatures; // used to extract duration data from input
   Array<int>    Blines;            // used to extract beat data from input
   Array<int>    Dlines;            // used to extract duration data from input

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
      infile.analyzeRhythm(beatbase);

      Array<int> tickanalysis;
      tickanalysis.setSize(infile.getNumLines());
      tickanalysis.setAll(0);
      int tickfactor = 1;

      if (tickQ) {
         tickfactor = doTickAnalysis(tickanalysis, infile);
      }

      if (tpwQ) {
         cout << infile.getMinTimeBase() * tickfactor << endl;
         exit(0);
      } else if (tpqQ) {
         cout << infile.getMinTimeBase() * tickfactor /4.0 << endl;
         exit(0);
      }

      fillAttackArray(infile, Attacks);
      extractBeatFeatures(infile, Blines, Bfeatures);
      extractDurFeatures(infile, Dlines, Dfeatures);

      if (Bsearch.getSize() > 0 || Dsearch.getSize() > 0) {
         printSearchResults(infile, Bfeatures, Blines, Dfeatures, Dlines);
      } else {
         printOutput(infile, Bfeatures, Blines, Dfeatures, Dlines, 
               tickanalysis);
      }
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////



//////////////////////////////
//
// doTickAnalysis --
//

int doTickAnalysis(Array<int>& tickanalysis, HumdrumFile& infile) {
   int i;
   tickanalysis.setSize(infile.getNumLines());

   Array<RationalNumber> pretick(tickanalysis.getSize());

   int minrhy = infile.getMinTimeBase();
   if (minrhy <= 0.0) {
      return 1;
   }
   RationalNumber value;
   int monitor = 0;
   for (i=0; i<infile.getNumLines()-1; i++) {
      value = ((infile[i+1].getAbsBeatR()-infile[i].getAbsBeatR())/4)*minrhy;
      pretick[i] = value;
      if (value.getDenominator() != 1) {
         monitor = 1;
      }
   }

   if (monitor == 0) {
      for (i=0; i<pretick.getSize(); i++) {
         tickanalysis[i] = pretick[i].getNumerator();
      }
      return 1;
   }

   for (i=0; i<pretick.getSize(); i++) {
      // estimate a multiplication of 4 to remove fractional part.
      tickanalysis[i] = pretick[i].getNumerator() * 4;
   }

   return 4;
}



//////////////////////////////
//
// fillAttackArray --
//

void fillAttackArray(HumdrumFile& infile, Array<int>& attacks) {
   int i;
   attacks.setSize(infile.getNumLines());
   attacks.allowGrowth(0);
   attacks.setAll(0);

   if (Attack <= 0) {
      // don't need to waste time analyzing the attack structure of the data...
      return;
   }

   int count;
   for (i=0; i<infile.getNumLines(); i++) {
      count = 0;
      if (infile[i].isData()) {
         count = getCountForLine(infile, i);
      }
      attacks[i] = count;
   }
}



//////////////////////////////
//
// getCountForLine -- return the number of note attacks on the given
//   line in the Humdrum File.  Only **kern data is examined.  Rests
//   and tied notes are ignored when counting attacks.
//

int getCountForLine(HumdrumFile& infile, int line) {
   int j, k;
   char buffer[128] = {0};
   int output = 0;
   int tcount;
   for (j=0; j<infile[line].getFieldCount(); j++) {
      if (!infile[line].isExInterp(j, "**kern")) {
         continue;
      }
      tcount = infile[line].getTokenCount(j);
      for (k=0; k<tcount; k++) {
         infile[line].getToken(buffer, j, k);
         if (strchr(buffer, 'r') != NULL) {
            // ignore rests
            continue;
         }
         if (strcmp(buffer, ".") == 0) {
            // ignore null tokens
            continue;
         }
         if (strchr(buffer, '_') != NULL) {
            // ignore notes which are in the middle of a series of tied notes
            continue;
         }
         if (strchr(buffer, ']') != NULL) {
            // ignore notes which are at the end of a series of tied notes
            continue;
         }
         output++;
      }
   }

   return output;
}



//////////////////////////////
//
// printSearchResults -- Search for beat and/or duration patterns
//   in the composition rhythmic data 
//

void printSearchResults(HumdrumFile& infile, Array<RationalNumber>& Bfeatures,
      Array<int>& Blines, Array<RationalNumber>& Dfeatures, 
      Array<int>& Dlines) {
   Array<int> Bresults;
   Array<int> Dresults;
   Bresults.setSize(100000);
   Bresults.setGrowth(100000);
   Bresults.setSize(0);
   Dresults.setSize(100000);
   Dresults.setGrowth(100000);
   Dresults.setSize(0);

   if ((Bsearch.getSize() > 0) && (Dsearch.getSize() > 0)) {
      doBeatSearch(Bresults, infile, Bsearch, Bfeatures, Blines);
      doDurSearch(Dresults, infile, Dsearch, Dfeatures, Dlines);
      Array<int> finalresults;
      mergeResults(finalresults, Bresults, Dresults);

      cout << "!!parallel beat search: ";
      printSequence(Bsearch);
      cout << endl;
      cout << "!!parallel duration search: ";
      printSequence(Dsearch);
      cout << endl;

      printSearchResultsFinal(finalresults, infile, Bfeatures, Blines,
            Dfeatures, Dlines);
   } else if (Bsearch.getSize() > 0) {
      doBeatSearch(Bresults, infile, Bsearch, Bfeatures, Blines);
      cout << "!!beat search: ";
      printSequence(Bsearch);
      cout << endl;
      printSearchResultsFinal(Bresults, infile, Bfeatures, Blines,
            Dfeatures, Dlines);
   } else if (Dsearch.getSize() > 0) {
      doDurSearch(Dresults, infile, Dsearch, Dfeatures, Dlines);
      cout << "!!duration search: ";
      printSequence(Dsearch);
      cout << endl;
      printSearchResultsFinal(Dresults, infile, Bfeatures, Blines,
            Dfeatures, Dlines);
   } else {
      cout << "ERROR in search" << endl;
   }

}



//////////////////////////////
//
// printSequence --
//

void printSequence(Array<double>& pattern) {
   int i;
   for (i=0; i<pattern.getSize(); i++) {
      if (pattern[i] < 0) {
         cout << "*";
      } else {
         cout << pattern[i];
      }
      if (i < pattern.getSize()-1) {
         cout << ' ';
      }
   }
}

void printSequence(Array<RationalNumber>& pattern) {
   int i;
   for (i=0; i<pattern.getSize(); i++) {
      if (pattern[i] < 0) {
         cout << "*";
      } else {
         cout << pattern[i];
      }
      if (i < pattern.getSize()-1) {
         cout << ' ';
      }
   }
}



//////////////////////////////
//
// checkForWildcard -- returns true if any of the values are negative.
//

int checkForWildcard(Array<double>& sequence) {
   int i;
   for (i=0; i<sequence.getSize(); i++) {
      if (sequence[i] < 0.0) {
         return 1;
      }
   }
   return 0;
}



//////////////////////////////
//
// printMatchesWithData --
//

void printMatchesWithData(Array<int>& linematch, HumdrumFile& infile) {
   int i;
   int counter = 1;


   Array<int> lookup;
   lookup.setSize(infile.getNumLines());
   lookup.allowGrowth(0);
   lookup.setAll(-1);

   for (i=0; i<linematch.getSize(); i++) {
      lookup[linematch[i]] = counter++;
   }

   for (int i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
         /* case E_humrec_data_comment: break; */

         case E_humrec_data_kern_measure:
            if (prependQ) {
               cout << infile[i][0] << "\t";
               cout << infile[i] << "\n";
            } else if (appendQ) {
               cout << infile[i] << "\t";
               cout << infile[i][0] << "\n";
            } else {
               cout << infile[i][0] << "\n";
            }
            break;

         case E_humrec_interpretation:
            if (appendQ) {
               cout << infile[i] << "\t";
            } 

            if (strncmp(infile[i][0], "**", 2) == 0) {
               cout << "**match";
            } else if (strcmp(infile[i][0], "*-") == 0) {
               cout << "*-";
            } else {
               cout << "*";
            }

            if (prependQ) {
               cout << "\t" << infile[i];
            }
            cout << "\n";

            break;

         case E_humrec_data:
            if (appendQ) {
               cout << infile[i] << "\t";
            } 
            if (lookup[i] < 0) {
               cout << ".";
            } else {
               cout << lookup[i];
            }

            if (prependQ) {
               cout << "\t" << infile[i];
            }

            cout << "\n";

            break;
         case E_humrec_local_comment:
            if (appendQ) {
               cout << infile[i] << "\t";
            }
            cout << "!";
            if (prependQ) {
               cout << "\t" << infile[i];
            }
            cout << "\n";

            break;
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
         default:
            cout << infile[i] << "\n";
            break;
      }
   }
}



//////////////////////////////
//
// printSearchResultsFinal --
//

void printSearchResultsFinal(Array<int>& linematch, HumdrumFile& infile,
      Array<RationalNumber>& Bfeatures, Array<int>& Blines, 
      Array<RationalNumber>& Dfeatures, Array<int>& Dlines) {
   cout << "!!matches: " << linematch.getSize() << "\n";

   if (appendQ || prependQ) {
      printMatchesWithData(linematch, infile);
      return;
   }

   Array<double> measures;
   fillMeasureInfo(infile, measures);


   int hasBWildcard = checkForWildcard(Bsearch);
   int hasDWildcard = checkForWildcard(Dsearch);
	 
   int i;
   cout << "**line\t**bar\t**beat\t**absb";

   if (hasBWildcard) {
      cout << "\t**bseq";
   }
   if (hasDWildcard) {
      cout << "\t**dseq";
   }

   cout << "\n";
   for (i=0; i<linematch.getSize(); i++) {
      cout << linematch[i]-1;
      cout << "\t" << measures[linematch[i]];
      if (zeroQ) {
         cout << "\t" << infile[linematch[i]].getBeat()-1;
      } else {
         cout << "\t" << infile[linematch[i]].getBeat();
      }
      cout << "\t" << infile[linematch[i]].getAbsBeat();
      if (hasBWildcard) {
         cout << "\t";
         printSequence(Bfeatures, Blines, Bsearch, linematch[i]);	       
      }
      if (hasDWildcard) {
         cout << "\t";
         printSequence(Dfeatures, Dlines, Dsearch, linematch[i]);	       
      }
      cout << endl;
   }
   cout << "*-\t*-\t*-\t*-";
   if (hasBWildcard) {
      cout << "\t*-";
   }
   if (hasDWildcard) {
      cout << "\t*-";
   }
   cout << "\n";
}



///////////////////////////////
//
// printSequence --
// 

void printSequence(Array<double>& features, Array<int>& lines, 
     Array<double>& search, int startline) {

   int index = -1;
   int i;
   for (i=0; i<lines.getSize(); i++) {
      if (lines[i] == startline) {
         index = i;
         break;
      }
   }

   if (index < 0) {
      cout << ".";
      return;
   }

   int stopindex = index + search.getSize() - 1;
   for (i=index; (i<features.getSize()) && (i<=stopindex); i++) {
      cout << features[i];
      if (i < stopindex) {
         cout << " ";
      }
   }
}



void printSequence(Array<RationalNumber>& features, Array<int>& lines, 
     Array<double>& search, int startline) {

   int index = -1;
   int i;
   for (i=0; i<lines.getSize(); i++) {
      if (lines[i] == startline) {
         index = i;
         break;
      }
   }

   if (index < 0) {
      cout << ".";
      return;
   }

   int stopindex = index + search.getSize() - 1;
   for (i=index; (i<features.getSize()) && (i<=stopindex); i++) {
      cout << features[i];
      if (i < stopindex) {
         cout << " ";
      }
   }
}



/*
void printSequence(Array<double>& features, Array<int>& lines, 
     Array<double>& search, int startline) {

   int index = -1;
   int i;
   for (i=0; i<lines.getSize(); i++) {
      if (lines[i] == startline) {
         index = i;
         break;
      }
   }

   if (index < 0) {
      cout << ".";
      return;
   }

   int stopindex = index + search.getSize() - 1;
   for (i=index; (i<features.getSize()) && (i<=stopindex); i++) {
      cout << features[i];
      if (i < stopindex) {
         cout << " ";
      }
   }
}

*/

/*
 
void printSequence(Array<double>& features, Array<int>& lines, 
     Array<double>& search, int startline) {

   int index = -1;
   int i;
   for (i=0; i<lines.getSize(); i++) {
      if (lines[i] == startline) {
         index = i;
         break;
      }
   }

   if (index < 0) {
      cout << ".";
      return;
   }

   int stopindex = index + search.getSize() - 1;
   for (i=index; (i<features.getSize()) && (i<=stopindex); i++) {
      cout << features[i];
      if (i < stopindex) {
         cout << " ";
      }
   }
}

*/



void printSequence(Array<RationalNumber>& features, Array<int>& lines, 
     Array<RationalNumber>& search, int startline) {

   int index = -1;
   int i;
   for (i=0; i<lines.getSize(); i++) {
      if (lines[i] == startline) {
         index = i;
         break;
      }
   }

   if (index < 0) {
      cout << ".";
      return;
   }

   int stopindex = index + search.getSize() - 1;
   for (i=index; (i<features.getSize()) && (i<=stopindex); i++) {
      cout << features[i];
      if (i < stopindex) {
         cout << " ";
      }
   }
}



//////////////////////////////
//
// fillMeasureInfo --
//

void fillMeasureInfo(HumdrumFile& infile, Array<double>& measures) {
   int i;
   measures.setSize(infile.getNumLines());
   measures.allowGrowth(0);
   measures.setAll(0.0);

   double current = 0.0;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isMeasure()) {
         sscanf(infile[i][0], "=%lf", &current);
      }
      measures[i] = current;
   }
}


//////////////////////////////
//
// mergeResults -- do an intersection of two lists of sorted integers
//

void mergeResults(Array<int>& output, Array<int>& input1, Array<int>& input2) {
   int i, j;
   int maxsize = input1.getSize();
   if (input2.getSize() < maxsize) {
      maxsize = input2.getSize();
   }
   output.setSize(maxsize);
   output.setSize(0);
   if (maxsize == 0) {
      return;
   }
   int similar;
   j=0;
   for (i=0; i<input1.getSize(); i++) {
      while ((j<input2.getSize()) && (input2[j] < input1[i])) {
         j++;
      }
      if (j >= input2.getSize()) {
         break;
      }
      if (input2[j] == input1[i]) {
         similar = input2[j];
         output.append(similar);
      }
   }
}



//////////////////////////////
//
// doBeatSearch -- search for specific beat pattern in data.
//

void doBeatSearch(Array<int>& results, HumdrumFile& infile, 
      Array<double> search, Array<RationalNumber>& Bfeatures, 
      Array<int>& Blines) {
   // extractBeatFeatures(infile, Blines, Bfeatures);
   Array<double> doubleBfeatures;
   doubleBfeatures.setSize(Bfeatures.getSize());
   int i;
   for (i=0; i<doubleBfeatures.getSize(); i++) {
      doubleBfeatures[i] = Bfeatures[i].getFloat();
   }
   doComparison(results, Blines, search, doubleBfeatures, infile);
}



//////////////////////////////
//
// extractBeatFeatures --
//

void extractBeatFeatures(HumdrumFile& infile, Array<int>& line,
      Array<RationalNumber>& data) {
   line.setSize(infile.getNumLines());
   line.setSize(0);
   data.setSize(infile.getNumLines());
   data.setSize(0);

   int lval;
   RationalNumber bval;
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isData()) {
         continue;
      }
      bval = infile[i].getBeatR();
      lval = i;
      if (Attacks[i] < Attack) {
         // ignore lines which do not have enough note onsets
         continue;
      }
      line.append(lval);
      data.append(bval);
   }
}



//////////////////////////////
//
// extractDurFeatures --
//

void extractDurFeatures(HumdrumFile& infile, Array<int>& line,
      Array<RationalNumber>& data) {
   line.setSize(infile.getNumLines());
   line.setSize(0);
   data.setSize(infile.getNumLines());
   data.setSize(0);

   int lval;
   RationalNumber bval;
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isData()) {
         continue;
      }
      bval = infile[i].getDurationR();
      if (zeroQ) {
         bval = bval - 1;
      }
      lval = i;
      if (Attacks[i] < Attack) {
         // ignore lines which do not have enough note onsets,
	 // adding duration of current line to last data line
	 // which exceeds the onset threshold test.
	 if (data.getSize() > 0) {
	    data[data.getSize()-1] += bval;
	 }
         continue;
      }
      line.append(lval);
      data.append(bval);
   }
}



//////////////////////////////
//
// doComparison --
//

void doComparison(Array<int>& results, Array<int>& line, Array<double>& search,
     Array<double>& data, HumdrumFile& infile) {
   results.setSize(data.getSize() - search.getSize() + 1);
   results.setSize(0);

   double startdur;
   double stopdur;

   int match;
   int i, j;
   for (i=0; i<data.getSize() - search.getSize() + 1; i++) {
      match = 1;
      for (j=0; j<search.getSize(); j++) {
         if (search[j] < 0) {
            // for wildcard match (*)
            continue;
         }
         if (fabs(search[j] - data[i+j]) > Tolerance) {
            match = 0;
            break;
         }
      }
      if (match) {
         if (Rvalue > 0) {
            startdur = infile[line[i]].getAbsBeat();
            stopdur  = infile[line[i+search.getSize()-1]].getAbsBeat();
            if (fabs(Rvalue - (stopdur-startdur)) < Tolerance) {
               results.append(line[i]);
            }
         } else {
            results.append(line[i]);
         }
      }
   }
}



//////////////////////////////
//
// doDurSearch -- search for specific beat pattern in data.
//

void doDurSearch(Array<int>& results, HumdrumFile& infile, 
      Array<double> search, Array<double>& Dfeatures, Array<int>& Dlines) {
   // extractDurFeatures(infile, Dlines, Dfeatures);
   doComparison(results, Dlines, search, Dfeatures, infile);
}

void doDurSearch(Array<int>& results, HumdrumFile& infile, 
      Array<double> search, Array<RationalNumber>& Dfeatures, 
      Array<int>& Dlines) {
   int i;
   Array<double> doubleDfeatures(Dfeatures.getSize());
   for (i=0; i<doubleDfeatures.getSize(); i++) {
      doubleDfeatures[i] = Dfeatures[i].getFloat();
   }

   doComparison(results, Dlines, search, doubleDfeatures, infile);
}



//////////////////////////////
//
// getPickupDuration --
//

RationalNumber getPickupDuration(HumdrumFile& file) {
   int i;
   for (i=0; i<file.getNumLines(); i++) {
      if (!file[i].isMeasure()) {
         continue;
      }
      return file[i].getAbsBeatR();
   }

   RationalNumber nothing(-1,1);
   return nothing;
}



//////////////////////////////
//
// printOutput --
//

void printOutput(HumdrumFile& file, Array<RationalNumber>& Bfeatures, 
      Array<int>& Blines, Array<RationalNumber>& Dfeatures, Array<int>& Dlines,
      Array<int>& tickanalysis) {
   int lastmeasureline = -1;
   int pickupstate = 0;
   int suppressreturn = 0;
   int i;

   Array<unsigned long> abstick;
   if (tickQ) {
      unsigned long csum = 0;
      abstick.setSize(tickanalysis.getSize());
      abstick.setAll(0);
      for (i=0; i<tickanalysis.getSize(); i++) {
         abstick[i] = csum;
         csum += tickanalysis[i];
      }
   }

   RationalNumber minrhy(file.getMinTimeBase(), 4);
   RationalNumber rat;
   Array<RationalNumber> Binfo;
   Array<RationalNumber> Dinfo;
   Binfo.setSize(file.getNumLines());
   Binfo.allowGrowth(0);
   Binfo.setAll(-1);
   Dinfo.setSize(file.getNumLines());
   Dinfo.allowGrowth(0);
   Dinfo.setAll(-1);
   int measurecount = 0;

   for (i=0; i<Blines.getSize(); i++) {
      Binfo[Blines[i]] = Bfeatures[i];
      if (Binfo[Blines[i]] == file[Blines[i]].getAbsBeatR()) { 
         Binfo[Blines[i]]++;
         Binfo[Blines[i]] -= file.getPickupDurationR();
      }
      if (zeroQ) {
         Binfo[Blines[i]]--;
      }
   }
   for (i=0; i<Dlines.getSize(); i++) {
      Dinfo[Dlines[i]] = Dfeatures[i];
   }

   for (i=0; i<file.getNumLines(); i++) {
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
	    measurecount++;
            break;

         case E_humrec_interpretation:
            if (appendQ) {
               cout << file[i] << "\t";
            } 

            if (strncmp(file[i][0], "**", 2) == 0) {
               if (absQ && !tickQ) {
                  cout << "**absb";
	       } else if (absQ && tickQ && !rationalQ) {
                  cout << "**atick";
	       } else if (absQ && tickQ && rationalQ) {
                  cout << "**adur";
               } else if (tickQ && durQ && !rationalQ) {
                  cout << "**dtick";
               } else if (tickQ && durQ && rationalQ) {
                  cout << "**dur";
               } else if (durQ && !tickQ) {
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
               } else if (strncmp(file[i][0], "*MM", 3) == 0) {
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
               // cout << file[i].getDuration();
               if (Dinfo[i] >= 0) {
                  if (tickQ && !rationalQ) {
                     cout << tickanalysis[i];
                  } else if (tickQ && rationalQ) {
                     rat.setValue(tickanalysis[i], file.getMinTimeBase());
		     if (uQ) {
                        rat *= 4;
                     }
                     cout << rat << " = " 
			     << tickanalysis[i] << " x "
			     << file.getMinTimeBase();
                  } else {
                     cout << Dinfo[i].getFloat();
                  }
               } else {
                  if (nullQ || appendQ || prependQ) {
                     cout << ".";
                  } else {
                     suppressreturn = 1;
                  }
               }
            } else if (absQ) {
               if (tickQ && !rationalQ) {
                  cout << abstick[i];
	       } else if (tickQ && rationalQ) {
                  RationalNumber anumber(abstick[i], file.getMinTimeBase());
		  if (uQ) {
                     anumber *= 4;
                  }
                  anumber.printTwoPart(cout);
               } else {
                  cout << file[i].getAbsBeat();
               }
            } else if (sumQ) {
               if (lastmeasureline > 0) {
                  cout << fabs(file[lastmeasureline].getBeat());
                  pickupstate++;
                  lastmeasureline = -1;
               } else if (pickupstate < 1) {
                  if (!file.getPickupDurationR().isNegative()) {
                     if (measurecount == 0) {
                        cout << getDurationOfFirstMeasure(file).getFloat();
                     } else {
                        cout << file.getPickupDuration();
                     }
                  } else if (file.getPickupDurationR().isZero()) {
                     cout << file.getTotalDurationR().getFloat();
                  } else {
                     cout << file.getTotalDurationR().getFloat();
                  }
                  pickupstate++;
                  lastmeasureline = -1;
               } else {
                  if (appendQ || prependQ) {
                     cout << ".";
                  } else {
                     if (nullQ) {
                        cout << ".";
                     } else {
                        suppressreturn = 1;
                     }
                  }
               }
            } else if (beatQ) {
               if (Binfo[i] >= 0) {
                  if (!tickQ && !rationalQ) {
                     cout << Binfo[i].getFloat();
		  } else if (tickQ && !rationalQ) {
                     cout << (Binfo[i] * minrhy);
                  } else {
                     Binfo[i].printTwoPart(cout); 
                  }
               } else {
                  if (nullQ || appendQ || prependQ) {
                     cout << ".";
                  } else {
                     suppressreturn = 1;
                  }
               }
            }
            
            if (prependQ) {
               cout << "\t" << file[i];
            }

            if (suppressreturn) {
               suppressreturn = 0;
            } else {
               cout << "\n";
            }

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
// getDurationOfFirstMeasure --
//


RationalNumber getDurationOfFirstMeasure(HumdrumFile& file) {
   int i;
   RationalNumber output(0,1);
   for (i=0; i<file.getNumLines(); i++) {
      if (file[i].getType() == E_humrec_data_measure) {
         break;
      }
      if (file[i].getType() != E_humrec_data) {
         continue;
      }
      output += file[i].getDurationR();
   }
   return output;
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
   opts.define("c|abs|total-beat=b");   // cumulative, absolute beat location
   opts.define("d|dur|duration=b");     // display rhymic duration of records
   opts.define("s|sum=b");              // sum the duration of each measure
   opts.define("z|zero|zero-offset=b"); // first beat is represented by a 0
   opts.define("n|nulls|keep-nulls=b"); // doesn't remove nulls with -s option
   opts.define("B=s");                  // Do a composite beat search
   opts.define("D=s");                  // Do a composite duration search
   opts.define("R=d:-1.0");             // Limit total duration range of search
   opts.define("u|beatsize=s:4");       // beat unit
   opts.define("A|attacks|attack=i:1"); // Minimum num of note onsets for event
   opts.define("t|tick=b", "display durations as tick values");  
   opts.define("f|rational=b", "display durations as rational values");  
   opts.define("tpw=b", "display only ticks per whole note");  
   opts.define("tpq=b", "display only ticks per quarter note");  

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
      cout << argv[0] << ", version: 21 April 2010" << endl;
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
      zeroQ = 1;
   }
   appendQ   = opts.getBoolean("append");
   prependQ  = opts.getBoolean("prepend");
   durQ      = opts.getBoolean("duration");
   absQ      = opts.getBoolean("total-beat");
   beatQ     = opts.getBoolean("beat");
   sumQ      = opts.getBoolean("sum");
   nullQ     = opts.getBoolean("keep-nulls");
   tickQ     = opts.getBoolean("tick");
   rationalQ = opts.getBoolean("rational");
   tpwQ      = opts.getBoolean("tpw");
   tpqQ      = opts.getBoolean("tpq");
   Rvalue    = opts.getDouble("R");
   beatbase  = opts.getString("beatsize");
   uQ        = opts.getBoolean("beatsize");

   if (rationalQ) {
      tickQ = 1;
   }
 
   Attack = opts.getInteger("attack");
   if (Attack < 0) {
      Attack = 0;
   }

   Bsearch.setSize(0);
   Dsearch.setSize(0);

   if (opts.getBoolean("B")) {
      fillSearchString(Bsearch, opts.getString("B"));
   } 
   if (opts.getBoolean("D")) {
      fillSearchString(Dsearch, opts.getString("D"));
   }

   if (prependQ && appendQ) {
      prependQ = 1;
      appendQ = 0;
   }

   if (prependQ || appendQ) {
      nullQ = 1;
   }

   // display beat information if no other output option is given.
   if (!(absQ || durQ)) {
      beatQ = 1;
   }
}



//////////////////////////////
//
// fillSearchString --
//

void fillSearchString(Array<double>& searcher, const char* string) {
   int len = strlen(string);
   char* tempstr;
   tempstr = new char[len+1];
   strcpy(tempstr, string);
   char* ptr;
   ptr = strtok(tempstr, " \t\n:;,");
   double value;

   searcher.setSize(1000);
   searcher.setGrowth(1000);
   searcher.setSize(0);
   
   while(ptr != NULL) {
      if (strcmp(ptr, "*") == 0) {
         value = -1;
      } else {
         value = atof(ptr);
      }
      searcher.append(value);
      ptr = strtok(NULL, " \t\n:;,");
   }

   delete [] tempstr;
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



// md5sum: 73039771851e574d20683314636364f6 beat.cpp [20100602]

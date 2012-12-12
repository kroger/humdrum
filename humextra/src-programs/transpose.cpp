//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Apr 12 23:22:19 PDT 2004
// Last Modified: Mon Apr 12 23:22:22 PDT 2004
// Last Modified: Thu Feb 24 22:43:17 PST 2005 (added -k option)
// Last Modified: Wed Jun 24 15:39:58 PDT 2009 (updated for GCC 4.4)
// Last Modified: Sun Sep 13 12:34:51 PDT 2009 (added -s option)
// Last Modified: Wed Nov 18 14:01:20 PST 2009 (added *Tr markers)
// Last Modified: Thu Nov 19 14:08:32 PST 2009 (added -q, -d and -c options)
// Last Modified: Thu Nov 19 15:12:01 PST 2009 (added -I options and *ITr marks)
// Last Modified: Thu Nov 19 19:28:26 PST 2009 (added -W and -C options)
// Filename:      ...sig/examples/all/transpose.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/transpose.cpp
// Syntax:        C++; museinfo
//
// Description:   Transpose **kern musical data.
//

#include "humdrum.h"
#include "PerlRegularExpression.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

#ifndef OLDCPP
   #include <iostream>
   #include <fstream>
#else
   #include <iostream.h>
   #include <fstream.h>
#endif

#define STYLE_CONCERT 0
#define STYLE_WRITTEN 1

// function declarations:
void      checkOptions          (Options& opts, int argc, char** argv);
void      example               (void);
void      usage                 (const char* command);
void      printFile             (HumdrumFile& infile);
void      processFile           (HumdrumFile& infile, Array<int>& spines);
void      printNewKernString    (const char* string, int transval);
void      printHumdrumKernToken (HumdrumRecord& record, int index, 
                                 int transval);
void      printHumdrumDataRecord(HumdrumRecord& record, 
                                 Array<int>& spineprocess);
int       getBase40ValueFromInterval(const char* string);
void      printNewKeyInterpretation(HumdrumRecord& aRecord, int index,
                                  int transval);
void      printNewKeySignature   (const char* keysig, int trans);
void      convertScore           (HumdrumFile& infile, int style);
void      printTransposedToken   (HumdrumFile& infile, int row, int col, 
                                  int transval);
void      processInterpretationLine(HumdrumFile& infile, int line, 
                                  Array<int>& tvals, int style);
int       isKeyMarker            (const char* string);
int       getTransposeInfo       (HumdrumFile& infile, int row, int col);
int       hasTrMarkers           (HumdrumFile& infile, int line);
void      convertToWrittenPitches(HumdrumFile& infile, int line, 
                                  Array<int>& tvals);
void      convertToConcertPitches(HumdrumFile& infile, int line, 
                                  Array<int>& tvals);

// auto transpose functions:
double    pearsonCorrelation     (int size, double* x, double* y);
void      doAutoTransposeAnalysis(HumdrumFile& infile);
void      addToHistogramDouble   (Array<Array<double> >& histogram, int pc, 
                                  double start, double dur, double tdur, 
                                  int segments);
double    storeHistogramForTrack (Array<Array<double> >& histogram, 
                                  HumdrumFile& infile, int track, int segments);
void      printHistograms        (int segments, Array<int> ktracks, 
                                  Array<Array<Array<double> > >& trackhist);
void      doAutoKeyAnalysis      (Array<Array<Array<double> > >& analysis, 
                                  int level, int hop, int count, int segments, 
                                  Array<int>& ktracks, 
                                  Array<Array<Array<double> > >& trackhist);
void      doTrackKeyAnalysis     (Array<Array<double> >& analysis, int level, 
                                  int hop, int count, 
                                  Array<Array<double> >& trackhist,
                                  Array<double>& majorweights,
                                  Array<double>& minorweights);
void      identifyKeyDouble      (Array<double>& correls, 
                                  Array<double>& histogram, 
                                  Array<double>& majorweights, 
                                  Array<double>& minorweights);
void      fillWeightsWithKostkaPayne(Array<double>& maj, Array<double>& min);
void      printRawTrackAnalysis  (Array<Array<Array<double> > >& analysis,
                                  Array<int>& ktracks);
void      doSingleAnalysis       (Array<double>& analysis, int startindex, 
                                  int length, Array<Array<double> >& trackhist, 
                                  Array<double>& majorweights, 
                                  Array<double>& minorweights);
void      identifyKey            (Array<double>& correls, 
                                  Array<double>& histogram,
                                  Array<double>& majorweights, 
                                  Array<double>& minorweights);
void      doTranspositionAnalysis(Array<Array<Array<double> > >& analysis);
int       calculateTranspositionFromKey(int targetkey, HumdrumFile& infile);

// spine list parsing functions:
void      processFieldEntry      (Array<int>& field, const char* string, 
                                  HumdrumFile& infile);
void      fillFieldData          (Array<int>& field, const char* fieldstring, 
                                  HumdrumFile& infile);
void      removeDollarsFromString(Array<char>& buffer, int maxtrack);


// User interface variables:
Options     options;
int         transval    = 0;     // used with -b option
int         ssetkeyQ     = 0;    // used with -k option
int         ssetkey      = 0;    // used with -k option
int         currentkey   = 0;
int         autoQ        = 0;    // used with --auto option
int         debugQ       = 0;    // used with --debug option
int         spineQ       = 0;    // used with -s option
const char* spinestring  = "";   // used with -s option
int         octave       = 0;    // used with -o option
int         concertQ     = 0;    // used with -C option
int         writtenQ     = 0;    // used with -W option
int         quietQ       = 0;    // used with -q option
int         instrumentQ  = 0;    // used with -I option


//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   // process the command-line options
   checkOptions(options, argc, argv);

   HumdrumFile infile;
   if (options.getArgCount() > 0) {
      infile.read(options.getArg(1));
   } else {
      infile.read(cin);
   }

   Array<int> spineprocess(infile.getMaxTracks());
   spineprocess.setGrowth(0);
   spineprocess.setAll(1);
   if (spineQ) {
      fillFieldData(spineprocess, spinestring, infile);
      if (debugQ) {
         int i;
         cout << "!! SPINE INFORMATION: ";
         for (i=0; i<spineprocess.getSize(); i++) {
            cout << spineprocess[i] << " ";
         }
         cout << endl;
      }
   }

   if (ssetkeyQ) {
      transval = calculateTranspositionFromKey(ssetkey, infile);
      transval = transval + octave * 40;
      if (debugQ) {
         cout << "!!Key TRANSVAL = " << transval;
      }
   }

   if (options.getBoolean("rotation")) {
      // returns the base-12 pitch transposition for use in conjunction
      // with the mkeyscape --rotate option
      int value = 60 - Convert::base40ToMidiNoteNumber(162 - transval);
      cout << value << endl;
      exit(0);
   }
	    
   if (concertQ) {
      convertScore(infile, STYLE_CONCERT);
   } else if (writtenQ) {
      convertScore(infile, STYLE_WRITTEN);
   } else if (autoQ) {
      doAutoTransposeAnalysis(infile);
   } else {
      processFile(infile, spineprocess);
   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// convertScore -- create a concert pitch score from
//     a written pitch score.  The function will search for *Tr
//     interpretations in spines, and convert them to *ITr interpretations
//     as well as transposing notes, and transposing key signatures and
//     key interpretations.  Or create a written score from a 
//     concert pitch score based on the style parameter.
//

void convertScore(HumdrumFile& infile, int style) {
   Array<int> tvals;  // transposition values for each spine
   tvals.setSize(infile.getMaxTracks() + 1);
   tvals.setAll(0);

   int ptrack;
   int i, j;
   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
         case E_humrec_interpretation:
            // scan the line for transposition codes
            // as well as key signatures and key markers
            processInterpretationLine(infile, i, tvals, style);
            break;

         case E_humrec_data:
	    // transpose notes according to tvals data
	    for (j=0; j<infile[i].getFieldCount(); j++) {
               ptrack = infile[i].getPrimaryTrack(j);
               if (tvals[ptrack] == 0) {
                  cout << infile[i][j];
               } else {
                  printTransposedToken(infile, i, j, tvals[ptrack]);
               }
               if (j < infile[i].getFieldCount() - 1) {
                  cout << "\t";
               }
            }
            cout << "\n";
            break;

         case E_humrec_data_comment:
         case E_humrec_data_kern_measure:
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
// processInterpretationLine --  Used in converting between 
//   concert pitch and written pitch scores.
//

void processInterpretationLine(HumdrumFile& infile, int line, 
     Array<int>& tvals, int style) {

   PerlRegularExpression pre;

   int j;
   int ptrack;

   if (hasTrMarkers(infile, line)) {
      switch (style) {
         case STYLE_CONCERT:
            convertToConcertPitches(infile, line, tvals);
            break;
         case STYLE_WRITTEN:
            convertToWrittenPitches(infile, line, tvals);
            break;
         default: cout << infile[line];
      }
      cout << "\n";
      return;
   }

   for (j=0; j<infile[line].getFieldCount(); j++) {
      ptrack = infile[line].getPrimaryTrack(j);

      // check for *ITr or *Tr markers
      // ignore *ITr markers when creating a Concert-pitch score
      // ignore *Tr  markers when creating a Written-pitch score

      if (pre.search(infile[line][j], "^\\*k\\[([a-gA-G\\#-]*)\\]", "")) {
         // transpose *k[] markers if necessary
         if (tvals[ptrack] != 0) {
            printNewKeySignature(pre.getSubmatch(1), tvals[ptrack]);
         } else {
            cout << infile[line][j];
         }

      } else if (isKeyMarker(infile[line][j])) {
         // transpose *C: markers and like if necessary
         if (tvals[ptrack] != 0) {
            printNewKeyInterpretation(infile[line], j, tvals[ptrack]);
         } else {
            cout << infile[line][j];
         }

      } else {
         // other interpretations just echoed to output:
         cout << infile[line][j];
      }
      if (j<infile[line].getFieldCount()-1) {
         cout << "\t";
      }
   }
   cout << "\n";

}



//////////////////////////////
//
// convertToWrittenPitches --
//

void convertToWrittenPitches(HumdrumFile& infile, int line, Array<int>& tvals) {
   PerlRegularExpression pre;
   int j;
   int base;
   int ptrack;
   char buffer1[128] = {0};
   char buffer2[128] = {0};
   for (j=0; j<infile[line].getFieldCount(); j++) {
      if (pre.search(infile[line][j], "^\\*ITrd[+-]?\\d+c[+-]?\\d+$", "")) {
         base = Convert::transToBase40(infile[line][j]);
	 strcpy(buffer1, "*Tr");
	 strcat(buffer1, Convert::base40ToTrans(buffer2, base));
         cout << buffer1;
         ptrack = infile[line].getPrimaryTrack(j);
         tvals[ptrack] = base;
      } else {
         cout << infile[line][j];
      }
      if (j < infile[line].getFieldCount() - 1) {
         cout << "\t";
      }
   }
}



//////////////////////////////
//
// convertToConcertPitches --
//

void convertToConcertPitches(HumdrumFile& infile, int line, Array<int>& tvals) {
   PerlRegularExpression pre;
   int j;
   int base;
   int ptrack;
   char buffer1[128] = {0};
   char buffer2[128] = {0};
   for (j=0; j<infile[line].getFieldCount(); j++) {
      if (pre.search(infile[line][j], "^\\*Trd[+-]?\\d+c[+-]?\\d+$", "")) {
         base = Convert::transToBase40(infile[line][j]);
	 strcpy(buffer1, "*ITr");
	 strcat(buffer1, Convert::base40ToTrans(buffer2, base));
         cout << buffer1;
         ptrack = infile[line].getPrimaryTrack(j);
         tvals[ptrack] = -base;
      } else {
         cout << infile[line][j];
      }
      if (j < infile[line].getFieldCount() - 1) {
         cout << "\t";
      }
   }
}



//////////////////////////////
//
// hasTrMarkers -- returns true if there are any tokens
//    which start with *ITr or *Tr and contains c and d
//    with numbers after each of them.
//

int hasTrMarkers(HumdrumFile& infile, int line) {
   PerlRegularExpression pre;
   int j;
   for (j=0; j<infile[line].getFieldCount(); j++) {
      if (pre.search(infile[line][j], "^\\*I?Trd[+-]?\\d+c[+-]?\\d+$", "")) {
         return 1;
      } 
   }

   return 0;
}



//////////////////////////////
//
// isKeyMarker -- returns true if the interpretation is
//    a key description, such as *C: for C major, or *a:.
//

int isKeyMarker(const char* string) {
   PerlRegularExpression pre;
   return pre.search(string, "^\\*[a-g]?[\\#-]?:", "i");
}



//////////////////////////////
//
// printTransposedToken -- print a Humdrum token with the given
//    base-40 transposition value applied.  Only **kern data is
//    know now to transpose, other data types are currently not
//    allowed to be transposed (but could be added here later).
//

void printTransposedToken(HumdrumFile& infile, int row, int col, int transval) {
   if (strcmp("**kern", infile[row].getExInterp(col)) != 0) {
      // don't know how to transpose this type of data, so leave it as is
      cout << infile[row][col]; 
      return;
   }

   printHumdrumKernToken(infile[row], col, transval);
}



//////////////////////////////
//
// calculateTranspositionFromKey --
//

int calculateTranspositionFromKey(int targetkey, HumdrumFile& infile) {

   int i, j;
   PerlRegularExpression pre;
   int base40 = 0;
   int currentkey = 0;
   int mode = 0;
   int found = 0;

   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isData()) {
         // no initial key label was found, so don't transpose.
         // in the future, maybe allow an automatic key analysis
         // to be performed on the data if there is not explicit
         // key designation.
         return 0;   
      }
      if (!infile[i].isInterpretation()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (!pre.search(infile[i][j], "^\\*([A-G][#-]?):", "i")) {
            continue;
         }

         mode = 0;  // major key
         if (islower(infile[i][j][1])) {
            mode = 1;  // minor key
         }
         base40 = Convert::kernToBase40(infile[i][j]);
         // base40 = base40 + transval;
         base40 = base40 + 4000;
         base40 = base40 % 40;
         base40 = base40 + (3 + mode) * 40;
         currentkey = base40;
	 found = 1;

         break;
      }
      if (found) {
         break;
      }
   }

   int trans = (targetkey%40 - currentkey%40);
   // base40 = targetkey + (3 + mode) * 40;
   if (trans > 40) {
      trans -= 40;
   } 
   if (trans > 20) {
      trans = 40 - trans;
      trans = -trans;
   }
   if (trans < -40) {
      trans += 40;
   }
   if (trans < -20) {
      trans = -40 - trans;
      trans = -trans;
   }

   return trans;
}



//////////////////////////////
//
// printTransposeInformation -- collect and print *Tr interpretations
//      at the start of the spine.  Looks for *Tr markers at the start
//      of the file before any data.
//

void printTransposeInformation(HumdrumFile& infile, Array<int>& spineprocess,
      int line, int transval) {
   int j;
   int ptrack;

   Array<int> startvalues;
   startvalues.setSize(infile.getMaxTracks()+1);
   startvalues.setAll(0);

   Array<int> finalvalues;
   finalvalues.setSize(infile.getMaxTracks()+1);
   finalvalues.setAll(0);

   for (j=0; j<infile[line].getFieldCount(); j++) {
      ptrack = infile[line].getPrimaryTrack(j);
      startvalues[ptrack] = getTransposeInfo(infile, line, j);
      // cout << "Found transpose value " << startvalues[ptrack] << endl;
   }

   int entry = 0;
   // check if any spine will be transposed after final processing
   for (j=0; j<infile[line].getFieldCount(); j++) {
      ptrack = infile[line].getPrimaryTrack(j);
      if (spineprocess[ptrack-1]) {
	 finalvalues[ptrack] = transval;
	 if (!instrumentQ) {
            finalvalues[ptrack] += startvalues[ptrack];
	 }
	 // cout << "New value is: " << finalvalues[ptrack] << endl;
         if (finalvalues[ptrack] != 0) {
            entry = 1;
         }
      } else {
         finalvalues[ptrack] = startvalues[ptrack];
         if (finalvalues[ptrack] != 0) {
            entry = 1;
         }
      }
   }

   if (!entry) {
      return;
   }

   char buffer[1024] = {0};
   for (j=0; j<infile[line].getFieldCount(); j++) {
      ptrack = infile[line].getPrimaryTrack(j);
      if (finalvalues[ptrack] == 0) {
         cout << "*";
      } else {
         if (instrumentQ) {
            cout << "*ITr";
            cout << Convert::base40ToTrans(buffer, -finalvalues[ptrack]);
         } else {
            cout << "*Tr";
            cout << Convert::base40ToTrans(buffer, finalvalues[ptrack]);
         }
      }
      if (j < infile[line].getFieldCount()-1) {
         cout << "\t";
      }

   }
   cout << "\n";
}



//////////////////////////////
//
// getTransposeInfo -- returns the Transpose information found in 
//    the specified spine starting at the current line, and searching
//    until data is found (or a *- record is found). Return value is a
//    base-40 number.
//

int getTransposeInfo(HumdrumFile& infile, int row, int col) {
   int i, j;
   int track = infile[row].getPrimaryTrack(col);
   int ptrack;

   PerlRegularExpression pre;

   int base;
   int output = 0;
 
   for (i=row; i<infile.getNumLines(); i++) {
      if (infile[i].isData()) {
         break;
      }
      if (!infile[i].isInterpretation()) { 
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         ptrack = infile[i].getPrimaryTrack(j);
         if (ptrack != track) {
            continue;
         }
         if (pre.search(infile[i][j], "^\\*Trd[+-]?\\d+c[+-]?\\d+$", "")) {
            base = Convert::transToBase40(infile[i][j]);
	    output += base;
	    // erase the *Tr value because it will be printed elsewhere
	    infile[i].changeField(j, "*deletedTr");
         }

      }

   }

   return output;
}



//////////////////////////////
//
// checkForDeletedLine -- check to see if a "*deletedTr
//

int checkForDeletedLine(HumdrumFile& infile, int line) {
   int j;
   if (!infile[line].isInterpretation()) {
      return 0;
   }

   int present = 0;
   int composite = 0;
   for (j=0; j<infile[line].getFieldCount(); j++) {
      if (strstr(infile[line][j], "deletedTr") != NULL) {
         present = 1;
      } else if (strcmp(infile[line][j], "*") == 0) {
         // do nothing: not composite
      } else {
         // not a *deletedTr token or a * token, so have to print line later
         composite = 1;
      }
   }

   if (present == 0) {
      // no *deletedTr records found on the currnet line, so process normally
      return 0;
   }

   if (composite == 0) {
      // *deletedTr found, but no other important data found on line.
      return 1;
   }

   // print non-deleted elements in line.
   for (j=0; j<infile[line].getFieldCount(); j++) {
      if (strcmp(infile[line][j], "*deletedTr") == 0) {
         cout << "*";
      } else {
         cout << infile[line][j];
      }
      if (j < infile[line].getFieldCount() - 1) {
         cout << "\t";
      }
   }
   cout << "\n";

   return 1;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile, Array<int>& spineprocess) {
   int i;
   int length;
   int diatonic;
   int j;
   PerlRegularExpression pre;
   const char* ptr;
   int interpstart = 0;

   for (i=0; i<infile.getNumLines(); i++) {
      if (!quietQ && (interpstart == 1)) {
	 interpstart = 2;
	 printTransposeInformation(infile, spineprocess, i, transval);
      }
      if (checkForDeletedLine(infile, i)) {
         continue;
      }

      switch (infile[i].getType()) {

         case E_humrec_data:
            printHumdrumDataRecord(infile[i], spineprocess);
            cout << "\n";
            break;

         case E_humrec_interpretation:

            for (j=0; j<infile[i].getFieldCount(); j++) {
	       if (strncmp(infile[i][j], "**", 2) == 0) {
                  interpstart = 1;
               }

               // check for key signature in a spine which is being
               // transposed, and adjust it.
               if (spineprocess[infile[i].getPrimaryTrack(j)-1] && 
                  pre.search(infile[i][j], "^\\*k\\[([a-gA-G\\#-]*)\\]", "")) {
                  printNewKeySignature(pre.getSubmatch(1), transval);
                  if (j<infile[i].getFieldCount()-1) {
                     cout << "\t";
                  }
                  continue;
               }

               // check for key tandem interpretation and tranpose
	       // if the spine data is being transposed.
               length = strlen(infile[i][j]);
               ptr = strrchr(infile[i][j], ':');
               if ((length < 3) || (ptr == NULL) || (ptr - infile[i][j] > 4)
                     || !spineprocess[infile[i].getPrimaryTrack(j)-1]) {
                  cout << infile[i][j];
                  if (j<infile[i].getFieldCount()-1) {
                     cout << "\t";
                  }
                  continue;
               }

               diatonic = tolower(infile[i][j][1]) - 'a';
               if (diatonic >= 0 && diatonic <= 6) {
                  printNewKeyInterpretation(infile[i], j, transval);
                  if (j<infile[i].getFieldCount()-1) {
                     cout << "\t";
                  }
                  continue;
               }

               cout << infile[i][j];
               if (j<infile[i].getFieldCount()-1) {
                  cout << "\t";
               }
            }
            cout << "\n";
            break;

         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
         case E_humrec_data_comment:
         case E_humrec_data_kern_measure:
         default:
            cout << infile[i] << "\n";
      }
   }
}



//////////////////////////////
//
// printNewKeySignature -- 
//

void printNewKeySignature(const char* keysig, int trans) {
   int counter = 0;
   int len = strlen(keysig);
   int i;
   for (i=0; i<len; i++) {
      switch(keysig[i]) {
         case '-':   counter--; break;
         case '#':   counter++; break;
      }
   }

   int xxx = Convert::base40IntervalToLineOfFifths(trans);
   int newkey = xxx + counter;
   cout << Convert::keyNumberToKern(newkey);
}



//////////////////////////////
//
// printNewKeyInterpretation -- 
//

void printNewKeyInterpretation(HumdrumRecord& aRecord, int index, 
      int transval) {

   int mode = 0;
   if (islower(aRecord[index][1])) {
      mode = 1;
   }
   int base40 = Convert::kernToBase40(aRecord[index]);
   currentkey = base40;
   base40 = base40 + transval;
   base40 = base40 + 4000;
   base40 = base40 % 40;
   base40 = base40 + (3 + mode) * 40;
   char buffer[128] = {0};

   cout << "*" << Convert::base40ToKern(buffer, base40) << ":";

   PerlRegularExpression pre;
   if (pre.search(aRecord[index], ":(.+)$", "")) {
      cout << pre.getSubmatch(1);
   }
}



//////////////////////////////
//
// printHumdrumDataRecord --
//

void printHumdrumDataRecord(HumdrumRecord& record, Array<int>& spineprocess) {
   int i;
   for (i=0; i<record.getFieldCount(); i++) {
      if (!spineprocess[record.getPrimaryTrack(i)-1]) {
         // don't try to transpose spines which were not indicated.
         cout << record[i];
         if (i<record.getFieldCount()-1) {
            cout << "\t";
         }
         continue;
      }
      if (record.getExInterpNum(i) != E_KERN_EXINT) {
         // don't try to transpose non-kern spines
         cout << record[i];
         if (i<record.getFieldCount()-1) {
            cout << "\t";
         }
         continue;
      }

      printHumdrumKernToken(record, i, transval);
      if (i<record.getFieldCount()-1) {
         cout << "\t";
      }
      continue;
   }
   
}



//////////////////////////////
//
// printHumdrumKernToken --
//

void printHumdrumKernToken(HumdrumRecord& record, int index, int transval) {
   if (strcmp(record[index], ".") == 0) {
      // null record element (no pitch).
      cout << ".";
      return;
   }

   int k;
   static char buffer[1024] = {0};
   int tokencount = record.getTokenCount(index);

   for (k=0; k<tokencount; k++) {
      record.getToken(buffer, index, k);
      printNewKernString(buffer, transval);
      if (k<tokencount-1) {
         cout << " ";
      }         
   }
}



//////////////////////////////
//
// printNewKernString --
//

void printNewKernString(const char* string, int transval) {
   if (strchr(string, 'r') != NULL) {
      // don't transpose rests...
      cout << string;
      return;
   }

   char buffer[1024] = {0};
   char buffer2[1024] = {0};
   strcpy(buffer, string);
   int base40 = Convert::kernToBase40(string);
   char* ptr1 = strtok(buffer, "ABCDEFGabcdefg#-n");
   char* ptr2 = strtok(NULL, "ABCDEFGabcdefg#-n");
   char* ptr3 = strtok(NULL, "ABCDEFGabcdefg#-n");
   
   if (ptr3 != NULL) {
      cout << "Error in **kern pitch token: " << string << endl;
      exit(1);
   }

   if (ptr1 != NULL) {
      cout << ptr1;
   }
   cout << Convert::base40ToKern(buffer2, base40 + transval);
   if (ptr2 != NULL) {
      cout << ptr2;
   }

}



//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("b|base40=i:0",    "the base-40 transposition value");
   opts.define("d|diatonic=i:0",  "the diatonic transposition value");
   opts.define("c|chromatic=i:0", "the chromatic transposition value");
   opts.define("o|octave=i:0",     "the octave addition to tranpose value");
   opts.define("t|transpose=s",    "musical interval transposition value");
   opts.define("k|setkey=s",       "transpose to the given key");
   opts.define("auto=b",         "auto. trans. inst. parts to concert pitch");
   opts.define("debug=b",        "print debugging statements");
   opts.define("s|spines=s",     "transpose only specified spines");
   opts.define("q|quiet=b",      "suppress *Tr interpretations in output");
   opts.define("I|instrument=b", "insert instrument code (*ITr) as well");
   opts.define("C|concert=b",    "transpose written score to concert pitch");
   opts.define("W|written=b",    "trans. concert pitch score to written score");
   opts.define("rotation=b",     "display transposition in half-steps");

   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, 12 Apr 2004" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 17 Nov 2009" << endl;
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

   transval     = opts.getInteger("base40");
   ssetkeyQ     = opts.getBoolean("setkey");
   ssetkey      = Convert::kernToBase40(opts.getString("setkey"));
   autoQ        = opts.getBoolean("auto");
   debugQ       = opts.getBoolean("debug");
   spineQ       = opts.getBoolean("spines");
   spinestring  = opts.getString("spines");
   octave       = opts.getInteger("octave");
   concertQ     = opts.getBoolean("concert");
   writtenQ     = opts.getBoolean("written");
   quietQ       = opts.getBoolean("quiet");
   instrumentQ  = opts.getBoolean("instrument");

   switch (opts.getBoolean("diatonic") + opts.getBoolean("chromatic")) {
      case 1:
         cerr << "Error: both -d and -c options must be specified" << endl;
         exit(1);
         break;
      case 2:
         {
            char buffer[128] = {0};
            sprintf(buffer, "d%dc%d", opts.getInt("d"), opts.getInt("c"));
            transval = Convert::transToBase40(buffer);
         }
         break;
   }
	     
   ssetkey = ssetkey % 40;

   if (opts.getBoolean("transpose")) {
      transval = getBase40ValueFromInterval(opts.getString("transpose"));
   }

   transval += 40 * octave;

}



//////////////////////////////
//
// getBase40ValueFromInterval -- note: only ninth interval range allowed
//

int getBase40ValueFromInterval(const char* string) {
   int sign = 1;
   if (strchr(string, '-') != NULL) {
      sign = -1;
   }

   int length = strlen(string);
   char* buffer = new char[length+1];
   strcpy(buffer, string);
   int i;
   for (i=0; i<length; i++) {
      if (buffer[i] == 'p') { buffer[i] = 'P'; }
      if (buffer[i] == 'a') { buffer[i] = 'A'; }
      if (buffer[i] == 'D') { buffer[i] = 'd'; }
   }

   int output = 0;

   if (strstr(buffer, "dd1") != NULL)      { output = -2; }
   else if (strstr(buffer, "d1") != NULL)  { output = -1; }
   else if (strstr(buffer, "P1") != NULL)  { output =  0; }
   else if (strstr(buffer, "AA1") != NULL) { output =  2; }
   else if (strstr(buffer, "A1") != NULL)  { output =  1; }

   else if (strstr(buffer, "dd2") != NULL) { output =  3; }
   else if (strstr(buffer, "d2") != NULL)  { output =  4; }
   else if (strstr(buffer, "m2") != NULL)  { output =  5; }
   else if (strstr(buffer, "M2") != NULL)  { output =  6; }
   else if (strstr(buffer, "AA2") != NULL) { output =  8; }
   else if (strstr(buffer, "A2") != NULL)  { output =  7; }

   else if (strstr(buffer, "dd3") != NULL) { output =  9; }
   else if (strstr(buffer, "d3") != NULL)  { output = 10; }
   else if (strstr(buffer, "m3") != NULL)  { output = 11; }
   else if (strstr(buffer, "M3") != NULL)  { output = 12; }
   else if (strstr(buffer, "AA3") != NULL) { output = 14; }
   else if (strstr(buffer, "A3") != NULL)  { output = 13; }

   else if (strstr(buffer, "dd4") != NULL) { output = 15; }
   else if (strstr(buffer, "d4") != NULL)  { output = 16; }
   else if (strstr(buffer, "P4") != NULL)  { output = 17; }
   else if (strstr(buffer, "AA4") != NULL) { output = 19; }
   else if (strstr(buffer, "A4") != NULL)  { output = 18; }

   else if (strstr(buffer, "dd5") != NULL) { output = 21; }
   else if (strstr(buffer, "d5") != NULL)  { output = 22; }
   else if (strstr(buffer, "P5") != NULL)  { output = 23; }
   else if (strstr(buffer, "AA5") != NULL) { output = 25; }
   else if (strstr(buffer, "A5") != NULL)  { output = 24; }

   else if (strstr(buffer, "dd6") != NULL) { output = 26; }
   else if (strstr(buffer, "d6") != NULL)  { output = 27; }
   else if (strstr(buffer, "m6") != NULL)  { output = 28; }
   else if (strstr(buffer, "M6") != NULL)  { output = 29; }
   else if (strstr(buffer, "AA6") != NULL) { output = 31; }
   else if (strstr(buffer, "A6") != NULL)  { output = 30; }

   else if (strstr(buffer, "dd7") != NULL) { output = 32; }
   else if (strstr(buffer, "d7") != NULL)  { output = 33; }
   else if (strstr(buffer, "m7") != NULL)  { output = 34; }
   else if (strstr(buffer, "M7") != NULL)  { output = 35; }
   else if (strstr(buffer, "AA7") != NULL) { output = 37; }
   else if (strstr(buffer, "A7") != NULL)  { output = 36; }

   else if (strstr(buffer, "dd8") != NULL) { output = 38; }
   else if (strstr(buffer, "d8") != NULL)  { output = 39; }
   else if (strstr(buffer, "P8") != NULL)  { output = 40; }
   else if (strstr(buffer, "AA8") != NULL) { output = 42; }
   else if (strstr(buffer, "A8") != NULL)  { output = 41; }

   else if (strstr(buffer, "dd9") != NULL) { output = 43; }
   else if (strstr(buffer, "d9") != NULL)  { output = 44; }
   else if (strstr(buffer, "m9") != NULL)  { output = 45; }
   else if (strstr(buffer, "M9") != NULL)  { output = 46; }
   else if (strstr(buffer, "AA9") != NULL) { output = 48; }
   else if (strstr(buffer, "A9") != NULL)  { output = 47; }

   return output * sign;
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



///////////////////////////////////////////////////////////////////////////
//
// Automatic transposition functions
//


//////////////////////////////
//
// doAutoTransposeAnalysis --
//

void doAutoTransposeAnalysis(HumdrumFile& infile) {
   Array<int> ktracks;
   ktracks.setSize(infile.getMaxTracks()+1);
   ktracks.setGrowth(0);
   ktracks.setAll(0);

   int i;
   for (i=1; i<=infile.getMaxTracks(); i++) {
      if (strcmp(infile.getTrackExInterp(i), "**kern") == 0) {
         ktracks[i] = 1 + i;
      }
   }

   infile.analyzeRhythm("4");
   int segments = int(infile.getTotalDuration()+0.5);
   if (segments < 1) {
      segments = 1;
   }

   Array<Array<Array<double> > > trackhist;
   trackhist.setSize(ktracks.getSize());
   trackhist.allowGrowth(0);

   trackhist[0].setSize(0);
   for (i=1; i<trackhist.getSize(); i++) {
      trackhist[i].setSize(0);
      if (ktracks[i]) {
         storeHistogramForTrack(trackhist[i], infile, i, segments);
      } 
   }

   if (debugQ) {
      cout << "Segment pitch histograms: " << endl;
      printHistograms(segments, ktracks, trackhist);
   }

   int level = 16;
   int hop   = 8;
   int count = segments / hop;

   if (segments < count * level / (double)hop) {
      level = level / 2;
      hop   = hop / 2;
   }
   if (segments < count * level / (double)hop) {
      count = count / 2;
   }

   if (segments < count * level / (double)hop) {
      level = level / 2;
      hop   = hop / 2;
   }
   if (segments < count * level / (double)hop) {
      count = count / 2;
   }

   Array<Array<Array<double> > > analysis;

   doAutoKeyAnalysis(analysis, level, hop, count, segments, ktracks, trackhist);

   // print analyses raw results

   cout << "Raw key analysis by track:" << endl;
   printRawTrackAnalysis(analysis, ktracks);


   doTranspositionAnalysis(analysis);


}



//////////////////////////////
//
// doTranspositionAnalysis --
//

void doTranspositionAnalysis(Array<Array<Array<double> > >& analysis) {
   int i, j, k;
   int value1;
   int value2;
   int value;

   for (i=0; i<1; i++) {
      for (j=2; j<3; j++) {
         for (k=0; k<analysis[i].getSize(); k++) {
            if (analysis[i][k][24] >= 0 && analysis[j][k][24] >= 0) {
	       value1 = (int)analysis[i][k][25];
	       if (value1 >= 12) {
                  value1 = value1 - 12;
               }
	       value2 = (int)analysis[j][k][25];
	       if (value2 >= 12) {
                  value2 = value2 - 12;
               }
	       value = value1 - value2;
	       if (value < 0) {
                  value = value + 12;
               }
               if (value > 6) {
                  value = 12 - value;
               }
               cout << value << endl;
            }
         }
      }
   }
}



//////////////////////////////
//
// printRawTrackAnalysis --
//

void printRawTrackAnalysis(Array<Array<Array<double> > >& analysis, 
      Array<int>& ktracks) {

   int i, j;
   int value;
   int value2;

   for (i=0; i<analysis[0].getSize(); i++) {
      cout << "Frame\t" << i << ":";
      for (j=0; j<analysis.getSize(); j++) {
         cout << "\t";
	 value = (int)analysis[j][i][24];
	 if (value >= 12) {
            value = value - 12;
         }
	 value2 = (int)analysis[j][i][25];
	 if (value2 >= 12) {
            value2 = value2 - 12;
         }
         cout << value;
	 // if (value != value2) {
	 //    cout << "," << value2;
         // }
      }
      cout << "\n";
   }
}



//////////////////////////////
//
// doAutoKeyAnalysis --
//

void doAutoKeyAnalysis(Array<Array<Array<double> > >& analysis, int level, 
      int hop, int count, int segments, Array<int>& ktracks, 
      Array<Array<Array<double> > >& trackhist) {

   Array<double> majorweights;
   Array<double> minorweights;
   fillWeightsWithKostkaPayne(majorweights, minorweights);

   int size = 0;
   int i;
   for (i=1; i<ktracks.getSize(); i++) {
      if (ktracks[i]) {
         size++;
      }
   }

   analysis.setSize(size);
   analysis.allowGrowth(0);
   for (i=0; i<analysis.getSize(); i++) {
      analysis[i].setSize(count);
      analysis[i].setSize(0);
   }

   ktracks.allowGrowth(0);
   int aindex = 0;
   for (i=1; i<ktracks.getSize(); i++) {
      if (!ktracks[i]) {
         continue;
      }
      doTrackKeyAnalysis(analysis[aindex++], level, hop, count, 
            trackhist[i], majorweights, minorweights);
   }
}



//////////////////////////////
//
// doTrackKeyAnalysis -- Do individual key analyses of sections of the
//   given track.
//

void doTrackKeyAnalysis(Array<Array<double> >& analysis, int level, int hop, 
      int count, Array<Array<double> >& trackhist, 
      Array<double>& majorweights, Array<double>& minorweights) {

   int i;
   for (i=0; i<count; i++) {
      if (i * hop + level > trackhist.getSize()) {
         break;
      }
      analysis.setSize(i+1);
      doSingleAnalysis(analysis[analysis.getSize()-1], i*hop+level, level,
            trackhist, majorweights, minorweights);
   }
}



//////////////////////////////
//
// doSingleAnalysis --
//

void doSingleAnalysis(Array<double>& analysis, int startindex, int length,
      Array<Array<double> >& trackhist, Array<double>& majorweights, 
      Array<double>& minorweights) {
   Array<double> histsum(12);
   histsum.allowGrowth(0);
   histsum.setAll(0);
   int i, k;


   for (i=0; (i<length) && (startindex+i+length<trackhist.getSize()); i++) {
      for (k=0; k<12; k++) {
         histsum[k] += trackhist[i+startindex][k];
      }
   }

   identifyKey(analysis, histsum, majorweights, minorweights);
}



///////////////////////////////
//
// fillWeightsWithKostkaPayne --
//

void fillWeightsWithKostkaPayne(Array<double>& maj, Array<double>& min) {
   maj.setSize(12);
   maj.allowGrowth(0);
   min.setSize(12);
   min.allowGrowth(0);

   // found in David Temperley: Music and Probability 2006
   maj[0]  = 0.748;	// C major weights
   maj[1]  = 0.060;	// C#
   maj[2]  = 0.488;	// D
   maj[3]  = 0.082;	// D#
   maj[4]  = 0.670;	// E
   maj[5]  = 0.460;	// F
   maj[6]  = 0.096;	// F#
   maj[7]  = 0.715;	// G
   maj[8]  = 0.104;	// G#
   maj[9]  = 0.366;	// A
   maj[10] = 0.057;	// A#
   maj[11] = 0.400;	// B
   min[0]  = 0.712;	// c minor weights
   min[1]  = 0.084;	// c#
   min[2]  = 0.474;	// d
   min[3]  = 0.618;	// d#
   min[4]  = 0.049;	// e
   min[5]  = 0.460;	// f
   min[6]  = 0.105;	// f#
   min[7]  = 0.747;	// g
   min[8]  = 0.404;	// g#
   min[9]  = 0.067;	// a
   min[10] = 0.133;	// a#
   min[11] = 0.330;	// b
}



////////////////////////////////////////
//
// identifyKey -- correls contains the 12 major key correlation
//      values, then the 12 minor key correlation values, then two
//      more values: index=24 is the best key, and index=25 is the 
//      second best key.  If [24] or [25] is -1, then that means that
//      all entries in the original histogram were zero (all rests).
//

void identifyKey(Array<double>& correls, Array<double>& histogram,
      Array<double>& majorweights, Array<double>& minorweights) {

   correls.setSize(26);
   correls.allowGrowth(0);
   correls.setAll(0);

   int i;
   double h[24];
   for (i=0; i<12; i++) {
      h[i] = histogram[i];
      h[i+12] = h[i];
   }

   double testsum = 0.0;
   for (i=0; i<12; i++) {
      testsum += h[i];
   }
   if (testsum == 0.0) {
      correls[24] = -1;
      correls[25] = -1;
      return;
   }

   for (i=0; i<12; i++) {
      correls[i]    = pearsonCorrelation(12, majorweights.getBase(), h+i);
      correls[i+12] = pearsonCorrelation(12, minorweights.getBase(), h+i);
   }

   // find max value
   int besti = 0;
   for (i=1; i<24; i++) {
      if (correls[i] > correls[besti]) {
         besti = i;
      }
   }

   // find second best key 
   int secondbesti = 0;
   if (besti == 0) {
      secondbesti = 1;
   }
   for (i=1; i<24; i++) {
      if (i == besti) {
         continue;
      }
      if (correls[i] > correls[secondbesti]) {
         secondbesti = i;
      }
   }

   correls[24] = besti;
   correls[25] = secondbesti;
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
// printHistograms -- 
//

void printHistograms(int segments, Array<int> ktracks, 
       Array<Array<Array<double> > >& trackhist) {
   int i, j, k;
   int start;

   for (i=0; i<segments; i++) {
//cout << "i=" << i << endl;
      cout << "segment " << i 
	   << " ==========================================\n";
      for (j=0; j<12; j++) {
         start = 0;
//cout << "j=" << i << endl;
         for (k=1; k<ktracks.getSize(); k++) {
//cout << "k=" << i << endl;
            if (!ktracks[k]) {
               continue;
            }
            if (!start) {
               cout << j;
               start = 1;
            }
            cout << "\t";
	    cout << trackhist[k][i][j];
         }
	 if (start) {
            cout << "\n";
         }
      }
   }
   cout << "==========================================\n";
}



//////////////////////////////
//
// storeHistogramForTrack --
//

double storeHistogramForTrack(Array<Array<double> >& histogram, 
      HumdrumFile& infile, int track, int segments) {

   histogram.setSize(segments);
   histogram.allowGrowth(0);

   int i;
   int j;
   int k;

   for (i=0; i<histogram.getSize(); i++) {
      histogram[i].setSize(12);
      histogram[i].setGrowth(0);
      histogram[i].setAll(0);
   }
	    
   infile.analyzeRhythm("4");
   double totalduration = infile.getTotalDuration();

   double duration;
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
	 if (infile[i].getPrimaryTrack(j) != track) {
            continue;
		    
         }	 
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
            addToHistogramDouble(histogram, pitch,
                  start, duration, totalduration, segments);
         }
      }
   }

   return totalduration;
}



//////////////////////////////
//
// addToHistogramDouble -- fill the pitch histogram in the right spots.
//

void addToHistogramDouble(Array<Array<double> >& histogram, int pc, 
      double start, double dur, double tdur, int segments) {

   pc = (pc + 12) % 12;

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


//
// Automatic transposition functions
//
///////////////////////////////////////////////////////////////////////////
//
// Spine field list extraction functions
//

//////////////////////////////
//
// fillFieldData --
//

void fillFieldData(Array<int>& field, const char* fieldstring, 
      HumdrumFile& infile) {

   int maxtrack = infile.getMaxTracks();

   field.setSize(maxtrack+1);
   field.setGrowth(0);
   field.setAll(0);

   Array<int> tempfield;
   tempfield.setSize(maxtrack);
   tempfield.setSize(0);

   PerlRegularExpression pre;
   Array<char> buffer;
   buffer.setSize(strlen(fieldstring)+1);
   strcpy(buffer.getBase(), fieldstring);
   pre.sar(buffer, "\\s", "", "gs");
   int start = 0;
   int value = 0;
   value = pre.search(buffer.getBase(), "^([^,]+,?)");
   while (value != 0) {
      start += value - 1;
      start += strlen(pre.getSubmatch(1));
      processFieldEntry(tempfield, pre.getSubmatch(), infile);
      value = pre.search(buffer.getBase() + start, "^([^,]+,?)");
   }

   int i;
   for (i=0; i<tempfield.getSize(); i++) {
     field[tempfield[i]-1] = 1; 
   }
}



//////////////////////////////
//
// processFieldEntry -- 
//   3-6 expands to 3 4 5 6
//   $   expands to maximum spine track
//   $-1 expands to maximum spine track minus 1, etc.
//

void processFieldEntry(Array<int>& field, const char* string, 
      HumdrumFile& infile) {

   int maxtrack = infile.getMaxTracks();

   PerlRegularExpression pre;
   Array<char> buffer;
   buffer.setSize(strlen(string)+1);
   strcpy(buffer.getBase(), string);

   // remove any comma left at end of input string (or anywhere else)
   pre.sar(buffer, ",", "", "g");

   // first remove $ symbols and replace with the correct values
   removeDollarsFromString(buffer, infile.getMaxTracks());

   if (pre.search(buffer.getBase(), "^(\\d+)-(\\d+)$")) {
      int firstone = strtol(pre.getSubmatch(1), NULL, 10);
      int lastone  = strtol(pre.getSubmatch(2), NULL, 10);

      if ((firstone < 1) && (firstone != 0)) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains too small a number at start: " << firstone << endl;
         cerr << "Minimum number allowed is " << 1 << endl;
         exit(1);
      }
      if ((lastone < 1) && (lastone != 0)) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains too small a number at end: " << lastone << endl;
         cerr << "Minimum number allowed is " << 1 << endl;
         exit(1);
      }
      if (firstone > maxtrack) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains number too large at start: " << firstone << endl;
         cerr << "Maximum number allowed is " << maxtrack << endl;
         exit(1);
      }
      if (lastone > maxtrack) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains number too large at end: " << lastone << endl;
         cerr << "Maximum number allowed is " << maxtrack << endl;
         exit(1);
      }

      int i;
      if (firstone > lastone) {
         for (i=firstone; i>=lastone; i--) {
            field.append(i);
         }
      } else {
         for (i=firstone; i<=lastone; i++) {
            field.append(i);
         }
      }
   } else if (pre.search(buffer.getBase(), "^(\\d+)")) {
      int value = strtol(pre.getSubmatch(1), NULL, 10);
      if ((value < 1) && (value != 0)) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains too small a number at end: " << value << endl;
         cerr << "Minimum number allowed is " << 1 << endl;
         exit(1);
      }
      if (value > maxtrack) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains number too large at start: " << value << endl;
         cerr << "Maximum number allowed is " << maxtrack << endl;
         exit(1);
      }
      field.append(value);
   }
}



//////////////////////////////
//
// removeDollarsFromString -- substitute $ sign for maximum track count.
//

void removeDollarsFromString(Array<char>& buffer, int maxtrack) {
   PerlRegularExpression pre;
   char buf2[128] = {0};
   int value2;

   if (pre.search(buffer.getBase(), "\\$$")) {
      sprintf(buf2, "%d", maxtrack);
      pre.sar(buffer, "\\$$", buf2);
   }

   if (pre.search(buffer.getBase(), "\\$(?![\\d-])")) {
      // don't know how this case could happen, however...
      sprintf(buf2, "%d", maxtrack);
      pre.sar(buffer, "\\$(?![\\d-])", buf2, "g");
   }

   if (pre.search(buffer.getBase(), "\\$0")) {
      // replace $0 with maxtrack (used for reverse orderings)
      sprintf(buf2, "%d", maxtrack);
      pre.sar(buffer, "\\$0", buf2, "g");
   }

   while (pre.search(buffer.getBase(), "\\$(-?\\d+)")) {
      value2 = maxtrack - (int)fabs(strtol(pre.getSubmatch(1), NULL, 10));
      sprintf(buf2, "%d", value2);
      pre.sar(buffer, "\\$-?\\d+", buf2);
   }

}


//
// Spine field list extraction functions
//
///////////////////////////////////////////////////////////////////////////



/* BRIEF DOCUMENTATION


transpose options:

   -t interval = transpose music by the specified interval, where 
                 interval is of the form:
                    P1 = perfect unison (no transposition)
                    m2 = up a minor second
                   -m2 = down a minor second
                    M3 = up a major third
                   -A4 = down an augmented fourth
                    d5 = up a diminished fifth

   -b interval = transpose by the base-40 equivalent to the -t option interval
                     0 = perfect unison (no transposition)
                     6 = up a minor second
                    -6 = down a minor second
                    12 = up a major third
                   -18 = down an augmented fourth
                    22 = up a diminished fifth


   -o octave  = transpose (additionally by given octave)
		transpose -t m3 -o 1 = transpose up by an octave and a minor
			               third.


   -s fieldstring = transpose only the given list of data spines.
		    example:
 			transpose -f1-2,4 -t P4 fourpart.krn
 			transpose -f3     -t P4 fourpart.krn
		   Note: does not work yet with the -k option

                    

##########################################################################
##
## EXAMPLES
##



input: file.krn  -- an example with the key being a minor:

**kern
*a:
4A
4B
4c
4d
4e
4f
4g
4a
*-


#####################################################################
#
# Transpose the file up a minor third (so that it is in C Minor):
#

tranpose -t m3 file.krn

**kern
*c:
4c
4d
4e-
4f
4g
4a-
4b-
4cc
*-

#####################################################################
#
# Transpose the file down a minor third (so that it is in F# Minor):
#

tranpose -t -m3 file.krn

**kern
*f#:
4F#
4G#
4A
4B
4c#
4d
4e
4f#
*-


#####################################################################
#
# Transpose the file up a perfect octave:
#

tranpose -t P8 file.krn

**kern
*a:
4A
4B
4cc
4dd
4ee
4ff
4gg
4aa
*-




#####################################################################
#
# Force the file to a tonic on C rather than a:
#

transpose -k c file.krn

**kern
*c:
4c
4d
4e-
4f
4g
4a-
4b-
4cc
*-


# case -k option value is irrelevant:

transpose -k C file.krn

**kern
*c:
4c
4d
4e-
4f
4g
4a-
4b-
4cc
*-


# Transpose from A Minor to G# Minor:

transpose -k G# file.krn

**kern
*g#:
4G#
4A#
4B
4c#
4d#
4e
4f#
4g#
*-


# Changing input files to:

**kern
*C:
4c
4d
4e
4f
*G:
4g
4a
4b
4cc
*-


# Using -k option will convert all keys to same in output:

transpose -k e file.krn

**kern
*E:
4e
4f#
4g#
4a
*E:
4e
4f#
4g#
4a
*-



##############################
##
## octave transpositions
##


# Back to original data file, transposing up a minor tenth:

transpose -o 1 -t m3 file.krn

**kern
*E-:
8ee-
8ff
8gg
8aa-
8bb-
8ccc
8ddd
8eee-
*-

# transpose down two octaves:

transpose -o -2 file.krn

**kern
*a:
4AAA
4BBB
4CC
4DD
4EE
4FF
4GG
4AA
*-


####################
##
## base 40 -b option instead of -t option
##


# Transpose down two octaves:
transpose -b -80 file.krn

**kern
*a:
4AAA
4BBB
4CC
4DD
4EE
4FF
4GG
4AA
*-

*/


// md5sum: e92e0977d1f92f5fa48d3d3dc2bcacfb transpose.cpp [20100602]

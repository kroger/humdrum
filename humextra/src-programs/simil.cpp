// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Note:          Adapted from the Humdrum Toolkit program "simil":
//                   simil.c 1.3 92/04/21 by Keith Orpen, University of 
//                   Waterloo, Ontario.
// Creation Date: Tue Nov 17 14:35:26 PST 2009
// Last Modified: Tue Dec  8 20:06:25 PST 2009
// Filename:      ...sig/examples/all/simil.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/simil.cpp
// Syntax:        C++; museinfo
//
// Description:   Measure Damerau-Levenshtein edit distance between a Humdrum
//                source file and template file.
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


// function declarations:
void      checkOptions          (Options& opts, int argc, char** argv);
void      example               (void);
void      usage                 (const char* command);
void      readTemplateContents  (Array<Array<char> >& storage, 
                                 const char* filename);
void      readTemplateContents  (Array<Array<char> >& storage, 
                                 istream& filename);
void      printTemplate         (Array<Array<char> >& storage);
int       chooseSpine           (const char* interp, HumdrumFile& infile);
void      fillSourceData        (Array<Array<char> >& sourcedata, 
                                 Array<int>& datalines, 
		                 HumdrumFile& infile, int spine, int nulltest);
void      usual_thing           (Array<double>& results, 
                                 Array<Array<char> >& sourcedata, 
                                 Array<Array<char> >& templatedata);
void      unusual_thing         (Array<Array<double> >& results, 
                                 Array<Array<char> >& sourcedata, 
				 Array<Array<char> >& templatedata, 
                                 int sublen);
void      printResults          (Array<double>& results, 
                                 Array<Array<char> >& sourcedata,
                                 Array<Array<char> >& templatedata);
void      printResultsSubString (Array<Array<double> >& results, 
                                 Array<Array<char> >& sourcedata, 
                                 Array<Array<char> >& templatedata);
double    dlv                   (Array<Array<char> >& s1, int offset1, 
                                 Array<Array<char> >& s2, int offset2);
void      doDamerauLevenshteinAnalysis(Array<Array<char> >& sourcedata, 
                                 Array<Array<char> >& templatedata);
double    dlvcharstar           (char** set1, int len1, char** set2, int len2);
double    dlvstring             (Array<Array<char> >& set1, int offset1, 
                                 Array<Array<char> >& set2, int offset2,
                                 int temlen);
double    dlvint                (int *s1, int len1, int *s2, int len2);
double    dlv                   (Array<Array<char> >& set1, int offset1, 
                                 Array<Array<char> >& set2, int offset2);
int       getMinIndex           (Array<double>& list);
void      printSubStringInfo    (Array<double>& list, double target);
void      printWeights          (void);
void      readEditWeights       (const char* filename);
void      printSequence         (Array<Array<char> >& sourcedata, int index, 
                                 int size, int flag);

// User interface variables:
Options     options;
int         debugQ    =  0;  // used with --debug option
int         rawQ      =  0;  // used with -R option
int         xlen      =  0;  // used with -x option
int         reverseQ  =  0;  // used with -r option
int         scalingQ  =  0;  // used with -n option
const char* interp    =  ""; // used with -i option
int         pweightQ  = 0;   // used with -W option
int         nullQ     = 0;   // used with -N option
int         appendQ   = 0;   // used with -a option (not implemented)
int         prependQ  = 0;   // used with -p option (not implemented)
int         sequenceQ = 0;   // used with -s option
int         spacesQ   = 1;   // used with -S option
double      threshold = 0.0; // used with -t option

double      weight_R1 = 1.0; // --R1: deleting a repeated element of S1
double      weight_R2 = 1.0; // --R2: deleting a repeated element of S2
double      weight_D1 = 1.0; // --R1: deleting a non-repeated element of S1
double      weight_D2 = 1.0; // --R1: deleting a non-repeated element of S2
double      weight_S0 = 1.0; // --S0: sub by a non-repeated element
double      weight_S1 = 1.0; // --S1: sub by an element repeated in S1
double      weight_S2 = 1.0; // --S2: sub by an element repeated in S2
double      weight_S3 = 1.0; // --S3: sub by an element repeated in S1 and S2


//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   // process the command-line options
   checkOptions(options, argc, argv);
   HumdrumFile infile;
   Array<Array<char> > templatedata;
   Array<Array<char> > sourcedata;
   Array<int> datalines;

   if ((options.getArgCount() < 1) || (options.getArgCount() > 2)) {
      usage(options.getCommand());
      exit(1);
   }

   int sourcearg   = 1;
   int templatearg = 2;
   if (reverseQ) {
      sourcearg   = 2;
      templatearg = 1;
   }
   
   if (options.getArgCount() == 2) {
      infile.read(options.getArg(sourcearg));
      readTemplateContents(templatedata, options.getArg(templatearg));

   } else {
      // read second file from standard input
      if (sourcearg == 1) {
         infile.read(options.getArg(1));
	 readTemplateContents(templatedata, cin);
      } else {
         infile.read(cin);
	 readTemplateContents(templatedata, options.getArg(1));
      }
   }

   int spine = chooseSpine(interp, infile);

   fillSourceData(sourcedata, datalines, infile, spine, nullQ);

   if (debugQ) {
      cout << "SOURCE DATA: " << endl;
      printTemplate(sourcedata);
      cout << "TEMPLATE DATA: " << endl;
      printTemplate(templatedata);
   }
        

   doDamerauLevenshteinAnalysis(sourcedata, templatedata);

   if (pweightQ) {
      printWeights();
   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////



//////////////////////////////
//
// fillSourceData -- extract tokens from source Humdrum file for use
//    in analysis.
//

void fillSourceData(Array<Array<char> >& sourcedata, Array<int>& datalines, 
      HumdrumFile& infile, int spine, int nulltest) {

   sourcedata.setSize(infile.getNumLines());
   sourcedata.setSize(0);
   datalines.setSize(infile.getNumLines());
   datalines.setSize(0);
   
   int strsize;
   int i, j;
   int index;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isData()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (infile[i].getPrimaryTrack(j) != spine) {
            continue;
         }
         if (nulltest && (strcmp(infile[i][j], ".") == 0)) {
            break;
         }
         strsize = strlen(infile[i][j]);
         index   = sourcedata.getSize();
         sourcedata.setSize(index + 1);
         sourcedata[index].setSize(strsize + 1);
         strcpy(sourcedata[index].getBase(), infile[i][j]);
         datalines.append(i);
         break;
      }
   }

   sourcedata.allowGrowth(0);
   datalines.allowGrowth(0);
}



//////////////////////////////
//
// chooseSpine -- select the first column of data; otherwise, locate
//      the first spine of the specified exclusive interpretation type.
//

int chooseSpine(const char* interp, HumdrumFile& infile) {
   if (interp == NULL) {
     return 1;
   }
   if (strcmp(interp, "") == 0) {
     return 1;
   }

   int maxtracks = infile.getMaxTracks();
   int i;
   for (i=1; i<=maxtracks; i++) {
      if (strcmp(interp, infile.getTrackExInterp(i)) == 0) {
         return i;
      }
   }
	     
   cerr << "Error: A " << interp 
        << " data spine is not present in the source" << endl;
   exit(1);

   return 1;
}



//////////////////////////////
//
// printTemplate --
//

void printTemplate(Array<Array<char> >& storage) {
   int i;
   for (i=0; i<storage.getSize(); i++) {
      cout << storage[i].getBase() << "\n";
   }
}


//////////////////////////////
//
// readEditWeights --
//

void readEditWeights(const char* filename) {
   #ifndef OLDCPP
      ifstream weightfile(filename, ios::in);
   #else
      ifstream weightfile(filename, ios::in | ios::nocreate);
   #endif

   if (weightfile.eof()) {
      cerr << "Error opening weightfile file: " << filename << endl;
      exit(1);
   }

   PerlRegularExpression pre;
   PerlRegularExpression pre2;

   Array<char> storage;
   char buffer[1024] = {0};
   int strsize;

   weightfile.getline(buffer, 1024);
   while (!weightfile.eof()) {
      strsize = strlen(buffer);
      storage.setSize(strsize+1);
      strcpy(storage.getBase(), buffer);
      weightfile.getline(buffer, 1024);
      pre.sar(storage, "\\s*#.*", "", "");
      if (pre.search(storage, 
            "^\\s*(d1|d2|r1|r2|s0|s1|s2|s3)\\s+([\\d.+\\-]+)", "i")) {
         if (pre2.search(pre.getSubmatch(1), "D1", "i")) {
            weight_D1 = atof(pre.getSubmatch(2));
         } else if (pre2.search(pre.getSubmatch(1), "D2", "i")) {
            weight_D2 = atof(pre.getSubmatch(2));
         } else if (pre2.search(pre.getSubmatch(1), "R1", "i")) {
            weight_R1 = atof(pre.getSubmatch(2));
         } else if (pre2.search(pre.getSubmatch(1), "R2", "i")) {
            weight_R2 = atof(pre.getSubmatch(2));
         } else if (pre2.search(pre.getSubmatch(1), "S0", "i")) {
            weight_S0 = atof(pre.getSubmatch(2));
         } else if (pre2.search(pre.getSubmatch(1), "S1", "i")) {
            weight_S1 = atof(pre.getSubmatch(2));
         } else if (pre2.search(pre.getSubmatch(1), "S2", "i")) {
            weight_S2 = atof(pre.getSubmatch(2));
         } else if (pre2.search(pre.getSubmatch(1), "S3", "i")) {
            weight_S3 = atof(pre.getSubmatch(2));
         }
      }
   }

   weightfile.close();
}



//////////////////////////////
//
// readTemplateContents -- read the contents of the template file
//

void readTemplateContents(Array<Array<char> >& storage, const char* filename) {
   #ifndef OLDCPP
      ifstream templatefile(filename, ios::in);
   #else
      ifstream templatefile(filename, ios::in | ios::nocreate);
   #endif
   readTemplateContents(storage, templatefile);
   templatefile.close();
}


void readTemplateContents(Array<Array<char> >& storage, istream& instream) {
   if (instream.eof()) {
      cerr << "Error opening template file" << endl;
      exit(1);
   }

   storage.setSize(1000);
   storage.setSize(0);
   storage.setGrowth(100000);

   char buffer[1024] = {0};
   int index;
   int strsize;

   instream.getline(buffer, 1024);
   while (!instream.eof()) {
      strsize = strlen(buffer);
      storage.setSize(storage.getSize()+1);
      index = storage.getSize() - 1;
      storage[index].setSize(strsize+1);
      strcpy(storage[index].getBase(), buffer);
      instream.getline(buffer, 1024);
   }

   storage.allowGrowth(0);
}




//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   options.define("n|no-normalize=b",  "do not normalize similarity measures");
   options.define("r|reverse=b",       "reverse source and template arguments");
   options.define("R|raw=b",           "don't normalize the DLV values");
   options.define("i|interp=s",        "use first spine in excl. interp.");
   options.define("x|length=i:0",      "subordinate pattern length");
   options.define("W|print-weights=b", "print edit operation scores");
   options.define("w|weight-file=s",   "load editing weights from file");
   options.define("N|exclude-null-tokens=b", "do not consider null tokens");
   options.define("a|append=b",        "append analysis to input data");
   options.define("p|prepend=b",       "prepend analysis to input data");
   options.define("debug=b",           "print debugging information");
   options.define("s|sequence=b",      "print search sequences");
   options.define("S|no-spaces=b",     "print search sequences without spaces");
   options.define("t|threshold=d:0.0", "similarity threshold for output");

   options.define("R1|r1=d:1.0", "scr for deleting a repeated element of S1");
   options.define("R2|r2=d:1.0", "scr for deleting a repeated element of S2");
   options.define("D1|d1=d:1.0", "scr for deleting non-repeated element of S1");
   options.define("D2|d2=d:1.0", "scr for deleting non-repeated element of S2");
   options.define("S0|s0=d:1.0", "scr for sub by a non-repeated element");
   options.define("S1|s1=d:1.0", "scr for sub by element repeated in S1");
   options.define("S2|s2=d:1.0", "scr for sub by element repeated in S2");
   options.define("S3|s3=d:1.0", "scr for sub by element repeated in S1 & S2");

   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Adapted by Craig Stuart Sapp 2009" 
           << " from the simil program written by Keith Opern 1992"
	   << endl;
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
   
   xlen      =  options.getInteger("x");
   reverseQ  =  options.getBoolean("r");
   scalingQ  = !options.getBoolean("n");
   interp    =  NULL;
   if (options.getBoolean("i")) {
      interp =  options.getString("i");
   }
   debugQ    =  options.getBoolean("debug");
   rawQ      =  options.getBoolean("raw");
   pweightQ  =  options.getBoolean("print-weights");
   nullQ     =  options.getBoolean("exclude-null-tokens");
   appendQ   =  options.getBoolean("append");
   prependQ  =  options.getBoolean("prepend");
   sequenceQ =  options.getBoolean("sequence");
   spacesQ   = !options.getBoolean("no-spaces");
   threshold =  options.getDouble("threshold");

   if (options.getBoolean("weight-file")) {
      readEditWeights(options.getString("weight-file"));
   }

   if (options.getBoolean("R1")) { weight_R1 = options.getDouble("R1"); }
   if (options.getBoolean("R2")) { weight_R2 = options.getDouble("R2"); }
   if (options.getBoolean("D1")) { weight_D1 = options.getDouble("D1"); }
   if (options.getBoolean("D2")) { weight_D2 = options.getDouble("D2"); }
   if (options.getBoolean("S0")) { weight_S0 = options.getDouble("S0"); }
   if (options.getBoolean("S1")) { weight_S1 = options.getDouble("S1"); }
   if (options.getBoolean("S2")) { weight_S2 = options.getDouble("S2"); }
   if (options.getBoolean("S3")) { weight_S3 = options.getDouble("S3"); }
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



//////////////////////////////
//
// doDamerauLevenshteinAnalysis --
//

void doDamerauLevenshteinAnalysis(Array<Array<char> >& sourcedata, 
      Array<Array<char> >& templatedata) {
   
   if (xlen <= 0) {
      Array<double> sresults;
      usual_thing(sresults, sourcedata, templatedata);
      printResults(sresults, sourcedata, templatedata);
   } else {
      Array<Array<double> > mresults;
      unusual_thing(mresults, sourcedata, templatedata, xlen);
      printResultsSubString(mresults, sourcedata, templatedata);
   }
}



double normalize1(double d, int len) {
   return exp(-d/len);
}

double normalize2(double d, int len) {
   return exp(-d);
}



//////////////////////////////
//
// unusual_thing -- Do sub-string matching
//

void unusual_thing(Array<Array<double> >& results, 
      Array<Array<char> >& sourcedata, Array<Array<char> >& templatedata, 
      int sublen) {

   int i, j;
   int len = (int)fabs(sourcedata.getSize() - sublen + 1);
   int subcount = templatedata.getSize() - sublen + 1;

   results.setSize(0);
   if (len <= 0) {
      return;
   }
   results.setSize(len);

   for (i=0; i<results.getSize(); i++) {
      results[i].setSize(subcount);
   }

   for (i=0; i<len; i++) {
      for (j=0; j<subcount; j++) {
         results[i][j] = dlvstring(sourcedata, i, templatedata, j, sublen);
      }
   }
}



//////////////////////////////
//
// getMinIndex --  need to get the index of the minimum value
//    on the line, because this is the raw value before scaling.
//    Scaling will flip the ordering so that the minimum value
//    will become the maximum value.
//

int getMinIndex(Array<double>& list) {
   int i;
   int output = 0;
   if (list.getSize() <= 0) {
      return -1;
   }

   for (i=1; i<list.getSize(); i++) {
      if (list[i] < list[output]) {
         output = i;
      }
   }

   return output;
}



//////////////////////////////
//
// printSubStringInfo -- print all of the substrings which are
//   at an equal or higher similarity measurement value than
//   the target.  The thresholding is done on the raw edit 
//   distance values, so the threshold is for a minimum rather
//   than a maximum.
//

void printSubStringInfo(Array<double>& list, double target) {
   int i;
   int counter = 0;
   for (i=0; i<list.getSize(); i++) {
      if (list[i] <= target) {
         counter++;
         if (counter > 1) {
            cout << ",";
         }
         cout << i+1;
      }
   }

}



//////////////////////////////
//
// printResultsSubString --
//

void printResultsSubString(Array<Array<double> >& results, 
      Array<Array<char> >& sourcedata, Array<Array<char> >& templatedata) {
   int i;
   double value;
   cout << "**simil\t**simxrf" << endl;
   int len = xlen;
   if (len <= 0) {
      len = templatedata.getSize();
   }

   int maxi;

   for (i=0; i<results.getSize(); i++) {
      maxi = getMinIndex(results[i]);
      value = results[i][maxi];
      if (rawQ) {
         cout << value;
      } else if (scalingQ) {
         value = normalize1(value, len);
	 value = int(value * 100.0 + 0.5) / 100.0;
	 printf("%.2lf", value);
      } else {
	 value = int(value * 100.0 + 0.5) / 100.0;
         value = normalize2(value, len);
	 printf("%.2lf", value);
      }
      cout << "\t";
      printSubStringInfo(results[i], results[i][maxi]);
      cout << "\n";
   }

   // int dots = templatedata.getSize() - 1;
   int dots = xlen - 1;
   for (i=0; i<dots; i++) {
      cout << ".\t.\n";
   }

   cout << "*-\t*-" << endl;
}



//////////////////////////////
//
// printResults --
//

void printResults(Array<double>& results, Array<Array<char> >& sourcedata,
      Array<Array<char> >& templatedata) {
   int i;
   double value;

   cout << "**simil";
   if (sequenceQ) {
      cout << "\t**seq";
   }
   cout << endl;

   int len = templatedata.getSize();

   for (i=0; i<results.getSize(); i++) {
      value = results[i];
      if (normalize1(value, len) < threshold) {
         cout << ".";
         if (sequenceQ) {
            cout << "\t";
            cout << ".";
	    cout << "\n";
         } else {
            cout << "\n";
         }
         continue;
      }
      if (rawQ) {
         cout << value;
      } else if (scalingQ) {
         value = normalize1(value, len);
	 value = int(value * 100.0 + 0.5) / 100.0;
	 printf("%.2lf", value);
      } else {
	 value = int(value * 100.0 + 0.5) / 100.0;
         value = normalize2(value, len);
	 printf("%.2lf", value);
      }
      if (sequenceQ) {
         cout << "\t";
         printSequence(sourcedata, i, templatedata.getSize(), spacesQ);
      }
      cout << "\n";
   }

   int dots = templatedata.getSize() - 1;
   for (i=0; i<dots; i++) {
      cout << ".";
      if (sequenceQ) {
         cout << "\t.";
      }
      cout << "\n";
   }

   cout << "*-";
   if (sequenceQ) {
      cout << "\t*-";
   }
   cout << endl;
}



//////////////////////////////
//
// printSequence --
// 

void printSequence(Array<Array<char> >& sourcedata, int index, int size, 
      int flag) {
   int i;
   if (size <= 0) {
      cout << ".";
      return;
   }

   cout << sourcedata[index].getBase();

   for (i=index+1; i<index+size; i++) {
      if (flag) {
         cout << ' ';
      }
      cout << sourcedata[i].getBase();
   }
}



//////////////////////////////
//
// usual_thing --
//

void usual_thing(Array<double>& results, Array<Array<char> >& sourcedata, 
      Array<Array<char> >& templatedata) {

   int len = (int)fabs(sourcedata.getSize() - templatedata.getSize() + 1);

   results.setSize(0);
   if (len <= 0) {
      return;
   }
   results.setSize(len);
   results.setAll(0.0);

   for (int i=0; i<len; i++) {
      results[i] = dlv(sourcedata, i, templatedata, 0);
   }
}


//////////////////////////////
//
// printWeights --
//

void printWeights(void) {
   cout << "!! weight_R1: " << weight_R1 << "\n";
   cout << "!! weight_R2: " << weight_R2 << "\n";
   cout << "!! weight_D1: " << weight_D1 << "\n";
   cout << "!! weight_D2: " << weight_D2 << "\n";
   cout << "!! weight_S0: " << weight_S0 << "\n";
   cout << "!! weight_S1: " << weight_S1 << "\n";
   cout << "!! weight_S2: " << weight_S2 << "\n";
   cout << "!! weight_S3: " << weight_S3 << endl;
}



//////////////////////////////
//
// dlv -- Compute the Damerau-Levenshtein distance of two arrays of 
//      integers,  subject to specified weights.           
//      
//      dlv         = DLV raw value from Array<char> using dlvcharstar.
//      dlvstring   = DLV raw value from Array<char> data.
//      dlvcharstar = DLV raw value from char* data.
//      dlvint      = DLV raw value from int data. (original function)
//

double dlv(Array<Array<char> >& set1, int offset1, 
      Array<Array<char> >& set2, int offset2) {

   Array<char*> list1;
   Array<char*> list2;
   if (offset1 < 0) { offset1 = 0; }
   if (offset2 < 0) { offset2 = 0; }
   
   int size1 = set1.getSize() - offset1;
   int size2 = set2.getSize() - offset2;

   if ((size1 < 0) || (size2 < 0)) {
      cerr << "Error in offset values: " << size1 << ", " << size2 << endl;
      exit(1);
   }

   list1.setSize(size1);
   list2.setSize(size2);

   int i;
   for (i=0; i<size1; i++) {
      list1[i] = set1[i+offset1].getBase();
   }
   for (i=0; i<size2; i++) {
      list2[i] = set2[i+offset2].getBase();
   }

   return dlvcharstar(list1.getBase(), size1, list2.getBase(), size2);
}



double dlvstring(Array<Array<char> >& set1, int offset1, 
      Array<Array<char> >& set2, int offset2, int temlen = -1) {

   int len1 = set1.getSize() - offset1;
   int len2 = set2.getSize() - offset2;

   if (temlen == -1) {
      // tem len is the length of the substring to consider
      temlen = set2.getSize();
   }

   int len = len1;   // the length of the sequences to compare

   if (len1 > len2) {
      len = len2;
   } 

   if (temlen < len) {
      len = temlen;
   }

   double cost, val, m;
   int i, j;
   int mindex;
   int s1index, s2index;
   int n = 0;

   if (len+1 > n)  {
      n   = len + 1;
   }

   double min[n];
   int rep1;
   int rep2;

   mindex = 0;
   s1index = offset1;
   m = 0.0;
   min[mindex] = 0.0;

   for (i=0; i<len; i++, s1index++)  {
      m = min[mindex++];
      if ((s1index>0) && 
          (strcmp(set1[s1index].getBase(), set1[s1index-1].getBase()) == 0)) {
         m += weight_R1;
      } else {
         m += weight_D1;
      }
      min[mindex] = m;
   }
   
   s2index = offset2;
   for (j=0; j<len; j++, s2index++)  {
      rep2 = (s2index>0 && (strcmp(set2[s2index].getBase(), 
				   set2[s2index-1].getBase()) == 0));
      mindex = 0;
      cost = min[mindex];
      m = cost + (rep2 ? weight_R2 : weight_D2);
   
      min[mindex++] = m;
   
      s1index = offset1;
      for (i=0; i<len; i++, s1index++)  {
         rep1 = (s1index>0) && 
             (strcmp(set1[s1index].getBase(), 
                     set1[s1index-1].getBase()) == 0);
	       
         m += rep1 ? weight_R1 : weight_D1;
   
         if (strcmp(set1[s1index].getBase(),set2[s2index].getBase()) == 0) {
            val = cost;
         } else  {
            if (rep1) {
               val = cost + (rep2? weight_S3:weight_S1);
            } else { 
               val = cost + (rep2? weight_S2:weight_S0);
            }
         }
         if (val < m) {
            m = val;
         }
   
	 cost = min[mindex];
         val = cost + (rep2 ? weight_R2 : weight_D2);
         if (val < m) {
            m = val;
         }
   
	 min[mindex++] = m;
      }
   }

   return (double)m;
}



double dlvint(int *s1, int len1, int *s2, int len2) {
   float cost, val, m;
   int i, j;
   float *mptr;
   int *s1ptr, *s2ptr;
   int n = 0;
   float *min = NULL;

   if (len1+1 > n)  {
      min = new float[len1+1];
      n   = len1 + 1;
   }

   mptr = min;

   s1ptr = s1;
   m = *mptr = 0.0;

   for (i=len1; --i>=0; s1ptr++)  {
      m = *mptr++;
      if (s1ptr>s1 && *s1ptr==*(s1ptr-1)) {
         m += weight_R1;
      } else {
         m += weight_D1;
      }
   
      *mptr = m;
      }
   
      s2ptr = s2;
      for (j=len2; --j>=0; s2ptr++)  {
      char rep2 = (s2ptr>s2 && *s2ptr==*(s2ptr-1));
   
      mptr = min;
      m = (cost = *mptr) + (rep2? weight_R2:weight_D2);
   
      *mptr++ = m;
   
      s1ptr = s1;
      for (i=len1; --i>=0; s1ptr++)  {
         char rep1 = (s1ptr>s1 && *s1ptr==*(s1ptr-1));
   
         m += (rep1? weight_R1:weight_D1);
   
         if (*s1ptr==*s2ptr)
         val = cost;
         else  {
         if (rep1)
            val = cost + (rep2? weight_S3:weight_S1);
         else
            val = cost + (rep2? weight_S2:weight_S0);
         }
         if (val < m) { m = val; }
   
         val = (cost = *mptr) + (rep2? weight_R2:weight_D2);
         if (val < m) m = val;
   
         *mptr++ = m;
      }
   }

   delete [] min;
   return (double)m;
}



double dlvcharstar(char** set1, int len1, char** set2, int len2) {

   int len = len1;   // the length of the sequences to compare

   if (len1 > len2) {
      len = len2;
   } 

   double cost, val, m;
   int i, j;
   int mindex;
   int s1index, s2index;
   int n = 0;

   if (len+1 > n)  {
      n   = len + 1;
   }

   double min[n];
   int rep1;
   int rep2;

   mindex = 0;
   s1index = 0;
   m = 0.0;
   min[mindex] = 0.0;

   for (i=0; i<len; i++, s1index++)  {
      m = min[mindex++];
      if ((s1index>0) && 
          (strcmp(set1[s1index], set1[s1index-1]) == 0)) {
         m += weight_R1;
      } else {
         m += weight_D1;
      }
      min[mindex] = m;
   }
   
   s2index = 0;
   for (j=0; j<len; j++, s2index++)  {
      rep2 = (s2index>0 && (strcmp(set2[s2index], 
				   set2[s2index-1]) == 0));
      mindex = 0;
      cost = min[mindex];
      m = cost + (rep2 ? weight_R2 : weight_D2);
   
      min[mindex++] = m;
   
      s1index = 0;
      for (i=0; i<len; i++, s1index++)  {
         rep1 = (s1index>0) && 
             (strcmp(set1[s1index], 
                     set1[s1index-1]) == 0);
	       
         m += rep1 ? weight_R1 : weight_D1;
   
         if (strcmp(set1[s1index],set2[s2index]) == 0) {
            val = cost;
         } else  {
            if (rep1) {
               val = cost + (rep2? weight_S3:weight_S1);
            } else { 
               val = cost + (rep2? weight_S2:weight_S0);
            }
         }
         if (val < m) {
            m = val;
         }
   
	 cost = min[mindex];
         val = cost + (rep2 ? weight_R2 : weight_D2);
         if (val < m) {
            m = val;
         }
   
	 min[mindex++] = m;
      }
   }

   return (double)m;
}


// md5sum: 9d1b6764bcd4aa329ba525b7385aa3f2 simil.cpp [20100722]

// Creation Date: Mon Jan 26 08:53:14 PST 2004
// Last Modified: Fri Apr  9 19:02:18 PDT 2004 rhythmic feature extraction
// Last Modified: Mon Apr 12 14:05:52 PDT 2004 adding more rhythm features
// Last Modified: Fri Apr 16 18:01:03 PDT 2004 added directory processing
// Last Modified: Sun Aug  8 18:44:36 PDT 2004 added more rhythm features
// Last Modified: Thu Oct 30 10:38:19 PST 2008 closedir fix for OSX
// Last Modified: Wed Jul  1 16:11:07 PDT 2009 added polyphonic extraction
// Last Modified: Mon Aug 30 13:57:51 PDT 2010 3... rhythm disallowed in dur
// Last Modified: Mon Aug 30 13:57:51 PDT 2010 -2147483648 rhythm --> X
// Last Modified: Wed Sep  1 13:37:32 PDT 2010 added metric position
// Last Modified: Sun Sep 12 21:01:37 PDT 2010 added bibliographic indexing
// Last Modified: Fri Nov 12 07:22:40 PST 2010 added instrument name tag
// Last Modified: Sun Nov 21 16:15:48 PST 2010 add rest boundary marker
// Last Modified: Sat Nov 27 17:23:58 PST 2010 added -D and -d options
// Last Modified: Thu Jan 13 03:15:14 PST 2011 renamed themebuilderx -> tindex
// Last Modified: Sun Jan 16 06:36:42 PST 2011 chaned default behavior to -a
// Last Modified: Tue Jan 18 08:36:29 PST 2011 added --verbose option
// Last Modified: Thu Feb 24 17:35:31 PST 2011 added --file option
// Last Modified: Sat Apr  2 18:04:05 PDT 2011 added L=Long & B=Breve for durs.
// Last Modified: Mon Apr  9 17:27:58 PDT 2012 NBC changes to features
// Last Modified: Thu May 24 12:28:08 PDT 2012 added -u and -I options
// Last Modified: Mon Nov 12 13:56:29 PST 2012 added !noff: processing
// Filename:      ...museinfo/examples/all/tindex.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/tindex.cpp
// Syntax:        C++; museinfo
//
// Description:   Converts Humdrum files into themefinder search index records.
//
// Classical Thema index order:
// is: [Zz] { # : % } j J M
//
// Meaning of the tracer symbols:
//  [Zz] = major/minor key 
//  {    = 12-tone interval
//  #    = pitch refined contour
//  :    = pitch gross contour
//  %    = scale degree
//  }    = musical interval
//  j    = 12-tone pitch
//  J    = absolute pitch
//  M    = metric description
// Added rhythmic markers:
//  ~    = duration gross contour
//  ^    = duration refined contour
//  ;    = duration (IOI)
//  &    = beat level
//  @    = metric gross contour
//  `    = metric refined contour
//  '    = metric level
//  =    = metric position
//
// Todo: When a medial or final tie does not match to 
// an opening tie, that tied note should be indexed.
// This case occurs at multiple repeat endings in scores.

// character markers in index file:
#define P_PITCH_CLASS_MARKER              'J'
#define P_DIATONIC_INTERVAL_MARKER        '}'
#define P_SCALE_DEGREE_MARKER             '%'
#define P_12TONE_INTERVAL_MARKER          '{'
#define P_PITCH_REFINED_CONTOUR_MARKER    '#'
#define P_GROSS_CONTOUR_MARKER            ':'
#define P_12TONE_PITCH_CLASS_MARKER       'j'
#define R_DURATION_GROSS_CONTOUR_MARKER   '~'
#define R_DURATION_REFINED_CONTOUR_MARKER '^'
#define R_DURATION_MARKER                 ';'
#define R_BEAT_LEVEL_MARKER               '&'
#define R_METRIC_POSITION_MARKER          '='
#define R_METRIC_LEVEL_MARKER             '\''
#define R_METRIC_GROSS_CONTOUR_MARKER     '@'
#define R_METRIC_REFINED_CONTOUR_MARKER   '`'

#define RESTDUR -1000

#include "humdrum.h"
#include "PerlRegularExpression.h"

#ifndef OLDCPP
   #include <iostream>
   #include <fstream>
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #include <iostream.h>
   #include <fstream.h>
   #ifdef VISUAL
      #include <strstrea.h>     /* for windows 95 */
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif


// includes needed for file/directory processing:
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

class ISTN {
   public:
      char *istn;
      char *filename;
      ISTN(void) { istn = NULL; filename = NULL; }
      ISTN(ISTN& anISTN) {
         int len;
         if (anISTN.istn == NULL) {
            istn = NULL;
         } else {
            len = strlen(anISTN.istn);
            istn = new char[len+1];
            strcpy(istn, anISTN.istn);
         }
         if (anISTN.filename == NULL) {
            filename = NULL;
         } else {
            len = strlen(anISTN.filename);
            filename = new char[len+1];
            strcpy(filename, anISTN.filename);
         }
      }

      void print(void) {
         if (istn != NULL) {
            cout << istn;
         } else {
            cout << ".";
         }
         cout << "\t";
         if (filename != NULL) {
            cout << filename;
         } else {
            cout << ".";
         }
         cout << "\n";
      }

      int is_valid(void) {
         if (filename == NULL) return 0;
         if (istn == NULL) return 0;
         return 1;
      }

      void setFilename(const char* string) {
         if (filename != NULL) {
            delete [] filename;
         }
         filename = new char[strlen(string)+1];
         strcpy(filename, string);
      }

      void setIstn(const char* string) {
         if (istn != NULL) {
            delete [] istn;
         }
         istn = new char[strlen(string)+1];
         strcpy(istn, string);
      }

      ISTN& operator=(ISTN& anISTN) {
         if (this == &anISTN) return *this;
         clear();
         int len;
         if (anISTN.istn == NULL) {
            istn = NULL;
         } else {
            len = strlen(anISTN.istn);
            istn = new char[len+1];
            strcpy(istn, anISTN.istn);
         }
         if (anISTN.filename == NULL) {
            filename = NULL;
         } else {
            len = strlen(anISTN.filename);
            filename = new char[len+1];
            strcpy(filename, anISTN.filename);
         }
         return *this;
      }
     ~ISTN() { clear(); }
      void clear(void) { 
         if (istn != NULL) delete [] istn;
         if (filename != NULL) delete [] filename;
         istn = NULL;
         filename = NULL;
      }
};
   

// function declarations:
void      checkOptions           (Options& opts, int argc, char** argv);
void      example                (void);
void      usage                  (const char* command);
void      createIndex            (HumdrumFile& infile, const char* filename);
void      createIndexEnding      (HumdrumFile& infile, int track, int layer);
void      extractPitchSequence   (Array<int>& pitches, HumdrumFile& infile,
                                  int track, int layer);
void      extractDurationSequence(Array<double>& durations, HumdrumFile& infile,
                                  int track, int layer);
void      extractMetricSequence  (Array<double>& metriclevels, 
                                  Array<RationalNumber>& metricpositions,
                                  HumdrumFile& infile, int track, int layer);
void      getKey                 (HumdrumFile& infile, int& mode, int& tonic);
void      printKey               (int mode, int tonic);
void      printMeter             (HumdrumFile& infile);
int       getMaxLayer            (HumdrumFile& infile, int track);

// pitch sequence printing:
void      printPitch             (Array<int>& pitches);
void      printGrossContour      (Array<int>& pitches);
void      printRefinedContour    (Array<int>& pitches);
void      print12toneInterval    (Array<int>& pitches);
void      print12tonePitch       (Array<int>& pitches);
void      printScaleDegree       (Array<int>& pitches, int tonic);
void      printMusicalInterval   (Array<int>& pitches);

// rhythm sequence printing:
void      printGrossContourRhythm (Array<double>& durations);
void      printRefinedContourRhythm(Array<double>& durations);
void      printMetricLevel        (Array<double>& levels);
void      printMetricRefinedContour     (Array<double>& levels);
void      printMetricGrossContour (Array<double>& levels);
void      printBeatLevel          (Array<double>& levels);
void      printDuration           (Array<double>& levels);
void      printMetricPosition     (Array<RationalNumber>& positions);

void      extractFeatureSet      (const char* features);
int       is_directory           (const char* path);
int       is_file                (const char* path);
void      processArgument        (const char* path);
void      fillIstnDatabase       (Array<ISTN>& istndatabase, 
                                  const char* istnfile);
const char* getIstn              (const char* filename);
int       bibsort                (const void* a, const void* b);
void      processBibRecords      (ostream& out, HumdrumFile &infile,
                                  const char* bibfilter);
void      printInstrument        (HumdrumFile& infile, int track);
char      identifyLongMarker     (HumdrumFile& infile);
void      printSpineNoteInfo(HumdrumFile& infile, int track, int subtrack);
char*     getOriginalFileName    (char* buffer, HumdrumFile& infile, 
                                  const char* filename);


// User interface variables:
Options     options;
int         debugQ   = 0;      // used with --debug option
int         polyQ    = 0;      // used with --moly option
int         monoQ    = 0;      // used with --mono option
int         poly2Q   = 0;      // used with --poly2 option
int         dirQ     = 1;      // used with -D option
int         endQ     = 0;      // used with --end option
int         graceQ   = 1;      // used with -G option
int         quietQ   = 0;      // used with -q option
int         restQ    = 0;      // used with --rest option
int         phraseQ  = 0;      // used with --phrase option
int         fermataQ = 0;      // used with --fermata option
int         rhythmQ  = 1;      // used with -r option
int         verboseQ = 0;      // used with --verbose option
int         pitchQ   = 1;      // used with -p option
int         extraQ   = 1;      // used with -E option
int         limitQ   = 0;      // used with -l option
int         limit    = 20;     // used with -l option
int         istnQ    = 0;      // used with --istn option
int         bibQ     = 0;      // used with -b option
int         fileQ    = 0;      // used with --file option
const char* Filename = "";     // used with --file option
char        FileBuffer[1024] = {0}; // used with !!original-filename:
int         instrumentQ = 0;   // used with -i option
int         dirprefixQ = 0;    // used with -d option
Array<char> dirprefix;         // used with -d option

const char* bibfilter = "";    // used with -B option
const char* istnfile= "";      // used with --istn option
Array<ISTN> istndatabase;      // used with --istn option

#define PSTATESIZE 128
int pstate[PSTATESIZE] = {0};   // true for printing of particular feature
#define pHumdrumFormat          0
//pitch printing states:
#define pPitch                  1
#define pGrossContour           2
#define pRefinedContour         3
#define p12toneInterval         4
#define p12tonePitch            5
#define pScaleDegree            6
#define pMusicalInterval        7
// rhythm printing states:
#define pDurationGrossContour   8
#define pDurationRefinedContour 9
#define pDuration              10
#define pBeat                  11
#define pMetricLevel           12
#define pMetricRefinedContour  13
#define pMetricGrossContour    14
#define pMetricPosition        15


//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   checkOptions(options, argc, argv); // process the command-line options

   int i;
   int numinputs = options.getArgCount();
   HumdrumFile infile;

   // use --verbose to print default settings.
   if (!quietQ) {
      if (!graceQ) {
         cout << "#NOGRACE" << endl;
      } else {
         if (verboseQ) {
            cout << "#GRACE" << endl;
         }
      }
      if (restQ) {
         cout << "#REST" << endl;
      } else {
         if (verboseQ) {
            cout << "#NOREST" << endl;
         }
      }
      if (fermataQ) {
         cout << "#FERMATA" << endl;
      } else {
         if (verboseQ) {
            cout << "#NOFERMATA" << endl;
         }
      }
      if (phraseQ) {
         cout << "#PHRASE" << endl;
      } else {
         if (verboseQ) {
            cout << "#NOPHRASE" << endl;
         }
      }
   }


   for (i=0; i<numinputs || i==0; i++) {
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
         createIndex(infile, "");
      } else {
         processArgument(options.getArg(i+1));
      }
   }

   return 0;
}


//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// processArgument -- check if the argument is a file or a directory.
//    if a directory, then process all files/subdirectories in it.
//

void processArgument(const char* path) {
   HumdrumFile infile;
   DIR* dir = NULL;
   char* fullname;
   struct dirent* entry;
   int namelen = 0;
   int valid = 0;

   if (is_file(path)) {
      namelen = strlen(path);
      valid = 0;
      if (strcmp(&(path[namelen-4]), ".thm") == 0) {
         valid = 1;
      } else if (strcmp(&(path[namelen-4]), ".krn") == 0) {
         valid = 1;
      } else if (strcmp(&(path[namelen-4]), ".THM") == 0) {
         valid = 1;
      } else if (strcmp(&(path[namelen-4]), ".KRN") == 0) {
         valid = 1;
      }
      if (!valid) {
         return;
      }
      infile.read(path);
      createIndex(infile, path);
   } else if (is_directory(path)) {
      dir = opendir(path);
      if (dir == NULL) {
         return;
      }
      entry = readdir(dir);
      while (entry != NULL) {
         if (strncmp(entry->d_name, ".", 1) == 0) {
            entry = readdir(dir);
            continue;
         }

         fullname = new char[strlen(path) + 1 + strlen(entry->d_name) + 1];
         strcpy(fullname, path);
         strcat(fullname, "/");
         strcat(fullname, entry->d_name);
         processArgument(fullname);
         entry = readdir(dir);
      }
   }

   if (dir != NULL) {
      // can't close a NULL dir in OS X or program will crash.
      closedir(dir);
   }
}


//////////////////////////////
//
// is_file -- returns true if the string is a file.
//

int is_file(const char* path) {
   struct stat filestat;
   stat(path, &filestat);
   return S_ISREG(filestat.st_mode);
}



//////////////////////////////
//
// is_directory -- returns true if the string is a directory.
//

int is_directory(const char* path) {
   struct stat filestat;
   stat(path, &filestat);
   return S_ISDIR(filestat.st_mode);
}



//////////////////////////////
//
// getIstn --
//

const char* getIstn(const char* filename) {
   int i;
   const char* output = NULL;
   for (i=0; i<istndatabase.getSize(); i++) {
      if (strcmp(istndatabase[i].filename, filename) == 0) {
         output = istndatabase[i].istn;
         break;
      }
   }

   if (output == NULL) {
      output = filename;
   }

   return output;
}



//////////////////////////////
//
// createIndex -- The classical fixed order for the thema command
// is: [Zz] { # : % } j J M
//

void createIndex(HumdrumFile& infile, const char* filename) {
   int i;
   int maxtracks = infile.getMaxTracks();

   if (fileQ) {
      // used to spoof filename for standard input
      filename = Filename;
   } else {
      filename = getOriginalFileName(FileBuffer, infile, filename);
   }

   PerlRegularExpression pre;
   Array<char> printname;
   if (dirprefixQ) {
      // remove the directory given with the filename
      pre.search(filename, "([^\\/]*)$", "");
      printname.setSize(strlen(dirprefix.getBase()) + 
            strlen(pre.getSubmatch(1)) + 1);
      strcpy(printname.getBase(), dirprefix.getBase());
      strcat(printname.getBase(), pre.getSubmatch());
   } else if (!dirQ) {
      pre.search(filename, "([^\\/]*)$", "");
      printname.setSize(strlen(pre.getSubmatch(1)) + 1);
      strcpy(printname.getBase(), pre.getSubmatch());
   } else {
      printname.setSize(strlen(filename) + 1);
      strcpy(printname.getBase(), filename);
   }

   if (polyQ) {
      for (i=1; i<=maxtracks; i++) {
         if (strcmp("**kern", infile.getTrackExInterp(i)) != 0) {
            continue;
         }
         if (istnQ) {
            cout << getIstn(filename);
         } else {
            cout << printname;
         }
         cout << ":";
         if (instrumentQ) {
            printInstrument(infile, i);
         }
         // cout << ":" << i;
         cout << ":";
         printSpineNoteInfo(infile, i, 1);
         if (strcmp("**kern", infile.getTrackExInterp(i)) == 0) {
            createIndexEnding(infile, i, 1);
            cout << "\n";
         }
      }
   } else if (poly2Q) {
      for (i=1; i<=maxtracks; i++) {
         if (strcmp("**kern", infile.getTrackExInterp(i)) != 0) {
            continue;
         }
         if (istnQ) {
            cout << getIstn(filename);
         } else {
            cout << printname;
         }

         // print voice label
         cout << ":";
         if (instrumentQ) {
            printInstrument(infile, i);
         }

         // print spine, subspine and note offset values
         // cout << ":" << i;
         cout << ":";
         printSpineNoteInfo(infile, i, 1);

         createIndexEnding(infile, i, 1);
         cout << "\n";
         int maxlayer = getMaxLayer(infile, i);
         int j;
         for (j=2; j<=maxlayer; j++) {
            if (istnQ) {
               cout << getIstn(filename);
            } else {
               cout << printname;
            }
            cout << ":";
            if (instrumentQ) {
               printInstrument(infile, i);
            }
            //cout << ":" << i << "." << j;
            cout << ":";
            printSpineNoteInfo(infile, i, j);

            createIndexEnding(infile, i, j);
            cout << "\n";
         }
      }
   } else if (monoQ) {
      if (istnQ) {
         cout << getIstn(filename);
      } else {
         cout << printname;
      }
      for (i=1; i<=maxtracks; i++) {
         if (strcmp("**kern", infile.getTrackExInterp(i)) == 0) {
            createIndexEnding(infile, i, 1);
            cout << "\n";
            break;
	 }
      }
   } else {
      cerr << "Strange error: no extraction model" << endl;
      exit(1);
   }
}



//////////////////////////////
//
// getOriginalFileName --
//

char* getOriginalFileName(char* buffer, HumdrumFile& infile, 
      const char* filename) {
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isGlobalComment()) {
         continue;
      }
      if (strncmp(infile[i][0], "!!original-filename:", 20) == 0) {
         PerlRegularExpression pre;
         pre.search(infile[i][0], "!!original-filename:\\s*(.*?)\\s*$");
         strcpy(buffer, pre.getSubmatch(1));
         return buffer;
      }
   }
   strcpy(buffer, filename);
   return buffer;
}



//////////////////////////////
//
// printSpineNoteInfo -- print the track, subtrack and note offset.
//    Note offsets are for indexes of extractions of pieces which
//    will need to be aligned to the original data later on.
//       !noff:1.2;23
//    before any data in a (sub)spine would indicate the revised data.
//    In this case, track = 1, subtrack = 2, note offset = 23
//    Or using default track/subtrack found in data:
//       !noff:23
//    Would mean for that track/subtrack, the note offset value is 23.
//

void printSpineNoteInfo(HumdrumFile& infile, int track, int subtrack) {
   int i, j;
   int t, st;

   int newt = -1;
   int newst = -1;
   int newoffset = -1;
   
   PerlRegularExpression pre;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isData()) {
         break;
      }
      if (!infile[i].isLocalComment()) {
         continue;
      }
      st = 0;
      for (j=0; j<infile[i].getFieldCount(); j++) {
         t = infile[i].getPrimaryTrack(j);
         if (t != track) {
            continue;
         }
         st++;
         if (st != subtrack) {
            continue;
         }
         if (pre.search(infile[i][j], "^\\!noff:(\\d+)\\.?(\\d+)?;(\\d+)")) {
            newt      = atoi(pre.getSubmatch(1));
            newst     = atoi(pre.getSubmatch(2));
            newoffset = atoi(pre.getSubmatch(3));
         } else if (pre.search(infile[i][j], "^\\!noff:(\\d+)")) {
            newoffset = atoi(pre.getSubmatch(1));
         }
      }
   }

   // print the track number
   if (newt >= 0) {
      cout << newt;
   } else {
      cout << track;
   }

   if (newst > 1) {
      cout << newt;
   } else if (subtrack > 1) {
      cout << "." << subtrack;
   }

   if (newoffset > 0) {
      cout << ';' << newoffset;
   }
}



//////////////////////////////
//
// getMaxLayer -- return the maximum number of subspines for the
// given spine on any line.
//

int getMaxLayer(HumdrumFile& infile, int track) {
   int i, j;
   int maxlayer = 0;
   int linelayer = 0;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isData()) {
         continue;
      } 
      linelayer = 0;
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (infile[i].getPrimaryTrack(j) == track) {
            linelayer++;
         }
      }
      if (linelayer > maxlayer) {
         maxlayer = linelayer;
      }
   }
   return maxlayer;
}



///////////////////////////////
//
// printInstrument --  print the instrument name for the track
//    which is stored in a *I" record before the first data line
//    int that track.
//

void printInstrument(HumdrumFile& infile, int track) {
   PerlRegularExpression pre;
   int i, j;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isData()) {
         return;
      }
      if (!infile[i].isInterpretation()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (track != infile[i].getPrimaryTrack(j)) {
            continue;
         }
         if (pre.search(infile[i][j], "^\\*I\"(.*)$", "")) {
            Array<char> iname;
            iname.setSize(strlen(pre.getSubmatch(1)) + 1);
            strcpy(iname.getBase(), pre.getSubmatch());
            pre.sar(iname, ":", "", "g");
            cout << iname;
            return;
         }
      }

   }
}



//////////////////////////////
//
// createIndexEnding -- The classical fixed order for the thema command
// is: [Zz] { # : % } j J M
//
// Returns the maximum number of layers occuring at any point in the file.
//

void createIndexEnding(HumdrumFile& infile, int track, int layer) {
   Array<int>    pitches;
   Array<double> durations;
   Array<double> metriclevels;
   Array<RationalNumber> metricpositions;
   extractPitchSequence(pitches, infile, track, layer);

   int mode = 0;
   int tonic = 2;
   getKey(infile, mode, tonic);

   if (extraQ) {
      cout << '\t';	printKey(mode, tonic);
   }

   if (pstate[p12toneInterval]) {
      cout << '\t';	print12toneInterval(pitches);
   }

   if (pstate[pRefinedContour]) {
      cout << '\t';	printRefinedContour(pitches);
   }

   if (pstate[pGrossContour]) {
      cout << '\t';	printGrossContour(pitches);
   } 

   if (pstate[pScaleDegree]) {
      cout << '\t';	printScaleDegree(pitches, tonic);
   }

   if (pstate[pMusicalInterval]) {
      cout << '\t';	printMusicalInterval(pitches);
   }

   if (pstate[p12tonePitch]) {
      cout << '\t';	print12tonePitch(pitches);
   }

   if (pstate[pPitch]) {
      cout << '\t';	printPitch(pitches);
   }

   if (extraQ) {
      cout << '\t';	printMeter(infile);
   }

   if (rhythmQ) {
      extractDurationSequence(durations, infile, track, layer);
      extractMetricSequence(metriclevels, metricpositions, infile, track, layer);
   }

   if (pstate[pDurationGrossContour]) {
      cout << '\t';	printGrossContourRhythm(durations);
   }

   if (pstate[pDurationRefinedContour]) {
      cout << '\t';	printRefinedContourRhythm(durations);
   }

   if (pstate[pDuration]) {
      cout << '\t';	printDuration(durations);
   }
   
   if (pstate[pBeat]) {
      cout << '\t';	printBeatLevel(metriclevels);
   }
   
   if (pstate[pMetricLevel]) {
      cout << '\t';	printMetricLevel(metriclevels);
   }
   
   if (pstate[pMetricRefinedContour]) {
      cout << '\t';	printMetricRefinedContour(metriclevels);
   }
   
   if (pstate[pMetricGrossContour]) {
      cout << '\t';	printMetricGrossContour(metriclevels);
   }

   if (pstate[pMetricPosition]) {
      cout << '\t';	printMetricPosition(metricpositions);
   }

   if (bibQ) {
      processBibRecords(cout, infile, bibfilter);
   }
}



//////////////////////////////
//
// processBibRecords -- print bibliographic records sorted into
//    alphabetical order
//

void processBibRecords(ostream& out, HumdrumFile &infile, 
      const char* bibfilter) {

   Array<HumdrumRecord*> bibs;
   bibs.setSize(infile.getNumLines());
   bibs.setSize(0);
   int i, j;
   PerlRegularExpression pre;

   Array<Array<char> > bfilt;
   bfilt.setSize(100);
   bfilt.setGrowth(10000);
   bfilt.setSize(0);
   if (strcmp(bibfilter, "") != 0) {
      PerlRegularExpression::getTokens(bfilt, "[:,\\s]+", bibfilter);
   }

   int valid;
   char buffer[1024] = {0};
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isBibliographic()) {
         if (bfilt.getSize() > 0) {
            valid = 0;
            infile[i].getBibKey(buffer, 1000);
            for (j=0; j<bfilt.getSize(); j++) {
               if (pre.search(buffer, bfilt[j].getBase(), "")) {
                  valid = 1;
                  break;
               }
            }
            if (valid == 0) {
               continue;
            }
         }
         bibs.increase(1);
         bibs.last() = &(infile[i]);
      }
   }

   qsort(bibs.getBase(), bibs.getSize(), sizeof(void*), bibsort);

   Array<char> record;
   for (i=0; i<bibs.getSize(); i++) {
      record.setSize(strlen((*(bibs[i]))[0]) + 1);
      strcpy(record.getBase(), (*(bibs[i]))[0]);
      pre.sar(record, "\\t", " ", "g");
      pre.sar(record, "\\s\\s+", " ", "g");
      out << '\t' << record.getBase();
   }
}



//////////////////////////////
//
// bibsort -- for sorting the tracks
//

int bibsort(const void* a, const void* b) {
   HumdrumRecord& abib = **((HumdrumRecord**)a);
   HumdrumRecord& bbib = **((HumdrumRecord**)b);
   return strcmp(abib[0], bbib[0]);
}



//////////////////////////////
//
// printGrossContourRhythm --
//

void printGrossContourRhythm(Array<double>& durations) {
   int i;
   cout << R_DURATION_GROSS_CONTOUR_MARKER;
   for (i=1; i<durations.getSize(); i++) {
      if (durations[i-1] < 0.0) {
         cout << "R";
         continue;
      }
      if (durations[i] < 0.0) {
         // ignore rest, will be printed in next loop.
         continue;
      }
      if (durations[i] - durations[i-1] > 0) {
         cout << '>';
      } else if (durations[i] - durations[i-1] < 0) {
         cout << '<';
      } else {
         // what is this line?
         cout << R_METRIC_POSITION_MARKER;
      }
   }
}



//////////////////////////////
//
// printRefinedContourRhythm --
//

void printRefinedContourRhythm(Array<double>& durations) {
   int i;
   cout << R_DURATION_REFINED_CONTOUR_MARKER;
   double value;
   for (i=1; i<durations.getSize(); i++) {
      if (durations[i-1] < 0.0) {
         cout << "R";
         continue;
      }
      if (durations[i] < 0.0) {
         // ignore rest, will be printed in next loop.
         continue;
      }
      if (durations[i-1] == 0.0) {
         if (durations[i] == 0.0) {
            cout << "=";
         } else {
            cout << "]";
         }
      } else {
         value = durations[i]/durations[i-1];
         if (value > 2.0)       { cout << "]"; }
         else if (value > 1.0)  { cout << ">"; }
         else if (value == 1.0) { cout << "="; }
         else if (value >= 0.5) { cout << "<"; }
         else if (value < 0.5)  { cout << "["; }
         else                   { cout << "X"; }
      }
   }
}

/* Old definition (a ratio between adjacent notes)

void printRefinedContourRhythm(Array<double>& durations) {
   int i;
   cout << "^";
   double value;
   int ivalue;
   for (i=1; i<durations.getSize(); i++) {
      if (durations[i-1] != 0.0) {
         value = durations[i]/durations[i-1];
         ivalue = (int)(value * 1000.0 + 0.5);
      } else {
         ivalue = 100000;
      }
      cout << ivalue << " ";
   }
}

*/


//////////////////////////////
//
// printDuration -- [2011/04/02] change code for Longa from 00 to L.
//     and Breve from 0 to B so that grace notes do not interact with
//     breve duration.
//

void printDuration(Array<double>& durations) {
   int i;
   int j;
   int k;
   int len;
   char buffer[128] = {0};
   cout << R_DURATION_MARKER;
   SSTREAM temps;
   int count;
   for (i=0; i<durations.getSize(); i++) {
      if (durations[i] < 0) {
         temps << "R ";
         continue;
      }

      if (durations[i] == 16.0) {
         strcpy(buffer, "L");
      } else if (durations[i] == 8.0) {
         strcpy(buffer, "B");
      } else if (durations[i] == 12.0) {
         strcpy(buffer, "B.");
      } else {
         Convert::durationToKernRhythm(buffer, durations[i]);
      }

      if (durations[i] > 0 && (buffer[0] == 'q')) {
         count = (int)durations[i];
         for (j=0; j<count; j++) {
            temps << "4";
         }
         if (durations[i] - count > 0) {
            Convert::durationToKernRhythm(buffer, durations[i]-count);
         }
	 len = strlen(buffer);
	 for (k=0; k<len; k++) {
            if (buffer[k] == '.') {
               temps << "d";
            } else {
               temps << buffer[k];
            }
         }
         temps << " ";
         
      } else {
	 len = strlen(buffer);
	 for (k=0; k<len; k++) {
            if (buffer[k] == '.') {
               temps << "d";
            } else {
               temps << buffer[k];
            }
         }
         temps << " ";
      }
   }

   temps << ends;
   Array<char> temps2;
   temps2.setSize(strlen(temps.CSTRING)+1);
   strcpy(temps2.getBase(), temps.CSTRING);

   PerlRegularExpression pre;
   // disallow 3.., and 3... rhythms
   pre.sar(temps2, "3\\.\\.+", "X", "g");
   // convert unknown rhythms into X:
   pre.sar(temps2, "-2147483648", "X", "g");
   pre.sar(temps2, "444448", "X", "g"); // wholenote tied to dotted quarter

   cout << temps2.getBase() << flush;
}



//////////////////////////////
//
// printMetricLevel --
//

void printMetricLevel(Array<double>& levels) {
   int i;
   cout << R_METRIC_LEVEL_MARKER;
   double value;
   int ivalue;
   for (i=0; i<levels.getSize(); i++) {
      value = levels[i];
      if (value < -1000.0) {
         // print rest marker.
         cout << "R ";
         continue;
      }
      ivalue = (int)value;
      if (ivalue > 0) {
         cout << "p";
      } else if (ivalue < 0) {
         cout << "m";
         ivalue = -ivalue;
      }
      cout << ivalue << " ";
   }
}



//////////////////////////////
//
// printMetricPosition --
//

void printMetricPosition(Array<RationalNumber>& positions) {
   int i;
   cout << R_METRIC_POSITION_MARKER;
   RationalNumber value;
   for (i=0; i<positions.getSize(); i++) {
      if (positions[i] < -1000) {
         cout << "R ";
         continue;
      }
      value = positions[i];
      cout << "x";
      value.printTwoPart(cout, "_");
      cout << ' ';
   }
}



//////////////////////////////
//
// printMetricRefinedContour --
//

void printMetricRefinedContour(Array<double>& levels) {
   int i;
   cout << R_METRIC_REFINED_CONTOUR_MARKER;
   double value;
   int ivalue;
   double bvalue;
   int bivalue;
   int zvalue;
   for (i=1; i<levels.getSize(); i++) {
      if (levels[i-1]  < -1000.0) {
         // print a rest marker
         cout << "R";
         continue;
      }
      if (levels[i] < -1000.0) {
         // ignore, rest printed in next loop
         continue;
      }
      value = levels[i];
      ivalue = (int)value;
      bvalue = levels[i-1];
      bivalue = (int)bvalue;
      zvalue = ivalue - bivalue;
      if (zvalue > 1) {
         cout << "U";
      } else if (zvalue == 1) {
         cout << "u";
      } else if (zvalue == 0) {
         cout << "S";
      } else if (zvalue == -1) {
         cout << "d";
      } else if (zvalue < -1) {
         cout << "D";
      } else {
         cout << "x";
      }
   }
}



//////////////////////////////
//
// printMetricGrossContour --
//

void printMetricGrossContour(Array<double>& levels) {
   int i;
   cout << R_METRIC_GROSS_CONTOUR_MARKER;
   double value;
   int ivalue;
   double bvalue;
   int bivalue;
   int zvalue;
   for (i=1; i<levels.getSize(); i++) {
      if (levels[i-1]  < -1000.0) {
         // print a rest marker
         cout << "R";
         continue;
      }
      if (levels[i] < -1000.0) {
         // ignore, rest printed in next loop
         continue;
      }
      value = levels[i];
      ivalue = (int)value;
      bvalue = levels[i-1];
      bivalue = (int)bvalue;
      zvalue = ivalue - bivalue;
      if (zvalue > 0) {
         cout << "U";
      } else if (zvalue < 0) {
         cout << "D";
      } else {
         cout << "S";
      }
   }
}



//////////////////////////////
//
// printBeatLevel --
//

void printBeatLevel(Array<double>& levels) {
   int i;
   cout << R_BEAT_LEVEL_MARKER;
   double value;
   int ivalue;
   for (i=0; i<levels.getSize(); i++) {
      if (levels[i] < -1000.0) {
         // print rest
         cout << "R";
         continue;
      }
      value = levels[i];
      ivalue = (int)value;
      if (ivalue >= 0) {
         cout << 1;
      } else {
         cout << 0;
      }
   }
}




//////////////////////////////
//
// printMusicalInterval --
//

void printMusicalInterval(Array<int>& pitches) {
   cout << P_DIATONIC_INTERVAL_MARKER;
   int octave;
   int interval;
   int degree;
   int direction;
   int i;
   for (i=1; i<pitches.getSize(); i++) {
      if (pitches[i-1] < 0) {
         // print a rest
         cout << "R";
         continue;
      }
      if (pitches[i] < 0) {
         // skip, will be printed in next loop
         continue;
      }
      interval = pitches[i] - pitches[i-1];
      if (interval < 0) {
         direction = -1;
         interval = -interval;
      } else {
         direction = +1;
      }
      octave   = interval / 40;
      degree   = (Convert::base40ToDiatonic(interval+2)%7)+1 + octave * 7;

      // need the direction for augmented/diminished unisons...
      //if (degree != 1) {
      //   if (direction < 0) {
      //      cout << 'x';
      //   } else {
      //      cout << 'X';
      //   }
      //}
      if (direction < 0) {
         cout << 'x';
      } else if (interval != 0) {
         cout << 'X';
      }

      int accidental = Convert::base40ToAccidental(interval+2);
      switch ((degree-1) % 7) {
         case 0:   // 1st
            switch (direction * abs(accidental)) {
               case -2:  cout << "dd"; break;
               case -1:  cout << "d";  break;
               case  0:  cout << "P";  break;
               case +1:  cout << "A";  break;
               case +2:  cout << "AA"; break;
            }
            break;
         case 3:   // 4th
         case 4:   // 5th
            switch (accidental) {
               case -2:  cout << "dd"; break;
               case -1:  cout << "d";  break;
               case  0:  cout << "P";  break;
               case +1:  cout << "A";  break;
               case +2:  cout << "AA"; break;
            }
            break;
         case 1:   // 2nd
         case 2:   // 3rd
         case 5:   // 6th
         case 6:   // 7th
            switch (accidental) {
               case -3:  cout << "dd"; break;
               case -2:  cout << "d";  break;
               case -1:  cout << "m";  break;
               case  0:  cout << "M";  break;
               case +1:  cout << "A";  break;
               case +2:  cout << "AA"; break;
            }
      }
      cout << degree;
   }
}



//////////////////////////////
//
// printScaleDegree --
//

void printScaleDegree(Array<int>& pitches, int tonic) {
   cout << P_SCALE_DEGREE_MARKER;
   int i;
   for (i=0; i<pitches.getSize(); i++) {
      if (pitches[i] < 0) {
         cout << "R";
         continue;
      }
      cout << (Convert::base40ToDiatonic(pitches[i]-tonic+2+40)%7)+1;
   }
}



//////////////////////////////
//
// printMeter -- 
//

void printMeter(HumdrumFile& infile) {
   int i;
   int top;
   int bottom;
   int count = 0;
   cout << "M";
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isInterpretation()) {
         if (infile[i][0][1] != 'M') {
            continue;
         }
         if (!isdigit(infile[i][0][2])) {
            if (strcmp("*MX", infile[i][0]) == 0) {
               cout << "irregular";
               return;
            }
            continue;
         }
         count = sscanf(infile[i][0], "*M%d/%d", &top, &bottom);
         if (count != 2) {
            continue;
         }
         cout << &(infile[i][0][2]);
         switch (top) {
            case 4:
            case 12:
               cout << "quadruple";
               break;
            case 3:
            case 9:
               cout << "triple";
               break;
            case 2:
            case 6:    
               cout << "duple";       
               break;
            default:   cout << "irregular";
         }
         switch (top) {
            case 6:
            case 9:
            case 12:
            case 16:
               cout << "compound";
               break;
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
               cout << "simple";
               break;
         }
         break;
      }
   }
}



//////////////////////////////
//
// printKey --
//

void printKey(int mode, int tonic) {
   char buffer[128] = {0};

   if (tonic < 0) {
      // unknown key
      cout << "ZX=";
      return;
   }

   if (mode) {
      cout << 'z';   // minor
   } else {
      cout << 'Z';   // major
   }

   cout << Convert::base40ToKern(buffer, tonic + 3*40);
   cout << '=';
}



//////////////////////////////
//
// getKey --
//

void getKey(HumdrumFile& infile, int& mode, int& tonic) {

   tonic = 2;  // C
   mode  = 0;  // major

   int i;
   int length;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isInterpretation()) {
         length = strlen(infile[i][0]);
         if (length < 2 || length > 5) {
            continue;
         }
         if (infile[i][0][length-1] != ':') {
            continue;
         }
         if (!isalpha(infile[i][0][1])) {
            continue;
         }
         if (islower(infile[i][0][1])) {
            mode = 1;  // minor
         } else {
            mode = 0;  // major
         }
         tonic = Convert::kernToBase40(&(infile[i][0][1]));
         if (tonic >= 0) {
            tonic = tonic % 40;
         }
         if (infile[i][0][1] == 'x' || infile[i][0][1] == 'X') {
            tonic = -1;   // unknown key
         }
         break;

      }
   }
}



//////////////////////////////
//
// print12tonePitch --
//

void print12tonePitch(Array<int>& pitches) {
   cout << P_12TONE_PITCH_CLASS_MARKER;
   int i;
   int midi;
   for (i=0; i<pitches.getSize(); i++) {
      if (pitches[i] < 0) {
         cout << "R";
         continue;
      }
      midi = Convert::base40ToMidiNoteNumber(pitches[i]) % 12;
      if (midi < 10) {
         cout << midi;
      } else if (midi == 10) {
         cout << 'A';
      } else if (midi == 11) {
         cout << 'B';
      } else {
         cout << 'X';
      }
   }
}



//////////////////////////////
//
// print12toneInterval --
//

void print12toneInterval(Array<int>& pitches) {
   Array<int> midi(pitches.getSize());
   cout << P_12TONE_INTERVAL_MARKER;
   int i;
   for (i=0; i<pitches.getSize(); i++) {
      if (pitches[i] < 0) {
         midi[i] = -1000000;
      } else {
         midi[i] = Convert::base40ToMidiNoteNumber(pitches[i]);
      }
   }

   for (i=1; i<midi.getSize(); i++) {
      if (midi[i-1] < -1000) {
         // print a rest marker
         cout << "R";
         continue;
      }
      if (midi[i] < -1000) {
         // ignore rest, printed on next round
         continue;
      }
      if (midi[i] > midi[i-1]) {
         cout << 'p' << midi[i] - midi[i-1];
      } else if (midi[i] < midi[i-1]) {
         cout << 'm' << midi[i-1] - midi[i];
      } else {
         cout << "p0";
      }
   }
}



//////////////////////////////
//
// printRefinedContour -- augmented second is assigned to be a step
//

void printRefinedContour(Array<int>& pitches) {
   int i;
   cout << P_PITCH_REFINED_CONTOUR_MARKER;

   for (i=1; i<pitches.getSize(); i++) {
      if (pitches[i-1] < 0) {
         // process a rest
         cout << "R";
         continue;
      }
      if (pitches[i] < 0) {
         continue;
      }
      if (pitches[i] < pitches[i-1]) {
         if (pitches[i-1] - pitches[i] < 9) {
            cout << 'd';
         } else {
            cout << 'D';
         }
      } else if (pitches[i] > pitches[i-1]) {
         if (pitches[i] - pitches[i-1] < 9) {
            cout << 'u';
         } else {
            cout << 'U';
         }
      } else {
         cout << 's';
      }
   }
}



//////////////////////////////
//
// printGrossContour --
//

void printGrossContour(Array<int>& pitches) {
   int i;
   cout << P_GROSS_CONTOUR_MARKER;

   for (i=1; i<pitches.getSize(); i++) {
      if (pitches[i-1] < 0) {
         // process a rest
         cout << "R";
         continue;
      }
      if (pitches[i] < 0) {
         continue;
      }
      if (pitches[i] < pitches[i-1]) {
         cout << 'D';
      } else if (pitches[i] > pitches[i-1]) {
         cout << 'U';
      } else {
         cout << 'S';
      }
   }
}



//////////////////////////////
//
// printPitch --
//

void printPitch(Array<int>& pitches) {
   int i;
   int j;
   char buffer[128] = {0};
   cout << P_PITCH_CLASS_MARKER;
   for (i=0; i<pitches.getSize(); i++) {
      if (pitches[i] < 0) {
         // process a rest marker
         cout << "R ";
         continue;
      }
      Convert::base40ToKern(buffer, (pitches[i] % 40) + 3 * 40);
      j = 0;
      while (buffer[j] != '\0') {
         if (buffer[j] == '-') {
            cout << 'b';
         } else {
            cout << buffer[j];
         }
         j++;
      }
      cout << " ";
      // when not printing a terminal " ":
      //if (i < pitches.getSize()-1) {
      //   cout << " ";
      //}
   }
}



//////////////////////////////
//
// extractMetricSequence --
//

void extractMetricSequence(Array<double>& metriclevels, 
      Array<RationalNumber>& metricpositions, HumdrumFile& infile, 
      int track, int layer) {
   PerlRegularExpression pre;
   Array<int> metlev;
   infile.analyzeMetricLevel(metlev);
   infile.analyzeRhythm();  // should already be done

   int i, j;
   metriclevels.setSize(metlev.getSize());

   metriclevels.setSize(1000);
   metriclevels.setSize(0);
   metriclevels.allowGrowth();

   metricpositions.setSize(1000);
   metricpositions.setSize(0);
   metricpositions.allowGrowth();

   RationalNumber aposition;
   RationalNumber bignegative(-1000000,1);

   int lastlayercount = 0;
   int layercount = 0;

   int pitch = 0;
   double level;
   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
         case E_humrec_data:
            lastlayercount = layercount;
            layercount = 0;
            for (j=0; j<infile[i].getFieldCount(); j++) {
               if (infile[i].getPrimaryTrack(j) != track) {
                  continue;
               }
               layercount++;
               if (layer != layercount) {
                  continue;
               }
               if (strcmp(infile[i][j], ".") == 0) {
                  // ignore null tokens
                  break;
               }

               if ((lastlayercount != 0) && (lastlayercount < layer)) {
                  // insert segmentation marker into the data
                  // since this desired layer does not continue
                  // directly from the last occurance of the layer 
                  if (metricpositions.last() != bignegative) {
                     // only append segmentation marker if a 
                     // segmentation marker is not present in the
                     // pitch sequence already.
                     level = -1000000.0;
                     metriclevels.append(level);
                     metricpositions.append(bignegative);
                  }
               }

               // all notes in a chord should have the same duration
               // so not bothering with adjusting for the --end option.

               if ((!graceQ) && pre.search(infile[i][j], "q", "i")) {
                  // don't count grace notes if not wanted
                  continue;
               }
               if (strchr(infile[i][j], '_') != NULL) {
                  // ignore continuing ties
                  break;
               }
               if (strchr(infile[i][j], ']') != NULL) {
                  // ignore ending ties
                  break;
               }
               if (strchr(infile[i][j], 'r') != NULL) {
                  if (pitch < 0) {
                     // don't repeat segmentation markers
                     break;
                  }
                  if (phraseQ && pre.search(infile[i][j], "}", "")) {
                     pitch = -1;
                     level = -1000000.0;
                     metriclevels.append(level);
                     metricpositions.append(bignegative);
                     break;
                  }
                  if (!restQ) {
                     // ignore rests
                     break;
                  }
                  pitch = -1;
                  level = -1000000.0;
                  metriclevels.append(level);
                  metricpositions.append(bignegative);
                  break;
               }
               pitch = 0;  // used to keep track of rests
               level = -(double)metlev[i];
               aposition = infile[i].getBeatR();
               metriclevels.append(level);
               metricpositions.append(aposition);
               if (fermataQ && pre.search(infile[i][j], "^[^\\s]*;", "")) {
                  pitch = -1;
                  level = -1000000.0;
                  metriclevels.append(level);
                  metricpositions.append(bignegative);
               } else if (phraseQ && pre.search(infile[i][j], "}", "")) {
                  pitch = -1;
                  level = -1000000.0;
                  metriclevels.append(level);
                  metricpositions.append(bignegative);
               }
               break;
            }
            break;
      }
   }

}



//////////////////////////////
//
// extractDurationSequence -- Disallow 3... rhythms from being stored
//    (equivalent to dotted half tied to eighth note?)
//    Translate special "l" markers for longs into Long durations.
//

void extractDurationSequence(Array<double>& durations, HumdrumFile& infile, 
      int track, int layer) {
   PerlRegularExpression pre;
   durations.setSize(10000);
   durations.setSize(0);
   durations.allowGrowth();
   char longchar = identifyLongMarker(infile);
   double dur = 0;
   int i, j;
   int pitch = 0;

   int lastlayercount = 0;
   int layercount = 0;

   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_bibliography:
         case E_humrec_global_comment:
         case E_humrec_data_comment:
         case E_humrec_interpretation:
         case E_humrec_data_kern_measure:
            break;
         case E_humrec_data:
            lastlayercount = layercount;
            layercount = 0;
            for (j=0; j<infile[i].getFieldCount(); j++) {
               if (infile[i].getPrimaryTrack(j) != track) {
                  continue;
               }
               layercount++;
               if (layer != layercount) {
                  continue;
               }
               if (strcmp(infile[i][j], ".") == 0) {
                  // ignore null tokens
                  break;
               }

               if ((lastlayercount != 0) && (lastlayercount < layer)) {
                  // insert segmentation marker into the data
                  // since this desired layer does not continue
                  // directly from the last occurance of the layer 
                  if (durations.last() != -1.0) {
                     // only append segmentation marker if a 
                     // segmentation marker is not present in the
                     // pitch sequence already.
                     dur   = -1.0;
                     durations.append(dur);
                  }
               }

               // all notes in a chord should have the same duration
               // so not bothering with adjusting for the --end option.

               if ((!graceQ) && pre.search(infile[i][j], "q", "i")) {
                  // don't count grace notes if not wanted
                  break;
               }
               if (strchr(infile[i][j], '_') != NULL) {
                  // ignore continuing ties
                  break;
               }
               if (strchr(infile[i][j], ']') != NULL) {
                  // ignore ending ties
                  break;
               }
               if (strchr(infile[i][j], 'r') != NULL) {
                  if (pitch < 0) {
                     // ignore repeated rests
                     break;
                  }
                  if (phraseQ && pre.search(infile[i][j], "}", "")) {
                     pitch = -1;
                     dur   = -1.0;
                     durations.append(dur);
                     break;
                  }
                  if (!restQ) {
                     // ignore rests
                     break;
                  }
                  pitch = -1;
                  dur   = RESTDUR;
                  durations.append(dur);
                  break;
               }
               pitch = Convert::kernToBase40(infile[i][j]);
               if ((pitch < 0) || (pitch > 10000)) {
                  // ignore rests and other strange things
                  break;
               }
               dur = infile.getTiedDuration(i, j);
               if (longchar && (strchr(infile[i][j], longchar) != NULL)) {
                  dur = 16.0;
               }
               if ((!graceQ) && (dur <= 0.0)) {
                  // for some reason grace note was not filtered before, 
                  // so filter it now.
                  break;
               }
               durations.append(dur); 
               if (limitQ) {
                  if (durations.getSize() >= limit) {
                     return;
                  }
               }
               if (fermataQ && pre.search(infile[i][j], "^[^\\s]*;", "")) {
                  pitch = -1;
                  dur   = -1.0;
                  durations.append(dur);
               } else if (phraseQ && pre.search(infile[i][j], "}", "")) {
                  pitch = -1;
                  dur   = -1.0;
                  durations.append(dur);
               }
               break;
            }
            break;
         default:
            break;
      }
   }

}




//////////////////////////////
//
// extractPitchSequence --
// restrictions:
//   (1) **kern data expected is track being searched
//   (2) chords will be ignored, only first note in chord will be processed.
//

void extractPitchSequence(Array<int>& pitches, HumdrumFile& infile, 
      int track, int layer) {
   pitches.setSize(10000);
   pitches.setGrowth(10000);
   pitches.setSize(0);
   pitches.allowGrowth();
   int pitch = 0;
   int i, j;
   PerlRegularExpression pre;

   char notebuf[1024] = {0};
   int subtokens = 0;

   int lastlayercount = 0;
   int layercount = 0;

   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_bibliography:
         case E_humrec_global_comment:
         case E_humrec_data_comment:
         case E_humrec_interpretation:
         case E_humrec_data_kern_measure:
            break;
         case E_humrec_data:
            lastlayercount = layercount;
            layercount = 0;
            for (j=0; j<infile[i].getFieldCount(); j++) {
               if (infile[i].getPrimaryTrack(j) != track) {
                  continue;
               }
               layercount++;
               if (layer != layercount) {
                  continue;
               }
               if (strcmp(infile[i][j], ".") == 0) {
                  // ignore null tokens
                  break;
               }

               if ((lastlayercount != 0) && (lastlayercount < layer)) {
                  // insert segmentation marker into the data
                  // since this desired layer does not continue
                  // directly from the last occurance of the layer 
                  if (pitches.getSize() > 0 && pitches.last() >= 0) {
                     // only append segmentation marker if a 
                     // segmentation marker is not present in the
                     // pitch sequence already.
                     pitch = -1;
                     pitches.append(pitch);
                  }
               }

               subtokens = infile[i].getTokenCount(j);
               if (subtokens == 1) {
                  strcpy(notebuf, infile[i][j]);
               } else if (endQ) {
                  infile[i].getToken(notebuf, j, subtokens-1, 1000);
               } else {
                  infile[i].getToken(notebuf, j, 0, 1000);
               }
               
               if ((!graceQ) && pre.search(notebuf, "q", "i")) {
                  // don't count grace notes if not wanted
                  continue;
               }
               if (strchr(notebuf, '_') != NULL) {
                  // ignore continuing ties
                  break;
               }
               if (strchr(notebuf, ']') != NULL) {
                  // ignore ending ties
                  break;
               }
               if (strchr(notebuf, 'r') != NULL) {
                  if (pitches.getSize() > 0 && pitches.last() < 0) {
                     // already stored one rest, so ignore this one
                     break;
                  }
                  if (phraseQ && pre.search(notebuf, "}", "")) {
                     pitch = -1;
                     pitches.append(pitch);
                     break;
                  }
                  if (!restQ) {
                     // ignore rests
                     break;
                  }
                  pitch = -1;
                  pitches.append(pitch);
                  break;
               }
               pitch = Convert::kernToBase40(notebuf);
               if ((pitch < 0) || (pitch > 10000)) {
                  // ignore rests and other strange things
                  break;
               }
               pitches.append(pitch); 
               if (limitQ) {
                  if (pitches.getSize() >= limit) {
                     return;
                  }
               }
               if (fermataQ && pre.search(notebuf, "^[^\\s]*;", "")) {
                  pitch = -1;
                  pitches.append(pitch);
               } else if (phraseQ && pre.search(infile[i][j], "}", "")) {
                  // observe that phrase marks only occur once
                  // in a multi-stop token, so have to search
                  // the entire multi-stop token for a phrase ending mark
                  // which is usually at the end of the token.
                  pitch = -1;
                  pitches.append(pitch);
                  break;
               }
               break;
            }
            break;
         default:
            break;
      }
   }

}



//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("debug=b",        "print debug information"); 
   opts.define("poly|moly=b",    "create polyphonic"); 
   opts.define("mono=b",         "extract only the first column of data");
   opts.define("poly2=b",        "create polyphonic, all layers"); 
   opts.define("end=b",          "use last note in chord tokens"); 
   opts.define("rest|rests=b",   "encode rest boundaries"); 
   opts.define("i|instrument=b", "encode instrument names in tag"); 
   opts.define("b|bib|ref=b",    "store bibliographic records"); 
   opts.define("B|bibfilter=s",  "bibliographic record filter string"); 
   opts.define("D|no-dir=b",     "don't include directory in filename tag"); 
   opts.define("d|dir-prefix=s", "append directory to filename tag"); 
   opts.define("E|no-extra=b",   "do not print extra information");
   opts.define("G|no-grace=b",   "do not print extra information");
   opts.define("r|rhythm=b",     "extract rhythm information"); 
   //opts.define("p|pitch=b",      "extract pitch information"); 
   opts.define("pitch-only=b",    "extract pitch information"); 
   opts.define("P|not-pitch=b",  "do not extract pitch information"); 
   opts.define("a|all=b",        "extract all possible musical features");
   opts.define("H|humdrum=b",    "format output as a humdrum file");
   opts.define("q|Q|quiet=b",      "don't suppress ^# messages");
   opts.define("fermata=b",      "add R markers for fermatas in input");
   opts.define("phrase=b",       "add R markers for phrase endings in input");
   opts.define("verbose=b",      "Display all control settings");
   opts.define("f|features=s",   "extract the list of features");

   // parameter matching to themax:
   opts.define("u|duration=b",  "duration (IOI)");
   opts.define("I|MI|mi|DI|di|INT|int|pitch-musical-interval|interval=b",  
                                     "musical interval");
   opts.define("p|PCH|pch|PC|pc|pitch-class|pitch=b", "pitch class");

   opts.define("file=s",         "filename to use for standard input data");
   opts.define("t|istn|translate=s", "translation file which contains istn values");
   opts.define("l|limit=i:20",   "limit the number of extracted features");

   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("h|help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Jan 2004" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 26 Jan 2004" << endl;
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

   // poly2 is now the default behavior, use --moly or --mono to
   // change
   // poly2Q      = opts.getBoolean("poly2");
   poly2Q = 1;
   monoQ = opts.getBoolean("mono");
   polyQ       = opts.getBoolean("poly");
   if (monoQ || polyQ) {
      poly2Q = 0;
   }
   
   instrumentQ = opts.getBoolean("instrument");
   debugQ      = opts.getBoolean("debug");
   dirQ        =!opts.getBoolean("no-dir");
   endQ        = opts.getBoolean("end");
   restQ       = opts.getBoolean("rest");
   fermataQ    = opts.getBoolean("fermata");
   phraseQ     = opts.getBoolean("phrase");
   graceQ      =!opts.getBoolean("no-grace");
   quietQ      = opts.getBoolean("quiet");
   rhythmQ     = opts.getBoolean("rhythm");
   fileQ       = opts.getBoolean("file");
   if (fileQ) {
      Filename = opts.getString("file");
   }
   pitchQ  = 1;
   rhythmQ = 1;
   if (opts.getBoolean("rhythm")) {
      pitchQ = 0;     // if -r is given, then turn off -p
   }
   if (opts.getBoolean("pitch")) {
      rhythmQ = 0;     // if -p is given, then turn off -r
   }
   extraQ      = !opts.getBoolean("no-extra");
   limitQ      = opts.getBoolean("limit");
   limit       = opts.getInteger("limit");
   istnQ       = opts.getBoolean("istn");
   bibQ        = opts.getBoolean("bib");
   istnfile    = opts.getString("istn");
   dirprefixQ  = opts.getBoolean("dir-prefix");
   verboseQ    = opts.getBoolean("verbose");

   if (dirprefixQ) {
      dirprefix.setSize(strlen(opts.getString("dir-prefix")) + 1);
      strcpy(dirprefix.getBase(), opts.getString("dir-prefix"));
      if (dirprefix[dirprefix.getSize()-2] != '/') {
         dirprefix.increase(1);
         strcat(dirprefix.getBase(), "/");
      }
   } else {
      dirprefix.setSize(1);
      dirprefix[0] = '\0';
   }

   if (opts.getBoolean("bibfilter")) {
      bibfilter = opts.getString("bibfilter");
   }

   if (istnQ) {
      fillIstnDatabase(istndatabase, istnfile);
   }

   if (opts.getBoolean("all")) {
      rhythmQ = 1;
      pitchQ  = 1;
   }

   if (options.getBoolean("not-pitch")) {
      pitchQ = 0;
   }

   if (!opts.getBoolean("humdrum")) {
      pstate[pHumdrumFormat] = 1;
   }

   if (opts.getBoolean("features")) {
      pitchQ = 0;    // no need to turn off, but just in case
      rhythmQ = 0;   // no need to turn off, but just in case
      extractFeatureSet(opts.getString("features"));
   } else if (opts.getBoolean("duration")) {
      pitchQ = 0;    // no need to turn off, but just in case
      rhythmQ = 0;   // no need to turn off, but just in case
      // do stuff later
   } else if (opts.getBoolean("interval")) {
      pitchQ = 0;    // no need to turn off, but just in case
      rhythmQ = 0;   // no need to turn off, but just in case
      // do stuff later
   } else if (opts.getBoolean("pitch-class")) {
      pitchQ = 0;    // no need to turn off, but just in case
      rhythmQ = 0;   // no need to turn off, but just in case
      // do stuff later
   } else {
      // set up the printing options.
      pstate[pGrossContour]           = pitchQ;
      pstate[pRefinedContour]         = pitchQ;
      pstate[p12toneInterval]         = pitchQ;
      pstate[p12tonePitch]            = pitchQ;
      pstate[pScaleDegree]            = pitchQ;
      pstate[pMusicalInterval]        = pitchQ;
      pstate[pPitch]                  = pitchQ;
      pstate[pDurationGrossContour]   = rhythmQ;
      pstate[pDurationRefinedContour] = rhythmQ;
      pstate[pDuration]               = rhythmQ;
      pstate[pBeat]                   = rhythmQ;
      pstate[pMetricLevel]            = rhythmQ;
      pstate[pMetricRefinedContour]   = rhythmQ;
      pstate[pMetricGrossContour]     = rhythmQ;
      pstate[pMetricPosition]         = rhythmQ;
   }

   if (opts.getBoolean("duration")) {
      pstate[pDuration] = 1;
      rhythmQ = 1;
   }

   if (opts.getBoolean("interval")) {
      pstate[pMusicalInterval] = 1;
      pitchQ = 1;
   }

   if (opts.getBoolean("pitch-class")) {
      pstate[pPitch] = 1;
      pitchQ = 1;
   }

}


//////////////////////////////
//
// extractFeatureSet --
//
// Pitch Abbreviations
//    PCH, P, PC, DPC = Pitch
//    GC, PGC, CON    = GrossContour
//    RC, PRC         = RefinedContour
//    12I             = 12toneInterval
//    12P             = 12tonePitch
//    SD, S, D        = ScaleDegree
//    MI, I           = MusicalInterval
// Rhythm Abbreviations:
//    RGC, DGC        = RhythmGrossContour
//    RRC, DRC        = RhythmRefinedContour
//    DUR, IOI        = Duration
//    BLV             = BeatLevel
//    MLV             = Metric Level
//    MLI             = Metric Interval
//

void extractFeatureSet(const char* features) {
   char *buffer;
   int length = strlen(features);
   buffer = new char[length+1];
   int i;
   for (i=0; i<length; i++) {
      buffer[i] = toupper(features[i]);
   }
   buffer[length] = '\0';

   char* ptr = strtok(buffer, " :;-\n\t");
   while (ptr != NULL) {
      if (strcmp(ptr, "GC") == 0)       { pstate[pGrossContour]           = 1; }
      else if (strcmp(ptr, "PGC") == 0) { pstate[pGrossContour]           = 1; }
      else if (strcmp(ptr, "GC" ) == 0) { pstate[pGrossContour]           = 1; }
      else if (strcmp(ptr, "CON") == 0) { pstate[pGrossContour]           = 1; }
      else if (strcmp(ptr, "RC" ) == 0) { pstate[pRefinedContour]         = 1; }
      else if (strcmp(ptr, "PRC") == 0) { pstate[pRefinedContour]         = 1; }
      else if (strcmp(ptr, "12I") == 0) { pstate[p12toneInterval]         = 1; }
      else if (strcmp(ptr, "12P") == 0) { pstate[p12tonePitch]            = 1; }
      else if (strcmp(ptr, "SD" ) == 0) { pstate[pScaleDegree]            = 1; }
      else if (strcmp(ptr, "S"  ) == 0) { pstate[pScaleDegree]            = 1; }
      else if (strcmp(ptr, "D"  ) == 0) { pstate[pScaleDegree]            = 1; }
      else if (strcmp(ptr, "MI" ) == 0) { pstate[pMusicalInterval]        = 1; }
      else if (strcmp(ptr, "DI" ) == 0) { pstate[pMusicalInterval]        = 1; }
      else if (strcmp(ptr, "INT") == 0) { pstate[pMusicalInterval]        = 1; }
      else if (strcmp(ptr, "I"  ) == 0) { pstate[pMusicalInterval]        = 1; }
      else if (strcmp(ptr, "RGC") == 0) { rhythmQ = 1; pstate[pDurationGrossContour]   = 1; }
      else if (strcmp(ptr, "RRC") == 0) { rhythmQ = 1; pstate[pDurationRefinedContour] = 1; }
      else if (strcmp(ptr, "IOI") == 0) { rhythmQ = 1; pstate[pDuration]               = 1; }
      else if (strcmp(ptr, "DUR") == 0) { rhythmQ = 1; pstate[pDuration]               = 1; }
      else if (strcmp(ptr, "BLV") == 0) { rhythmQ = 1; pstate[pBeat]                   = 1; }
      else if (strcmp(ptr, "MLV") == 0) { rhythmQ = 1; pstate[pMetricLevel]            = 1; }
      else if (strcmp(ptr, "MRC") == 0) { rhythmQ = 1; pstate[pMetricRefinedContour]   = 1; }
      else if (strcmp(ptr, "MGC") == 0) { rhythmQ = 1; pstate[pMetricGrossContour]     = 1; }
      else if (strcmp(ptr, "MPS") == 0) { rhythmQ = 1; pstate[pMetricPosition]         = 1; }
      else if (strcmp(ptr, "PCH") == 0) { pstate[pPitch]                  = 1; }
      else if (strcmp(ptr, "PC" ) == 0) { pstate[pPitch]                  = 1; }
      else if (strcmp(ptr, "DPC") == 0) { pstate[pPitch]                  = 1; }
      else if (strcmp(ptr, "P"  ) == 0) { pstate[pPitch]                  = 1; }

      ptr = strtok(NULL, " :;-\n\t");
   }
}



//////////////////////////////
//
// fillIstnDatabase --
//

void fillIstnDatabase(Array<ISTN>& istndatabase, const char* istnfile) {
   HumdrumFile infile;
   infile.read(istnfile);
   int i, j;
   ISTN entry;
   istndatabase.setSize(infile.getNumLines());
   istndatabase.setSize(0);
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() != E_humrec_data) {
         continue;
      }
      entry.clear();
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strcmp(infile[i].getExInterp(j), "**theme") == 0) {
            entry.setFilename(infile[i][j]);
         } else if (strcmp(infile[i].getExInterp(j), "**istn") == 0) {
            entry.setIstn(infile[i][j]);
         }
      }
      if (entry.is_valid()) {
         istndatabase.append(entry);
      }
   }

   istndatabase.allowGrowth(0);
}



//////////////////////////////
//
// identifyLongMarker --
//
//

char identifyLongMarker(HumdrumFile& infile) {
   int i;
   PerlRegularExpression pre;
   int hasLongQ = 0;
   int longchar = 0;
   for (i=infile.getNumLines()-1; i>=0; i--) {
      if (!infile[i].isBibliographic()) {
         continue;
      }       
      if (pre.search(infile[i][0], 
            "^!!!RDF\\*\\*kern\\s*:\\s*([^\\s=])\\s*=.*long", "i")) {
         hasLongQ = 1;
         longchar = pre.getSubmatch(1)[0];
         break;
      }
   }

   return longchar;
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



// md5sum: ce8d0989f31b866712089d9ed7bfd11f tindex.cpp [20120614]

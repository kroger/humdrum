//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu> Creation Date:
// Mon Jan 26 08:53:14 PST 2004
// Last Modified: Fri Apr  9 19:02:18 PDT 2004 (rhythmic feature extraction)
// Last Modified: Mon Apr 12 14:05:52 PDT 2004 (adding more rhythm features)
// Last Modified: Fri Apr 16 18:01:03 PDT 2004 (added directory processing)
// Last Modified: Sun Aug  8 18:44:36 PDT 2004 (added more rhythm features)
// Last Modified: Thu Oct 30 10:38:19 PST 2008 (closedir fix for OSX)
// Last Modified: Wed Jul  1 16:11:07 PDT 2009 (added polyphonic extraction)
// Last Modified: Mon Aug 30 13:57:51 PDT 2010 (3... rhythm disallowed in dur)
// Last Modified: Mon Aug 30 13:57:51 PDT 2010 (-2147483648 rhythm --> X)
// Last Modified: Wed Sep  1 13:37:32 PDT 2010 (added metric position)
// Filename:      ...museinfo/examples/all/themebuilderx.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/themebuilderx.cpp
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
//  !    = duration (IOI)
//  &    = beat level
//  @    = metric gross contour
//  `    = metric refined contour
//  '    = metric level
//  =    = metric position
//

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
void      createIndex            (HumdrumFile& hfile, const char* filename);
void      createIndexEnding      (HumdrumFile& hfile, int index);
void      extractPitchSequence   (Array<int>& pitches, HumdrumFile& hfile,
                                  int track);
void      extractDurationSequence(Array<double>& durations, HumdrumFile& hfile,
                                  int track);
void      extractMetricSequence  (Array<double>& metriclevels, 
                                  Array<RationalNumber>& metricpositions,
                                  HumdrumFile& hfile, int track);
void      getKey                 (HumdrumFile& hfile, int& mode, int& tonic);
void      printKey               (int mode, int tonic);
void      printMeter             (HumdrumFile& hfile);

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


// User interface variables:
Options     options;
int         debugQ  = 0;       // used with --debug option
int         polyQ   = 0;       // used with --poly option
int         rhythmQ = 0;       // used with -r option
int         pitchQ  = 1;       // used with -p option
int         extraQ  = 1;       // used with -E option
int         limitQ  = 0;       // used with -l option
int         limit   = 20;      // used with -l option
int         istnQ   = 0;       // used with --istn option
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
   HumdrumFile hfile;

   for (i=0; i<numinputs || i==0; i++) {
      hfile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         hfile.read(cin);
         createIndex(hfile, "");
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
   HumdrumFile hfile;
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
      hfile.read(path);
      createIndex(hfile, path);
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

void createIndex(HumdrumFile& hfile, const char* filename) {

   int i;
   int maxtracks = hfile.getMaxTracks();
   if (polyQ) {
      for (i=1; i<=maxtracks; i++) {
         if (strcmp("**kern", hfile.getTrackExInterp(i)) != 0) {
            continue;
         }
         if (istnQ) {
            cout << getIstn(filename);
         } else {
            cout << filename;
         }
         cout << "::" << i;
         if (strcmp("**kern", hfile.getTrackExInterp(i)) == 0) {
            createIndexEnding(hfile, i);
            cout << "\n";
         }
      }
   } else {
      if (istnQ) {
         cout << getIstn(filename);
      } else {
         cout << filename;
      }
      for (i=1; i<=maxtracks; i++) {
         if (strcmp("**kern", hfile.getTrackExInterp(i)) == 0) {
            createIndexEnding(hfile, i);
            cout << "\n";
            break;
	 }
      }
   }
}



//////////////////////////////
//
// createIndexEnding -- The classical fixed order for the thema command
// is: [Zz] { # : % } j J M
//


void createIndexEnding(HumdrumFile& hfile, int index) {
   Array<int>    pitches;
   Array<double> durations;
   Array<double> metriclevels;
   Array<RationalNumber> metricpositions;
   extractPitchSequence(pitches, hfile, index);

   int mode = 0;
   int tonic = 2;
   getKey(hfile, mode, tonic);

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
      cout << '\t';	printMeter(hfile);
   }


   if (rhythmQ) {
      extractDurationSequence(durations, hfile, index);
      extractMetricSequence(metriclevels, metricpositions, hfile, index);
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
   }
}



//////////////////////////////
//
// printGrossContourRhythm --
//

void printGrossContourRhythm(Array<double>& durations) {
   int i;
   cout << "~";
   for (i=1; i<durations.getSize(); i++) {
      if (durations[i] - durations[i-1] > 0) {
         cout << '>';
      } else if (durations[i] - durations[i-1] < 0) {
         cout << '<';
      } else {
         cout << '=';
      }
   }
}



//////////////////////////////
//
// printRefinedContourRhythm --
//

void printRefinedContourRhythm(Array<double>& durations) {
   int i;
   cout << "^";
   double value;
   for (i=1; i<durations.getSize(); i++) {
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
// printDuration --
//

void printDuration(Array<double>& durations) {
   int i;
   int j;
   int k;
   int len;
   char buffer[128] = {0};
   cout << "!";
   SSTREAM temps;
   int count;
   for (i=0; i<durations.getSize(); i++) {
      Convert::durationToKernRhythm(buffer, durations[i]);
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

   cout << temps2.getBase() << flush;
}



//////////////////////////////
//
// printMetricLevel --
//

void printMetricLevel(Array<double>& levels) {
   int i;
   cout << "\'";
   double value;
   int ivalue;
   for (i=0; i<levels.getSize(); i++) {
      value = levels[i];
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
   cout << "=";
   RationalNumber value;
   for (i=0; i<positions.getSize(); i++) {
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
   cout << "`";
   double value;
   int ivalue;
   double bvalue;
   int bivalue;
   int zvalue;
   for (i=1; i<levels.getSize(); i++) {
      value = levels[i];
      ivalue = (int)value;
      bvalue = levels[i-1];
      bivalue = (int)bvalue;
      zvalue = ivalue - bivalue;
      if (zvalue > 1) {
         cout << "H";
      } else if (zvalue == 1) {
         cout << "h";
      } else if (zvalue == 0) {
         cout << "S";
      } else if (zvalue == -1) {
         cout << "w";
      } else if (zvalue < -1) {
         cout << "W";
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
   cout << "@";
   double value;
   int ivalue;
   double bvalue;
   int bivalue;
   int zvalue;
   for (i=1; i<levels.getSize(); i++) {
      value = levels[i];
      ivalue = (int)value;
      bvalue = levels[i-1];
      bivalue = (int)bvalue;
      zvalue = ivalue - bivalue;
      if (zvalue > 0) {
         cout << "H";
      } else if (zvalue < 0) {
         cout << "W";
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
   cout << "&";
   double value;
   int ivalue;
   for (i=0; i<levels.getSize(); i++) {
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
   cout << '}';  // print the musical interval identification marker
   int octave;
   int interval;
   int degree;
   int direction;
   int i;
   for (i=1; i<pitches.getSize(); i++) {
      interval = pitches[i] - pitches[i-1];
      if (interval < 0) {
         direction = -1;
         interval = -interval;
      } else {
         direction = +1;
      }
      octave   = interval / 40;
      degree   = Convert::base40ToDiatonic(interval+2)+1 + octave * 7;
      if (degree != 1) {
         if (direction < 0) {
            cout << 'x';
         } else {
            cout << 'X';
         }
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
   cout << '%';  // print the scale-degree identification marker
   int i;
   for (i=0; i<pitches.getSize(); i++) {
      cout << Convert::base40ToDiatonic(pitches[i]-tonic+2+40)+1;
   }
}



//////////////////////////////
//
// printMeter -- 
//

void printMeter(HumdrumFile& hfile) {
   int i;
   int top;
   int bottom;
   int count = 0;
   cout << "M";
   for (i=0; i<hfile.getNumLines(); i++) {
      if (hfile[i].isInterpretation()) {
         if (hfile[i][0][1] != 'M') {
            continue;
         }
         if (!isdigit(hfile[i][0][2])) {
            if (strcmp("*MX", hfile[i][0]) == 0) {
               cout << "irregular";
               return;
            }
            continue;
         }
         count = sscanf(hfile[i][0], "*M%d/%d", &top, &bottom);
         if (count != 2) {
            continue;
         }
         cout << &(hfile[i][0][2]);
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

void getKey(HumdrumFile& hfile, int& mode, int& tonic) {

   tonic = 2;  // C
   mode  = 0;  // major

   int i;
   int length;
   for (i=0; i<hfile.getNumLines(); i++) {
      if (hfile[i].isInterpretation()) {
         length = strlen(hfile[i][0]);
         if (length < 2 || length > 5) {
            continue;
         }
         if (hfile[i][0][length-1] != ':') {
            continue;
         }
         if (!isalpha(hfile[i][0][1])) {
            continue;
         }
         if (islower(hfile[i][0][1])) {
            mode = 1;  // minor
         } else {
            mode = 0;  // major
         }
         tonic = Convert::kernToBase40(&(hfile[i][0][1]));
         if (tonic >= 0) {
            tonic = tonic % 40;
         }
         if (hfile[i][0][1] == 'x' || hfile[i][0][1] == 'X') {
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
   cout << 'j';  // print the 12-tone pitch class identification marker
   int i;
   int midi;
   for (i=0; i<pitches.getSize(); i++) {
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
   cout << '{';  // print the refined-contour identification marker
   int i;
   for (i=0; i<pitches.getSize(); i++) {
      midi[i] = Convert::base40ToMidiNoteNumber(pitches[i]);
   }

   for (i=1; i<midi.getSize(); i++) {
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
   cout << '#';  // print the refined-contour identification marker

   for (i=1; i<pitches.getSize(); i++) {
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
   cout << ':';  // print the gross-contour identification marker

   for (i=1; i<pitches.getSize(); i++) {
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
   cout << 'J';  // print the pitch identification marker
   for (i=0; i<pitches.getSize(); i++) {
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
      Array<RationalNumber>& metricpositions, HumdrumFile& hfile, int track) {
   Array<int> metlev;
   hfile.analyzeMetricLevel(metlev);
   hfile.analyzeRhythm();  // should already be done

   int i, j;
   metriclevels.setSize(metlev.getSize());

   metriclevels.setSize(1000);
   metriclevels.setSize(0);
   metriclevels.allowGrowth();

   metricpositions.setSize(1000);
   metricpositions.setSize(0);
   metricpositions.allowGrowth();

   RationalNumber aposition;

   double level;
   for (i=0; i<hfile.getNumLines(); i++) {
      switch (hfile[i].getType()) {
         case E_humrec_data:
            for (j=0; j<hfile[i].getFieldCount(); j++) {
               if (hfile[i].getPrimaryTrack(j) != track) {
                  continue;
               }
               if (strcmp(hfile[i][j], ".") == 0) {
                  // ignore null tokens
                  break;
               }
               if (strchr(hfile[i][j], '_') != NULL) {
                  // ignore continuing ties
                  break;
               }
               if (strchr(hfile[i][j], ']') != NULL) {
                  // ignore ending ties
                  break;
               }
               if (strchr(hfile[i][j], 'r') != NULL) {
                  // ignore rests
                  break;
               }
               level = -(double)metlev[i];
               aposition = hfile[i].getBeatR();
               metriclevels.append(level);
               metricpositions.append(aposition);
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
//

void extractDurationSequence(Array<double>& durations, HumdrumFile& hfile, 
      int track) {
   durations.setSize(10000);
   durations.setSize(0);
   durations.allowGrowth();
   double dur = 0;
   int i, j;
   int pitch;

   for (i=0; i<hfile.getNumLines(); i++) {
      switch (hfile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_bibliography:
         case E_humrec_global_comment:
         case E_humrec_data_comment:
         case E_humrec_interpretation:
         case E_humrec_data_kern_measure:
            break;
         case E_humrec_data:
            for (j=0; j<hfile[i].getFieldCount(); j++) {
               if (hfile[i].getPrimaryTrack(j) != track) {
                  continue;
               }
               if (strcmp(hfile[i][j], ".") == 0) {
                  // ignore null tokens
                  break;
               }
               if (strchr(hfile[i][j], '_') != NULL) {
                  // ignore continuing ties
                  break;
               }
               if (strchr(hfile[i][j], ']') != NULL) {
                  // ignore ending ties
                  break;
               }
               if (strchr(hfile[i][j], 'r') != NULL) {
                  // ignore rests
                  break;
               }
               pitch = Convert::kernToBase40(hfile[i][j]);
               if ((pitch < 0) || (pitch > 10000)) {
                  // ignore rests and other strange things
                  break;
               }
               dur = hfile.getTiedDuration(i, j);
               durations.append(dur); 
               if (limitQ) {
                  if (durations.getSize() >= limit) {
                     return;
                  }
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

void extractPitchSequence(Array<int>& pitches, HumdrumFile& hfile, int track) {
   pitches.setSize(10000);
   pitches.setGrowth(10000);
   pitches.setSize(0);
   pitches.allowGrowth();
   int pitch = 0;
   int i, j;

   for (i=0; i<hfile.getNumLines(); i++) {
      switch (hfile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_bibliography:
         case E_humrec_global_comment:
         case E_humrec_data_comment:
         case E_humrec_interpretation:
         case E_humrec_data_kern_measure:
            break;
         case E_humrec_data:
            for (j=0; j<hfile[i].getFieldCount(); j++) {
               if (hfile[i].getPrimaryTrack(j) != track) {
                  continue;
               }
		           
               if (strcmp(hfile[i][j], ".") == 0) {
                  // ignore null tokens
                  break;
               }
               if (strchr(hfile[i][j], '_') != NULL) {
                  // ignore continuing ties
                  break;
               }
               if (strchr(hfile[i][j], ']') != NULL) {
                  // ignore ending ties
                  break;
               }
               if (strchr(hfile[i][j], 'r') != NULL) {
                  // ignore rests
                  break;
               }
               pitch = Convert::kernToBase40(hfile[i][j]);
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
   opts.define("poly=b",         "create poliphonic"); 
   opts.define("E|no-extra=b",   "do nto print extra information");
   opts.define("r|rhythm=b",     "extract rhythm information"); 
   opts.define("p|pitch=b",      "extract pitch information"); 
   opts.define("P|not-pitch=b",  "do not extract pitch information"); 
   opts.define("a|all=b",        "extract all possible musical features");
   opts.define("H|humdrum=b",    "format output as a humdrum file");
   opts.define("f|features=s",   "extract the list of features");
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
   
   debugQ      = opts.getBoolean("debug");
   polyQ       = opts.getBoolean("poly");
   rhythmQ     = opts.getBoolean("rhythm");
   pitchQ      = 1;
   extraQ      = !opts.getBoolean("no-extra");
   limitQ      = opts.getBoolean("limit");
   limit       = opts.getInteger("limit");
   istnQ       = opts.getBoolean("istn");
   istnfile    = opts.getString("istn");

   if (istnQ) {
      fillIstnDatabase(istndatabase, istnfile);
   }

   if (rhythmQ) {
      pitchQ = opts.getBoolean("pitch");
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
      extractFeatureSet(opts.getString("features"));
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
}



//////////////////////////////
//
// extractFeatureSet --
//
// Pitch Abbreviations
//    P        = Pitch
//    GC, PGC  = GrossContour
//    RC, PRC  = RefinedContour
//    12I      = 12toneInterval
//    12P      = 12tonePitch
//    SD, S, D = ScaleDegree
//    MI, I    = MusicalInterval
// Rhythm Abbreviations:
//    RGC      = RhythmGrossContour
//    RRC      = RhythmRefinedContour
//    IOI      = Duration
//    BLV      = BeatLevel
//    MLV      = Metric Level
//    MLI      = Metric Interval
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
      else if (strcmp(ptr, "RC" ) == 0) { pstate[pRefinedContour]         = 1; }
      else if (strcmp(ptr, "PRC") == 0) { pstate[pRefinedContour]         = 1; }
      else if (strcmp(ptr, "12I") == 0) { pstate[p12toneInterval]         = 1; }
      else if (strcmp(ptr, "12P") == 0) { pstate[p12tonePitch]            = 1; }
      else if (strcmp(ptr, "SD" ) == 0) { pstate[pScaleDegree]            = 1; }
      else if (strcmp(ptr, "S"  ) == 0) { pstate[pScaleDegree]            = 1; }
      else if (strcmp(ptr, "D"  ) == 0) { pstate[pScaleDegree]            = 1; }
      else if (strcmp(ptr, "MI" ) == 0) { pstate[pMusicalInterval]        = 1; }
      else if (strcmp(ptr, "I"  ) == 0) { pstate[pMusicalInterval]        = 1; }
      else if (strcmp(ptr, "RGC") == 0) { pstate[pDurationGrossContour]   = 1; }
      else if (strcmp(ptr, "RRC") == 0) { pstate[pDurationRefinedContour] = 1; }
      else if (strcmp(ptr, "IOI") == 0) { pstate[pDuration]               = 1; }
      else if (strcmp(ptr, "DUR") == 0) { pstate[pDuration]               = 1; }
      else if (strcmp(ptr, "BLV") == 0) { pstate[pBeat]                   = 1; }
      else if (strcmp(ptr, "MLV") == 0) { pstate[pMetricLevel]            = 1; }
      else if (strcmp(ptr, "MLI") == 0) { pstate[pMetricRefinedContour]   = 1; }
      else if (strcmp(ptr, "MGC") == 0) { pstate[pMetricGrossContour]     = 1; }
      else if (strcmp(ptr, "MPS") == 0) { pstate[pMetricGrossContour]     = 1; }
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



// md5sum: 54788cc8480d3e60fb4eaabb5cf07c60 themebuilderx.cpp [20100903]

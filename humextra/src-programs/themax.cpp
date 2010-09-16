//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Oct 27 10:42:08 PST 2008
// Last Modified: Fri Jun 26 11:23:35 PDT 2009 (converted from original version)
// Last Modified: Thu Jul  2 15:59:04 PDT 2009 (intial PCRE implementation)
// Last Modified: Wed Aug 25 15:55:23 PDT 2010 (made -t input case insensitive)
// Last Modified: Thu Aug 26 14:32:01 PDT 2010 (add cleaning for pitch queries)
// Last Modified: Wed Sep  1 15:43:34 PDT 2010 (added metric position)
// Last Modified: Thu Sep  2 17:40:01 PDT 2010 (added feature linking)
// Last Modified: Fri Sep  3 13:31:22 PDT 2010 (added --count feature)
// Filename:      ...museinfo/examples/all/themax.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/themax.cpp
// Syntax:        C++; museinfo
//
// Description:   searches an index created by themebuilderx.  Themax
//                is a C++ implementation of the original thema command
//                which was written by David Huron in 1996/1998, and
//                modified with a few bug fixes during the implementation 
//                of Themefinder.org by Craig Sapp (1999-2001).
//
// Classical themebuilder entry order:
//    fileid [Zz] { # : % } j J M
// Additional rhythmic marks:
//    ~ ^ ! & @ ` '
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
//
// Options:
// -a   anchor to start          (implemented)
// -M   major keys               (implemented)
// -m   minor keys               (implemented)
// -t   tonic                    (implemented)
// -T   meter                    (implemented)
//
// PITCH FEATURES: 
//
// -i { 12-tone interval         (implemented)
// -c # refined contour          (implemented)
// -C : gross contour            (implemented)
// -d % scale degree             (implemented)
// -I } musical interval         (implemented)
// -P j 12-tone pitch class      (implemented)
// -p J pitch-class name         (implemented)
//   -D diatonic pitch-class name(implemented)
//
// RHYTHM FEATURES:
//
// -R ~ duration gross contour   (implemented)
// -r ^ duration refined contour (implemented)
// -u ! duration (IOI)           (implemented)
// -b & beat level               (implemented)
// -E @ metric gross contour     (implemented)
// -e ` metric refined contour   (implemented)
// -L ' metric level             (implemented)
// -l = metric position          (implemented)
//
// Todo: Add --repeat option which allows for any repeated notes
// between pitch features.
//

#include "humdrum.h"
#include "PerlRegularExpression.h"

#ifndef OLDCPP
   #include <cstdlib>
   #include <fstream>
   #include <iostream>
   using namespace std;
#else
   #include <cstdlib.h>
   #include <fstream.h>
   #include <iostream.h>
#endif

// character markers in index file:
#define PITCH_TWELVE_TONE_INTERVAL_MARKER '{'
#define PITCH_REFINED_CONTOUR_MARKER      '#'
#define PITCH_GROSS_CONTOUR_MARKER        ':'
#define PITCH_SCALE_DEGREE_MARKER         '%'
#define PITCH_MUSICAL_INTERVAL_MARKER     '}'
#define PITCH_TWELVE_TONE_MARKER          'j'
#define PITCH_CLASS_MARKER                'J'
#define DURATION_GROSS_CONTOUR_MARKER     '~'
#define DURATION_REFINED_CONTOUR_MARKER   '^'
#define DURATION_IOI                      '!'
#define BEAT_LEVEL                        '&'
#define METRIC_GROSS_CONTOUR              '@'
#define METRIC_REFINED_CONTOUR            '`'
#define METRIC_LEVEL                      '\''
#define METRIC_POSITION                   '='


// function declarations:
void      checkOptions           (Options& opts, int argc, char** argv);
void      example                (void);
void      usage                  (const char* command);
void      appendString           (Array<char>& ss, const char* string);
void      appendToSearchString   (Array<char>& ss, const char* string, 
                                  char marker, int anchor);
void      showCleanedParameters  (void);
int       searchForMatches       (const char* filename, Array<char>& ss,
                                  PerlRegularExpression& re);
int       searchForMatches       (istream& inputfile, Array<char>& ss, 
                                  PerlRegularExpression& re);
void      prepareInterval        (Array<char>& data);
int       checkLink              (string& line);
void      getSimpleLocation      (Array<int>& positions, string& line, 
                                  Array<char>& feature, char searchanchor,
                                  Array<int>& checklocs, int featurewidth = 1);
void      getSeparatorLocation   (Array<int>& positions, string& line, 
                                  Array<char>& feature, char searchanchor,
                                  Array<int>& checklocs, char separator = ' ');
void      getMusicalIntervalLocation(Array<int>& positions, string& line, 
                                  Array<char>& feature, char searchanchor,
                                  Array<int>& checklocs);
// void      findIntersection       (Array<int>& aa, Array<int>& bb);


// user input sanitation functions:
void      cleanPpitchClass      (Array<char>& data);
void      cleanPmusicalInterval (Array<char>& data);
void      cleanPscaleDegree     (Array<char>& data);
void      cleanPgrossContour    (Array<char>& data);
void      cleanPrefinedContour  (Array<char>& data);
void      cleanP12tonePitchClass(Array<char>& data);
void      cleanP12toneInterval  (Array<char>& data);
void      cleanUpRangeSyntaxNoOutsideDigitsOrComma(Array<char>& data);
void      cleanRbeatLevel       (Array<char>& data);
void      cleanRduration        (Array<char>& data);
void      cleanRgrossContour    (Array<char>& data);
void      cleanRrefinedContour  (Array<char>& data);
void      cleanRmgc             (Array<char>& data);
void      cleanRmrc             (Array<char>& data);
void      cleanRmetricLevel     (Array<char>& data);
void      cleanRmetricPosition  (Array<char>& data);


// User interface variables:
Options   options;

int         showcleanQ   = 0;       // used with --cleaned
int         verboseQ     = 0;       // used with --verbose option
int         verbose2Q    = 0;       // used with --verbose2 option
int         cleanQ       = 1;       // used with --no-clean option
int         anchoredQ    = 0;       // used with -a option
int         keyfilterQ   = 0;       // used with -m -M and -t options
int         minorQ       = 0;       // used with -m option
int         majorQ       = 0;       // used with -M option
int         debugQ       = 0;       // used with --debug option
int         regexQ       = 0;       // used with --regex option
int         shortQ       = 0;       // used with --short option
int         tonicQ       = 0;       // used with -t option
int         diatonicQ    = 0;       // used with -D option
// const char* tonicstring  = "";      // used with -t option
Array<char> tonicstring;            // used with -t option
int         meterQ       = 0;       // used with -T option
const char* meterstring  = "";      // used with -T option
Array<char> meterss;                
int         totalQ       = 0;       // used with --total option
int         countQ       = 0;       // used with --count option
int         locationQ    = 0;       // used with --location option
int         notQ         = 0;       // used with --not option
int         unlinkQ      = 0;       // used with --unlink option
int         featureCount = 0;       // used with --unlink option
int         smartQ       = 0;       // used with --smart option

Array<char> tonicss; // tonic search string

// classical searches
int P12toneintervalQ              = 0;    // used with -i option
int PgrosscontourQ                = 0;    // used with -c option
int PrefinedcontourQ              = 0;    // used with -C option
int PscaledegreeQ                 = 0;    // used with -d option
int PmusicalintervalQ             = 0;    // used with -I option
int P12tonepitchclassQ            = 0;    // used with -P option
int PpitchclassQ                  = 0;    // used with -p option

Array<char> P12toneinterval;              // used with -i option
Array<char> Pgrosscontour;                // used with -c option
Array<char> Prefinedcontour;              // used with -C option
Array<char> Pscaledegree;                 // used with -d option
Array<char> Pmusicalinterval;             // used with -I option
Array<char> P12tonepitchclass;            // used with -P option
Array<char> Ppitchclass;                  // used with -p option

// extended rhythm searches
int RgrosscontourQ                = 0;    // used with -R option
int RrefinedcontourQ              = 0;    // used with -r option
int RdurationQ                    = 0;    // used with -D option
int RbeatlevelQ                   = 0;    // used with -b option
int RmetriclevelQ                 = 0;    // used with -L option
int RmetricpositionQ              = 0;    // used with -l option
int RmetricrefinedcontourQ        = 0;    // used with -e option
int RmetricgrosscontourQ          = 0;    // used with -E option

Array<char> Rgrosscontour;                // used with -R option
Array<char> Rrefinedcontour;              // used with -r option
Array<char> Rduration;                    // used with -D option
Array<char> Rbeatlevel;                   // used with -b option
Array<char> Rmetriclevel;                 // used with -L option
Array<char> Rmetricposition;              // used with -l option
Array<char> Rmetricrefinedcontour;        // used with -e option
Array<char> Rmetricgrosscontour;          // used with -E option


// final search feature strings for use with link checking
Array<char> xP12toneinterval;
Array<char> xPgrosscontour;
Array<char> xPrefinedcontour;
Array<char> xPscaledegree;
Array<char> xPmusicalinterval;
Array<char> xP12tonepitchclass;
Array<char> xPpitchclass;
Array<char> xRgrosscontour;
Array<char> xRrefinedcontour;
Array<char> xRduration;
Array<char> xRbeatlevel;
Array<char> xRmetriclevel;
Array<char> xRmetricposition;
Array<char> xRmetricrefinedcontour;
Array<char> xRmetricgrosscontour;





//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   tonicstring.setSize(1);
   tonicstring[0] = '\0';

   checkOptions(options, argc, argv); // process the command-line options

   Array<char> searchstring;
   Array<char>& ss = searchstring;
   ss.setSize(10000);
   ss.setGrowth(10000);
   ss.setSize(0);


// order of data in index file:
//  [Zz] = major/minor key  //////////////////////////////////////////////////

   tonicss.setSize(0);
   if (keyfilterQ) {
     tonicss.setSize(1);
     tonicss[0] = '\0';
     tonicss.setSize(0);
    
     if (majorQ && !minorQ) {
        appendString(tonicss, "Z");
     } else if (minorQ && !majorQ) {
        appendString(tonicss, "z");
     } else {
        appendString(tonicss, "[Zz]");
     }
     // place the tonic search next if given
     if (tonicQ) {
        appendString(tonicss, tonicstring.getBase());
     } else {
        appendString(tonicss, "[^=]*");
     }
     appendString(tonicss, "=");
     // place the end of key information marker:
     appendString(ss, tonicss.getBase());
     appendString(ss, ".*");
   }

//  {    = 12-tone interval //////////////////////////////////////////////////
   if (P12toneintervalQ) {
      if (cleanQ) { cleanP12toneInterval(P12toneinterval); }
      appendToSearchString(ss, P12toneinterval.getBase(), 
         PITCH_TWELVE_TONE_INTERVAL_MARKER, anchoredQ);
   }

//  #    = pitch refined contour /////////////////////////////////////////////
   if (PrefinedcontourQ) {
      if (cleanQ) { cleanPrefinedContour(Prefinedcontour); }
      appendToSearchString(ss, Prefinedcontour.getBase(), 
         PITCH_REFINED_CONTOUR_MARKER, anchoredQ);
   }

//  :    = pitch gross contour ///////////////////////////////////////////////
   if (PgrosscontourQ) {
      if (cleanQ) { cleanPgrossContour(Pgrosscontour); }
      appendToSearchString(ss, Pgrosscontour.getBase(), 
         PITCH_GROSS_CONTOUR_MARKER, anchoredQ);
   }

//  %    = scale degree //////////////////////////////////////////////////////
   if (PscaledegreeQ) {
      if (cleanQ) { cleanPscaleDegree(Pscaledegree); }
      appendToSearchString(ss, Pscaledegree.getBase(), 
         PITCH_SCALE_DEGREE_MARKER, anchoredQ);
   }

//  }    = musical interval //////////////////////////////////////////////////
   if (PmusicalintervalQ) {
      if (cleanQ) { cleanPmusicalInterval(Pmusicalinterval); }
      appendToSearchString(ss, Pmusicalinterval.getBase(), 
         PITCH_MUSICAL_INTERVAL_MARKER, anchoredQ);
   }

//  j    = 12-tone pitch class ///////////////////////////////////////////////
   if (P12tonepitchclassQ) {
      if (cleanQ) { cleanP12tonePitchClass(P12tonepitchclass); }
      appendToSearchString(ss, P12tonepitchclass.getBase(), 
         PITCH_TWELVE_TONE_MARKER, anchoredQ);
   }

//  J    = pitch class name //////////////////////////////////////////////////
   if (PpitchclassQ) {
      if (cleanQ) { cleanPpitchClass(Ppitchclass); }
      appendToSearchString(ss, Ppitchclass.getBase(), 
         PITCH_CLASS_MARKER, anchoredQ);
   }

//  M    = metric description ////////////////////////////////////////////////
   meterss.setSize(0);
   if (meterQ) {
      appendString(meterss, "M");
      if (!isdigit(meterstring[0])) {
         appendString(meterss, "[^\\t]*?");
      }
      appendString(meterss, meterstring);
      appendString(ss, meterss.getBase());
      appendString(ss, ".*");
   }

// Added rhythmic markers:

//  ~    = duration gross contour  ///////////////////////////////////////////
   if (RgrosscontourQ) {
      if (cleanQ) { cleanRgrossContour(Rgrosscontour); }
      appendToSearchString(ss, Rgrosscontour.getBase(), 
         DURATION_GROSS_CONTOUR_MARKER, anchoredQ);
   }

//  ^    = duration refined contour  /////////////////////////////////////////
   if (RrefinedcontourQ) {
      if (cleanQ) { cleanRrefinedContour(Rrefinedcontour); }
      appendToSearchString(ss, Rrefinedcontour.getBase(), 
         DURATION_REFINED_CONTOUR_MARKER, anchoredQ);
   }

//  !    = duration (IOI)  ///////////////////////////////////////////////////
   if (RdurationQ) {
      if (cleanQ) { cleanRduration(Rduration); }
      appendToSearchString(ss, Rduration.getBase(), 
         DURATION_IOI, anchoredQ);
   }

//  &    = beat level  ///////////////////////////////////////////////////////
   if (RbeatlevelQ) {
      if (cleanQ) { cleanRbeatLevel(Rbeatlevel); }
      appendToSearchString(ss, Rbeatlevel.getBase(), 
         BEAT_LEVEL, anchoredQ);
   }

//  '    = metric level  /////////////////////////////////////////////////////
   if (RmetriclevelQ) {
      if (cleanQ) { cleanRmetricLevel(Rmetriclevel); }
      appendToSearchString(ss, Rmetriclevel.getBase(), 
         METRIC_LEVEL, anchoredQ);
   }

//  `    = metric refined contour  ///////////////////////////////////////////
   if (RmetricrefinedcontourQ) {
      if (cleanQ) { cleanRmrc(Rmetricrefinedcontour); }
      appendToSearchString(ss, Rmetricrefinedcontour.getBase(), 
         METRIC_REFINED_CONTOUR, anchoredQ);
   }

//  @    = metric gross contour  /////////////////////////////////////////////
   if (RmetricgrosscontourQ) {
      if (cleanQ) { cleanRmgc(Rmetricgrosscontour); }
      appendToSearchString(ss, Rmetricgrosscontour.getBase(), 
         METRIC_GROSS_CONTOUR, anchoredQ);
   }

//  =    = metric position ///////////////////////////////////////////////////
   if (RmetricpositionQ) {
      if (cleanQ) { cleanRmetricPosition(Rmetricposition); }
      appendToSearchString(ss, Rmetricposition.getBase(), 
         METRIC_POSITION, anchoredQ);
   }


//////////////////////////////////////////////////////////////////////////////

   // terminate the search string
   char ch = '\0';
   ss.append(ch);
   ss.setSize(ss.getSize()-1);

   PerlRegularExpression pre;

   if (pre.search(ss, "\\.\\*$", "")) {
      ss.setSize(ss.getSize()-2);
   }
   ss.append(ch);
   ss.setSize(ss.getSize()-1);
   ss.append(ch);

   if (regexQ) {
      cout << ss.getBase() << endl;
      exit(0);
   }

   if (showcleanQ) {
      showCleanedParameters();
      cout << "Final Regular Expression: " << ss.getBase() << endl;
      exit(0);
   }

   pre.initializeSearchAndStudy(ss.getBase());
   int i;
   int totalcount = 0;
   if (options.getArgCount() == 0) {
      // standard input
      totalcount += searchForMatches(cin, ss, pre);
   } else {
      for (i=1; i<=options.getArgCount(); i++) {
         totalcount += searchForMatches(options.getArgument(i), ss, pre);
      }
   }

   if (totalQ) {
      cout << totalcount << endl;
   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// searchForMatches --
//

int searchForMatches(const char* filename, Array<char>& ss, 
      PerlRegularExpression& pre) {
   PerlRegularExpression blanktest;
   string line;
   int state;
   int i;
   int counter = 0;
   ifstream inputfile;
   inputfile.open(filename);
   if (!inputfile.is_open()) {
      return 0;
   }
   while (!inputfile.eof()) {
      getline(inputfile, line);
      if (blanktest.search(line.c_str(), "^\\s*$", "")) {
         continue;
      }
      state = pre.search(line.c_str());
      if (state && (!unlinkQ) && (!anchoredQ) && (featureCount > 1)) {
         state = checkLink(line);
      } else if (state && (countQ || locationQ)) {
         counter += checkLink(line);
         continue;
      }
      if ((state && !notQ) || (notQ && !state)) {
         counter++;
         if (verboseQ) {
            cout << "Matches in " << filename << endl;
         }
         if (totalQ) {
            continue;
         }
         if (shortQ) {
            i = 0;
            while ((line.c_str()[i] != '\0') && (line.c_str()[i] != '\t')) {
               cout << line.c_str()[i];
               i++;
            }
            cout << "\n";
         } else {
            cout << line << "\n";
         }
      } 

   }
   inputfile.close();
   return counter;
}



//////////////////////////////
//
// searchForMatches --
//

int searchForMatches(istream& inputfile, Array<char>& ss, 
      PerlRegularExpression& pre) {
   PerlRegularExpression blanktest;
   string line;
   int state;
   int i;
   int counter = 0;
   while (!inputfile.eof()) {
      getline(inputfile, line);
      if (blanktest.search(line.c_str(), "^\\s*$", "")) {
         continue;
      }
      state = pre.search(line.c_str());
      if (state && (!unlinkQ) && (!anchoredQ) && (featureCount > 1)) {
         state = checkLink(line);
      } else if (state && (countQ || locationQ)) {
         counter += checkLink(line);
         continue;
      }
      if ((state && !notQ) || (notQ && !state)) {
         counter++;
         if (verboseQ) {
            cout << "Matches in <STDIN>" << endl;
         }
         if (totalQ) {
            continue;
         }
         if (shortQ) {
            i = 0;
            while ((line.c_str()[i] != '\0') && (line.c_str()[i] != '\t')) {
               cout << line.c_str()[i];
               i++;
            }
            cout << "\n";
         } else {
            cout << line << "\n";
         }
      } 

   }
   return counter;
}



//////////////////////////////
//
// checkLink -- make sure that every search feature starts at the same
//    note number in the data.  Returns the number of matches found
//    on the input line.
//

int checkLink(string& line) {

   Array<int> target;
   target.setSize(0);
   Array<int> temptarget;

   if (P12toneintervalQ) {
      getSimpleLocation(temptarget, line, P12toneinterval,
            PITCH_TWELVE_TONE_INTERVAL_MARKER, target, 2);
      if (temptarget.getSize() == 0) {
         return 0;
      } else {
         target = temptarget;
      }
   }

   if (PgrosscontourQ) {
      getSimpleLocation(temptarget, line, Pgrosscontour, 
            PITCH_GROSS_CONTOUR_MARKER, target);
      if (temptarget.getSize() == 0) {
         return 0;
      } else {
         target = temptarget;
      }
   }

   if (PrefinedcontourQ) {
      getSimpleLocation(temptarget, line, Prefinedcontour,
            PITCH_REFINED_CONTOUR_MARKER, target);
      if (temptarget.getSize() == 0) {
         return 0;
      } else {
         target = temptarget;
      }
   }

   if (PscaledegreeQ) {
      getSimpleLocation(temptarget, line, Pscaledegree, 
            PITCH_SCALE_DEGREE_MARKER, target);
      if (temptarget.getSize() == 0) {
         return 0;
      } else {
         target = temptarget;
      }
   }

   if (PmusicalintervalQ) {
      getMusicalIntervalLocation(temptarget, line, Pmusicalinterval, 
            PITCH_MUSICAL_INTERVAL_MARKER, target);
      if (temptarget.getSize() == 0) {
         return 0;
      } else {
         target = temptarget;
      }
   }

   if (P12tonepitchclassQ) {
      getSimpleLocation(temptarget, line, P12tonepitchclass,
            PITCH_TWELVE_TONE_MARKER, target);
      if (temptarget.getSize() == 0) {
         return 0;
      } else {
         target = temptarget;
      }
   }

   if (PpitchclassQ) {
      getSeparatorLocation(temptarget, line, Ppitchclass,
            PITCH_CLASS_MARKER, target, ' ');
      if (temptarget.getSize() == 0) {
         return 0;
      } else {
         target = temptarget;
      }
   }

   if (RgrosscontourQ) {
      getSimpleLocation(temptarget, line, Rgrosscontour,
            DURATION_GROSS_CONTOUR_MARKER, target);
      if (temptarget.getSize() == 0) {
         return 0;
      } else {
         target = temptarget;
      }
   }

   if (RrefinedcontourQ) {
      getSimpleLocation(temptarget, line, Rrefinedcontour,
            DURATION_REFINED_CONTOUR_MARKER, target);
      if (temptarget.getSize() == 0) {
         return 0;
      } else {
         target = temptarget;
      }
   }

   if (RdurationQ) {
      getSeparatorLocation(temptarget, line, Rduration,
            DURATION_IOI, target, ' ');
      if (temptarget.getSize() == 0) {
         return 0;
      } else {
         target = temptarget;
      }
   }

   if (RbeatlevelQ) {
      getSimpleLocation(temptarget, line, Rbeatlevel, BEAT_LEVEL, target);
      if (temptarget.getSize() == 0) {
         return 0;
      } else {
         target = temptarget;
      }
   }

   if (RmetriclevelQ) {
      getSeparatorLocation(temptarget, line, Rmetriclevel,
            METRIC_LEVEL, target, ' ');
      if (temptarget.getSize() == 0) {
         return 0;
      } else {
         target = temptarget;
      }
   }

   if (RmetricpositionQ) {
      getSeparatorLocation(temptarget, line, Rmetricposition,
            METRIC_POSITION, target, ' ');
      if (temptarget.getSize() == 0) {
         return 0;
      } else {
         target = temptarget;
      }
   }

   if (RmetricrefinedcontourQ) {
      getSimpleLocation(temptarget, line, Rmetricrefinedcontour,
            METRIC_REFINED_CONTOUR, target);
      if (temptarget.getSize() == 0) {
         return 0;
      } else {
         target = temptarget;
      }
   }

   if (RmetricgrosscontourQ) {
      getSimpleLocation(temptarget, line, Rmetricgrosscontour,
            METRIC_GROSS_CONTOUR, target);
      if (temptarget.getSize() == 0) {
         return 0;
      } else {
         target = temptarget;
      }
   }

   if ((locationQ || countQ) && (target.getSize() > 0)) {
      int i = 0;
      const char* ptr = line.c_str();
      while ((ptr[i] != '\0') && (ptr[i] != '\t')) {
         cout << ptr[i];                  
         i++;
      }
      if (countQ) {
         cout << '\t' << target.getSize();
      } else {
         // locationQ
         cout << '\t';
         for (i=0; i<target.getSize(); i++)  {
            cout << target[i] + 1;
            if (i < target.getSize() -1) {
               cout << " ";
            }
         }
      }
      cout << '\n';
   }

   // all position filters returned success
   return target.getSize();
}


/* no longer needed, but keeping just in case:

//////////////////////////////
//
// findIntersection -- return the values which are present in both
//    lists.  The lists are presumed to be sorted.  Output is stored
//    in first array of input parameters.
//

void findIntersection(Array<int>& aa, Array<int>& bb) {
   Array<int> output(aa.getSize());
   output.setSize(0);
   int aSize = aa.getSize();
   int bSize = bb.getSize();
   int aIndex = 0;
   int bIndex = 0;
   for (aIndex = 0; aIndex < aSize; aIndex++) {
      if (bb[bIndex] < aa[aIndex]) {
         do {
            bIndex++;
            if (bIndex >= bSize) {
               break;
            }
         } while (bb[bIndex] < aa[aIndex]);
      }
      if (bIndex >= bSize) {
         break;
      }
      if (bb[bIndex] == aa[aIndex]) {
         output.append(aa[aIndex]);
      } 
   }

   aa = output;
}

*/


//////////////////////////////
//
// getMusicalIntervalLocation -- return the element number in the feature
//    list (offset from 1) on the given line in the specified feature,
//    when the features are sepearated by according to musical interval
//    feature format, such as:
//    }Xm2XM2P1xm2Xm2XM2xM2P1xM2P1P1P1P1xm3P1xm3xM3XM2XM2Xm2XP5P1XP4xm2xM2
//       *  * *  *  *  *  * *  * * * * *  * *  *  *  *  *  *  * *  *  *  *
//    (checking for segmentation when a digits is preceded by a non-digit.)
//

void getMusicalIntervalLocation(Array<int>& positions, string& line, 
      Array<char>& feature, char searchanchor, Array<int>& checklocs) {

   positions.setSize(0);
   PerlRegularExpression pre;
   Array<char> startmarker;
   startmarker.setSize(0);
   appendString(startmarker, "(\\t");
   startmarker.append(searchanchor);
   appendString(startmarker, ")");
   pre.search(line.c_str(), startmarker.getBase(), "");
   int startindex = pre.getSubmatchEnd(1);
   int i;

   // If checklocs is non-zero, then only check the feature
   // at the given locations; otherwise, check every position.
   if (checklocs.getSize() == 0) {

      // check every location in feature for a match (allowing overlaps)
      PerlRegularExpression gumby;
      gumby.setAnchor();
      gumby.initializeSearchAndStudy(feature.getBase());
      const char* str = line.c_str();
      int location = -1; // need to start at -1 since first spot will increment
      i = startindex;
      while ((str[i] != '\0') && (str[i] != '\t')) {
         if ((toupper(str[i]) == 'X') || 
             ((str[i] == 'P') && (toupper(str[i-1]) != 'X'))) {
            location++;
         }
         if (gumby.search(str+i)) {
            positions.append(location);
         }
         i++;
      }

   } else {
      // Check only locations specified by checklocs.
      // The positions in checklocs are presumed to be sorted.
      PerlRegularExpression pokey;
      pokey.setAnchor();
      pokey.initializeSearchAndStudy(feature.getBase());
      const char* str = line.c_str();
      int location = 0;
      i = startindex;
      int targetindex = 0;
      int tsize = checklocs.getSize();
      while ((str[i] != '\0') && (str[i] != '\t') && (targetindex < tsize)) {
         // Increase the target location if it is smaller than 
         // the current search location.
         if (checklocs[targetindex] < location) {
            targetindex++;
            continue;
         }
         // If a separator character is found, then increment location.
         if ((toupper(str[i]) == 'X') || 
             ((str[i] == 'P') && (toupper(str[i-1]) != 'X'))) {
            location++;
         }
         if (location == checklocs[targetindex]) {
            if (pokey.search(str+i)) {
               positions.append(location);
               targetindex++;
            }
         }
         i++;
      }
   }

   if (verbose2Q) {
      cout << "ORIGINAL LINE: " << line << endl;
      cout << "\tCOUNT IS " << positions.getSize() << " FOR FEATURE SEARCH " 
           << feature.getBase() << endl;
      cout << "\tMATCHES AT: ";
      int j;
      for (j=0; j<positions.getSize(); j++) {
         cout << positions[j] << " ";
      }
      cout << endl;
   }

}



//////////////////////////////
//
// getSeparatorLocation -- return the element number in the feature
//    list (offset from 0) on the given line in the specified feature,
//    when the features are sepearated by spaces (or whatever the 
//    separator character is).
//    Default value: separator = ' '
//

void getSeparatorLocation(Array<int>& positions, string& line, 
      Array<char>& feature, char searchanchor, Array<int>& checklocs, 
      char separator) {
   positions.setSize(0);
   PerlRegularExpression pre;
   Array<char> startmarker;
   startmarker.setSize(0);
   appendString(startmarker, "(\\t");
   startmarker.append(searchanchor);
   appendString(startmarker, ")");
   pre.search(line.c_str(), startmarker.getBase(), "");
   int startindex = pre.getSubmatchEnd(1);
   int i;

   // If checklocs is non-zero, then only check the feature
   // at the given locations; otherwise, check every position.
   if (checklocs.getSize() == 0) {

      // check every location in feature for a match (allowing overlaps)
      PerlRegularExpression gumby;
      gumby.setAnchor();
      gumby.initializeSearchAndStudy(feature.getBase());
      const char* str = line.c_str();
      int location = 0;
      i = startindex;
      while ((str[i] != '\0') && (str[i] != '\t')) {
         if (str[i] == separator) {
            i++;
            location++;
            continue;
         }
         if (gumby.search(str+i)) {
            positions.append(location);
         }
         i++;
      }

   } else {
      // Check only locations specified by checklocs.
      // The positions in checklocs are presumed to be sorted.
      PerlRegularExpression pokey;
      pokey.setAnchor();
      pokey.initializeSearchAndStudy(feature.getBase());
      const char* str = line.c_str();
      int location = 0;
      i = startindex;
      int targetindex = 0;
      int tsize = checklocs.getSize();
      while ((str[i] != '\0') && (str[i] != '\t') && (targetindex < tsize)) {
         // Increase the target location if it is smaller than 
         // the current search location.
         if (checklocs[targetindex] < location) {
            targetindex++;
            continue;
         }
         // If a separator character is found, then increment location.
         if (str[i] == separator) {
            i++;
            location++;
            continue;
         }
         if (location == checklocs[targetindex]) {
            if (pokey.search(str+i)) {
               positions.append(location);
               targetindex++;
            }
         }
         i++;
      }
   }

   if (verbose2Q) {
      cout << "ORIGINAL LINE: " << line << endl;
      cout << "\tCOUNT IS " << positions.getSize() << " FOR FEATURE SEARCH " 
           << feature.getBase() << endl;
      cout << "\tMATCHES AT: ";
      int j;
      for (j=0; j<positions.getSize(); j++) {
         cout << positions[j] << " ";
      }
      cout << endl;
   }

}



//////////////////////////////
//
// getSimpleLocation -- return the element number in the feature list
//   (offset from 0) on the given line in the specified feature
//   Default value: featurewidth = 1
//

void getSimpleLocation(Array<int>& positions, string& line, 
      Array<char>& feature, char searchanchor, Array<int>& checklocs,
      int featurewidth) {
   positions.setSize(0);
   PerlRegularExpression pre;
   Array<char> startmarker;
   startmarker.setSize(0);
   appendString(startmarker, "(\\t");
   startmarker.append(searchanchor);
   appendString(startmarker, ")");
   pre.search(line.c_str(), startmarker.getBase(), "");
   int startindex = pre.getSubmatchEnd(1);
   int i;

   // if checklocs is non-zero, then only check the feature
   // at the given locations; otherwise, check every position.
   if (checklocs.getSize() == 0) {
      // check every location in feature for a match (allowing overlaps)
      PerlRegularExpression gumby;
      gumby.setAnchor();
      gumby.initializeSearchAndStudy(feature.getBase());
      const char* str = line.c_str();
      int location;
      int charloc = 0;
      i = startindex;
      while ((str[i] != '\0') && (str[i] != '\t')) {
         if (gumby.search(str+i)) {
            location = charloc / featurewidth;
            positions.append(location);
         }
         charloc++;
         i++;
      }
   } else {
      // check only locations specified by checklocs
      // The feature is presumed to be of equal length
      // to other features, and an end of string problem
      // will therefore not be checked.  Only the start
      // of a feature will be checked (no partial
      // start features will be examined, but could be
      // added).
      PerlRegularExpression pokey;
      pokey.setAnchor();
      pokey.initializeSearchAndStudy(feature.getBase());
      const char* str = line.c_str() + startindex;
      for (i=0; i<checklocs.getSize(); i++) {
         if (pokey.search(str+(checklocs[i] * featurewidth))) {
             positions.append(checklocs[i]);
         }
         
      }
   }

   if (verbose2Q) {
      cout << "ORIGINAL LINE: " << line << endl;
      cout << "\tCOUNT IS " << positions.getSize() << " FOR FEATURE SEARCH " 
           << feature.getBase() << endl;
      cout << "\tMATCHES AT: ";
      int j;
      for (j=0; j<positions.getSize(); j++) {
         cout << positions[j] << " ";
      }
      cout << endl;
   }

}



//////////////////////////////
//
// showCleanedParameters --  Show what the post processed input search
//    parameters look like for each search field (if it was used).
//

void showCleanedParameters(void) {

   if (tonicss.getSize() > 1) {
      cout << "tonic:\t\t" << tonicss.getBase() << endl;
   }
   if (P12toneinterval.getSize() > 1) {
      cout << "twelve-tone interval:\t" << P12toneinterval.getBase() << endl;
   }
   if (Pgrosscontour.getSize() > 1) {
      cout << "Pitch gross contour:\t" << Pgrosscontour.getBase() << endl;
   }
   if (Prefinedcontour.getSize() > 1) {
      cout << "Pitch refined contour:\t" << Prefinedcontour.getBase() << endl;
   }
   if (Pscaledegree.getSize() > 1) {
      cout << "Scale-degree:\t" << Pscaledegree.getBase() << endl;
   }
   if (Pmusicalinterval.getSize() > 1) {
      cout << "Interval:\t" << Pmusicalinterval.getBase() << endl;
   }
   if (P12tonepitchclass.getSize() > 1) {
      cout << "Twelve-tone pitch-class:\t" << P12tonepitchclass.getBase() 
           << endl;
   }
   if (Ppitchclass.getSize() > 1) {
      cout << "Pitch-class:\t" << Ppitchclass.getBase() << endl;
   }
   if (meterss.getSize() > 1) {
      cout << "meter:\t\t" << meterss.getBase() << endl;
   }
   if (Rgrosscontour.getSize() > 1) {
      cout << "Rhythm gross contour:\t" << Rgrosscontour.getBase() << endl;
   }
   if (Rrefinedcontour.getSize() > 1) {
      cout << "Rhythm refined contour:\t" << Rrefinedcontour.getBase() << endl;
   }
   if (Rduration.getSize() > 1) {
      cout << "Duration:\t" << Rduration.getBase() << endl;
   }
   if (Rbeatlevel.getSize() > 1) {
      cout << "Beat level:\t" << Rbeatlevel.getBase() << endl;
   }
   if (Rmetriclevel.getSize() > 1) {
      cout << "Metric level:\t" << Rmetriclevel.getBase() << endl;
   }
   if (Rmetricrefinedcontour.getSize() > 1) {
      cout << "Metric refined contour:\t" << Rmetricrefinedcontour.getBase() 
           << endl;
   }
   if (Rmetricgrosscontour.getSize() > 1) {
      cout << "Metric gross contour:\t" << Rmetricgrosscontour.getBase() 
           << endl;
   }
   if (Rmetricposition.getSize() > 1) {
      cout << "Metric position:\t" << Rmetricposition.getBase() << endl;
   }

}



//////////////////////////////
//
// appendToSearchString --
//

void appendToSearchString(Array<char>& ss, const char* string, 
      char marker, int anchor) {
   char ch;

   // add back quote for certain markers
   switch (marker) {
      case '{':
      case '}':
      case '^':
         ch = '\\'; ss.append(ch);
   }
   ss.append(marker);
   char nullchar = '\0';
   ss.append(nullchar);
   ss.setSize(ss.getSize()-1);


   // add [^\t]* if not anchored:
   //
   //
   if (!anchor) {
      appendString(ss, "[^\\t]*?");
   }
   appendString(ss, string);

   // for pitch-class names, the next character after the search
   // string must be a space or a tab to prevent accidentals
   // from matching on natural-note search endings.
   if (marker == PITCH_CLASS_MARKER) {
      appendString(ss, "[ \\t]");
   }

   appendString(ss, ".*");
}




//////////////////////////////
//
// appendString --
//

void appendString(Array<char>& ss, const char* string) {
   int i;
   char ch;
   int length = strlen(string);
   for (i=0; i<length; i++) {
      ch = string[i];
      ss.append(ch);
   }
   // place a dummy null character at end in case
   // the string is printed directly from the data.
   ch = '\0'; 
   ss.append(ch);
   ss.setSize(ss.getSize()-1);
}



//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   PerlRegularExpression pre;

   opts.define("a|A|anchored=b",     "anchored search at start");
   opts.define("m|minor=b",          "search only minor keys");
   opts.define("M|major=b",          "search only minor keys");
   opts.define("t|tonic=s:",         "search only given tonic");
   opts.define("T|meter=s:",         "search only given meters");

   opts.define("i|pitch-12tone-interval=s:",   "12-tone interval");
   opts.define("C|pitch-gross-contour=s:",     "pitch gross contour");
   opts.define("c|pitch-refined-contour=s:",   "pitch refined contour");
   opts.define("d|pitch-scale-degree=s:",      "pitch scale degree");
   opts.define("I|pitch-musical-interval=s:",  "musical interval");
   opts.define("P|pitch-12tonepc=s:",          "12-tone pitch class");
   opts.define("p|pitch-class=s:",             "pitch class");
   opts.define("D|diatonic-pitch-class=b:",    "diatonic pitch class");

   opts.define("R|duration-gross-contour=s:",   "duration gross contour");
   opts.define("r|duration-refined-contour=s:", "duration refined contour");
   opts.define("u|duration=s:",                 "duration (IOI)");
   opts.define("b|beat-level=s:",               "beat level");
   opts.define("L|metric-level=s:",             "metric level");
   opts.define("l|metric-position=s:",          "metric position");
   opts.define("e|metric-refined-contour=s:",   "metric refined contour");
   opts.define("E|metric-gross-contour=s:",     "metric-gross contour");

   //opts.define("i|pitch-twelvetone-interval=s:", "12-tone interval");

   opts.define("verbose=b",              "verbose display");
   opts.define("verbose2=b",             "verbose display");
   opts.define("raw|no-clean=b",         "do not preprocess searches");
   opts.define("cleaned|queries|features=b",  "show cleaned input fields");
   opts.define("short|trim=b",           "display only filename in output");
   opts.define("v|V|invert-match|not=b", "negate search query");

   // output formatting options
   opts.define("total=b",            "return a count of the matches");
   opts.define("count=b",            "return count in matched lines");
   opts.define("location=b",         "return count in matched lines");

   opts.define("debug=b",            "debugging statements");
   opts.define("regex=b",            "print regular expression construct");
   opts.define("unlink=b",           "unlink search features");
   opts.define("smart=b",            "do a smart search");

   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("h|help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Oct 2008" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 24 Nov 2008" << endl;
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
   
   ////////////////////////////////////////////////
   
   shortQ                 =  opts.getBoolean("short");
   verboseQ               =  opts.getBoolean("verbose");
   verbose2Q              =  opts.getBoolean("verbose2");
   cleanQ                 = !opts.getBoolean("no-clean");
   showcleanQ             =  opts.getBoolean("cleaned");
   anchoredQ              =  opts.getBoolean("anchored");
   debugQ                 =  opts.getBoolean("debug");
   regexQ                 =  opts.getBoolean("regex");
   smartQ                 =  opts.getBoolean("smart");
   majorQ                 =  opts.getBoolean("major");
   minorQ                 =  opts.getBoolean("minor");
   tonicQ                 =  opts.getBoolean("tonic");
   unlinkQ                =  opts.getBoolean("unlink");
   notQ                   =  opts.getBoolean("not");
   tonicstring.setSize(strlen(opts.getString("tonic")) + 1);
   strcpy(tonicstring.getBase(), opts.getString("tonic"));
   pre.sar(tonicstring, "-sharp", "#", "g");
   pre.sar(tonicstring, "-flat", "-", "g");
   pre.tr(tonicstring, "abcdefg", "ABCDEFG");
   totalQ                 = opts.getBoolean("total");
   countQ                 = opts.getBoolean("count");
   locationQ              = opts.getBoolean("location");

   keyfilterQ             = majorQ || minorQ || tonicQ;
   meterQ                 =  opts.getBoolean("meter");
   meterstring            =  opts.getString("meter");

   P12toneintervalQ       =  opts.getBoolean("pitch-12tone-interval");
   PgrosscontourQ         =  opts.getBoolean("pitch-gross-contour");
   PrefinedcontourQ       =  opts.getBoolean("pitch-refined-contour");
   PscaledegreeQ          =  opts.getBoolean("pitch-scale-degree");
   PmusicalintervalQ      =  opts.getBoolean("pitch-musical-interval");
   P12tonepitchclassQ     =  opts.getBoolean("pitch-12tonepc");

   PpitchclassQ           =  opts.getBoolean("pitch-class");
   diatonicQ              =  opts.getBoolean("diatonic-pitch-class");

   P12toneinterval.setSize(strlen(opts.getString("pitch-12tone-interval"))+1);
   strcpy(P12toneinterval.getBase(), opts.getString("pitch-12tone-interval"));
   Pgrosscontour.setSize(strlen(opts.getString("pitch-gross-contour"))+1);
   strcpy(Pgrosscontour.getBase(), opts.getString("pitch-gross-contour"));
   Prefinedcontour.setSize(strlen(opts.getString("pitch-refined-contour"))+1);
   strcpy(Prefinedcontour.getBase(), opts.getString("pitch-refined-contour"));
   Pscaledegree.setSize(strlen(opts.getString("pitch-scale-degree"))+1);
   strcpy(Pscaledegree.getBase(), opts.getString("pitch-scale-degree"));
   Pmusicalinterval.setSize(strlen(opts.getString("pitch-musical-interval"))+1);
   strcpy(Pmusicalinterval.getBase(), opts.getString("pitch-musical-interval"));
   P12tonepitchclass.setSize(strlen(opts.getString("pitch-12tonepc"))+1);
   strcpy(P12tonepitchclass.getBase(), opts.getString("pitch-12tonepc"));
   Ppitchclass.setSize(strlen(opts.getString("pitch-class"))+1);
   strcpy(Ppitchclass.getBase(), opts.getString("pitch-class"));

   RgrosscontourQ         = opts.getBoolean("duration-gross-contour");
   RrefinedcontourQ       = opts.getBoolean("duration-refined-contour");
   RdurationQ             = opts.getBoolean("duration");
   RbeatlevelQ            = opts.getBoolean("beat-level");
   RmetriclevelQ          = opts.getBoolean("metric-level");
   RmetricpositionQ       = opts.getBoolean("metric-position");
   RmetricrefinedcontourQ = opts.getBoolean("metric-refined-contour");
   RmetricgrosscontourQ   = opts.getBoolean("metric-gross-contour");

   Rgrosscontour.setSize(strlen(opts.getString("duration-gross-contour"))+1);
   strcpy(Rgrosscontour.getBase(), opts.getString("duration-gross-contour"));
   Rrefinedcontour.setSize(
      strlen(opts.getString("duration-refined-contour"))+1);
   strcpy(Rrefinedcontour.getBase(), 
      opts.getString("duration-refined-contour"));
   Rduration.setSize(strlen(opts.getString("duration"))+1);
   strcpy(Rduration.getBase(), opts.getString("duration"));
   Rbeatlevel.setSize(strlen(opts.getString("beat-level"))+1);
   strcpy(Rbeatlevel.getBase(), opts.getString("beat-level"));
   Rmetriclevel.setSize(strlen(opts.getString("metric-level"))+1);
   strcpy(Rmetriclevel.getBase(), opts.getString("metric-level"));
   Rmetricposition.setSize(strlen(opts.getString("metric-position"))+1);
   strcpy(Rmetricposition.getBase(), opts.getString("metric-position"));
   Rmetricrefinedcontour.setSize(
      strlen(opts.getString("metric-refined-contour"))+1);
   strcpy(Rmetricrefinedcontour.getBase(), 
      opts.getString("metric-refined-contour"));
   Rmetricgrosscontour.setSize(
      strlen(opts.getString("metric-gross-contour"))+1);
   strcpy(Rmetricgrosscontour.getBase(), 
      opts.getString("metric-gross-contour"));


   // count the number of parallel features being searched
   featureCount = 0;
   if (P12toneintervalQ)       { featureCount++; }
   if (PgrosscontourQ)         { featureCount++; }
   if (PrefinedcontourQ)       { featureCount++; }
   if (PscaledegreeQ)          { featureCount++; }
   if (PmusicalintervalQ)      { featureCount++; }
   if (P12tonepitchclassQ)     { featureCount++; }
   if (PpitchclassQ)           { featureCount++; }
   if (RgrosscontourQ)         { featureCount++; }
   if (RrefinedcontourQ)       { featureCount++; }
   if (RdurationQ)             { featureCount++; }
   if (RbeatlevelQ)            { featureCount++; }
   if (RmetriclevelQ)          { featureCount++; }
   if (RmetricpositionQ)       { featureCount++; }
   if (RmetricrefinedcontourQ) { featureCount++; }
   if (RmetricgrosscontourQ)   { featureCount++; }
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

cout << 
"THEMA     : Search thematic database; identify the composer and origin.\n"
"\n"
"     This command searches a thematic database according to user-defined\n"
"     search keys.  Themes that match the search key are identified\n"
"     by composer, title, instrumentation, opus, movement, etc.\n"
"\n"
"     Search keys (\"incipits\") may be defined in several different ways, in-\n"
"     cluding rough up/down contour (-C), up/down contour with step and leap\n"
"     intervals distinguished (-c), by scale degree (-d), by semitone interval\n"
"     (-i), by interval-class (-I), by pitch (-p), by relative rhythm (-r),\n"
"     and by absolute rhythm (R).  Several search keys can be combined in a\n"
"     single search, and searches can be limited to structural tones only (-s).\n"
"\n"
"Syntax:\n"
"\n"
"     thema [-m|M] [-s] [-C incipit] [-c incipit] [-d incipit] [-i incipit]\n"
"      [-I incipit] [-p incipit] [-r incipit] [-R incipit] [-t tonic]\n"
"\n"
"Options:\n"
"\n"
"\n"
"  -C incipit  : identify according to up/down/unison(same) contour [UDS]\n"
"  -c incipit  : identify according to up/down/unison step/leap contour [UDuds]\n"
"  -d incipit  : identify according to scale degree [1234567]\n"
"  -i incipit  : identify according to semitone intervals [+-1234567890]\n"
"  -m          : limit search to themes in minor keys\n"
"  -M          : limit search to themes in major keys\n"
"  -P incipit  : identify according to pitch-class [#-abcdefg]\n"
"  -p incipit  : identify according to absolute pitch [#-ABCDEFGabcdefg]\n"
"  -r incipit  : identify according to longer/shorter/same rhythm [<>=]\n"
"  -R incipit  : identify according to long/short/medium rhythm [LSM]\n"
"  -s          : limit search to structural tones only\n"
"  -t tonic    : limit search to themes in key of \"tonic\" (major or minor)\n"
"\n"
"  Refer to reference manual for further details." 
<<endl;

}


///////////////////////////////////////////////////////////////////////////
//
// Cleaning functions -- these functions will filter user input to
//    attempt to make it conformant to the index search
//


///////////////////////////////
//
// cleanRmgc -- metric gross contour
//

void cleanRmgc(Array<char>& data) {
   PerlRegularExpression pre;

   // remove invalid characters
   pre.sar(data, "[^SsHhwW=<>0-9,.{}()+*-]", "", "g");
   pre.tr(data, "w=sh", "WWSH");
}



///////////////////////////////
//
// cleanRmrc -- metric refined contour
//

void cleanRmrc(Array<char>& data) {
   PerlRegularExpression pre;

   // remove invalid characters
   pre.sar(data, "[^SsLlWw=<>0-9,.{}()+*-]", "", "g");
   pre.tr(data, "=", "w");
}



///////////////////////////////
//
// cleanRgrossContour --
//

void cleanRgrossContour(Array<char>& data) {
   PerlRegularExpression pre;

   // remove invalid characters
   pre.sar(data, "[^SsLl=<>0-9,.{}()+*-]", "", "g");
   pre.tr(data, "SsLl", "<<>>");
}



///////////////////////////////
//
// cleanRrefinedContour --
//

void cleanRrefinedContour(Array<char>& data) {
   PerlRegularExpression pre;

   // remove invalid characters
   pre.sar(data, "[^SsLl=<>0-9,.{}()+*-]", "", "g");
   pre.tr(data, "sl", "<>");
   pre.sar(data, "S", "\\[", "g");
   pre.sar(data, "L", "\\]", "g");
}


//////////////////////////////
//
// cleanRbeatLevel --
//

void cleanRbeatLevel(Array<char>& data) {
   PerlRegularExpression pre;

   // remove invalid characters
   pre.sar(data, "[^0-9,.{}()|*+?BbSs]", "", "g");
   // convert B to 1 and S to 0 if they are used (derived from metric level)
   pre.tr(data, "BbSs", "1100");

   // adjust meaning of dot so that it does not match to tab character:
   pre.sar(data, "\\.", "[^\\t]", "g");
   
}



//////////////////////////////
//
// cleanRDuration --
//

void cleanRduration(Array<char>& data) {
   PerlRegularExpression pre;

   // remove invalid characters
   pre.sar(data, "[^0-9Dd,.{}()|*+\\sxX]", "", "g");

   // change dots into "d":
   pre.tr(data, ".Dx", "ddX");

   // change spaces to single, adding one at end, removing
   // any from start
   pre.sar(data, "\\s+", " ", "g");
   pre.sar(data, "^\\s+", "", "");
   pre.sar(data, "\\s*$", " ", "");

   // work on allowing regular expressions here
   
}



//////////////////////////////
//
// cleanRmetricLevel --
//

void cleanRmetricLevel(Array<char>& data) {
   PerlRegularExpression pre;

   // remove invalid characters
   pre.sar(data, "[^0-9,.{}()|*+\\s+-pmPM]", "", "g");

   // change dots into "d":
   pre.tr(data, "+-PMbs", "pmpmBS");

   pre.sar(data, "B", " B ", "g");
   pre.sar(data, "S", " S ", "g");

   // change spaces to single, adding one at end, removing
   // any from start
   pre.sar(data, "\\s+", " ", "g");
   pre.sar(data, "^\\s+", "", "");
   pre.sar(data, "\\s*$", " ", "");

   // expand B and S markers
   pre.sar(data, "B", "(?:p\\d+|0)", "g");
   pre.sar(data, "S", "m\\d+", "g");
}



//////////////////////////////
//
// cleanRmetricPosition --
//

void cleanRmetricPosition(Array<char>& data) {
   PerlRegularExpression pre;

   // remove invalid characters
   pre.sar(data, "[^0-9,.{}()|*+\\~\\^\\s+_\\- \\/]", "", "g");

   // change dashes into underscores
   pre.tr(data, "-", "_");
   // make sure underscores do not have any adjacent spaces:
   pre.sar(data, "\\s*_\\s*", "_", "g");

   // make dots surrounded by spaces:
   pre.sar(data, "\\s*\\.\\s*", " . ", "g");

   // change spaces to single space, adding one at end, removing
   // any from start
   pre.sar(data, "\\s+~\\s+", "~ ", "g");  // remove any spaces before ~
   pre.sar(data, "\\s+\\^\\s+", "~ ", "g");  // remove any spaces before ^
   pre.sar(data, "\\s+", " ", "g");
   pre.sar(data, "^\\s+", "", "");
   pre.sar(data, "\\s*$", " ", "");

   // attach offbeat values to beats
   pre.sar(data, " (?=\\d+\\/\\d+)", "_", "g");

   // expand ~ marker (which means beat and any possible subbeat)
   pre.sar(data, "\\~", "(?:_\\d+\\/\\d+)?", "g");

   // expand ^ marker (which means any possible subbeat of a beat)
   pre.sar(data, "\\^", "_(?:\\d+\\/\\d+)?", "g");

   // add "x" markers to starts of beats
   pre.sar(data, "\\s+", " x", "g");
   pre.sar(data, "x$", "", "");
   pre.sar(data, "^", "x", "");
   
   // convert dots into form to match any beat position
   pre.sar(data, "\\.", "\\d+(_\\d+\\/\\d+)?", "g");
}



//////////////////////////////
//
// cleanPgrossContour --
//

void cleanPgrossContour(Array<char>& data) {
   PerlRegularExpression pre;
   // remove invalid characters
   pre.sar(data, "[^0-9/\\\\UuDdSsRr*+.{},()|=-]", "", "g");  

   // collapse alias chars
   pre.tr(data, "=-/\\\\udsRr",  "SSUDUDSSS"); 

   // convert "." into [^\t] since tab characters should not match "."
   pre.sar(data, "\\.", "[^\\t]", "g");
}



//////////////////////////////
//
// cleanPrefinedContour --
//

void cleanPrefinedContour(Array<char>& data) {
   PerlRegularExpression pre;
   // remove invalid characters
   pre.sar(data, "[^0-9UuDdSsRr*+.{},()|-]", "", "g");  

   // collapse alias chars
   pre.tr(data, "-sRr",  "SSSS"); 

   // convert "." into [^\t] since tab characters should not match "."
   pre.sar(data, "\\.", "[^\\t]", "g");
}



//////////////////////////////
//
// cleanPscaleDegree -- This function cleans user input so that it
//   can be used to search the data which is formatted in a particular way.
//

void cleanPscaleDegree(Array<char>& data) {
   PerlRegularExpression pre;

   // remove any characters which are not 1-7
   pre.sar(data, "[^1-7]", "", "g");
}



//////////////////////////////
//
// cleanP12toneInterval -- This function cleans user input so that it
//   can be used to search the data which is formatted in a particular way.
//

void cleanP12toneInterval(Array<char>& data) {
   PerlRegularExpression pre;


   // place a plus sign in front of any intervals which don't have signs
   pre.sar(data, "\\s(?=\\d)", " +", "g");
   if (pre.search(data, "^\\d", "")) {
      pre.sar(data, "^", "p", "");
   }

   // change plus and minus into "p" and "m"
   pre.tr(data, "PM+-", "pmpm");

   // change any m0 to p0:
   pre.sar(data, "m0", "p0", "g");

   // remove any disallowed characters
   pre.sar(data, "[^0-9pm]", "", "g");
}


//////////////////////////////
//
// cleanP12tonePitchClass -- This function cleans user input so that it
//   can be used to search the data which is formatted in a particular way.
//

void cleanP12tonePitchClass(Array<char>& data) {
   PerlRegularExpression pre;

   // change lowercase to upper case
   // Y => A, Z => B.  May use pitch classes in the future so
   // minimizing use of A and B in raw search from user.
   pre.tr(data, "abyzYZ", "ABABAB");

   if (!pre.search(data, "[AB]", "")) {
      // convert "10" and "11" to "A" and "B" if they are present in query
      pre.sar(data, "\\b10\\b", "A", "g");
      pre.sar(data, "\\b11\\b", "B", "g");
   }

   // remove any characters which are invalid
   pre.sar(data, "[^AB0-9]", "", "g");
}



//////////////////////////////
//
// cleanPmusicalInterval -- based on processIntervalClassQuary in original
//    PERL interface.  This function cleans user input so that it can be 
//    used to search the data which is formatted in a particular way.
//
//    Todo: add ^ and # wildcards.
//

void cleanPmusicalInterval(Array<char>& data) {
   PerlRegularExpression pre;
   PerlRegularExpression pre2;

   Array<Array<char> > pieces;
   pieces.setSize(1000);
   pieces.setSize(0);
   pieces.setGrowth(100000);

   // remove invalid characters
   pre.sar(data, "[^0-9\\+\\-MmPpAaDd\\.\\*\\?]", "", "g");
 
   // translate characters
   pre.tr(data, "\\+\\-Dap", "XxdAP");

   // extract individual intervals from data string:
  
   const char* ptr;
   int index;
   while (pre.search(data, 
   "(\\*\\s*|\\.\\s*\\??|[Xx]?[PdmMA]?\\d+\\s*\\??|[Xx]?[PdmMA]\\s*\\??|[Xx]\\s*\\?\?)",
      "")) {
      ptr = pre.getSubmatch(1);
      if (pre2.search(ptr, "^\\s*$")) {
         continue;
      }
      index = pieces.getSize();
      pieces.setSize(index+1);
      pieces[index].setSize(0);
      appendString(pieces[index], ptr);
      prepareInterval(pieces[index]);
      pre.sar(data, 
      "(\\*\\s*|\\.\\s*\\??|[Xx]?[PdmMA]?\\d+\\s*\\??|[Xx]?[PdmMA]\\s*\\??|[Xx]\\s*\\?\?)",
         "", "");
   }

   data.setSize(0);
   int i;
   for (i=0; i<pieces.getSize(); i++) {
      appendString(data, pieces[i].getBase());
   }

}



//////////////////////////////
//
// prepareInterval -- Process a single interval item from the function
//      cleanPmusicalInterval.  Original function in PERL interface has
//      the same name.
//

void prepareInterval(Array<char>& data) {
   PerlRegularExpression pre;
   // step 1: check to see which components are present
   pre.search(data, "([Xx])?([MmPAd])?([0-9]+)?(.*)", "");
   Array<char> direction;
   Array<char> quality;
   Array<char> ssize;
   Array<char> other;

   direction.setSize(1);
   quality.setSize(1);
   ssize.setSize(1);
   other.setSize(1);
   
   direction[0] = '\0';
   quality[0]   = '\0';
   ssize[0]     = '\0';
   other[0]     = '\0';

   direction.setSize(0);
   quality.setSize(0);
   ssize.setSize(0);
   other.setSize(0);

   appendString(direction, pre.getSubmatch(1));
   appendString(quality,   pre.getSubmatch(2));
   appendString(ssize,     pre.getSubmatch(3));
   appendString(other,     pre.getSubmatch(4));

   if (strcmp(other.getBase(), ".") == 0) {
      data.setSize(0);
      appendString(data, "([Xx]?[mMPAd][1-9][0-9]?)");
      return;
   }

   if (strcmp(other.getBase(), "*") == 0) {
      data.setSize(0);
      appendString(data, "([Xx]?[mMPAd][1-9][0-9]?)*");
      return;
   }

   if ((strcmp(direction.getBase(), "") == 0) 
       && (strcmp(quality.getBase(), "P") != 0) 
       && (strcmp(ssize.getBase(), "1") != 0))  {
      direction.setSize(0);
      appendString(direction, "[Xx]");
   }

   if (strcmp(quality.getBase(), "") == 0) {
      quality.setSize(0);
      appendString(quality, "[mMPAd]");
   }

   if (strcmp(ssize.getBase(), "") == 0) {
      ssize.setSize(0);
      appendString(ssize, "[1-9][0-9]?");
   }

   data.setSize(1);
   data[0] = '\0';
   data.setSize(0);
   if (direction.getSize() > 0) {
      appendString(data, direction.getBase());
   }
   if (quality.getSize() > 0) {
      appendString(data, quality.getBase());
   }
   if (ssize.getSize() > 0) {
      appendString(data, ssize.getBase());
   }

   PerlRegularExpression pre2;

   // Perfect unisons do not have a direction so remove if present
   pre2.sar(data, "[X|x]P1$", "P1", "");

   if (other.getSize() > 0) {
      if (pre2.search(other, "\\?", "")) {
         Array<char> tempdata;
         tempdata.setSize(0);
         appendString(tempdata, "(");
         appendString(tempdata, data.getBase());
         appendString(tempdata, ")?");
         data.setSize(0);
         appendString(data, tempdata.getBase());
      }
   }


}



//////////////////////////////
//
// cleanPpitchClass -- based on processPitchQuery in original
//   PERL interface.  This function cleans user input so that it
//   can be used to search the data which is formatted in a particular way.
//

void cleanPpitchClass(Array<char>& data) {
   PerlRegularExpression pre;

   pre.sar(data,  "( |-)?is",    "#", "gi"); // replace German sharp
   pre.sar(data,  "( |-)?es",    "-", "gi"); // replace German flat

   pre.sar(data,  "( |-)?sharp", "#", "gi"); // replace acc names with symbols
   pre.sar(data,  "( |-)?flat",  "-", "gi"); // replace acc names with symbols
   
   // convert solfege syllables into English note names.
   int solfege = 0;                    // true if using solfege syllables
   if (pre.sar(data, "ut|do",  "C", "gi")) {  solfege = 1; }
   if (pre.sar(data, "re",     "D", "gi")) {  solfege = 1; }
   if (pre.sar(data, "mi",     "E", "gi")) {  solfege = 1; }
   if (pre.sar(data, "sol?",   "G", "gi")) {  solfege = 1; }
   if (pre.sar(data, "la",     "A", "gi")) {  solfege = 1; }
   if (pre.sar(data, "[ts]i",  "B", "gi")) {  solfege = 1; }
   // if any other solfege syllables have been used, then convert
   // "fa" into F; otherwise, assume that "fa" means "F A";
   if (solfege) {
      pre.sar(data, "fa", "F", "gi");
   }

   // remove duplicate spaces
   pre.sar(data, "\\s+",  " ",  "g"); 

   // adjust for aliases
   pre.tr (data, "\n\txa-hsSmM-",  "  XA-H##bbb");

   // expand double sharps
   pre.sar(data, "X",  "##",  "gi"); 

   // remove invalid chars
   pre.sar(data, "[^A-Hb# H(){},.?+^*0-9]",  "",  "g"); 

   // make sure {} operator has valid syntax
   cleanUpRangeSyntaxNoOutsideDigitsOrComma(data);

   // convert parentheses temporarily into letters and store 
   // real regex operators in temporary letter values
   pre.sar(data, "\\(", "Q", "g");
   pre.sar(data, "\\)", "q", "g");
   pre.sar(data, "q\\*", "qS", "g");
   pre.sar(data, "q\\+", "qP", "g");
   pre.sar(data, "q\\?", "qN", "g");
   

   // if a German B-natural is presents, map B->Bb and H->B
   if (pre.search(data.getBase(), "H")) {           
      pre.sar(data, "B", "Bb", "gi");     // transmute German B
      pre.sar(data, "H", "B",  "gi");     // transmute German H
   }

   // add spaces between notes
   pre.sar(data, "(?<=[^ ])(?=[A-GQq{.])",  " ",  "g");

   // change meaning of * which means match to zero or more notes.
   pre.sar(data, "\\*",  "[YZ]*",  "g");        

   // change meaning of "." which means any one pitch.
   pre.sar(data, "\\.",  "\\[A-G\\]\\[#-\\]\\*",  "g"); 

   // remove duplicate spaces again
   pre.sar(data, "\\s+", " ", "g");                 

   // put parens around each note, then clean up:
   
   // remove any leading spaces
   pre.sar(data, "^\\s+",  "");                   

   // remove any trailing spaces
   pre.sar(data, "\\s+$",  "");                   

   // if the -D option is specified, then than means the
   // pitch names can contain any accidental (the search query
   // should not have any accidentals listed).
   if (diatonicQ) {
      pre.sar(data, " ", "^ ", "g");
      pre.sar(data, "(?<=[A-G])$", "^", "g");
   }

   // put a parenthesis at very beginning and very end of string
   pre.sar(data, "^",  "(?:");               
   pre.sar(data, "$",  ")");               

   // now put parentheses at each space separating a pitch.
   pre.sar(data, " ",  ") (?:",  "g");

   // move "?" outside of parens
   pre.sar(data, "\\?\\) ",      ")? ",  "g");
   pre.sar(data, "\\?\\)$",      ")?"       );

   // fix the space/"?" association 
   pre.sar(data, "\\)\\?\\s",    " )?",  "g");   

   // fix the space/"+" association 
   pre.sar(data, "\\+\\)\\s",    " )+",  "g");    

   // change meaning of ^ which means any chromatic alteration of the note
   // (including no alteration), in other words search for diatonic part
   // with any chromatic alteration
   pre.sar(data, "\\^",  "(?:#+|b+)?",   "g");  

   // finalize change of *
   pre.sar(data, "Z",         "\t",      "g");                
   pre.sar(data, "Y",         "^",     "g");               
   pre.sar(data, "\\*\\) ",   "*)",      "g");      

   // change )( to ) (
   pre.sar(data, "\\)\\(",    ") (",     "g");      

   // get rid of null notes
   pre.sar(data, "\\(\\)",    "",        "g");

   // get rid of parentheses around {} operator
   pre.sar(data, "\\(\\?:\\{","{",       "g");
   pre.sar(data, "\\}\\)",    "}",       "g");

   // get rid of space before {} operator
   pre.sar(data, " \\{", "{", "g");

   pre.sar(data, "\\( ",        "(",     "g");
   pre.sar(data, " \\)",        ")",     "g");

   // convert Q and q back into parentheses
   pre.sar(data, "\\(\\?:Q\\) ",    "(", "g");
   pre.sar(data, "\\(\\?:q",        "q", "g");
   pre.sar(data, "(?<=q)\\) ?",     "",  "g");
   pre.sar(data, "(?<=q[^ )])\\) ?","",  "g");
   pre.sar(data, "q",               ")", "g");

   // put regex operators back to normal:
   pre.sar(data, "S", "*", "g");
   pre.sar(data, "P", "+", "g");
   pre.sar(data, "N", "?", "g");


   // thema command adds the space later, so get rid of any at the end 
   pre.sar(data, " $", "");                      

   // added 10 Dec 2000
   pre.sar(data, "^ +", "");

}



//////////////////////////////
//
// cleanUpRangeSyntax: disallow digits or commas to exist
//   anywhere outsize of the {} regular-expression operator.
//

void cleanUpRangeSyntaxNoOutsideDigitsOrComma(Array<char>& data) {
   int i;
   int inside = 0;
   SSTREAM tempdata;

   for (i=0; i<data.getSize()-1; i++) {
      if ((inside == 0) && (data[i] == '{')) {
         inside = 1;
         tempdata << data[i];
         continue;
      }
      if ((inside == 1) && (data[i] == '}')) {
         inside = 0;
         tempdata << data[i];
      }
      if (inside) {
         if (isdigit(data[i]) || (data[i] == ',')) {
            tempdata << data[i];
         }
         continue;
      } else {
         if (isdigit(data[i]) || (data[i] == ',') || (data[i] == '}') ||
               (data[i] == '{')) {
            continue;
         } else {
            tempdata << data[i];
            continue;
         }
      }
   }

   tempdata << ends;
   int len = strlen(tempdata.CSTRING);
   data.setSize(len+1);
   strcpy(data.getBase(), tempdata.CSTRING);
}


////////////////////////////////////////////////////////////////////////////
//
// smart search code ideas
//
// If --smart is added, then second argument is a **kern file which
// represents a search query as **kern data.   (or if no second argument,
// then first argument is the **kern file, and standard input is the 
// search index).
//
// * If there is a meter, then a search on pitches and metric position 
// filtered for that meter will be done.
//
// * If there is a key designation, then that key will be used to filter
// the keys.
//
// For a smart search, if the most explicit search returns no results, then
// other less strict search features will be searched.  For example if the
// exact pitches, duration and metric positions are not generating a match,
// then try pitches and durations without metric position.  If that returns
// no matches, then try 12tone pitch classes (enharmonic spellings), then
// try pitch without durations, allowing repeated notes between pitches,
// allowing various chromatic alterations of the pitches, then musical 
// intervals with durations, 12tone intervals with durations, 
// musical intervals without durations, refined contour with durations,
// refined contour with refined duration contour, gross contour with refined 
// duration contour, pitch gross contour with duration gross contour.
//

// md5sum: c3738863aae4c35f19bec9ff165b3249 themax.cpp [20090525]

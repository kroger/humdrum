//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Oct 27 10:42:08 PST 2008
// Last Modified: Fri Jun 26 11:23:35 PDT 2009 (converted from original version)
// Last	Modified: Thu Jul  2 15:59:04 PDT 2009 (intial PCRE implementation)
// Filename:      ...museinfo/examples/all/themax.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/themax.cpp
// Syntax:        C++; museinfo
//
// Description:   searches an index created by themebuilderx.  Themax
//                is a C++ implementation of the original thema command
//                which was written by David Huron in 1996/1998, and
//                modified with a few bug fixes during the implementation 
//                of Themefinder.org by Craig Sapp  (1999-2001).
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
// -a   anchor to start (implemented)
// -M   major keys      (implemented)
// -m   minor keys      (implemented)
// -t   mode            (implemented)
// -T   meter           (implemented)
//
// -i { 12-tone interval    (implemented)
// -c # refined contour	    (implemented)
// -C : gross contour	    (implemented)
// -d % scale degree        (implemented)
// -I } musical interval    (implemented)
// -P j 12-tone pitch class (implemented)
// -p J pitch-class name    (implamented)
//
// do assign an option letter:
//  ~ duration gross contour
//  ^ duration refined contour
//  ! duration (IOI)
//  & beat level
//  @ metric gross contour
//  ` metric refined contour
//  ' metric level
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


// function declarations:
void      checkOptions           (Options& opts, int argc, char** argv);
void      example                (void);
void      usage                  (const char* command);
void      appendString           (Array<char>& ss, const char* string);
void      appendToSearchString   (Array<char>& ss, const char* string, 
                                  char marker, int anchor);
void      showCleanedParameters  (void);
void      searchForMatches       (const char* filename, Array<char>& ss,
                                  PerlRegularExpression& re);


// user input sanitation functions:
void      cleanPpitchClass      (Array<char>& data);
void      cleanPgrossContour    (Array<char>& data);
void      cleanUpRangeSyntaxNoOutsideDigitsOrComma(Array<char>& data);



// User interface variables:
Options   options;

int         showcleanQ   = 0;       // used with --cleaned
int         verboseQ     = 0;       // used with -v option
int         cleanQ       = 1;       // used with --no-clean option
int         anchoredQ    = 0;       // used with -a option
int         keyfilterQ   = 0;       // used with -m -M and -t options
int         minorQ       = 0;       // used with -m option
int         majorQ       = 0;       // used with -M option
int         debugQ       = 0;       // used with --debug option
int         shortQ       = 0;       // used with --short option
int         tonicQ       = 0;       // used with -t option
const char* tonicstring  = "";      // used with -t option
int         meterQ       = 0;       // used with -T option
const char* meterstring  = "";      // used with -T option

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
int RmetricrefinedcontourQ        = 0;    // used with -e option
int RmetricgrosscontourQ          = 0;    // used with -E option

const char* Rgrosscontour         = "";   // used with -R option
const char* Rrefinedcontour       = "";   // used with -r option
const char* Rduration             = "";   // used with -D option
const char* Rbeatlevel            = "";   // used with -b option
const char* Rmetriclevel          = "";   // used with -L option
const char* Rmetricrefinedcontour = "";   // used with -e option
const char* Rmetricgrosscontour   = "";   // used with -E option


//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   checkOptions(options, argc, argv); // process the command-line options

   Array<char> searchstring;
   Array<char>& ss = searchstring;
   ss.setSize(10000);
   ss.setGrowth(10000);
   ss.setSize(0);

//   appendString(ss, "egrep ");

   // start regular expression string
//   appendString(ss, "'");

// order of data in index file:
//  [Zz] = major/minor key  //////////////////////////////////////////////////

   if (keyfilterQ) {
     if (majorQ && !minorQ) {
        appendString(ss, "Z");
     } else if (minorQ && !majorQ) {
        appendString(ss, "z");
     } else {
        appendString(ss, "[Zz]");
     }
     // place the tonic search next if given
     if (tonicQ) {
        appendString(ss, tonicstring);
     } else {
        appendString(ss, "[^=]*");
     }
     // place the end of key information marker:
     appendString(ss, "=.*");
   }


/*

//  {    = 12-tone interval //////////////////////////////////////////////////
   if (P12toneintervalQ) {
      appendToSearchString(ss, P12toneinterval.getBase(), 
         PITCH_TWELVE_TONE_INTERVAL_MARKER, anchoredQ);
   }

//  #    = pitch refined contour /////////////////////////////////////////////
   if (PrefinedcontourQ) {
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
      appendToSearchString(ss, Pscaledegree.getBase(), 
         PITCH_SCALE_DEGREE_MARKER, anchoredQ);
   }

//  }    = musical interval //////////////////////////////////////////////////
   if (PmusicalintervalQ) {
      appendToSearchString(ss, Pmusicalinterval.getBase(), 
         PITCH_MUSICAL_INTERVAL_MARKER, anchoredQ);
   }

//  j    = 12-tone pitch class ///////////////////////////////////////////////
   if (P12tonepitchclassQ) {
      appendToSearchString(ss, P12tonepitchclass.getBase(), 
         PITCH_TWELVE_TONE_MARKER, anchoredQ);
   }
   */

//  J    = pitch class name //////////////////////////////////////////////////
   if (PpitchclassQ) {
      if (cleanQ) { cleanPpitchClass(Ppitchclass); }
      appendToSearchString(ss, Ppitchclass.getBase(), 
         PITCH_CLASS_MARKER, anchoredQ);
   }
   /*

//  M    = metric description ////////////////////////////////////////////////
   if (meterQ) {
      appendString(ss, "M");
      if (!isdigit(meterstring[0])) {
         appendString(ss, "[^\t]*");
      }
      appendString(ss, meterstring);
      appendString(ss, ".*");
   }

// Added rhythmic markers:

//  ~    = duration gross contour  ///////////////////////////////////////////
   if (RgrosscontourQ) {
      appendToSearchString(ss, Rgrosscontour, 
         DURATION_GROSS_CONTOUR_MARKER, anchoredQ);
   }

//  ^    = duration refined contour  /////////////////////////////////////////
   if (RrefinedcontourQ) {
      appendToSearchString(ss, Rrefinedcontour, 
         DURATION_REFINED_CONTOUR_MARKER, anchoredQ);
   }

//  !    = duration (IOI)  ///////////////////////////////////////////////////
   if (RdurationQ) {
      appendToSearchString(ss, Rduration, 
         DURATION_IOI, anchoredQ);
   }

//  &    = beat level  ///////////////////////////////////////////////////////
   if (RbeatlevelQ) {
      appendToSearchString(ss, Rbeatlevel, 
         BEAT_LEVEL, anchoredQ);
   }

//  @    = metric gross contour  /////////////////////////////////////////////
   if (RmetriclevelQ) {
      appendToSearchString(ss, Rmetriclevel, 
         METRIC_LEVEL, anchoredQ);
   }

//  `    = metric refined contour  ///////////////////////////////////////////
   if (RmetricrefinedcontourQ) {
      appendToSearchString(ss, Rmetricrefinedcontour, 
         METRIC_REFINED_CONTOUR, anchoredQ);
   }

//  '    = metric level  /////////////////////////////////////////////////////
   if (RmetricgrosscontourQ) {
      appendToSearchString(ss, Rmetricgrosscontour, 
         METRIC_GROSS_CONTOUR, anchoredQ);
   }

*/

//////////////////////////////////////////////////////////////////////////////

   // terminate the search string
   char ch = '\0';
   ss.append(ch);
   if (debugQ) {
      cout << "### Search string: " << ss.getBase() << endl;
   }

   if (showcleanQ) {
      showCleanedParameters();
      cout << "Final Regular Expression: " << ss.getBase() << endl;
      exit(0);
   }

   PerlRegularExpression re;
   re.initializeSearchAndStudy(ss.getBase());
   ifstream inputfile;
   int i;
   for (i=1; i<=options.getArgCount(); i++) {
      searchForMatches(options.getArgument(i), ss, re);
   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// searchForMatches --
//

void searchForMatches(const char* filename, Array<char>& ss, 
      PerlRegularExpression& re) {
   string line;
   int i;
   int counter = 0;
   ifstream inputfile;
   inputfile.open(filename);
   if (!inputfile.is_open()) {
      return;
   }
   while (!inputfile.eof()) {
      getline(inputfile, line);
      if (re.search(line.c_str())) {
         counter++;
         if (verboseQ) {
            cout << "Matches in " << filename << endl;
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
}



//////////////////////////////
//
// showCleanedParameters --  Show what the post processed input search
//    parameters look like for each search field (if it was used).
//

void showCleanedParameters(void) {
   if (Ppitchclass.getSize() > 1) {
      cout << "Pitch-class        ::" << Ppitchclass.getBase() << "::" << endl;
   }
   if (Pgrosscontour.getSize() > 1) {
      cout << "Pitch gross contour::" << Pgrosscontour.getBase() << "::" 
           << endl;
   }
}



//////////////////////////////
//
// appendToSearchString
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

   // add [^\t]* if not anchored:
   if (!anchor) {
      appendString(ss, "[^\t]*");
   }
   appendString(ss, string);

   // for pitch-class names, the next character after the search
   // string must be a space or a tab to prevent accidentals
   // from matching on natural-note search endings.
//   if (marker == PITCH_CLASS_MARKER) {
//      appendString(ss, "[ \t]");
//   }

//   appendString(ss, ".*");
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
}



//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
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

   opts.define("R|duration-gross-contour=s:",   "duration gross contour");
   opts.define("r|duration-refined-contour=s:", "duration refined contour");
   opts.define("D|duration=s:",                 "duration (IOI)");
   opts.define("b|beat-level=s:",               "beat level");
   opts.define("L|metric-level=s:",             "metric level");
   opts.define("e|metric-refined-contour=s:",   "metric refined contour");
   opts.define("E|metric-gross-contour=s:",     "metric-gross contour");

   //opts.define("i|pitch-twelvetone-interval=s:", "12-tone interval");

   opts.define("v|verbose=b",                   "verbose display");
   opts.define("no-clean=b",                    "do not preprocess searches");
   opts.define("cleaned=b",                     "show cleaned input fields");
   opts.define("short=b",            "display only filename in output");

   opts.define("debug=b",                       "debugging statements");

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
   cleanQ                 = !opts.getBoolean("no-clean");
   showcleanQ             =  opts.getBoolean("cleaned");
   anchoredQ              =  opts.getBoolean("anchored");
   debugQ                 =  opts.getBoolean("debug");
   majorQ                 =  opts.getBoolean("major");
   minorQ                 =  opts.getBoolean("minor");
   tonicQ                 =  opts.getBoolean("tonic");
   tonicstring            =  opts.getString("tonic");
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
   RmetricrefinedcontourQ = opts.getBoolean("metric-refined-contour");
   RmetricgrosscontourQ   = opts.getBoolean("metric-gross-contour");

   Rgrosscontour          = opts.getString("duration-gross-contour");
   Rrefinedcontour        = opts.getString("duration-refined-contour");
   Rduration              = opts.getString("duration");
   Rbeatlevel             = opts.getString("beat-level");
   Rmetriclevel           = opts.getString("metric-level");
   Rmetricrefinedcontour  = opts.getString("metric-refined-contour");
   Rmetricgrosscontour    = opts.getString("metric-gross-contour");

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


//////////////////////////////
//
// cleanPgrossContour --
//

void cleanPgrossContour(Array<char>& data) {
   PerlRegularExpression re;
   // remove invalid characters
   re.sar(data, "[^0-9/\\\\UuDdSs*+.{},-]", "", "g");  

   // collapse alias chars
   re.tr (data, "-/\\\\uds",  "SUDUDS"); 
}



//////////////////////////////
//
// cleanPpitchClass -- based on processPitchQuery in original
//   PERL interface.  This function cleans user input so that it
//   can be used to search the data which is formatted in a particular way.
//

void cleanPpitchClass(Array<char>& data) {
   PerlRegularExpression re;

   re.sar(data,  "( |-)?is",    "#", "gi"); // replace German sharp
   re.sar(data,  "( |-)?es",    "#", "gi"); // replace German flat

   re.sar(data,  "( |-)?sharp", "#", "gi"); // replace acc names with symbols
   re.sar(data,  "( |-)?flat",  "-", "gi"); // replace acc names with symbols
   
   // convert solfege syllables into English note names.
   int solfege = 0;                    // true if using solfege syllables
   if (re.sar(data, "ut|do",  "C", "gi")) {  solfege = 1; }
   if (re.sar(data, "re",     "D", "gi")) {  solfege = 1; }
   if (re.sar(data, "mi",     "E", "gi")) {  solfege = 1; }
   if (re.sar(data, "sol?",   "G", "gi")) {  solfege = 1; }
   if (re.sar(data, "la",     "A", "gi")) {  solfege = 1; }
   if (re.sar(data, "[ts]i",  "B", "gi")) {  solfege = 1; }
   // if any other solfege syllables have been used, then convert
   // "fa" into F; otherwise, assume that "fa" means "F A";
   if (solfege) {
      re.sar(data, "fa", "F", "gi");
   }

   // remove duplicate spaces
   re.sar(data, "\\s+",  " ",  "g"); 

   // adjust for aliases
   re.tr (data, "\n\txa-hsSmM-",  "  XA-H##bbb");

   // expand double sharps
   re.sar(data, "X",  "##",  "g"); 

   // remove invalid chars
   re.sar(data, "[^A-Hb# H(){},.?+^*0-9]",  "",  "g"); 

   // make sure {} operator has valid syntax
   cleanUpRangeSyntaxNoOutsideDigitsOrComma(data);

   // convert parentheses temporarily into letters and store 
   // real regex operators in temporary letter values
   re.sar(data, "\\(", "Q", "g");
   re.sar(data, "\\)", "q", "g");
   re.sar(data, "q\\*", "qS", "g");
   re.sar(data, "q\\+", "qP", "g");
   re.sar(data, "q\\?", "qN", "g");
   

   // if a German B-natural is presents, map B->Bb and H->B
   if (re.search(data.getBase(), "H")) {           
      re.sar(data, "B", "Bb", "gi");     // transmute German B
      re.sar(data, "H", "B",  "gi");     // transmute German H
   }

   // add spaces between notes
   re.sar(data, "(?<=[^ ])(?=[A-GQq{.])",  " ",  "g");

   // change meaning of * which means match to zero or more notes.
   re.sar(data, "\\*",  "[YZ]*",  "g");        

   // change meaning of "." which means any one pitch.
   re.sar(data, "\\.",  "\\[A-G\\]\\[#-\\]\\*",  "g"); 

   // remove duplicate spaces again
   re.sar(data, "\\s+", " ", "g");                 

   // put parens around each note, then clean up:
   
   // remove any leading spaces
   re.sar(data, "^\\s+",  "");                   

   // remove any trailing spaces
   re.sar(data, "\\s+$",  "");                   

   // put a parenthesis at very beginning and very end of string
   re.sar(data, "^",  "(?:");               
   re.sar(data, "$",  ")");               

   // now put parentheses at each space separating a pitch.
   re.sar(data, " ",  ") (?:",  "g");

   // move "?" outside of parens
   re.sar(data, "\\?\\) ",      ")? ",  "g");
   re.sar(data, "\\?\\)$",      ")?"       );

   // fix the space/"?" association 
   re.sar(data, "\\)\\?\\s",    " )?",  "g");   

   // fix the space/"+" association 
   re.sar(data, "\\+\\)\\s",    " )+",  "g");    

   // change meaning of ^ which means any chromatic alteration of the note
   // (including no alteration), in other words search for diatonic part
   // with any chromatic alteration
   re.sar(data, "\\^",  "(?:#+|b+)?",   "g");  

   // finalize change of *
   re.sar(data, "Z",         "\t",      "g");                
   re.sar(data, "Y",         "^",     "g");               
   re.sar(data, "\\*\\) ",   "*)",      "g");      

   // change )( to ) (
   re.sar(data, "\\)\\(",    ") (",     "g");      

   // get rid of null notes
   re.sar(data, "\\(\\)",    "",        "g");

   // get rid of parentheses around {} operator
   re.sar(data, "\\(\\?:\\{","{",       "g");
   re.sar(data, "\\}\\)",    "}",       "g");

   // get rid of space before {} operator
   re.sar(data, " \\{", "{", "g");

   re.sar(data, "\\( ",        "(",     "g");
   re.sar(data, " \\)",        ")",     "g");

   // convert Q and q back into parentheses
   re.sar(data, "\\(\\?:Q\\) ",    "(", "g");
   re.sar(data, "\\(\\?:q",        "q", "g");
   re.sar(data, "(?<=q)\\) ?",     "",  "g");
   re.sar(data, "(?<=q[^ )])\\) ?","",  "g");
   re.sar(data, "q",               ")", "g");

   // put regex operators back to normal:
   re.sar(data, "S", "*", "g");
   re.sar(data, "P", "+", "g");
   re.sar(data, "N", "?", "g");


   // thema command adds the space later, so get rid of any at the end 
   re.sar(data, " $", "");                      

   // added 10 Dec 2000
   re.sar(data, "^ +", "");
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








// md5sum: c3738863aae4c35f19bec9ff165b3249 themax.cpp [20090525]

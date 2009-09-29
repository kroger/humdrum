//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Oct 11 02:27:50 PDT 2008
// Last Modified: Wed Oct 15 08:45:23 PDT 2008 (adding tuplet rhythm parsing)
// Last Modified: Fri Oct 17 19:00:46 PDT 2008 (added layers, invisible, caut.)
// Last Modified: Sun Oct 26 04:58:48 PST 2008 (added misc features)
// Last Modified: Wed Dec  3 20:19:29 PST 2008 (end of data format fix)
// Last Modified: Wed Jun 24 15:37:21 PDT 2009 (updated for GCC 4.4)
// Last Modified: Mon Jul 20 16:32:01 PDT 2009 (added 8ba treble clef)
// Last Modified: Mon Jul 20 16:32:01 PDT 2009 (added M:none time signature)
// Last Modified: Wed Jul 22 13:03:44 PDT 2009 (added editorial slur dashing)
// Last Modified: Mon Aug 24 14:00:34 PDT 2009 (more work on 8ba treble clef)
// Last Modified: Mon Sep 21 17:13:53 PDT 2009 (added --no-tempo option)
// Last Modified: Mon Sep 21 17:29:50 PDT 2009 (accidental spelling by octave)
// Filename:      ...sig/examples/all/hum2abc.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/hum2abc.cpp
// Syntax:        C++; museinfo
//
// Description:   Converts Humdrum files into ABC+ (polyphonic ABC) files
//
// Note: Not completed yet, particularly dynamics and lyrics.
//
// Todo:	   
//                 lyrics
//                 add local comments as [r: comment] fields
//                 cross staff stemming/beaming?
//                 up/down stem controls (if possible)
//                 display numeric plots underneath a staff
//                 dynamics
//                 figured bass
//                 suppress accidentals on tied notes after barlines
//                 handle invisible articulations 
//                 allow trills on a single note in a chord (e.g. s03-1;m23)
//                 don't display accidental on tied notes (_ and ])
//                 don't store accidental state of tied notes
//                 some characters with accents (incomplete list implemented)
//                 allow fermatas on barlines (including final / repeats)
//
// Done in v1.0.0  accidentals
//                 measure numbers can be boxed with --box option
//                 invisible rests using "y"
//                 added option to print invisible rests and other objects
//                 first/second endings
//                 cautionary accidentals (with X)
//                 deal with tenor and etc clef as an initial clef
//                 printing invisible rests in second layer
//                 print the first barnumber if there a pickup and barnum'g = 1
//                 multi-measure rests
//		   tuplet rhythms (make rhythms more robust)
//                 notes w/no rhythms are displayed as quarter notes w/no stems
//		   break tuplet rhythm groupings by beaming
//                 added veritas checking/marking
//		   breaking subbeams
//                 add brokenrhythms   A>B = A3/2B/2
//                 abc+ extended information records
//                 -sharp -flat -natural map to \# \b \= in titles; \b\= no work
//                 slurs (some bugs in abcm2ps rendering of dashed slurs)
//                 ties
//                 gracenotes (with [q] and without [Q] slashes)
//                 phrases
//                 piano system automatic identification (partially done)
//                 clef changes during music
//                 time signature changes during the piece
//                 add rhythm shorthands / = /2, // = /4
//                 key signtures changes during piece
//                 beaming
//                 ornaments
//                 articulations
//                 if the initial keysignature comes after the
//                    first barline, but before the music, set it
//                    as the initial meter in the header.
//
// Notes:
//
// * abcm2ps src:  http://trillian.mit.edu/~jc/music/abc/src/abcm2ps/abc2ps.c
// * Format parameters for abcm2ps 5.4.4: 
//     http://br.geocities.com/hfmlacerda/abc/format.html
// * Creating new symbols in abcm2ps:
//     br.geocities.com/hfmlacerda/abc/decomanual-en.pdf 
// * interesting webpage: http://www.ucolick.org/~sla/abcmusic/piano/piano.html
// 
// * http://www.formulus.com/hymns/abcm2ps.txt
//    alto1= C1 soprano cleff, alto2 = C2 mezzo-soprano clef
//    bass3 = F3
//
// * Use the F: record to encode the File URL
//   e.g.: hum2abc -F "http://this.location" file.krn > file.abc
// 

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>

#ifndef OLDCPP
   #include <iostream>
   #include <sstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
#else
   #include <iostream.h>
   #ifdef VISUAL
      #include <strstrea.h>
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif

#include "humdrum.h"

#define EMPTY '\0'
#define TOLERANCE 0.0001

#define AA ('A'-'A')
#define BB ('B'-'A')
#define CC ('C'-'A')
#define DD ('D'-'A')
#define EE ('E'-'A')
#define FF ('F'-'A')
#define GG ('G'-'A')
#define HH ('H'-'A')
#define II ('I'-'A')
#define JJ ('J'-'A')
#define KK ('K'-'A')
#define LL ('L'-'A')
#define MM ('M'-'A')
#define NN ('N'-'A')
#define OO ('O'-'A')
#define PP ('P'-'A')
#define QQ ('Q'-'A')
#define RR ('R'-'A')
#define SS ('S'-'A')
#define TT ('T'-'A')
#define UU ('U'-'A')
#define VV ('V'-'A')
#define WW ('W'-'A')
#define XX ('X'-'A')
#define YY ('Y'-'A')
#define ZZ ('Z'-'A')

//////////////////////////////////////////////////////////////////////////

class Coordinate {
   public:
      int row;
      int col;
};

class VoiceMap {
   public:
      VoiceMap(void);
     ~VoiceMap();
      int primary;               // primary spine of the staff
      int voicenumber;           // voice number in abc output
      void clear(void);
      // store dynamics location 
      // store lyrics location(s)
};

VoiceMap::VoiceMap(void) {
   clear();
}

VoiceMap::~VoiceMap() {
   clear();
}

void VoiceMap::clear(void) {
   primary = -1;
   voicenumber = -1;
}

class MeasureInfo {
   public:
      MeasureInfo(void);
     ~MeasureInfo();
      void clear(void);
      int startline;              // line number in the humdrum file which
                                  // start the measure (with a barline)
      int endline;                // line number in the humdrum file which
                                  // ends the measure (may be a barline or not)
      int measurenum;             // an explicit number found for the barline
      int newkey;                 
      int currkey;
      int fullrest;               // -1 or 0 if not a full measure of rest
                                  // otherwise a count of the number of
                                  // full measures of rests including
                                  // the current one which have occured
                                  // in sequence before the current one.
      int ending;                 // -1 if not the start of an ending
                                  // 0 if the end of a set of repeat
                                  // endings.  1 or greater if the start
                                  // of a repeat.
};

MeasureInfo::MeasureInfo(void) {
   clear();
}

MeasureInfo::~MeasureInfo() {
   clear();
}

void MeasureInfo::clear(void) {
   startline  = -1;
   endline    = -1;
   measurenum = -1;
   newkey     = -100;
   currkey    = -100;
   fullrest   = -100;
   ending     = -1;
}
   
class TupletInfo {
   public:
      TupletInfo(void);
     ~TupletInfo();
      void clear(void);
      int top;
      int bot;
      int shortcut;
      int count;
};

TupletInfo::TupletInfo(void) {
   clear();
}

TupletInfo::~TupletInfo() {
   clear();
}

void TupletInfo::clear(void) {
   shortcut = count = top = bot = -1;
}

class BeamInfo {
   public:
        BeamInfo(void);
       ~BeamInfo();
   void clear(void);
   int left;                   // number of beams on left side of note
   int right;                  // number of beams on right side of note
   // maybe add flags as well
};

BeamInfo::BeamInfo(void) {
   clear();
}

BeamInfo::~BeamInfo() {
   clear();
}

void BeamInfo::clear(void) {
   left = right = -1;
}



//////////////////////////////////////////////////////////////////////////

// function declarations:
void      checkOptions         (Options& opts, int argc, char** argv, 
                                int fcount, HumdrumFile& cinfile);
void      storeOptionSet       (Options& opts);
void      example              (void);
void      usage                (const char* command);
void      printHeader          (ostream& out, HumdrumFile& infile,
                                Array<char*>& header,
                                Array<MeasureInfo>& measures, int xval, 
                                const char* filename);
void      parseBibliographic   (Array<char*>& header, HumdrumFile& infile);
void      convertHumdrumToAbc  (ostream& out, HumdrumFile& infile, int xval,
                                const char* filename);
void      calculateBestRhythmUnit(HumdrumFile& infile, int& Ltop, int& Lbot);
void      getBibPieces         (const char* string, const char* string2, 
		                  Array<char>& marker,  Array<char>& contents);
void      storeHeaderRecord    (Array<char*>& header, char recordletter, 
                                const char* string);
void      translateSpecialCharacters(Array<char>& output, const char* input);
void      calculateQRecord     (Array<char>& QRecord, double tempo, 
                                const char* omdstring, int top, int bot);
void      printBody            (ostream& out, HumdrumFile& infile,
                                Array<MeasureInfo>& measures);
void      createVoiceMap       (Array<VoiceMap>& voicemap, HumdrumFile& infile);
void      printVoiceDeclaration(ostream& out, VoiceMap vmap, int voicenum, 
                                HumdrumFile& infile);
void      getMeasureInfo       (Array<MeasureInfo>& measures, 
                                HumdrumFile& infile);
void      printMeasures        (ostream& out, HumdrumFile& infile, 
                                Array<MeasureInfo>& measures, 
                                int startmeasure, int endmeasure,
				Array<VoiceMap>& voicemap);
void      printSingleMeasure   (ostream& out, const Array<VoiceMap>& voiceinfo, 
                                int vindex, int vmapsize, 
                                Array<MeasureInfo>& measureinfo, 
                                int mindex, HumdrumFile& infile, 
                                int staffnumber);
void      buildNoteAddresses   (Array<Array<Coordinate> >& noteaddresses,
                                HumdrumFile& infile, const VoiceMap& voiceinfo, 
                                const MeasureInfo& measureinfo);
void      printLayer           (int layer, ostream& out, 
                                const VoiceMap& voiceinfo, 
                                const MeasureInfo& measureinfo, 
                                HumdrumFile& infile, 
                                Array<Array<Coordinate> >& address,
                                Array<Coordinate>& meterclef);
int       getMaxLayer          (int track, int start, int endd, 
                                HumdrumFile& infile);
void      printMeasureLine     (ostream& out, HumdrumFile& infile, int line,
                                int primary, int staffnumber,
                                Array<MeasureInfo>& measureinfo, int mindex,
                                int vindex);
void      getNoteDurations     (Array<double>& notedurations, 
		                Array<int>& iindex, int layer, 
                                Array<Array<Coordinate> >& address, 
                                HumdrumFile& infile);
void      printKernTokenAsAbc  (ostream& out, HumdrumFile& infile, int row, 
                                int col, Array<int>& accident, double notedur, 
                                int brokenq, double brokendur, int top, 
                                int bot, int& slursuppress, int groupflag);
void      getBrokenRhythms     (Array<int>& broken, Array<double>& brokendurs, 
                                Array<Coordinate>& nogracelist, 
                                Array<double>& nogracedurs, 
                                HumdrumFile& infile);
int       getBeamState         (const char* token);
char*     base40ToAbcPitch     (char* buffer, int base40);
int       countDots            (char* buffer);
int       getRhythm            (const char* buffer);
void      printKeySignature    (ostream& out, HumdrumFile& infile);
void      setAccidentals       (Array<int>& accident, int key);
void      printAbcKeySignature (ostream& out, int keynum);
void      printAccidental      (ostream& out, int base40, const char* token, 
                                Array<int>& accident);
void      adjustAccidentalStates(Array<int>& accident, int step, 
                                Array<Array<Coordinate> >& address, 
                                HumdrumFile& infile);
void      adjustAccidentalStates2(Array<int>& accident, HumdrumFile& infile, 
                                int row, int col);
void      printArticulations   (ostream& out, const char* string);
void      getMeterAndClefChanges(Array<Coordinate>& meterchanges, 
                                HumdrumFile& infile, const VoiceMap& voiceinfo,
                                const MeasureInfo& measureinfo);
void      printMeterAndClefChanges(ostream& out, int testrow, 
                                Array<Coordinate>& meterclef, 
                                HumdrumFile& infile);
void      printAbcClef         (ostream& out, const char* string, int track,
                                Array<int>& cleftranspose);
void      printAbcMeter        (ostream& out, const char* string);
int       checkForAllTies      (HumdrumFile& infile, int row, int col);
int       findRecord           (const char* key, Array<char>& value, 
                                HumdrumFile& infile, Array<int>& bibfields);
int       findMultipleRecords  (const char* key, Array<Array<char> >& values, 
                                HumdrumFile& infile, Array<int>& bibfields);
void      printAbcExtendedInformationFields(ostream& out, HumdrumFile& infile);
void      primeFactorization   (Array<int>& factors, int input);
void      printRhythm1         (ostream& out, char* buffer, int top, int bot);
int       printRhythm2         (ostream& out, double dur, int brokenstate, 
                                double brokendur, int top, int bot, 
                                int chordQ, const char* token);
void      simplifyFraction     (int& top, int& bot);
void      printDurationAsRhythm(ostream& out, double dur, int top, int bot);
void      getNoGraceList       (Array<Coordinate>& notelist, 
                                Array<Array<Coordinate> >& address, 
                                int layer, HumdrumFile& infile);
void      getNoGraceDurs       (Array<double>& nogracedurs, 
                                Array<double>& notedurs, 
                                Array<Array<Coordinate> >& address, 
                                int layer);
void      identifyTuplets      (Array<TupletInfo>& tupletstuff, 
                                Array<double>& nogracedurs, 
                                Array<Coordinate>& nogracelist, 
	                        HumdrumFile& infile, Array<int>& broken, 
                                Array<double>& brokendurs,
                                Array<BeamInfo>& beaminfo);
int       getTupletInfo        (TupletInfo& tupletsutff, 
                                Coordinate& nogracelist, HumdrumFile& infile);
int       getNonDuplePrimes    (Array<int>& factors);
int       getDuplePrimes       (Array<int>& factors);
int       getNextLowerPowerOfTwo(int number);
void      identifyTupletShortCuts(Array<TupletInfo>& tupletstuff,
                                Array<BeamInfo>& beaminfo, 
                                Array<double>& nogracedurs,
                                Array<Coordinate>& nogracelist,
                                HumdrumFile& infile);
void      printTupletInfo      (ostream& out, TupletInfo& tinfo);
void      flipCommaParts       (Array<char>& contents);
int       roundTempoToNearestStandard(double tempo);
int       characterCount       (const char* string, char character);
void      getBeamInfo          (Array<BeamInfo>& beaminfo, 
                                Array<Coordinate>& nogracelist, 
                                HumdrumFile& infile);
void      separateCountsByBeams(Array<TupletInfo>& tupletstuff, 
                                Array<BeamInfo>& beaminfo,
                                Array<Coordinate>& nogracelist,
                                HumdrumFile& infile);
void      identifyWholeRestsInMeasures(Array<MeasureInfo>& measures, 
                                VoiceMap& voicemap, HumdrumFile& infile);
int       checkForWholeRestInMeasure(int startline, int endline, int primary,
                                HumdrumFile& infile);
void      printMultiRest       (ostream& out, Array<MeasureInfo>& measureinfo, 
                                int mindex);
void      printInvisibleRest   (ostream& out, double duration);
void      getRepeatInfo        (Array<int>& segments, Array<int>& repeatinfo, 
                                HumdrumFile& infile);
int       checkForRepeatMeasure(HumdrumFile& infile, Array<int>& segments, 
                                Array<int>& repeatinfo, int mindex);
int       getFirstMeasureNumber(HumdrumFile& infile);
void      printVoiceClef       (ostream& out, VoiceMap voicemap, 
		                HumdrumFile& infile);
void      getRestInfo          (Array<int>& rests, 
                                Array<Coordinate>& nogracelist, 
                                HumdrumFile& infile);
unsigned long string_cksum     (const char* buf, int length);
void     printVeritas          (ostream& out, HumdrumFile& infile);
void     prepareBibliographicData(Array<Array<char> >& markers, 
                                Array<Array<char> >& contentses, 
                                HumdrumFile& infile);
void     addContentToString    (Array<char>& newtitle, Array<char>& key, 
                                Array<Array<char> >& markers, 
                                Array<Array<char> >& contentses);
void     printNumberFromString (ostream& out, const char* filename, 
                                const char* ending);
void     printFilenameBase     (ostream& out, const char* filename);
int      norhythm              (const char* buffer);
void     getFileListing        (Array<Array<char> >& filelist, 
		                const char* directoryname, 
                                const char* filemask);
int      sortfiles             (const void* a, const void* b);
void     printGraceRhythm      (ostream& out, const char* buffer, 
		                int top, int bot, int groupflag);
int     getGraceNoteGroupFlag  (Array<double>& notedurs, int index);
void    checkForLineBreak      (ostream& out, HumdrumFile& infile, 
                                int row, int col);

// User interface variables:
Options options;
int    debugQ    = 0;             // used with --debug option
int    labelQ    = 0;             // used with --label option
int    veritasQ  = 1;             // used with --no-veritas option
int    continueQ = 1;             // used with --no-autoformat option
int    boxQ      = 0;             // used with --box option
int    notespacingQ = 0;          // used with --spacing option
double notespacing  = 0;          // used with --spacing option
int    linemeasure  = 4;          // used with -m option
int    barnumberingstyle = 0;     // used with -n option
int    invisibleQ   = 1;          // used with --no-invisible
int    footerQ      = 0;          // used with -f option
const char* footer   = "";        // used with -f option
int    headerQ      = 0;          // used with -h option
const char* header   = "";        // used with -h option
int    musicscaleQ  = 0;          // used with -s option
double musicscale   = 0.75;       // used with -s option
int    landscapeQ   = 0;          // used with --landscape option
const char* parameterstring = ""; // used with -p option
const char* invisiblerest = "x";  // used with --no-invisible option
int    filenumQ     = 0;          // used with --filenum option
const char* filenumstring = "";   // used with --filenum option
int    filenametitleQ = 0;        // used with --filetitle option
int    titleexpansionQ = 0;       // used with --TT option
const char* titleexpansion = "";  // used with --TT option
int    graceQ = 1;                // used with --no-grace option
int    directoryQ = 0;            // used with --dir option
const char* directoryname = ".";  // used with --dir option
const char* filemask = ".krn";    // used with --mask option
int    nonaturalQ  = 0;           // used with --nn option
int    linebreakQ  = 0;           // used with --linebreak option
int    notempoQ    = 0;           // used with --no-tempo option

Array<char*> Header;


// score variables
int Ltop = 1;              // used for creating rhythm values
int Lbot = 4;              // used for creating rhythm values
int StartKey = 0;          
Array<VoiceMap> Voicemap;  // used printing polyphonic data
Array<int> clefstates;     // keep track of transpositions for tenor clef
int stemlessQ = 0;         // used with -p "%%nostems" spoofing

//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   HumdrumFile hfile;

   // initial processing of the command-line options
   checkOptions(options, argc, argv, 1, hfile);

   int i = 0;
   if (directoryQ) {
      Array<Array<char> > filelist;
      getFileListing(filelist, directoryname, filemask);

      for (i=0; i<filelist.getSize(); i++) {
         hfile.read(filelist[i].getBase());
         checkOptions(options, argc, argv, i+1, hfile);
         convertHumdrumToAbc(cout, hfile, i+1, filelist[i].getBase());
         if (i < filelist.getSize() - 1) {
            cout << "\n\n\n";
         }
      }

   } else if (options.getArgumentCount() == 0) {
      convertHumdrumToAbc(cout, hfile, 1, "");
   } else {
      for (i=1; i<=options.getArgumentCount(); i++) {
         // process the command-line options
         checkOptions(options, argc, argv, i, hfile);
         hfile.read(options.getArg(i));
         convertHumdrumToAbc(cout, hfile, i, options.getArg(i));
         if (i < options.getArgumentCount()) {
            cout << "\n\n\n";
         }
      }
   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////



//////////////////////////////
//
// getFileListing --
//

void getFileListing(Array<Array<char> >& filelist, const char* directoryname, 
      const char* filemask) {

   filelist.setSize(1000000);
   filelist.setGrowth(1000000);
   filelist.setSize(0);
   int length;
   int dirlen = strlen(directoryname);
   const char* connector = "/";
   if (directoryname[dirlen-1] == '/') {
      connector = "";
   }
   int conlen = strlen(connector);
   int storlen;

   DIR *d;
   struct dirent *dir;
   d = opendir(directoryname);

   if (!d) {
      cerr << "Error: could not open directory " << directoryname
           << " for reading." << endl;
      exit(1);
   }

   while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, filemask) == NULL)  {
         // ignore files which do not match to mask.
         continue;
      }
      filelist.setSize(filelist.getSize()+1);
      length = strlen(dir->d_name);
      storlen = length + dirlen + conlen + 1;
      filelist[filelist.getSize()-1].setSize(storlen);
      strcpy(filelist[filelist.getSize()-1].getBase(), directoryname);
      strcat(filelist[filelist.getSize()-1].getBase(), connector);
      strcat(filelist[filelist.getSize()-1].getBase(), dir->d_name);
   }
   closedir(d);


   // sort the filelist (it is not sorted currently)
   
   qsort(filelist.getBase(), filelist.getSize(), sizeof(Array<char>), 
      sortfiles); 

}



//////////////////////////////
//
// sortfiles --
//

typedef Array<char> chararray;

int sortfiles(const void* a, const void* b) {
   return strcmp((((chararray*)a))->getBase(), (((chararray*)b))->getBase());
}



//////////////////////////////
//
// convertHumdrumToAbc --
//

void convertHumdrumToAbc(ostream& out, HumdrumFile& infile, int xval,
      const char* filename) {
   clefstates.setSize(infile.getMaxTracks()+1);
   clefstates.setAll(0);
   clefstates.allowGrowth(0);
   infile.analyzeRhythm("4");
   Array<MeasureInfo> measures;
   createVoiceMap(Voicemap, infile);
   getMeasureInfo(measures, infile);

   printHeader(out, infile, Header, measures, xval, filename);
   printBody(out, infile, measures);
}



//////////////////////////////
//
// printBody -- print the musical data for the score.
//

void printBody(ostream& out, HumdrumFile& infile, 
      Array<MeasureInfo>& measures) {

   // print on a single line if requested by the
   // user here; otherwise, currently print in
   // 4-measure chunks.

   int i;
   if (debugQ) {
      out << "%\n% Measure information in input file:\n";
      out << "%\tindex\tnumber\tstart\tstop\tlines\tcurkey\tnewkey\n";
      for (i=0; i<measures.getSize(); i++) {
         out << "%\tM:" << i << "\t" 
             << measures[i].measurenum << "\t"
             << measures[i].startline  << "\t"
             << measures[i].endline    << "\t"
             << measures[i].endline - measures[i].startline << "\t"
             << measures[i].currkey    << "\t"
             << measures[i].newkey
	     << endl;
      } 
      out << "%\n";
   }

   int increment = linemeasure;
   int maxmeasure;
   for (i=0; i<measures.getSize(); i+=increment) {
      maxmeasure = i+increment;
      if (maxmeasure >= measures.getSize()) {
         maxmeasure = measures.getSize();
      }
      printMeasures(out, infile, measures, i, maxmeasure, Voicemap);
   }
}



//////////////////////////////
//
// printMeasures --
//

void printMeasures(ostream& out, HumdrumFile& infile, 
      Array<MeasureInfo>& measures, int startmeasure, int stopmeasure, 
      Array<VoiceMap>& voicemap) {
 
   SSTREAM staves[voicemap.getSize()];


   int i, j;
   for (i=startmeasure; i<stopmeasure; i++) {
      for (j=0; j<voicemap.getSize(); j++) {
         printSingleMeasure(staves[j], voicemap, j, voicemap.getSize(), 
               measures, i, infile, j);
      }
   }

   for (i=0; i<voicemap.getSize(); i++) {
      staves[i] << ends;
      if (staves[i].CSTRING[0] == '\0') {
         continue;
      }
      if (voicemap.getSize() > 1) {
         out << "[V:" << voicemap[i].voicenumber << "] ";
         out << staves[i].CSTRING << "\n";
      } else {
         // multi-line rests will generate blank lines, so
         // suppress them here when blank lines occur.
         if (strlen(staves[i].CSTRING) > 0) {
            out << staves[i].CSTRING << "\n";
         }
      }
   }

}



//////////////////////////////
//
// printSingleMeasure --
//

void printSingleMeasure(ostream& out, const Array<VoiceMap>& voiceinfo, 
      int vindex, int vmapsize, Array<MeasureInfo>& measureinfo, int mindex, 
      HumdrumFile& infile, int staffnumber) {

   MeasureInfo& MI = measureinfo[mindex];

   int maxlayers = getMaxLayer(voiceinfo[vindex].primary, MI.startline,
      MI.endline, infile);

   if (MI.newkey > -10) {
      out << " [K:";
      printAbcKeySignature(out, MI.newkey);
      out << "] ";
   }

   Array<Coordinate> meterclef;
   getMeterAndClefChanges(meterclef, infile, voiceinfo[vindex], MI);

   Array<Array<Coordinate> > noteaddresses;
   noteaddresses.setSize(maxlayers);
   noteaddresses.allowGrowth(0);
   buildNoteAddresses(noteaddresses, infile, voiceinfo[vindex], MI);

   if (noteaddresses.getSize() == 0) {
      // no notes to print in this measure
      return;
   }

   int multirestQ = 0;
   if ((vmapsize == 1) && measureinfo[mindex].fullrest > 1) {
      if (mindex < measureinfo.getSize() - 1) {
         if (measureinfo[mindex+1].fullrest < 1) {
            multirestQ = 1;
         } else {
            multirestQ = 0;
         }
      } else {
         multirestQ = 1;
      }
   }

   if (multirestQ) {
      // print a multi-rest if at the end of a string
      // of measures of full rests.  This algorithm
      // is not fully generalized, and needs to be 
      // extended eventually.  It will absorb repeat
      // barlines, which is should not do.  It will also
      // count split bars as two measures instead of
      // one at the moment.
      printMultiRest(out, measureinfo, mindex);
      if (infile[MI.endline].getType() == E_humrec_data_measure) {
         printMeasureLine(out, infile, MI.endline, voiceinfo[vindex].primary, 
         staffnumber, measureinfo, mindex, vindex);
      } 
   } else if (measureinfo[mindex].fullrest > 1) {
      // do nothing;
   } else if ((mindex < measureinfo.getSize() - 1) && 
              (measureinfo[mindex+1].fullrest > 1) ) {
      // do nothing; still waiting for end of multi rest
   } else {
      printLayer(1, out, voiceinfo[vindex], MI, infile, noteaddresses, 
         meterclef);
   
      if (maxlayers > 1) {
         out << " & ";
         printLayer(2, out, voiceinfo[vindex], MI, infile, noteaddresses,
            meterclef);
      }
      // don't know how to print more than 2 layers...
      // so ignore layers greater than 2

      if (infile[MI.endline].getType() == E_humrec_data_measure) {
         printMeasureLine(out, infile, MI.endline, voiceinfo[vindex].primary, 
         staffnumber, measureinfo, mindex, vindex);
      } 
   }
}



//////////////////////////////
//
// printMultiRest --
//

void printMultiRest(ostream& out, Array<MeasureInfo>& measureinfo, int mindex) {
   int count = 0;
   int i;
   for (i=mindex; i>=0; i--) {
      if (measureinfo[i].fullrest) {
         count++;
      } else {
         break;
      }
   }
   out << "Z";
   if (count > 1) {
      out << count;
   }
}



//////////////////////////////
//
// getMeterAndClefChanges -- find the location of meter changes in the music
//    for the particular measure and voice.
//

void getMeterAndClefChanges(Array<Coordinate>& meterchanges, 
      HumdrumFile& infile, const VoiceMap& voiceinfo, 
      const MeasureInfo& measureinfo) {

   Array<Coordinate> temp;
   temp.setSize(0);
   Coordinate tcord;
   int top;
   int bot;

   int i, j;
   for (i=measureinfo.startline; i<=measureinfo.endline; i++) {
      if (infile[i].getType() == E_humrec_interpretation) {
         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (infile[i].getPrimaryTrack(j) != voiceinfo.primary) {
               continue;
            }
            if (sscanf(infile[i][j], "*M%d/%d", &top, &bot) == 2) {
               tcord.row = i;
               tcord.col = j;
               temp.append(tcord);
            } else if (strcmp(infile[i][j], "*met(C)") == 0) {
               tcord.row = i;
               tcord.col = j;
               if (temp.getSize() == 0) {
                  temp.append(tcord);
               } else {
                  temp[temp.getSize()-1] = tcord;
               }
            } else if (strcmp(infile[i][j], "*met(c)") == 0) {
               tcord.row = i;
               tcord.col = j;
               if (temp.getSize() == 0) {
                  temp.append(tcord);
               } else {
                  temp[temp.getSize()-1] = tcord;
               }
            } else if (strcmp(infile[i][j], "*met(C|)") == 0) {
               tcord.row = i;
               tcord.col = j;
               if (temp.getSize() == 0) {
                  temp.append(tcord);
               } else {
                  temp[temp.getSize()-1] = tcord;
               }
            } else if (strcmp(infile[i][j], "*met(c|)") == 0) {
               tcord.row = i;
               tcord.col = j;
               if (temp.getSize() == 0) {
                  temp.append(tcord);
               } else {
                  temp[temp.getSize()-1] = tcord;
               }
            } else if (strncmp(infile[i][j], "*clef", 5) == 0) {
               tcord.row = i;
               tcord.col = j;
               temp.append(tcord);
            }
            break;
         }
      }
   }

   // reverse the order of the meter and key changes 
   // so that they can be popped off of the array after being
   // printed.
   meterchanges.setSize(temp.getSize());
   meterchanges.setSize(0);
   for (i=temp.getSize()-1; i>=0; i--) {
      tcord = temp[i];
      meterchanges.append(tcord);
   }
}



//////////////////////////////
//
// printMeasureLine --
//     .| or : = dashed measure line
//     |       = regular line
//     [|      = !| style for humdrum measures
//     |[|     = |!| style for humdrum measures
//     ::      = :!!: style for humdrum measures
//     [|]     = invisible measurel line (- for humdrum measures)
//     |]      = |! or == for humdrum scores
//     ||      = || or (end of second repeat)
//     [1      = first repeat
//     [2      = second repeat
//     |1      = short first repeat
//     |2      = short second repeat
//

void printMeasureLine(ostream& out, HumdrumFile& infile, int line, 
      int primary, int staffnumber, Array<MeasureInfo>& measureinfo,
      int mindex, int vindex) {
   if (infile[line].getType() != E_humrec_data_measure) {
      return;
   }
   int col = -1;
   for (col=0; col<infile[line].getFieldCount(); col++) {
      if (infile[line].getPrimaryTrack(col) == primary) {
         break;
      }
   }

   if (infile[line].getPrimaryTrack(col) != primary) {
      return;
   }


   const char* token = infile[line][col];
   int measurenum = -100;
   sscanf(token, "=%d", &measurenum);

   int mdiff = 0;
   if (mindex > 0) {
      mdiff = measureinfo[mindex].measurenum -measureinfo[mindex-1].measurenum;
   }

   out << " ";  // separate barline from notes by a space

   if ((staffnumber == 0) && (measurenum >= 0) && ((mdiff > 1) || labelQ)) {
      out << "[I:setbarnb " << measurenum << "]";
   }

   if (strchr(token, '-') != NULL) {
      // print an invisible barline
      out << "[|] ";
      return;
   }
   
   if ((measureinfo[mindex].ending < 0) || (vindex != 0)) {
      if (strstr(token, ":|!|:") != NULL) { out << ":|]|: "; }
      else if (strstr(token, ":|!")   != NULL) { out << ":|] "; }
      else if (strstr(token, "!|:")   != NULL) { out << "]|: "; }
      else if (strstr(token, ":||:")  != NULL) { out << ":||: "; }
      else if (strstr(token, "||")    != NULL) { out << "|| "; }
      else if (strstr(token, "==")    != NULL) { out << "|] "; }
      else { out << "|"; } 
   } else {
      if (strstr(token, ":|!|:") != NULL) { out << ":|]|:"; }
      else if (strstr(token, ":|!")   != NULL) { out << ":|]"; }
      else if (strstr(token, "!|:")   != NULL) { out << "]|:"; }
      else if (strstr(token, ":||:")  != NULL) { out << ":||:"; }
      else if (strstr(token, "||")    != NULL) { out << "||"; }
      else if (strstr(token, "==")    != NULL) { out << "|]"; }
      else {
         if (measureinfo[mindex].ending == 0) {
            // the second ending must be ended by || barline in abcm2ps
            // and not a single barline
            out << "|";
            //out << " ||";
         } else {
            out << "|"; 
         } 
      }
      if (measureinfo[mindex].ending > 0) {
         out << "[";
         out << measureinfo[mindex].ending;
      }
   }
  
   out << " ";
}



//////////////////////////////
//
// getMaxLayer -- returns the maximum number of layers in a 
//      particular voice (staff).
//

int getMaxLayer(int track, int start, int endd, HumdrumFile& infile) {
   int i, j;
   int counter = 0;
   int maxx = 0;
   for (i=start; i<=endd; i++) {
      if (infile[i].getType() != E_humrec_data) {
         continue;
      }
      counter = 0;
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (infile[i].getPrimaryTrack(j) == track) {
            counter++;
         }
      }
      if (counter > maxx) {
         maxx = counter;
      }
   }

   return maxx;
}




//////////////////////////////
//
// printLayer -- print a single layer of a single staff.  But
//     the accidental states need to be monitored for all 
//     layers in the staff.
//

void printLayer(int layer, ostream& out, const VoiceMap& voiceinfo, 
      const MeasureInfo& measureinfo, HumdrumFile& infile,
      Array<Array<Coordinate> >& address, Array<Coordinate>& meterclef) {


   if (debugQ) {
      cout << "\%\% Printing measure";
      if (measureinfo.measurenum  > 0) {
         cout << " " << measureinfo.measurenum;
      }
      cout << " voice " << voiceinfo.primary 
	   << " layer " << layer << "==================\n%";
      cout << endl;
   }
	  
   // abcm2ps 5.9.1 has problems with slurs originating in
   // gracenote regions when there is another uncompleted
   // slur in progress, so suppress all slurs starting
   // from grace notes (at least for now).  In any case,
   // abcm2ps will automatically attach slurs between
   // grace notes and the next non-grace note in the
   // layer unless a flag is set to turn that feature off.
   int slursuppress = 0;

   int ilayer = layer - 1;
   const char* token;

   Array<int> accident(7*9);
   accident.allowGrowth(0);
   accident.setAll(0);
   setAccidentals(accident, measureinfo.currkey);

   Array<double> notedurs;   // duration of note events in layer, 
                             // (including grace notes)
   Array<int>    iindex;
   getNoteDurations(notedurs, iindex, layer, address, infile);

   // note list contains a list of notes in the current layer
   // without the grace notes
   Array<Coordinate> nogracelist;
   getNoGraceList(nogracelist, address, layer, infile);
   Array<double> nogracedurs;
   getNoGraceDurs(nogracedurs, notedurs, address, layer);

   Array<BeamInfo> beaminfo;
   getBeamInfo(beaminfo, nogracelist, infile);

   // broken rhythms are dotted rhythm patterns
   Array<int> broken;   // stores the brokenness for the first of a broken pair
   Array<double> brokendurs;  // store the revised duration for broken notes
   getBrokenRhythms(broken, brokendurs, nogracelist, nogracedurs, infile);

   Array<TupletInfo> tupletstuff;
   identifyTuplets(tupletstuff, nogracedurs, nogracelist, infile, broken, 
		         brokendurs, beaminfo);

   int beamstate = 0;
   int oldbeamstate = 0;
   int gracestate = 0;
  
   int lastrow = -1; 
   int lastcol = -1;
   int row = -1; 
   int col = -1;
   int counter = 0;
   int j;
   int nogracei = -1;
   double finaldur;
   int lastnotegraceQ = 0;
   
   double currentbeat = infile[measureinfo.startline].getAbsBeat();

   for (j=0; j<address[0].getSize(); j++) {
      row = address[ilayer][j].row;
      col = address[ilayer][j].col;
      if (linebreakQ) {
         checkForLineBreak(out, infile, row, col);
      }
      if (row >= 0) {
         lastrow = row;
         lastcol = col;

         //if (notedurs[counter] > 0.0) {
         if ((strchr(infile[row][col], 'q') == NULL)  &&
             (strchr(infile[row][col], 'Q') == NULL)) {
            nogracei++;
	    if (lastnotegraceQ) {
               // if the last note was a grace note, then do not
               // place a corrective invisible rest in the layer
               // To prevent the invisible rest from occurring
               // set the current beat to the absolute beat of the
               // note which is currently being processed.
               currentbeat = infile[row].getAbsBeat();
            }
	    lastnotegraceQ = 0;
	 } else {
            notedurs[counter] = 0.0;
	    lastnotegraceQ = 1;
         }


         if (currentbeat != infile[row].getAbsBeat()) {
//cout << "% current beat = " << currentbeat << " score beat "
//<< infile[row].getAbsBeat() << " diff = " 
//<< infile[row].getAbsBeat()  - currentbeat << endl;
            if ((strchr(infile[row][col], 'q') == NULL)  &&
                (strchr(infile[row][col], 'Q') == NULL)) {
               if (layer != 1) {
                  // don't allow invisible rests to occur in the first layer
                  printInvisibleRest(out, 
                     infile[row].getAbsBeat() - currentbeat);
               }
	        
            } else {
               // don't create any invisible rests when printing out
               // a grace note, since there metrical position in the
               // score is irrelevant.
            }
	      
         }

	 currentbeat = infile[row].getAbsBeat() + notedurs[counter];

         if ((layer == 1) && 
             (row > meterclef[meterclef.getSize()-1].row)) {
            printMeterAndClefChanges(out, row, meterclef, infile);
         }
         token = infile[row][col];
         oldbeamstate = beamstate;
         beamstate += getBeamState(token);
         if ((counter > 0) && (oldbeamstate == 0) &&
            (notedurs[counter] < 1.0) && (notedurs[counter-1] < 1.0)) {
            out << " ";  // put a space to prevent beaming
         }
         if ((!gracestate) && (nogracei>0) ) {
            // if not a grace note and note the first note:
            // It is assumbed that bi[i].left == bi[i-1].right
            // but might need to check it if this is no longer true.

            if (beaminfo[nogracei].right > beaminfo[nogracei].left) {
               if (beaminfo[nogracei-1].right < beaminfo[nogracei-1].left) {
                  if (beaminfo[nogracei].left > 0) {
                     out << "!beambr" << beaminfo[nogracei].left << "!";
                  }
               } 
            }

         }

	 if ((gracestate == 0) && (notedurs[counter] == 0.0)) {
             // grace notes (q) will have slashes added
	     // groupetto notes (Q) will not have slashes added
             if (strchr(token, 'q') != NULL) {
                out << "{/"; //q: start a grace note sequence with a slash
             } else {
                out << "{";  //Q: start a grace note sequence without a slash
             }
             gracestate = 1;
         }

	 if (notedurs[counter] > 0) {
            printTupletInfo(out, tupletstuff[nogracei]);
            finaldur = nogracedurs[nogracei];
	 } else {
            finaldur = 0.0;
         }
		  

         // print pitches of chords or single pitches (with accidentals)
         // plus articulations for note/chord.
	 
         int groupflag = getGraceNoteGroupFlag(notedurs, counter);
         printKernTokenAsAbc(out, infile, row, col, accident, 
               finaldur, broken[nogracei], brokendurs[nogracei], Ltop,
               Lbot, slursuppress, groupflag);

         if (notedurs[counter] >= 1) {
            beamstate = 0;   // fix any problems with beaming info
         } 

         if (gracestate) {
            if (counter == notedurs.getSize()-1) {
               out << "}";
               gracestate = 0;
            } else if (notedurs[counter+1] > 0.0) {
               out << "}";
               gracestate = 0;
            }
         }

         counter++;
      }
      adjustAccidentalStates(accident, j, address, infile);
   }

   if (currentbeat != infile[measureinfo.endline].getAbsBeat()) {
      // don't do a final correction if the last note was a grace note:
      if ((lastrow >= 0) && (lastcol >= 0) && 
          (strchr(infile[lastrow][lastcol], 'q') == NULL) &&
          (strchr(infile[lastrow][lastcol], 'Q') == NULL)) {
        printInvisibleRest(out, 
            infile[measureinfo.endline].getAbsBeat() - currentbeat);
      }
   }        

   // print any meter or clef changes at the end of a measure:
   if ((layer == 1) && (meterclef.getSize() > 0)) {
      printMeterAndClefChanges(out, 100000, meterclef, infile);
   }
}



//////////////////////////////
//
// checkForLineBreak --
//

void checkForLineBreak(ostream& out, HumdrumFile& infile, int row, int col) {
   int i;
   for (i=row-1; i>=0; i--) {
      if (infile[i].getType() == E_humrec_data) {
         return;
      }
      if (infile[i].getType() == E_humrec_local_comment) {
         if (strstr(infile[i][0], "linebreak") != NULL) {
            out << "\n";
         }
      }
   }
}



//////////////////////////////
//
// getGraceNoteGroupFlag --  There is probably a bug in abcm2ps
//     which requires that when more than one grace notes occurs
//    in a group together, the duration has to be increased by 4.0
//    to print the correct graphical rhythm, or increase by 2.0 
//    when there is only one grace note...
//

int getGraceNoteGroupFlag(Array<double>& notedurs, int index) {
   int counter = 1;
   int i;
   for (i=index+1; i<notedurs.getSize(); i++) {
      if (notedurs[i] < TOLERANCE) {
         counter++;
      } else {
         break;
      }
   }
   for (i=index-1; i>=0; i--) {
      if (notedurs[i] < TOLERANCE) {
         counter++;
      } else {
         break;
      }
   }

   if (counter == 1) {
      return 0;
   } else {
      return 1;
   }

}



//////////////////////////////
//
// printInvisibleRest --
//

void printInvisibleRest(ostream& out, double duration) {
   if (duration <= TOLERANCE) {
      return;
   }
   if (duration <= 0.001) {
      cout << "% INVISIBILE DURATION SUPPRESSED: " << duration << endl;
      // more aggressive suppression of tiny invisible rests...
      return;
   }
	  
   double value    = duration * Lbot / Ltop / 4.0;
   int a           = int(value + TOLERANCE);
   double fraction = value - a;
   int top;
   int bot;
   int b;
   if (fraction > TOLERANCE) {
      b = int(1.0 / fraction + TOLERANCE);   
      top = a * b + 1;   
      bot = b;
      simplifyFraction(top, bot);
   } else {
      top =  a;
      bot =  1;
   }

   out << invisiblerest;
   if (top > 1) {
      out << top;
   }
   if (bot > 1) {
      out << "/";
      if (bot == 2) {
      } else if (bot == 4) {
         out << "/";
      } else {
         out << bot;
      }
   }
}



//////////////////////////////
//
// getBeamInfo --
//

void getBeamInfo(Array<BeamInfo>& beaminfo, Array<Coordinate>& nogracelist, 
      HumdrumFile& infile) {

   beaminfo.setSize(nogracelist.getSize());
   beaminfo.allowGrowth(0);
   int i;
   int Lcount;
   int Jcount;
   int current = 0;
   int row;
   int col;

   for (i=0; i<nogracelist.getSize(); i++) {
      row = nogracelist[i].row;
      col = nogracelist[i].col;
      Lcount = characterCount(infile[row][col], 'L');
      Jcount = characterCount(infile[row][col], 'J');
      if (i == 0) {
         Jcount = 0;  // ignore cross-measure beaming if it occurs
      }
      beaminfo[i].left = current;
      current -= Jcount;
      current += Lcount;
      beaminfo[i].right = current;
   }
   beaminfo[beaminfo.getSize()-1].right = 0;
}



//////////////////////////////
//
// characterCount --
//

int characterCount(const char* string, char character) {
   int i = 0;
   int count = 0;
   while (string[i] != '\0') {
      if (string[i] == character) {
         count++;
      }
      i++;
   }
   return count;
}



//////////////////////////////
//
// printTupletInfo --
//

void printTupletInfo(ostream& out, TupletInfo& tinfo) {
   if (tinfo.top < 2) {
      return;
   }
   if (tinfo.count < 0) {
      return;
   }

   switch (tinfo.shortcut) {
      case 2: out << "(2"; return;
      case 3: out << "(3"; return;
      case 4: out << "(4"; return;
      case 5: out << "(5"; return;
      case 6: out << "(6"; return; // danger: as expected: factor of two off
      case 7: out << "(7"; return;
      case 8: out << "(8"; return;
      case 9: out << "(9"; return;
   }

   // print a longcut:
   out << "(" << tinfo.top << ":" << tinfo.bot << ":" << tinfo.count;
}



//////////////////////////////
//
// identifyTuplets -- find tuplets in the data.  The notedurs and 
//     nogracedurs, and brokendurs will be modified by this function.
//     to remove non-power-of-two rhythms from the list of notes to
//     print on the layer.
//

void identifyTuplets(Array<TupletInfo>& tupletstuff, 
      Array<double>& nogracedurs, Array<Coordinate>& nogracelist, 
      HumdrumFile& infile, Array<int>& broken, Array<double>& brokendurs,
      Array<BeamInfo>& beaminfo) {

   tupletstuff.setSize(nogracelist.getSize());
   tupletstuff.allowGrowth(0);

   if (tupletstuff.getSize() == 0) {
      // no notes (other than perhaps grace notes) in the measure
      return;
   }

   int dupleprimes;
   double newdur;
   int i;

   if (debugQ) {
      cout << "%\n% TUPLET INFO INPUT\n";
      cout << "%tuptop\ttupbot\tcount\tshort\tnewdur\ttoken\n";
      for (i=0; i<tupletstuff.getSize(); i++) {
         cout << "%" 
              << tupletstuff[i].top      << "\t"
              << tupletstuff[i].bot      << "\t"
              << tupletstuff[i].count    << "\t"
              << tupletstuff[i].shortcut << "\t"
              << nogracedurs[i]          << "\t"
              << infile[nogracelist[i].row][nogracelist[i].col] 
              << "\n";
      }
      cout << "%\n";
   }

   int scale;

   for (i=0; i<nogracelist.getSize(); i++) {
      dupleprimes = getTupletInfo(tupletstuff[i], nogracelist[i], infile);
      if (tupletstuff[i].top > 2) {
         newdur = 4.0 / (dupleprimes * tupletstuff[i].bot);
         scale = 0;
	 if (brokendurs[i] > 0) {
            if (broken[i] < 0) {
               // the current note is the first note in a broken
               // rhythm and it is the smaller 
               scale = -broken[i];
            } else if (i > 0) {
               // the previous note was part of the broken rhythm
               // and was longer than the current note.
               if (broken[i-1] > 0) {
                  scale = broken[i-1];
               }
            }
         }
         if (scale > 0) {
            newdur = newdur * (1 << scale);
         }
         nogracedurs[i] = newdur;
      }
   }

   identifyTupletShortCuts(tupletstuff, beaminfo, nogracedurs, nogracelist,
         infile);

   if (debugQ) {
      cout << "%\n% TUPLET INFO OUTPUT\n";
      cout << "%tuptop\ttupbot\tcount\tshort\tbeam\tbrok\tbrkdur\tnewdur\ttoken\n";
      for (i=0; i<tupletstuff.getSize(); i++) {
         cout << "%" 
              << tupletstuff[i].top      << "\t"
              << tupletstuff[i].bot      << "\t"
              << tupletstuff[i].count    << "\t"
              << tupletstuff[i].shortcut << "\t"
              << beaminfo[i].right << ":" << beaminfo[i].left << "\t"
	      << broken[i]               << "\t"
	      << int(brokendurs[i]*10000.0+0.5)/10000.0 << "\t"
              << nogracedurs[i]          << "\t"
              << infile[nogracelist[i].row][nogracelist[i].col] 
              << "\n";
      }
      cout << "%\n";
   }

}



//////////////////////////////
//
// separateCountsByBeams -- should also be separated by meter
//    maybe at the same time...
//

void separateCountsByBeams(Array<TupletInfo>& tupletstuff, 
     Array<BeamInfo>& beaminfo, Array<Coordinate>& nogracelist,
     HumdrumFile& infile) {
   int i, j;
   int counter;
   Array<int> rests;
   getRestInfo(rests, nogracelist, infile);
   for (i=0; i<tupletstuff.getSize(); i++) {
      if ((tupletstuff[i].count > 0) && 
          ((beaminfo[i].right > 0) || rests[i])) {
         counter = 1;
         for (j=1; j<tupletstuff[i].count; j++) {
            if ((beaminfo[i+j].left == 0) && (!rests[i+j-1])) {
               tupletstuff[i+j].count = tupletstuff[i].count - j;
               tupletstuff[i].count = j;
            }
         }
      }
   }
}



//////////////////////////////
//
// getRestInfo --
//

void getRestInfo(Array<int>& rests, Array<Coordinate>& nogracelist, 
      HumdrumFile& infile) {
   rests.setSize(nogracelist.getSize());
   int i;
   for (i=0; i<nogracelist.getSize(); i++) {
      if (strchr(infile[nogracelist[i].row][nogracelist[i].col], 'r') != NULL) {
         rests[i] = 1;
      } else {
         rests[i] = 0;
      }
   }
}



//////////////////////////////
//
// identifyTupletShortCuts --
//

void identifyTupletShortCuts(Array<TupletInfo>& tupletstuff,
      Array<BeamInfo>& beaminfo, Array<double>& nogracedurs,
      Array<Coordinate>& nogracelist, HumdrumFile& infile) {
   int top = -1;
   int bot = -1;
   int i;
   int length = tupletstuff.getSize();
   int counter = 1;

   top = tupletstuff[length-1].top;
   bot = tupletstuff[length-1].bot;
   for (i=tupletstuff.getSize()-2; i>= 0; i--) {
      if ((top == tupletstuff[i].top) && (bot == tupletstuff[i].bot)) {
         counter++;
      } else {
         if (top == -1) {
            counter = 1;
            top = tupletstuff[i].top;
            bot = tupletstuff[i].bot;
            continue;
         } else {
            tupletstuff[i+1].count = counter;
            counter = 1;
            top = tupletstuff[i].top;
            bot = tupletstuff[i].bot;
         }
      }
   }
   if (tupletstuff[0].top > 0) {
      tupletstuff[0].count = counter;
   }

   Array<TupletInfo>& t = tupletstuff;

   // separateCountsByBeams(tupletstuff, beaminfo, nogracelist, infile);

////////////////////////////////////////////////////////////////////////

   Array<double>& ng = nogracedurs;

   Array<double> metric;
   Array<double> frac;
   metric.setSize(nogracelist.getSize());
   frac.setSize(nogracelist.getSize());
   for (i=0; i<nogracelist.getSize(); i++) {
      metric[i] = infile[nogracelist[i].row].getAbsBeat();
      frac[i] = metric[i] - (int)metric[i];
   }

   Array<int> beamy;
   beamy.setSize(nogracelist.getSize());
   beamy.setAll(0);
   int bcount = 0;
   for (i=1; i<beaminfo.getSize(); i++) {
      if (beaminfo[i].left == 0) {
         bcount++;
      }
      beamy[i] = bcount;
   }

   // check for shortcuts
   for (i=0; i<t.getSize(); i++) {
      if ((t[i].top == 3) && (t[i].bot == 2) && (t[i].count == 3)) { 
         t[i].shortcut = 3;
         t[i].count = 0;    // shortcut, so don't use longcut
      }

      // triplet types of notes
      if ((t[i].top == 3) && (t[i].bot == 2) && (t[i].count % 3 == 0)
             && (t[i].count > 0)) { 

	 if ((t[i].count == 6) && (fabs(ng[i]   - 0.5) < TOLERANCE) &&
	     (fabs(ng[i+1] - 0.5) < TOLERANCE) && 
             (fabs(ng[i+2] - 0.5) < TOLERANCE)) {
	    // six triplet eighths in a row cannot be given a (6 shortcut)
	    // since it will be treated as six triplet sixteenths.
	    // The durations have already been de-tupletized, so
	    // a triplet eight is now encoded as an eighth
            t[i].shortcut = 3;
            t[i+3].shortcut = 3;
            t[i].count = 0;
            t[i+3].count = 0;
         } else if ((t[i].count > 3) &&
               (fabs(frac[i]) < TOLERANCE) &&
               (fabs(frac[i+1] - 1.0/3.0) < TOLERANCE) &&
               (fabs(frac[i+2] - 2.0/3.0) < TOLERANCE)) {
            // triplet eigths inside of one beat (assuming quarter-note beat)
            t[i].shortcut= 3;
            t[i+3].count = t[i].count - 3;
            t[i].count = 0;  // t[i] is now a shortcut so remove the count
         } else if ((t[i].count > 3) &&
               (fabs(frac[i] - 3.0/6.0) < TOLERANCE) &&
               (fabs(frac[i+1] - 4.0/6.0) < TOLERANCE) &&
               (fabs(frac[i+2] - 5.0/6.0) < TOLERANCE)) {
               // triplet 16ths on the second half of a beat
            t[i].shortcut = 3;
            t[i+3].count = t[i].count - 3;
            t[i].count = 0;  // t[i] is now a shortcut so remove the count
         } else if ((t[i].count > 3) &&
               (fabs(frac[i] - 0.0/6.0) < TOLERANCE) &&
               (fabs(frac[i+1] - 1.0/6.0) < TOLERANCE) &&
               (fabs(frac[i+2] - 2.0/6.0) < TOLERANCE)) {
            t[i].shortcut = 3;
            t[i+3].count = t[i].count - 3;
            t[i].count = 0;  // t[i] is now a shortcut so remove the count
         }
      }
   }

   // go back and fix tuplets which are beamed together:
   int lasti = -1;
   for (i=0; i<nogracelist.getSize(); i++) {
      if (t[i].shortcut == 3) {
         if (lasti < 0) {
            lasti = i;
         } else if (beamy[i] == beamy[lasti]) {
            if ((t[i].shortcut == 3) && (t[lasti].shortcut == 3)) {
               t[lasti].shortcut = 0;
               t[lasti].count = 6;
               t[i].shortcut = 0;
               t[i].count = -1;
            }
         }
         lasti = i;
      }

 
   }

}



//////////////////////////////
//
// getTupletInfo --
//
// identification algorithm for a non-duple tuplet:
//     (1) prime factorize the rhythm
//     (2) identify if there are any prime numbers other than 2
//     (3) if there are no other prime numbers then the note is
//         not a tuplet
//     (4) if there are other prime numbers, then multiply all of
//         the other prime numbers together and search for the next
//         lowest power of two.  The prime number will be the top
//         number in the tuplet ratio, and the next lower power of
//         two will be the bottom part of the ratio
//

int getTupletInfo(TupletInfo& tupletstuff, Coordinate& nogracelist, 
      HumdrumFile& infile) {
   int row = nogracelist.row;
   int col = nogracelist.col;
   char buffer[32] = {0};
   int rhythm;
   int nondupleprimes;
   int dupleprimes;
   int nextlowerpoweroftwo;
   Array<int> factors;

   infile[row].getToken(buffer, col, 0, 32);
   int i=0;
   while ((buffer[i] != EMPTY) && (!isdigit(buffer[i]))) {
     i++;
   }
   if (sscanf(&buffer[i], "%d", &rhythm) == 1) {
      primeFactorization(factors, rhythm);
      nondupleprimes = getNonDuplePrimes(factors);
      if (nondupleprimes <= 2) {
         tupletstuff.clear();
         return -1;
      } else {  // a tuplet has been identified, so mark it.
         dupleprimes = getDuplePrimes(factors);
         nextlowerpoweroftwo = getNextLowerPowerOfTwo(nondupleprimes);
         tupletstuff.top = nondupleprimes;
         tupletstuff.bot = nextlowerpoweroftwo;
         return dupleprimes;
      }
   } else {
      tupletstuff.clear();
   }

   return -1;
}



//////////////////////////////
//
// getNextLowerPowerOfTwo -- return the next lower power of two
//    for the given input number.  Example: 25 -> 16, 15 -> 8, 7->4, 5->4
//    if you give a power of two, it will return that same power of two.
//    if you give 1, then it will return 1.  if you give zero,
//    then it will return 0.
//

int getNextLowerPowerOfTwo(int number) {
   if (number <= 0) {
      return 0;
   } 
   if (number == 1) {
      return 1;
   }

   double value = log((double)number)/log(2.0);
   int output = int(pow(2.0, int(value+0.00001)));

   return output;
}



//////////////////////////////
//
// getNonDuplePrimes -- return the product of all numbers 
//    in the list which are greater than two.  All numbers
//    in the list are assumed to be prime numbers.
//

int getNonDuplePrimes(Array<int>& factors) {
   int i;
   int output = 1;
   for (i=0; i<factors.getSize(); i++) {
      if (factors[i] <= 2) {
         continue;
      }
      output = output * factors[i];
   }

   return output;
}



//////////////////////////////
//
// getDuplePrimes -- return the product of all numbers 
//    in the list which are greater than two.  All numbers
//    in the list are assumed to be prime numbers.
//

int getDuplePrimes(Array<int>& factors) {
   int i;
   int output = 1;
   for (i=0; i<factors.getSize(); i++) {
      if (factors[i] != 2) {
         continue;
      }
      output = output * 2;
   }

   return output;
}



//////////////////////////////
//
// printRhythm2 -- print the duration, version 2 which does not
//    yet address the tuplet notation, but does address the broken
//    rhythms.  Returns the number of broken rhythm characters
//    which should be printed after the ']' character in chords.
//

int printRhythm2(ostream& out, double dur, int brokenstate, 
      double brokendur, int top, int bot, int chordQ, const char* token) {
   int output = 0;

   if (norhythm(token)) {
      // this will remove stems from noteheads:
      out << "0";
   }

   if (stemlessQ) {
      // this will remove stems from noteheads:
      out << "0";
   }


   if (fabs(dur) <= TOLERANCE) {
      // don't print durations of grace notes.
      return 0;
   }

   char bch = 'X';
   if (brokenstate != 0) {
      // print Broken rhythm start.
      //printDurationAsRhythm(out, brokendur, top, bot);
      printDurationAsRhythm(out, dur, top, bot);
      if (brokenstate > 0) {
         bch = '>';
      } else {
         bch = '<';
      }
      int i;
      if (chordQ) {
         output = brokenstate;
      } else {
         for (i=0; i<abs(brokenstate); i++) {
            out << bch;
         }
      }
   } else if (brokendur != 0) {
      printDurationAsRhythm(out, dur, top, bot);
   } else {
      printDurationAsRhythm(out, dur, top, bot);
   }


   return output;
}



//////////////////////////////
//
// printDurationAsRhythm -- presuming non-tuplets at the moment
//

void printDurationAsRhythm(ostream& out, double dur, int top, int bot) {

   double fraction;
   double invfraction;
   double durunits;
   int durtop;
   int durbot;

   if (debugQ) {
//      cout << "\%PRINTING DURATION: " << dur 
//           << " in UNITS: " << top << "/" << bot << endl;
   }

   // fraction should be a power of two of some sort...
   durunits = dur * bot / top;
   fraction = durunits - (int)durunits;

   if (fabs(fraction) < TOLERANCE) {
      durtop = (int)durunits;
      durbot = 1;
   } else {
      invfraction = 1.0 / fraction;
      durtop = (int)durunits;
      durbot = (int)invfraction;
      fraction = invfraction - (int)invfraction;
      if (fabs(fraction) > TOLERANCE) {
         cout << "% FRACTION = " << fraction << "BUT SHOULD BE 0" << endl;
      }
   }

   durbot = durbot * 4;

   simplifyFraction(durtop, durbot);

   if (debugQ) {
//      cout << "%FINAL OUTPUT: " << durtop << "/" << durbot << endl;
   }

   if (durtop > 1) {
      out << durtop;
   }

   if (durbot == 1) {
      // don't print anything
   } else if (durbot == 2) {
      out << "/";
   } else if (durbot == 4) {
      out << "//";
   } else {
      out << "/" << durbot;
   }
}



//////////////////////////////
//
// simplifyFraction --
//

void simplifyFraction(int& top, int& bot) {
   Array<int> factorstop;
   Array<int> factorsbot;

   primeFactorization(factorstop, top);
   primeFactorization(factorsbot, bot);
   int i;
   int j;
   for (i=0; i<factorstop.getSize(); i++) {
      for (j=0; j<factorsbot.getSize(); j++) {
         if (factorstop[i] == factorsbot[j]) {
            factorstop[i] = 1;
            factorsbot[j] = 1;
            break;
         }
      }
   }

   top = 1;
   for (i=0; i<factorstop.getSize(); i++) {
      if (factorstop[i] > 1) {
         top = top * factorstop[i];
      }
   }

   bot = 1;
   for (i=0; i<factorsbot.getSize(); i++) {
      if (factorsbot[i] > 1) {
         bot = bot * factorsbot[i];
      }
   }

}



//////////////////////////////
//
// primeFactorization -- return a list of the prime factors of
//     the given input number, sorted from smallest to largest
//     factors.
//

void primeFactorization(Array<int>& factors, int input) {
   int i   = 3;
   int c   = input;
   int two = 2;
   factors.setSize(0);
   while ((c >= 2) && ((c%2) == 0)) {
      factors.append(two);
      c = c/2;
   }
   while (i <= (sqrt((double)c)+1)) {
      if ((c%i) == 0) {
         factors.append(i);
         c = c/i;
      }
      else {
         i = i+2;
      }
   }
   if (c > 1) {
      factors.append(c);
   }
}



//////////////////////////////
//
// getNoGraceDurs -- get the durations of all notes which are not
//  grace notes in the current layer.
//

void getNoGraceDurs(Array<double>& nogracedurs, Array<double>& notedurs, 
      Array<Array<Coordinate> >& address, int layer) {
   int i;
   int row;
   int col;
   double tdur;
   int ilayer = layer - 1;
   int counter = 0;
   nogracedurs.setSize(0);

   for (i=0; i<address[ilayer].getSize(); i++) {
      row = address[ilayer][i].row;
      col = address[ilayer][i].col;
      if (row >= 0) {
         if (notedurs[counter] > 0) {
            tdur = notedurs[counter];
            nogracedurs.append(tdur);
         }
	 counter++;
      }
   }
}



//////////////////////////////
//
// getNoGraceList -- get a list of notes for the current layer, 
//     ignoring the grace notes in the layer.
// 

void getNoGraceList(Array<Coordinate>& notelist, 
      Array<Array<Coordinate> >& address, int layer, HumdrumFile& infile) {
   Coordinate tcord;
   int ilayer = layer - 1;
   int i;
   notelist.setSize(address[0].getSize());
   notelist.setSize(0);
   for (i=0; i<address[ilayer].getSize(); i++) {
      tcord.row = address[ilayer][i].row;
      tcord.col = address[ilayer][i].col;
      if ((tcord.row > 0) && 
          (strchr(infile[tcord.row][tcord.col], 'q') == NULL) &&
          (strchr(infile[tcord.row][tcord.col], 'Q') == NULL)) {
         notelist.append(tcord);
      }
   }
}



//////////////////////////////
//
// printMeterAndClefChanges --
//

void printMeterAndClefChanges(ostream& out, int testrow, 
      Array<Coordinate>& meterclef, HumdrumFile& infile) {

   int row;
   int col;
   while ((meterclef.getSize() > 0) && 
          (meterclef[meterclef.getSize()-1].row < testrow)) {

         row = meterclef[meterclef.getSize()-1].row;
         col = meterclef[meterclef.getSize()-1].col;

         if (strncmp(infile[row][col], "*clef", 5) == 0) {
            out << "[K:clef=";
            printAbcClef(out, infile[row][col], 
                  infile[row].getPrimaryTrack(col), clefstates);
            out << "]";
         } else { // not a clef so must be a meter:
            out << "[M:";
            printAbcMeter(out, infile[row][col]);
            out << "]";
         }

      // shrink the array since the clef or meter signature at end was printed
      meterclef.setSize(meterclef.getSize()-1);
   }
}



///////////////////////////////
//
// printAbcMeter --
//

void printAbcMeter(ostream& out, const char* string) {
   int top;
   int bot;

   if (sscanf(string, "*M%d/%d", &top, &bot) == 2) {
      out << top << "/" << bot;
      return;
   }

   if (strcmp(string, "*met(C)") == 0) {
      out << "C";
      return;
   }
   
   if (strcmp(string, "*met(c)") == 0) {
      out << "C";
      return;
   }

   if (strcmp(string, "*met(C|)") == 0) {
      out << "C|";
      return;
   }
   
   if (strcmp(string, "*met(c|)") == 0) {
      out << "C|";
      return;
   }
   
   out << "?";

}



//////////////////////////////
//
// printAbcClef --
//

void printAbcClef(ostream& out, const char* string, int track, 
      Array<int>& cleftranspose) {
   if (strncmp(string, "*clef", 5) != 0) {
      out << "perc";
      cleftranspose[track] = 0;
      return;
   } else {
      if (strcmp(string, "*clefGv2") == 0) {
         out << "treble-8";
         cleftranspose[track] = +40;
         return;
      }
      if (strcmp(string, "*clefG2") == 0) {
         out << "treble";
         cleftranspose[track] = 0;
         return;
      }
      if (strcmp(string, "*clefF4") == 0) {
         out << "bass";
         cleftranspose[track] = 0;
         return;
      }
      if (strcmp(string, "*clefC3") == 0) {
         out << "alto";
         cleftranspose[track] = 0;
         return;
      }
      if (strcmp(string, "*clefC4") == 0) {
         out << "tenor";
         cleftranspose[track] = 0;
         return;
      }
      if ((string[5] == 'F') || (string[5] == 'C') || (string[5] == 'G')) {
         if (isdigit(string[6])) {
            out << string[5] << string[6];
            cleftranspose[track] = 0;
            return;
         }
      }

   }

   out << "perc";
   cleftranspose[track] = 0;
}



//////////////////////////////
//
// adjustAccidentalStates -- update the accidental states for the
//     notes at the current time (in all layers)
//

void adjustAccidentalStates(Array<int>& accident, int step, 
       Array<Array<Coordinate> >& address, HumdrumFile& infile) {

   int i;
   int row, col;
   for (i=0; i<address.getSize(); i++) { 
      if (address[i][step].row < 0) {
         continue;
      }
      row = address[i][step].row;
      col = address[i][step].col;
      adjustAccidentalStates2(accident, infile, row, col);
   }
}



//////////////////////////////
//
// adjustAccidentalStates2 --
//

void adjustAccidentalStates2(Array<int>& accident, HumdrumFile& infile, 
      int row, int col) {

   int count = infile[row].getTokenCount(col);
   char buffer[128] = {0};
   int acci;
   int diatonic;
   int base40;
   int i;
   int octave;

   for (i=0; i<count; i++) {
      infile[row].getToken(buffer, col, i);
      if (strchr(buffer, 'r') != NULL) {
         continue;
      }
      if (strchr(buffer, '_') != NULL) {
         // ignore storing the state of a continued tied note
         continue;
      }
      if (strchr(buffer, ']') != NULL) {
         // ignore storing the state of a ending tied note
         continue;
      }
      base40 = Convert::kernToBase40(buffer);
      octave = base40 / 40;
      acci = Convert::base40ToAccidental(base40);
      diatonic = Convert::base40ToDiatonic(base40);
      accident[diatonic+7*octave] = acci;
   }
}



//////////////////////////////
//
// setAccidentals --
//

void setAccidentals(Array<int>& accident, int key) {
   if (key < -7) {
      return;
   }
   #define XCX 0
   #define XDX 1
   #define XEX 2
   #define XFX 3
   #define XGX 4
   #define XAX 5
   #define XBX 6

   int octaves = accident.getSize() / 7;

   for (int i=0; i<octaves; i++) {
      if (key <= -7) { accident[XFX+i*7] = -1; }
      if (key <= -6) { accident[XCX+i*7] = -1; }
      if (key <= -5) { accident[XGX+i*7] = -1; }
      if (key <= -4) { accident[XDX+i*7] = -1; }
      if (key <= -3) { accident[XAX+i*7] = -1; }
      if (key <= -2) { accident[XEX+i*7] = -1; }
      if (key <= -1) { accident[XBX+i*7] = -1; }

      if (key >= +7) { accident[XBX+i*7] = +1; }
      if (key >= +6) { accident[XEX+i*7] = +1; }
      if (key >= +5) { accident[XAX+i*7] = +1; }
      if (key >= +4) { accident[XDX+i*7] = +1; }
      if (key >= +3) { accident[XGX+i*7] = +1; }
      if (key >= +2) { accident[XCX+i*7] = +1; }
      if (key >= +1) { accident[XFX+i*7] = +1; }
   }
}



//////////////////////////////
//
// printKernTokenAsAbc -- print the pitch and articulation
//     data for a note or a chord.  
//

void printKernTokenAsAbc(ostream& out, HumdrumFile& infile, int row, int col,
      Array<int>& accident, double notedur, int brokenq, double brokendur,
      int top, int bot, int& slursuppress, int groupflag) {
   int count = infile[row].getTokenCount(col);
   char buffer[128] = {0};
   char pitchbuffer[128] = {0};

   int base40;


   int alltie = 0;

   if (count == 1) {
      alltie = 1;
   } else {
      alltie = checkForAllTies(infile, row, col);
   }

   const char* token = infile[row][col];

   // phrase start, the ' marker after the
   // dashed slur indicates that it should 
   // be placed above the staff
   // Warning: if a phrase and slur start on the
   // same token, then the slur will also be
   // dashed in abcm2ps (probably a bug).
   if (strchr(token, '{') != NULL) {
      out << ".('";
   }

   // slur start
   const char* tptr;
   if ((tptr = strchr(token, '(')) != NULL) {
      if (notedur == 0.0) {
         slursuppress++;
      } else {
         if ((tptr+1)[0] == 'x') {
            // The slur mark is an editorial slur, so make it dashed
            out << ".";
         }
         out << "(";
      }
   }
   // multiple slurs and phrase markings cannot
   // be elided, but can be nested in ABC.

   infile[row].getToken(buffer, col, 0);
   printArticulations(out, buffer);

   int chordQ = 0;
   int brokencount = 0;  // used for printing broken rhythms with chords
   if (count > 1) {  // chord of more than one note
      out << "[";
      chordQ = 1;
   }
   int i;
   for (i=0; i<count; i++) {
      infile[row].getToken(buffer, col, i);
      base40 = Convert::kernToBase40(buffer);
      base40 += clefstates[infile[row].getPrimaryTrack(col)];

// cout << "CLEFSTATES" << infile[row].getPrimaryTrack(col) << " is "  << 
// clefstates[infile[row].getPrimaryTrack(col)] << endl;

      if ((strchr(buffer, 'r') != NULL) && (strchr(buffer, 'y') != NULL)) {
         strcpy(pitchbuffer, invisiblerest);      
      } else {
         base40ToAbcPitch(pitchbuffer, base40);
      }
      // print staccatos here (if not chord)
      printAccidental(out, base40, buffer, accident);
      out << pitchbuffer;

      // printRhythm1(out, buffer, Ltop, Lbot);
      if (notedur > 0.00000001) {
         brokencount = printRhythm2(out, notedur, brokenq, brokendur, 
            top, bot, chordQ, buffer);
      } else {
         // the note is a grace note, so suppress the
         // printing of any duration (maybe change later
         // to control the graphical view of the grace note).
         // Here is the change:
         printGraceRhythm(out, buffer, Ltop, Lbot, groupflag);
      }

      // print rhythm here...
      if ((alltie == 0) && (count != 1)) {
         if (strchr(buffer, '[') != NULL) {
            out << "-";
         } else if (strchr(buffer, '_') != NULL) {
            out << "-";
         }
      }
   }
      
   if (count > 1) {  // chord of more than one note
      out << "]";
      if (brokencount > 0) {
         for (i=0; i<brokencount; i++) {
            out << '>';
         }      
      } else if (brokencount < 0) {
         for (i=0; i<-brokencount; i++) {
            out << '<';
         }      
      }
   }

   // slur end
   if (strchr(infile[row][col], ')') != NULL) {
      if (slursuppress > 0) {
         slursuppress--;
      } else {
         out << ")";
      }
   }

   // phrasing end
   if (strchr(infile[row][col], '}') != NULL) {
      out << ")";
   }

   if (alltie) {
      infile[row].getToken(buffer, col, 0);
      if (strchr(buffer, '[') != NULL) {
         out << "-";
      } else if (strchr(buffer, '_') != NULL) {
         out << "-";
      }
   }


}



//////////////////////////////
//
// printGraceRhythm -- print the graphically displayed rhythm
//    for a grace note (not the logical rhythm which is zero).
// groupflag: true if the grace note is part of a larger group of
// grace notes than just a single grace note.
//

void printGraceRhythm(ostream& out, const char* buffer, int top, int bot,
      int groupflag) {
   int length = strlen(buffer);
   Array<char> buff2;
   buff2.setSize(length+1+20);
   buff2.setSize(0);
   int digitQ = 0;
   char ch;

   int i;
   for (i=0; i<length; i++) {
      if (toupper(buffer[i]) == 'Q') {
         continue;
      }
      if (isdigit(buffer[i])) {
         digitQ = 1;
      }
      ch = buffer[i];
      buff2.append(ch);
   }
   ch = '\0';
   buff2.append(ch);

   if (digitQ == 0) {
      strcpy(buff2.getBase(), "8");
   }
   double dur = Convert::kernToDuration(buff2.getBase());
   if (groupflag == 1) {
      // don't know why this has to be done.  Bug in abcm2ps?
      dur *= 4.0;
   } else if (groupflag == 0) {
      // don't know why this has to be done.  Bug in abcm2ps?
      dur *= 2.0;
   }
	     
// cout << "% BUFFER = " << buff2.getBase() << endl;

   printRhythm2(out, dur, 0, dur, 1, 4, 0, buff2.getBase());
}



//////////////////////////////
//
// checkForAllTies --
//

int checkForAllTies(HumdrumFile& infile, int row, int col) {
   char buffer[32] = {0};
   int count = infile[row].getTokenCount(col);
   int i;

   int counter = 0;
   for (i=0; i<count; i++) {
      infile[row].getToken(buffer, col, i);
      if ((strchr(buffer, '_') != NULL) || (strchr(buffer, '[') != NULL)) {
         counter++;
      }
   }
   
   if (counter == count) {
      return 1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// printArticulations --
//
// Articulations which are recognized:
//    NAME	HUMDRUM	ABC
//    staccato	'	.
//    staccatissimo `   !wedge!
//    sometime, Craig uses '' instead of `, so the following code reflects this
//    tenuto	~	!tenuto!
//    accent	^	!accent! or !>!
//    fermata	;	!fermata!
//    harmonic	o	!open!
//    mordent   m or M  !mordent!
//    inverted	w or W	!uppermordent!
//    mordent
//    breath    ,       !breath!
//    arpeggio  :       !arpeggio!
//    downbow   u       !downbow!
//    trill     t       !trill!           (semitone)
//    trill     T       !trill!           (wholetone)
//    upbow     v       !upbow!
//    sforzando z       !sfz!
//    general   O       ~
//    ornament
//
//    glissando H       no mapping
//    start
//    glissando h       no mapping
//    end
//    spiccato  s       no mapping   (spiccato in Humdrum might be martellato)
//    pizzacato "       no mapping?
//    Wagnerial $       no mapping?
//    turn  
//    general       I   no mapping?
//    articulation
//    con sordino   U   no mapping? or maybe !plus!
//    or mute
//

void printArticulations(ostream& out, const char* string) {
   // print articulations such as staccato
   // these must come before the "[" of a chord
   // and not inside of the chord brackets.
   
   if (strchr(string, '\'') != NULL)  {  // staccato present 
      if (strstr(string, "''") != NULL) {
         out << "!wedge!";   // staccatissimo
      } else {
         out << ".";         // staccato
      }
   }
   if (strchr(string, '`') != NULL)  {  // staccatissimo present 
      out << "!wedge!";
   }
   if (strchr(string, 'O') != NULL)  {  // general ornament (unspecified)
      out << "~";
   }

   if (strchr(string, '~') != NULL)  {  // tenuto present 
      out << "!tenuto!";
   }
   if (strchr(string, '^') != NULL)  {  // accent present 
      out << "!accent!";
   }
   if (strchr(string, ';') != NULL)  {  // fermata present 
      out << "!fermata!";
   }
   if (strchr(string, 'o') != NULL)  {  // fermata present 
      out << "!open!";
   }
   if (strchr(string, 'S') != NULL)  {  // some sort of turn present
      if (strchr(string, 'R') != NULL)  {  // inverted turn
         out << "!invertedturn!"; 
      } else {                             // regular turn
         out << "!turn!";
      }
   }
   if (strchr(string, 'm') != NULL)  {  // mordent present (semitone)
      out << "!mordent!";
   }
   if (strchr(string, 'M') != NULL)  {  // mordent present (wholetone)
      out << "!mordent!";
   }
   if (strchr(string, 'w') != NULL)  {  // inverted mordent present (semitone)
      out << "!uppermordent!";
   }
   if (strchr(string, 'W') != NULL)  {  // inverted mordent present (wholetone)
      out << "!uppermordent!";
   }
   if (strchr(string, ',') != NULL)  {  // breath mark (after note)
      out << "!breath!";
   }
   if (strchr(string, ':') != NULL)  {  // breath mark (after note)
      out << "!arpeggio!";
   }
   if (strchr(string, 'z') != NULL)  {  // sforzando
      out << "!sfz!";
   }
   if (strchr(string, 'u') != NULL)  {  // downbow
      out << "!downbow!";
   }
   if (strchr(string, 'v') != NULL)  {  // upbow
      out << "!upbow!";
   }
   if (strchr(string, 't') != NULL)  {  // upbow
      out << "!trill!";
   }
   if (strchr(string, 'T') != NULL)  {  // upbow
      out << "!trill!";
   }



}



//////////////////////////////
//
// printAccidental --
//

void printAccidental(ostream& out, int base40, const char* token, 
      Array<int>& accident) {
   if (base40 < 0) {
      return;  // ignore rests
   }
   int octave = base40 / 40;
   if (strchr(token, 'n') != NULL) {
      out << "=";
      return;
   }

   if (strchr(token, '_') != NULL) {
      // don't display the accidental on a continuation tied note
      return;
   }

   if (strchr(token, ']') != NULL) {
      // don't display the accidental on an ending tied note
      return;
   }

   int cautionary = 0;
   if (strchr(token, 'X') != NULL) {
      cautionary = 1;
   }

   int acci = Convert::base40ToAccidental(base40);
   int diatonic = Convert::base40ToDiatonic(base40);
   if ((!cautionary) && (acci == accident[diatonic+7*octave])) {
      return;   // the accidental matches the current display, so don't print
   }

   switch (acci) {
      case +2: out << "^^"; return;
      case +1: out << "^";  return;
      case  0: if (!nonaturalQ) { out << "="; } return;
      case -1: out << "_";  return;
      case -2: out << "__"; return;
   }

}



//////////////////////////////
//
// printRhythm1 -- handle tuplets later... still buggy at the moment
//

void printRhythm1(ostream& out, char* buffer, int top, int bot) {
   int rhythm = getRhythm(buffer);
   int dots = countDots(buffer);

   int rtop = 1;
   int rbot = rhythm;

   rtop = rtop * bot;
   rbot = rbot * top;
   int rtemptop;
   int rtempbot;


   if (dots == 1) {
      rtemptop = 3 * rtop * rbot;
      rtempbot = 2 * rtop * rbot;
	     
      rtop = rtemptop;
      rbot = rtempbot;
   } // handle other rhythms later


   if (((rtop % 8) == 0) && ((rbot % 8) == 0)) {
      rtop /= 8;
      rbot /= 8;
   }
   if (((rtop % 4) == 0) && ((rbot % 4) == 0)) {
      rtop /= 4;
      rbot /= 4;
   }
   if (((rtop % 2) == 0) && ((rbot % 2) == 0)) {
      rtop /= 2;
      rbot /= 2;
   }
   if (((rtop % 2) == 0) && ((rbot % 2) == 0)) {
      rtop /= 2;
      rbot /= 2;
   }

   if (rtop > 1) {
      out << rtop;
   }
   if (rbot > 1) {
      if (rbot == 2) {
         out << "/";           // shorthand for a /2 rhythm
      } else if (rbot == 4) {
         out << "//";          // shorthand for a /4 rhythm
      } else {
         out << "/" << rbot;
      }
   }

}



//////////////////////////////
//
// getRhythm --
//

int getRhythm(const char* buffer) {
   int rhythm = 0;
   int length = strlen(buffer);
   int i;
   for (i=0; i<length; i++) {
      if (isdigit(buffer[i])) {
         if (buffer[i] == '0') {
            if (isdigit(buffer[i+1])) {
               // do something for large durations greater than whole note:
               rhythm = -1 * (buffer[i+1] - '0');
            }
         } else {
            sscanf(&buffer[i], "%d", &rhythm);
         }
         break;
      }
   }
   return rhythm;
}



//////////////////////////////
//
// countDots --
//

int countDots(char* buffer) {
   int length = strlen(buffer);
   int i;
   int output = 0;
   for (i=0; i<length; i++) {
      if (buffer[i] == '.') {
         output++;
      }
   }
   return output;
}



//////////////////////////////
//
// getBrokenRhythm --
//

void getBrokenRhythms(Array<int>& broken, Array<double>& brokendurs, 
      Array<Coordinate>& nogracelist, Array<double>& nogracedurs, 
      HumdrumFile& infile) {

   broken.setSize(nogracelist.getSize());
   brokendurs.setSize(nogracelist.getSize());
   broken.setAll(0);
   brokendurs.setAll(0);
   int i;
   int row;
   int col;
   int row2;
   int col2;

   for (i=0; i<broken.getSize()-2; i++) {
      row = nogracelist[i].row;
      col = nogracelist[i].col;
      row2 = nogracelist[i+1].row;
      col2 = nogracelist[i+1].col;
      if ((strchr(infile[row][col], '.') == NULL) &&
          (strchr(infile[row2][col2], '.') == NULL)) {
         continue;
      }
	     
      if (fabs(nogracedurs[i] - 3.0 * nogracedurs[i+1]) <= TOLERANCE) {
         broken[i]   = +1;
         broken[i+1] =  0;
         brokendurs[i] = nogracedurs[i+1] * 2.0;
         brokendurs[i+1] = brokendurs[i];
         i+=1; // skip next note 
      } else if (fabs(3.0 * nogracedurs[i] - nogracedurs[i+1]) <= TOLERANCE) {
         broken[i]   = -1;
         broken[i+1] =  0;
         brokendurs[i] = nogracedurs[i] * 2.0;
         brokendurs[i+1] = brokendurs[i];
         i+=1; // skip next note 
      } else if (fabs(nogracedurs[i] - 7.0 * nogracedurs[i+1]) <= TOLERANCE) {
         broken[i]   = +2;
         broken[i+1] =  0;
         brokendurs[i] = nogracedurs[i+1] * 4.0;
         brokendurs[i+1] = brokendurs[i];
         i+=1; // skip next note 
      } else if (fabs(7.0 * nogracedurs[i] - nogracedurs[i+1]) <= TOLERANCE) {
         broken[i]   = -2;
         broken[i+1] =  0;
         brokendurs[i] = nogracedurs[i] * 4.0;
         brokendurs[i+1] = brokendurs[i];
         i+=1; // skip next note 
      } else if (fabs(nogracedurs[i] - 15.0 * nogracedurs[i+1]) <= TOLERANCE) {
         broken[i]   = +3;
         broken[i+1] =  0;
         brokendurs[i] = nogracedurs[i+1] * 8.0;
         brokendurs[i+1] = brokendurs[i];
         i+=1; // skip next note 
      } else if (fabs(15.0 * nogracedurs[i] - nogracedurs[i+1]) <= TOLERANCE) {
         broken[i]   = -3;
         broken[i+1] =  0;
         brokendurs[i] = nogracedurs[i] * 8.0;
         brokendurs[i+1] = brokendurs[i];
         i+=1; // skip next note 

      // abcm2ps cannot handle the following two cases of a quadruple dot:
      /*
      } else if (fabs(nogracedurs[i] - 31.0 * nogracedurs[i+1]) <= TOLERANCE) {
         broken[i]   = +4;
         broken[i+1] =  0;
         brokendurs[i] = nogracedurs[i+1] * 16.0;
         brokendurs[i+1] = brokendurs[i];
         i+=1; // skip next note 
      } else if (fabs(31.0 * nogracedurs[i] - nogracedurs[i+1]) <= TOLERANCE) {
         broken[i]   = -4;
         broken[i+1] =  0;
         brokendurs[i] = nogracedurs[i] * 16.0;
         brokendurs[i+1] = brokendurs[i];
         i+=1; // skip next note 
       */
      }
   }

   if (debugQ) {
      cout << "%\n%BROKEN RHYTHM ANALYSIS:\n";
      cout << "%broken\tbrokdur\tdur\ttoken\n";
      for (i=0; i<broken.getSize(); i++) {
         cout << "%"   << broken[i]
              << "\t"  << int(brokendurs[i] * 10000.0 + 0.5)/10000.0
              << "\t"  << int(nogracedurs[i] * 10000.0 + 0.5)/10000.0
              << "\t"  << infile[nogracelist[i].row][nogracelist[i].col]
              << endl;
      }
      cout << "%" << endl;
   }


   // replace the non-brokendurs with the broken duration:
   for (i=0; i<brokendurs.getSize(); i++) {
      if (brokendurs[i] != 0.0) {
         nogracedurs[i] = brokendurs[i];
      }
   }

}



//////////////////////////////
//
// getBrokenRhythms -- two rhythms which sum to a quarter note.
//    probably should check to see if starting on a beat, but
//    maybe that wouldn't be good anyway. Currently ignores
//    tuplet broken rhythms.  Don't know if that would be useful
//    to add...
//

void getBrokenRhythmsOld(Array<int>& broken, Array<int>& brokenrhythm, 
      Array<double>& notedurations, HumdrumFile& infile) {

   Array<double>& ND = notedurations;

   broken.setSize(ND.getSize());
   broken.setAll(0);

   brokenrhythm.setSize(ND.getSize());
   brokenrhythm.setAll(0);

   int i;
   for (i=0; i<ND.getSize()-1; i++) {
      if (ND[i] + ND[i+1] == 1.0) {

         if ((ND[i] == 0.75) && (ND[i] == 0.25)) {
            broken[i] = 1;  brokenrhythm[i] = 4;
         }
         else if ((ND[i] == 0.25) && (ND[i] == 0.75)) {
            broken[i] = -1;  brokenrhythm[i] = 4;
         }
         if ((ND[i] == 0.875) && (ND[i] == 0.125)) {
            broken[i] = 2;  brokenrhythm[i] = 4;
         }
         else if ((ND[i] == 0.125) && (ND[i] == 0.875)) {
            broken[i] = -2;  brokenrhythm[i] = 4;
         }
         if ((ND[i] == 0.9375) && (ND[i] == 0.0625)) {
            broken[i] = 3;  brokenrhythm[i] = 4;
         }
         else if ((ND[i] == 0.0625) && (ND[i] == 0.9375)) {
            broken[i] = -3;  brokenrhythm[i] = 4;
         }
      }
      else if (ND[i] + ND[i+1] == 0.5) {
         if ((ND[i] == 0.875) && (ND[i] == 0.125)) {
            broken[i] = 1;  brokenrhythm[i] = 8;
         }
         else if ((ND[i] == 0.125) && (ND[i] == 0.875)) {
            broken[i] = -1;  brokenrhythm[i] = 8;
         }
         if ((ND[i] == 0.9375) && (ND[i] == 0.0625)) {
            broken[i] = 2;  brokenrhythm[i] = 8;
         }
         else if ((ND[i] == 0.0625) && (ND[i] == 0.9375)) {
            broken[i] = -2;  brokenrhythm[i] = 8;
         }
         if ((ND[i] == 0.96875) && (ND[i] == 0.0625)) {
            broken[i] = 3;  brokenrhythm[i] = 8;
         }
         else if ((ND[i] == 0.03125) && (ND[i] == 0.96875)) {
            broken[i] = -3;  brokenrhythm[i] = 8;
         }
      }  
      else if (ND[i] + ND[i+1] == 0.25) {
         if ((ND[i] == 0.9375) && (ND[i] == 0.0625)) {
            broken[i] = 1;  brokenrhythm[i] = 16;
         }
         else if ((ND[i] == 0.0625) && (ND[i] == 0.9375)) {
            broken[i] = -1;  brokenrhythm[i] = 16;
         }
         if ((ND[i] == 0.96875) && (ND[i] == 0.0625)) {
            broken[i] = 2;  brokenrhythm[i] = 16;
         }
         else if ((ND[i] == 0.03125) && (ND[i] == 0.96875)) {
            broken[i] = -2;  brokenrhythm[i] = 16;
         }

      }

   }

   // don't allow for two broken rhythms in a row:
   for (i=0; i<broken.getSize() - 1; i++) {
      if (broken[i] && broken[i+1]) {
         broken[i+1] = 0;
         brokenrhythm[i+1] = 0;
      }
   }
}



//////////////////////////////
//
// getNoteDurations --
//

void getNoteDurations(Array<double>& notedurations, Array<int>& iindex, 
      int layer, Array<Array<Coordinate> >& address, HumdrumFile& infile) {

   int ilayer = layer - 1;
   notedurations.setSize(address[ilayer].getSize());
   notedurations.setSize(0);


   // iindex is a list of the indices into address for each sequential
   // note in the layer.
   iindex.setSize(address[ilayer].getSize());
   iindex.setSize(0);

   char buffer[128] = {0};
   double dur;
   int row, col;
   int i;
   for (i=0; i<address[ilayer].getSize(); i++) {
      if (address[ilayer][i].row >= 0) {
         row = address[ilayer][i].row;
         col = address[ilayer][i].col;
         infile[row].getToken(buffer, col, 0);
         if (norhythm(buffer)) {
            dur = 1.0;  // assign quarter note 
         } else {
            dur = Convert::kernToDuration(buffer);
         }
	 notedurations.append(dur);
	 iindex.append(i);
      }
   }

   if (debugQ) {
      int tcount = 0;
      cout << "%dur\ttoken\n";
      for (i=0; i<address[ilayer].getSize(); i++) {
         if (address[ilayer][i].row < 0) { 
            continue;
         }
         cout << "%" << int(notedurations[tcount]*10000.0 + 0.5)/10000.0 
              << "\t"
              << infile[address[ilayer][i].row][address[ilayer][i].col]
              << endl;
         tcount++;
      }
   }

}



//////////////////////////////
//
// norhythm -- return true if there is no rhythm specified in the
//    **kern data (for Gregorian Chant for example). 
//

int norhythm(const char* buffer) {
   int length = strlen(buffer);
   int i;
   for (i=0; i<length; i++) {
      if (isdigit(buffer[i]) || (toupper(buffer[i]) == 'Q')) {
         return 0;
      }
   }

   return 1;
}



//////////////////////////////
//
// checkForBeamStarts -- Ignoring partial beams (K to the right)
//    and k for partial beam to the left.  All chord notes are
//    expected to have the same beaming pattern, so only the
//    first note is examined for beams.
//

int getBeamState(const char* token) {
   int length = strlen(token);
   int i;
   int output = 0;
   for (i=0; i<length; i++) {
      if (token[i] == ' ') {
         // don't break if a space is found
	 // because the beaming info only can
	 // occur once in a chord, and can occur on 
	 // any note in the chord.
      }
      if (token[i] == 'L') {  // beam to right
         output++;
      }
      if (token[i] == 'J') {  // beam to right
         output--;
      }
   }

   return output;
}


//////////////////////////////
//
// buildNoteAddresses -- store the line and column number for
//    each note in the part;
//

void buildNoteAddresses(Array<Array<Coordinate> >& noteaddresses, 
      HumdrumFile& infile, const VoiceMap& voiceinfo, 
      const MeasureInfo& measureinfo) {

   int maxlines = measureinfo.endline - measureinfo.startline + 1;
   int i, j;
   // empty the previous contents, if any
   // the first dimension is given by the calling function and
   // represents the maximum voice layers in the part
   for (i=0; i<noteaddresses.getSize(); i++) {
      noteaddresses[i].setSize(maxlines);
      noteaddresses[i].setSize(0);
   }

   // rowinfo is used to keep track of what notes are played
   // for each Humdrum file line (for the given voice).
   // If there are any notes for that voice on each line,
   // then store the note address data in the noteaddresses 
   // list.
   Array<int> rowinfo;
   rowinfo.setSize(noteaddresses.getSize());
   rowinfo.allowGrowth(0);
   int notesfound;
   int layercounter;
   Coordinate coord;

   for (i=measureinfo.startline; i<=measureinfo.endline; i++) {
      if (infile[i].getType() != E_humrec_data) {
         continue;
      }
      rowinfo.setAll(-1);
      notesfound = 0;
      layercounter = -1;
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (infile[i].getPrimaryTrack(j) == voiceinfo.primary) {
            layercounter++;
            if (strcmp(infile[i][j], ".") == 0) { // ignore null tokens in layer
               continue;
            } else {
               rowinfo[layercounter] = j;
               notesfound = 1;
            }
         }
      }
      if (notesfound) {
         for (j=0; j<rowinfo.getSize(); j++) {
            coord.row  = -1;
            coord.col  = -1;
            if (rowinfo[j] >= 0) {
               coord.row = i;
               coord.col = rowinfo[j];
            }
            noteaddresses[j].append(coord);
         }
      }
   }
}



//////////////////////////////
//
// getMeasureInfo --  only check the primary layer for a staff
//     Also checks for repeat endings in a particular format.
//

void getMeasureInfo(Array<MeasureInfo>& measures, HumdrumFile& infile) {
   int i;
   int lastmeasure = -1;
   double duration;

   measures.setSize(infile.getNumLines());
   measures.setSize(0);
   MeasureInfo mtemp;
   int measureno;
   int lastmeasureno = -1;
   //infile.analyzeRhythm("4");
   int currentKey = -100;
   int datafound = 0;
   int sindex;

   Array<int> repeatinfo;
   Array<int> segments;
   getRepeatInfo(segments, repeatinfo, infile);

   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() == E_humrec_data) {
         if (datafound == 0) {
            datafound = 1;
            if (lastmeasure == -1) {
               lastmeasure = i;
            }
         }
         continue;
      }

      if ((infile[i].getType() == E_humrec_data_measure) ||
          ((infile[i].getType() == E_humrec_interpretation) && 
           (strcmp(infile[i][0], "*-") == 0)) ) {
         if (lastmeasure < 0) {
            lastmeasure = i;
         } else {
            mtemp.clear();
            mtemp.startline = lastmeasure;
            mtemp.measurenum = lastmeasureno;
            mtemp.endline   = i;
	    mtemp.newkey = currentKey;
            lastmeasure = i;
	    duration = infile[mtemp.startline].getAbsBeat();
	    duration = infile[mtemp.endline].getAbsBeat() - duration;
            sindex = checkForRepeatMeasure(infile, segments, 
                  repeatinfo, lastmeasure);
            if (sindex >= 0) {
               mtemp.ending = repeatinfo[sindex];
            } else {
               mtemp.ending = -1;
            }

	    if (duration > 0) {
               measures.append(mtemp);
            } else if (datafound && (mtemp.endline - mtemp.startline > 1)) {
               // also have to allow for non-rhythm notation
               // such as Gregorian chant...
               measures.append(mtemp);
            }
	    currentKey = -100;
         }
         if (sscanf(infile[i][0], "=%d", &measureno) == 1) {
	    lastmeasureno = measureno;
         } else {
	    lastmeasureno = -1;	     
         }
      } else if (infile[i].getType() == E_humrec_interpretation) {
         if (strncmp(infile[i][0], "*k[", 3) == 0) {
            currentKey = Convert::kernKeyToNumber(infile[i][0]);      
         }
      }
   }

/// This code is now obsolete, since StartKey is a naughty global variable,
/// and the getMeasureInfo is now called before StartKey is set...
//   if (measures.getSize() > 0) {
//      measures[0].currkey = StartKey;
//   }
   for (i=0; i<measures.getSize(); i++) {
      if (measures[i].newkey > -50) {
         measures[i].currkey = measures[i].newkey;
      } else if (i > 0) {
         measures[i].currkey = measures[i-1].currkey;
      }
   }

   // a key signature in the first measure is controlled by the
   // header K: field, so turn off any new key markers in the
   // first measure.
   if (measures.getSize() > 0) {
      measures[0].newkey = -100;
   }

   if (Voicemap.getSize() == 1) {
      identifyWholeRestsInMeasures(measures, Voicemap[0], infile);
      for (i=1; i<measures.getSize(); i++) {
         if (measures[i].fullrest) {
            measures[i].fullrest += measures[i-1].fullrest;
         }
      }
   }

}



//////////////////////////////
//
// checkForRepeatMeasure --
//

int checkForRepeatMeasure(HumdrumFile& infile, Array<int>& segments, 
      Array<int>& repeatinfo, int mindex) {

   int i;
   double measuredur = infile[mindex].getAbsBeat();
   double sdur;
   for (i=0; i<segments.getSize(); i++) {
      sdur = infile[segments[i]].getAbsBeat();
      if (fabs(sdur - measuredur) < TOLERANCE) {
         return i;
      }
   }

   return -1;
}



//////////////////////////////
//
// getRepeatInfo --
//

void getRepeatInfo(Array<int>& segments, Array<int>& repeatinfo, 
      HumdrumFile& infile) {
   int i;
   int length;

   segments.setSize(100);
   segments.setSize(0);

   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() == E_humrec_interpretation) {
         if (strchr(infile[i][0], '[') != NULL) {
            continue;
         }
         if (strncmp(infile[i][0], "*>", 2) == 0) {
            segments.append(i);
         }
      }
   }

   repeatinfo.setSize(segments.getSize());
   repeatinfo.setAll(-1);

   int number;
   int base;
   int baselength;
   for (i=1; i<segments.getSize(); i++) {
      length = strlen(infile[segments[i-1]][0]);
      if (strncmp(infile[segments[i]][0], infile[segments[i-1]][0],
            length) == 0) {
         base = i-1;
         baselength = length;
         length = strlen(infile[segments[i]][0]);
         while ((i < segments.getSize()) && (length > baselength) && 
                (sscanf(&infile[segments[i]][0][length-1], "%d", 
                      &number) == 1) &&
                (strncmp(infile[segments[i]][0], infile[segments[base]][0], 
                  baselength) == 0)) {
            repeatinfo[i] = number;            
            i++;
         }
      }
   }


   for (i=1; i<repeatinfo.getSize(); i++) {
      if ((repeatinfo[i-1] > 0) && (repeatinfo[i] < 0)) {
         repeatinfo[i] = 0;
      }
   }

   if (debugQ) {
      cout << "% MUSIC SEGMENTS: " << endl;
      for (i=0; i<repeatinfo.getSize(); i++) {
         cout << "% SEGMENT: " << infile[segments[i]][0] 
              << " number " << repeatinfo[i] << endl;
      }
      cout << "%\n";
   }

}



//////////////////////////////
//
// identifyWholeRestsInMeasures --
//

void identifyWholeRestsInMeasures(Array<MeasureInfo>& measures, 
      VoiceMap& voicemap, HumdrumFile& infile) {
   int i;

   for (i=0; i<measures.getSize(); i++) {
      measures[i].fullrest = checkForWholeRestInMeasure(measures[i].startline, 
            measures[i].endline, voicemap.primary, infile);
   }

}



//////////////////////////////
//
// checkForWholeRestInMeasure -- should also check for the duration
//    of the rest, but then would also need to know the meter 
//    of the measure which is currently not being analyzed.
//

int checkForWholeRestInMeasure(int startline, int endline, int primary,
      HumdrumFile& infile) {
   int i, j;
   int notecount = 0;
   int restcount = 0;
   for (i=startline; i<=endline; i++) {
      if (infile[i].getType() != E_humrec_data) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (infile[i].getPrimaryTrack(j) == primary) {
            if (strcmp(infile[i][j], ".") == 0) {
               continue;
            }
            if (strchr(infile[i][j], 'r') != NULL) {
               restcount++;
            } else {
               notecount++;
            }
         }
      }
   }

   if (notecount == 0) {
      return 1;
   }  else {
      return 0;
   }
}



//////////////////////////////
//
// printHeader -- 
//

void printHeader(ostream& out, HumdrumFile& infile, Array<char*>& header,
      Array<MeasureInfo>& measures, int xval, const char* filename) {

   parseBibliographic(header, infile);

   // first indicate the required tune number
   // which must come as the first record in the header
   // (excluding comment/blank lines)
   // Use the number 1 if there is no command-line input
   if (xval > 0) {
      out << "X: " << xval << "\n"; 
   } else if (strcmp(header[XX], "") == 0) {
      out << "X: 1\n";
   } else {
      out << "X: " << header[XX] << "\n";
   }

   int firstmeasure = getFirstMeasureNumber(infile);

   // display the title next (if given)
   if (strcmp(header[TT], "") != 0) {
      out << "T:";
      if (filenumQ) {
         printNumberFromString(out, filename, filenumstring);
      }
      if (filenametitleQ) {
         out << " ";
         printFilenameBase(out, filename);
      } 
      out << " " << header[TT] << "\n";
   } else if (filenametitleQ) {
      out << "T:";
      out << " ";
      printFilenameBase(out, filename);
      out << "\n";
   }

   // display the composer next if given
   if (strcmp(header[CC], "") != 0) {
      out << "C: " << header[CC] << "\n";
   } 

   printAbcExtendedInformationFields(out, infile);

   if ((barnumberingstyle == 1) && 
       (fabs(infile.getPickupDuration()) > TOLERANCE)) {
      // do something so that the first bar number will be printed
      // if there is a pickup measure
   } 
   if (firstmeasure > 1) {
      out << "%%setbarnb " << firstmeasure << "\n";
   }

   if (Voicemap.getSize() > 5) {
      // automatically suppress staff lines in music with more
      // than 5 staves when the line of music contains only
      // rests
      // Doesn't seem to work:
      //out << "%%staffnonote" << "\n";
   }
	     


   int i;   
   for (i=0; i<26; i++) {
      switch (i+'A') {
         case 'X':      // number of piece, must come at start of header
         case 'T':      // title already printed
         case 'C':      // composer already printed
         case 'K':	// key of piece, must come at end of header
         case 'V':	// ignore user input of this field (Voice declarations)
            // ignore
            break;

         case 'L':      // rhythmic unit already printed
            // determine and display the rhythmic unit (L: record)
            if (strcmp(header[LL], "") == 0) {
               calculateBestRhythmUnit(infile, Ltop, Lbot);
               if (Lbot == 1) {
                  out << "L: " << Ltop << "\n";
               } else {
                  out << "L: " << Ltop << "/" << Lbot << "\n";
               }
            } else {  // L field is specified in command-line arguments
               if (sscanf(header[LL], "%d/%d", &Ltop, &Lbot) == 2) {
                  out << "L: " << Ltop << "/" << Lbot << "\n";
               } else if (sscanf(header[LL], "%d", &Ltop) == 1) {
                  Lbot = 1;
                  out << "L: " << Ltop << "\n";
               } else {
                  calculateBestRhythmUnit(infile, Ltop, Lbot);
                  if (Lbot == 1) {
                     out << "L: " << Ltop << "\n";
                  } else {
                     out << "L: " << Ltop << "/" << Lbot << "\n";
                  }
               }
            }
            break;
         default:
            if (strcmp(header[i], "") != 0) {
               out << char(i+'A') << ": " << header[i] << "\n";
            }
      }
   }
   if (Voicemap.getSize() > 1) {
      // polyphonic music
      
      if (Voicemap.getSize() == 2) {
         // presume a piano staff brace. refine the decision later
         out << "%%staves {1 2}\n";
      } else if (Voicemap.getSize() == 4) {
         // presume a string-quartet-like system with a brace
         out << "%%staves [1 2 3 4]\n";
      }

      for (i=0; i<Voicemap.getSize(); i++) {
         printVoiceDeclaration(out, Voicemap[i], i+1, infile);
      }
   }

   if (strcmp(header[KK], "") == 0) {
      out << "K: ";
      printKeySignature(out, infile);
   } else {
      out << "K: " << header[KK];
   }
   if (Voicemap.getSize() == 1) {
      printVoiceClef(out, Voicemap[0], infile);
   }
   out << "\n";
}



//////////////////////////////
//
// printFilenameBase --
//

void printFilenameBase(ostream& out, const char* filename) {
   Array<char> strang;
   const char* cptr;
   char* ptr;
   int length = strlen(filename);
   strang.setSize(length+1);

   cptr = strrchr(filename, '/');
   if (cptr != NULL) {
      strcpy(strang.getBase(), cptr+1);
   } else {
      strcpy(strang.getBase(), filename);
   }

   ptr = strrchr(strang.getBase(), '.');
   if (ptr != NULL) {
      *ptr = '\0';
   }

   out << strang.getBase();
}



//////////////////////////////
//
// printNumberFromString --
//

void printNumberFromString(ostream& out, const char* filename, 
      const char* ending) {
   int foundQ = 0;
   int starti = -1;
   int length = strlen(filename);
   int slashi = -1;
   int i;

   if (strchr(filename, '/') != NULL) {
      for (i=length-1; i>=0; i--) {
         if (filename[i] == '/') {
            slashi = i + 1;
            break;
         }
      }
   } else {
      slashi = 0;
   }

   for (i=slashi; i<length; i++) {
      if (!isdigit(filename[i]) || (filename[i] == '0')) { 
         continue;
      } 
      foundQ = 1;
      starti = i;
      break;
   }

   if (!foundQ) {
      return;
   }
   out << " ";

   i = starti;
   while ((i<length) && (filename[i] != '.')) {
      out << filename[i];
      i++;
   }

   if (foundQ) {
      out << ending;
   }

}



//////////////////////////////
//
// printVoiceClef --
//

void printVoiceClef(ostream& out, VoiceMap voicemap, HumdrumFile& infile) {
   int i, j;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() == E_humrec_interpretation) {
         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (infile[i].getPrimaryTrack(j) == voicemap.primary) {
               if (strncmp(infile[i][j], "*clef", 5) == 0) {
                  out << " " << "clef=";
                  printAbcClef(out, infile[i][j], 
                        infile[i].getPrimaryTrack(j), clefstates);
                  return;
               }
            }
         }
      } else if (infile[i].getType() == E_humrec_data) {
         // no clef found before start of music, so don't choose a clef
         return;
      }
	       
   }
}



//////////////////////////////
//
// getFirstMeasureNumber --
//

int getFirstMeasureNumber(HumdrumFile& infile) {
   int i;
   int output = -1;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() == E_humrec_data) {
         break;
      }
      if (infile[i].getType() == E_humrec_data_measure) {
         sscanf(infile[i][0], "=%d", &output);
         break;
      }
   }
   return output;
}



//////////////////////////////
//
// printAbcExtendedInformationFields --
//

void printAbcExtendedInformationFields(ostream& out, HumdrumFile& infile) {

   Array<int> bibfields;
   bibfields.setSize(infile.getNumLines());
   bibfields.setSize(0);

   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() == E_humrec_bibliography) {
         bibfields.append(i);
      }
   }

   Array<Array<char> > values;
   Array<char> value;
   value[0] = EMPTY;
   value.setSize(0);

   out << "%%abc-version 2.0" << "\n";
   out << "%%abcx-abcm2ps-target-version 5.9.1 (29 Sep 2008)" << "\n";
   out << "%%abc-creator hum2abc beta\n";

   struct tm *current;
   time_t now;
   time(&now);
   current = localtime(&now);
   out << "%%abcx-conversion-date ";
   out << current->tm_year + 1900 << "/";
   if (current->tm_mon+1 < 10) {
      out << "0";
   }
   out << current->tm_mon + 1     << "/";
   if (current->tm_mday < 10) {
      out << "0";
   }
   out << current->tm_mday        << " ";

   if (current->tm_hour < 10) {
      out << "0";
   }
   out << current->tm_hour        << ":";

   if (current->tm_min < 10) {
      out << "0";
   }
   out << current->tm_min         << ":";
   if (current->tm_sec < 10) {
      out << "0";
   }
   out << current->tm_sec         << "\n";

   // YEC = copyright notice
   if (findRecord("YEC", value, infile, bibfields)) {
      out << "%%abc-copyright " << value.getBase() << "\n";
   }

   if (findMultipleRecords("EED", values, infile, bibfields)) {
      for (i=0; i<values.getSize(); i++) {
         out << "%%abc-edited-by " << values[i].getBase() << "\n";
      }
   } 
   if (findMultipleRecords("ENC", values, infile, bibfields)) {
      for (i=0; i<values.getSize(); i++) {
         out << "%%abc-edited-by " << values[i].getBase() << "\n";
      }
   }
   if (findRecord("END", value, infile, bibfields)) {
      out << "%%abcx-initial-encoding-date " << value.getBase() << "\n";
   } else if (findRecord("RDT", value, infile, bibfields)) {
      out << "%%abcx-initial-encoding-date " << value.getBase() << "\n";
   }

   // make a use option eventually
   // but in any case, a check should be made
   // beforehand that there are grace notes in the file
   // before printing the following line:
   // doesn't look so good for monophonic parts, so turn off
   // if there is only one voice
   if ((Voicemap.getSize() > 1) && graceQ) {   
      if ((parameterstring != NULL) && 
          (strstr(parameterstring, "gracespace") == NULL)) {
         // only print default gracespace if not in -p option
         out << "%%gracespace 0 6 6\n";
      }
   }

   // it would be nice to be able to control the indent from the
   // data file, but the following does not work in abcm2ps 5.9.1:
   // out << "%indent 2.25 cm\n";
   
   if (notespacingQ) {
      // a value of 2.0 will increase the density of the music
      // a value of 1.0 will stretch out the music
      // the default value is the square-root of 2 (1.414)
      out << "%%notespacingfactor " << notespacing << "\n";
   }

   if (veritasQ) {
      printVeritas(out, infile);
   }
   if (landscapeQ) {
      out << "%%landscape" << "\n";
   }
   if (footerQ) {
      out << "%%footer \"" << footer << "\"" << "\n";
   }
   if (headerQ) {
      out << "%%header \"" << footer << "\"" << "\n";
   }

   if (musicscaleQ) {
      out << "%%scale " << musicscale << "\n";
   }

   if (continueQ) {
      out << "%%continueall 1" << "\n";
   }

   if (strlen(parameterstring) > 0) {
      Array<char> cleanedstring; 
      translateSpecialCharacters(cleanedstring, parameterstring);
      out << cleanedstring.getBase() << "\n";
   }
   
   // by default measure numbering will be turned on here.
   // add a user-option control to turn off the bar numbering.
   // %%barnumbers 0 = number the start of each system
   // %%barnumbers -1 = don't number measures
   // %%barnumbers 5 = number every fifth measure
   // %%barnumbers 1 = number every measure
   out << "%%barnumbers " << barnumberingstyle << "\n";
   if (boxQ) {
      out << "%%measurebox 1" << "\n";
   }


}



//////////////////////////////
//
// printVeritas -- print cksum data for input file
//

void printVeritas(ostream& out, HumdrumFile& infile) {
   unsigned long veritas = 0;
   int i;

   SSTREAM alllines;
   SSTREAM onlydata;

   Array<char> marker;
   marker[0] = EMPTY;
   marker.setSize(0);

   Array<char> contents;
   contents[0] = EMPTY;
   contents.setSize(0);

   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() == E_humrec_bibliography) {
         if (strncmp(infile[i][0], "!!!VTS:", 7) == 0) {
            if (infile[i].getFieldCount() > 1) {
               getBibPieces(infile[i][0], infile[i][1], marker, contents);
            } else {
               getBibPieces(infile[i][0], "", marker, contents);
            }
            sscanf(contents.getBase(), "%lu", &veritas);
            // don't store this line in the buffers since
            // it is not counted when calculating the veritas...
            continue;
         }
      }

      // printing with 0x0a which is the linux version
      // of newlines.  That is the only newline which
      // I am considering valid.  Apple will tend to
      // used 0x0d in text files, and Windows will use
      // both to make things difficult.  So in any case,
      // you should have converted the newlines in the
      // file to linux format before calculating your VTS
      // entry in the original file if you want hum2abc
      // to validate the file properly.
      alllines << infile[i] << char(0x0a);
      if (infile[i].getType() == E_humrec_data) {
         onlydata << infile[i] << char(0x0a);
      }

   }


   alllines << ends;
   onlydata << ends;

   int lenall = strlen(alllines.CSTRING);
   int lendata = strlen(onlydata.CSTRING);

   unsigned long crcall  = string_cksum(alllines.CSTRING, lenall);
   unsigned long crcdata = string_cksum(onlydata.CSTRING, lendata);

   if (veritas != 0) {
      if (veritas == crcall) {
         out << "%%humdrum-veritas " << veritas << "\n";
      } else {
         out << "%%humdrum-veritas " << crcall << "\n";
         out << "%%humdrum-veritas-invalid: " << veritas << "\n";
      }
   } else {
      out << "%%humdrum-veritas " << crcall << "\n";
   }
   out << "%%humdrum-veritas-data " << crcdata << "\n";

}



//////////////////////////////
//
// findMultipleRecords -- find multiple records with the same key
//

int findMultipleRecords(const char* key, Array<Array<char> >& values, 
      HumdrumFile& infile, Array<int>& bibfields) {

   values.setSize(bibfields.getSize());
   values.setSize(0);

   Array<char> marker;
   Array<char> tempv;

   tempv.setSize(1);
   tempv[0] = EMPTY;
   tempv.setSize(0);

   const char* ptr;
   int keylen = strlen(key);
   int i;
   for (i=0; i<bibfields.getSize(); i++) {
      ptr = &(infile[bibfields[i]][0][3]);
      if (strncmp(ptr, key, keylen) == 0) {
         if (infile[bibfields[i]].getFieldCount() > 1) {
            getBibPieces(infile[bibfields[i]][0], infile[bibfields[i]][1], 
               marker, tempv);
         } else {
            getBibPieces(infile[bibfields[i]][0], "", marker, tempv);
         }
	 if (tempv.getSize() > 1) {
            values.append(tempv);
            tempv.setSize(1);
            tempv[0] = EMPTY;
            tempv.setSize(0);
	 } else {
            // do nothing since the record is empty
         }
      }
   }
   return values.getSize();
}



//////////////////////////////
//
// findRecord --
//

int findRecord(const char* key, Array<char>& value, HumdrumFile& infile,
      Array<int>& bibfields) {

   Array<char> marker;
   value.setSize(1);
   value[0] = EMPTY;
   value.setSize(0);

   const char* ptr;
   int keylen = strlen(key);
   int i;
   for (i=0; i<bibfields.getSize(); i++) {
      ptr = &(infile[bibfields[i]][0][3]);
      if (strncmp(ptr, key, keylen) == 0) {
         if (infile[bibfields[i]].getFieldCount() > 1) {
            getBibPieces(infile[bibfields[i]][0], infile[bibfields[i]][1], 
                  marker, value);
         } else {
            getBibPieces(infile[bibfields[i]][0], "", marker, value);
         }
	 if (value.getSize() > 0) {
            return 1;
	 } else {
            return 0;
         }
      }
   }
   return 0;
}



//////////////////////////////
//
// printKeySignature --
//

void printKeySignature(ostream& out, HumdrumFile& infile) {
   int i;
   int keynum;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() == E_humrec_data) {
         break;
      }
      if (infile[i].getType() != E_humrec_interpretation) {
         continue;
      }
      if (strncmp(infile[i][0], "*k[", 3) == 0) {
         keynum = Convert::kernKeyToNumber(infile[i][0]);      
         StartKey = keynum;
         printAbcKeySignature(out, keynum);
	 return;
      }
   }

   // didn't find a keysignature, so use C major's key signature:
   out << "C";
}



//////////////////////////////
//
// printAbcKeySignature --
//

void printAbcKeySignature(ostream& out, int keynum) {
   switch (keynum) {
      case  7:  out << "C#"; return;
      case  6:  out << "F#"; return;
      case  5:  out << "B";  return;
      case  4:  out << "E";  return;
      case  3:  out << "A";  return;
      case  2:  out << "D";  return;
      case  1:  out << "G";  return;
      case  0:  out << "C";  return;
      case -1:  out << "F";  return;
      case -2:  out << "Bb"; return;
      case -3:  out << "Eb"; return;
      case -4:  out << "Ab"; return;
      case -5:  out << "Db"; return;
      case -6:  out << "Gb"; return;
      case -7:  out << "Cb"; return;
   }
}



//////////////////////////////
//
// printVoiceDeclaration -- Consider coordinating with printAbcClef().
//

void printVoiceDeclaration(ostream& out, VoiceMap vmap, int voicenum, 
      HumdrumFile& infile) {
 
   out << "V: " << voicenum;

   char clef[1024] = {0};

   int i, j;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() == E_humrec_data) {
         // stop processing when the data is found
         break;
      }
      if (infile[i].getType() == E_humrec_interpretation) {
         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (vmap.primary != infile[i].getPrimaryTrack(j)) {
               continue;
            }
            if (strcmp(infile[i][j], "*clefG2") == 0) {
               strcpy(clef, "clef=treble");
            } else if (strcmp(infile[i][j], "*clefGv2") == 0) {
               strcpy(clef, "clef=treble-8");
               clefstates[infile[i].getPrimaryTrack(j)] = +40;
            } else if (strcmp(infile[i][j], "*clefF4") == 0) {
               strcpy(clef, "clef=bass");
            } else if (strcmp(infile[i][j], "*clefC3") == 0) {
               strcpy(clef, "clef=alto");
            } else if (strcmp(infile[i][j], "*clefC4") == 0) {
               strcpy(clef, "clef=tenor");
            }
         }
      }
   }

   // if there is no clef, probably check here for
   // what a good default clef would be: do a histogram
   // of all of the notes in the part (until an 
   // explicit clef is found)

   if (strcmp(clef, "") != 0) {
      out << " " << clef;
   }

   out << "\n";
}



//////////////////////////////
//
// createVoiceMap
//

void createVoiceMap(Array<VoiceMap>& voicemap, HumdrumFile& infile) {

   voicemap.setSize(200);
   voicemap.setGrowth(200);
   voicemap.setSize(0);

   int i, j;
   VoiceMap vtemp;
   

   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() != E_humrec_interpretation) {
         continue;
      }
      if (strncmp(infile[i][0], "**", 2) != 0) {
         cerr << "Error: expecting an exclusive interpretation line." << endl;
         cerr << "Line is instead: " << infile[i] << endl;
         exit(1);
      }

      for (j=infile[i].getFieldCount()-1; j>=0; j--) {
         if (infile[i].getExInterpNum(j) == E_KERN_EXINT) {
            vtemp.primary = j+1;             // primary tracks start at 1
            voicemap.append(vtemp);
         }
      }
      break;
   }

   for (i=0; i<voicemap.getSize(); i++) {
      voicemap[i].voicenumber = i+1;
   }
}



//////////////////////////////
//
// parseBibliographic -- read the bibliographic records
//     in the humdrum file and fill in any ABC header records.
// Mapping:
//    !!!COM --> C (composer)
//    !!!OTL --> T (title) (also XEN = Title Translated into English)
//       ??? --> A (Area)
//       ??? --> B (Book)
//       ??? --> D (Discography)
//       ??? --> F (Filename)
//       ??? --> G (Group)
//       ??? --> H (History)
//       ??? --> I (Information)
//     *?:   --> K ([initial] Key)
//       ??? --> N (Notes)
//     *M?/? --> M ([initial] Meter)
//       ??? --> O (Origin)
//       ??? --> R (Rhythm)
//       ??? --> S (Source)
//       ??? --> Z (Transcription notes)
// Notes:
//    Only the first !!OTL/XEN record in the file will be used.
//    Only the first !!COM record in the file will be used.
//    Only the last  !!OMD record in the file will be used.
//

void parseBibliographic(Array<char*>& header, HumdrumFile& infile) {
   int i;
   Array<char> marker;
   marker.setSize(100);
   marker.setGrowth(100);
   marker.setSize(0);
   Array<char> contents;
   contents.setSize(100);
   contents.setGrowth(100);
   contents.setSize(0);
   int length;
   char omdstring[1024] = {0};

   Array<Array<char> > markers;
   Array<Array<char> > contentses;

   prepareBibliographicData(markers, contentses, infile);

   for (i=0; i<markers.getSize(); i++) {
      marker = markers[i];
      contents = contentses[i];
      length = strlen(contents.getBase());
      if (length == 0) {
         continue;
      }

      if (strncmp(marker.getBase(), "OTL", 3) == 0) {
         //if (strcmp(header[TT], "") == 0) {
         if ((!options.getBoolean("T")) && (strcmp(header[TT], "") == 0)) {
            storeHeaderRecord(header, 'T', contents.getBase());
         }
      } else if (strcmp(marker.getBase(), "XEN") == 0) {
         //if (strcmp(header[TT], "") == 0) {
         if ((!options.getBoolean("T")) && (strcmp(header[TT], "") == 0)) {
            storeHeaderRecord(header, 'T', contents.getBase());
         }
      }

      if (strcmp(marker.getBase(), "COM") == 0) {
         //if (strcmp(header[CC], "") == 0) {
         if ((!options.getBoolean("C")) && (strcmp(header[CC], "") == 0)) {
            flipCommaParts(contents);
            storeHeaderRecord(header, 'C', contents.getBase());
         }
      }

      if (strcmp(marker.getBase(), "OMD") == 0) {
         strcpy(omdstring, contents.getBase());
      }

   }

   // also read the first meter sign in the file which
   // occurs before any musical data as well as the
   // key.
 
   char meterbuffer[1024] = {0};
   double tempo = -1;
   int j;
   int top, bot;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() == E_humrec_data) {
         break;
      }
      if (infile[i].getType() == E_humrec_interpretation) {
         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (infile[i].getExInterpNum(j) == E_KERN_EXINT) {

               if (sscanf(infile[i][j], "*M%d/%d", &top, &bot) == 2) {
                  sprintf(meterbuffer, "%d/%d", top, bot);
                  storeHeaderRecord(header, 'M', meterbuffer);
                  break;  // don't get redundant info on same line
               }

               if (strcmp(infile[i][j], "*met(C)") == 0) {
                  storeHeaderRecord(header, 'M', "C");
                  top = bot = 4;
                  break;  // don't get redundant info on same line
               }
               if (strcmp(infile[i][j], "*met(c)") == 0) {
                  storeHeaderRecord(header, 'M', "C");
                  top = bot = 4;
                  break;  // don't get redundant info on same line
               }
               if (strcmp(infile[i][j], "*met(C|)") == 0) {
                  storeHeaderRecord(header, 'M', "C|");
                  top = bot = 2;
                  break;  // don't get redundant info on same line
               }
               if (strcmp(infile[i][j], "*met(c|)") == 0) {
                  storeHeaderRecord(header, 'M', "C|");
                  top = bot = 2;
                  break;  // don't get redundant info on same line
               }

               if (sscanf(infile[i][j], "*MM%lf", &tempo) == 1) {
                  break;  // don't get redundant info on same line
               }

            }
         }
      }
   }

   if ((strcmp(meterbuffer, "") == 0) && (strcmp(header[MM],"") == 0)) { 
      storeHeaderRecord(header, 'M', meterbuffer);
   }
   
   if (!options.getBoolean("Q")) {
      Array<char> QRecord;
      QRecord.setSize(0);
      if (strcmp(header[QQ], "") == 0) {
         if ((!notempoQ) && tempo > 0 || (strcmp(omdstring, "") != 0)) {
            if (options.getBoolean("q")) {
               calculateQRecord(QRecord, -1, omdstring, top, bot);
            } else {
               calculateQRecord(QRecord, tempo, omdstring, top, bot);
            }
         }
         if (QRecord.getSize() > 0) {
            storeHeaderRecord(header, 'Q', QRecord.getBase());
         }
      }
   }
        
}



//////////////////////////////
//
// parseBibliographicData -- read in the bibliographic data from the
//      Humdrum file, and interpret any !!!title: record and place
//      its parsed contents into the !!!OTL: record.
//

void prepareBibliographicData(Array<Array<char> >& markers, 
      Array<Array<char> >& contentses, HumdrumFile& infile) {

   markers.setSize(infile.getNumLines());
   markers.setSize(0);

   contentses.setSize(infile.getNumLines());
   contentses.setSize(0);

   Array<char> marker;
   Array<char> contents;

   int length;
   int titleindex = -1;
   int OTLindex   = -1;

   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() != E_humrec_bibliography) {
         continue;
      }
      if (infile[i].getFieldCount() > 1) {
         getBibPieces(infile[i][0], infile[i][1], marker, contents);
      } else {
         getBibPieces(infile[i][0], "", marker, contents);
      }

      length = strlen(contents.getBase());
      if (length == 0) {
         continue;
      }
      markers.append(marker);
      contentses.append(contents);
      if (strcmp(marker.getBase(), "title") == 0) {
         titleindex = markers.getSize()-1;
      } else if (strncmp(marker.getBase(), "OTL", 3) == 0) {
         OTLindex = markers.getSize()-1;
      } 
//     cout << "% REF = " << marker.getBase() 
//          << " : " << contents.getBase()
//          << endl;	        
   }


   if (titleexpansionQ) {
      length = strlen("title");
      marker.setSize(length+1);
      strcpy(marker.getBase(), "title");
      markers.append(marker);

      length = strlen(titleexpansion);
      contents.setSize(length+1);
      strcpy(contents.getBase(), titleexpansion);
      contentses.append(contents);
      
      titleindex = markers.getSize() - 1;
   }

   if (titleindex < 0) {
      return;
   }

   Array<char>& ct = contentses[titleindex];

   int keyq = 0;
   Array<char> key;
   key.setSize(100);
   key.setGrowth(100);
   key.setSize(0);

   Array<char> newtitle;
   newtitle.setSize(1000);
   newtitle.setGrowth(1000);
   newtitle.setSize(0);
   char ch;

   for (i=0; i<ct.getSize(); i++) {
      ch = ct[i];
      if ((keyq == 0) && (ct[i] != '@')) {
         newtitle.append(ch);
      } else if ((ct[i] == '@') && (i < ct.getSize()-1) && (ct[i+1] == '{')) {
         keyq = 1;
         i++;
      } else if (keyq && (ct[i] == '}')) {
         keyq = 0;
         ch = '\0';
         key.append(ch);
         addContentToString(newtitle, key, markers, contentses);
	 key.setSize(0);
      } else if (keyq) {
         key.append(ch);
      } else {
         ch = ct[i];
         newtitle.append(ch);
      }
   }

   ch = '\0';
   newtitle.append(ch);

   if (OTLindex < 0) {
      // create a new OTL record
      Array<char> key;
      key.setSize(4);
      strcpy(key.getBase(), "OTL");
      Array<char> con;
      con = newtitle;
      markers.append(key);
      contentses.append(con);
   } else {
      // replace the OTL record
      contentses[OTLindex] = newtitle;
   }
}



//////////////////////////////
//
// addContentToString --
//

void addContentToString(Array<char>& newtitle, Array<char>& key, 
      Array<Array<char> >& markers, Array<Array<char> >& contentses) {
   int i;
   int j;
   char ch;
   for (i=0; i<markers.getSize(); i++) {
      if (strcmp(markers[i].getBase(), key.getBase()) == 0) {
         for (j=0; j<contentses[i].getSize(); j++) {
            if (contentses[i][j] == '\0') {
               break;
            } else {
               ch = contentses[i][j];
               newtitle.append(ch);
            }
         }
         break;
      }
   }
}



//////////////////////////////
//
// flipCommaParts -- reverse the order of the strings
//    which has one comma in it.   Intended for reversing 
//    the composer's last and first names.
//

void flipCommaParts(Array<char>& contents) {
   char* ptr1;  // the first occurence of a comma
   char* ptr2;  // the last occurence of a comma
   char* ptr3;  // the start of the string
   int length = contents.getSize();
   char buffer[length*2];
   int i;
   for (i=0; i<length; i++) {
      buffer[i] = EMPTY;
   }

   ptr1 = strchr(contents.getBase(), ',');
   ptr2 = strrchr(contents.getBase(), ',');
   ptr3 = contents.getBase();

   if (ptr1 == NULL) {
      return;  // there was no comma in the string.
   }

   if (ptr1 != ptr2) { 
      return;  // there were two commas in the string.
               // so return, since the correct comma
               // to flip cannot be decided.
   }


   i = 0;
   ptr1++;
   while ((*ptr1 != EMPTY) && isspace(*ptr1)) {
      ptr1++;
   }
   while ((i<length-1) && (*ptr1 != EMPTY)) {
      buffer[i++] = *ptr1++;
   }
   while (i>0) {
      if (isspace(buffer[i-1])) {
         i--;
      } else {
         break;
      }
   }
   buffer[i++] = ' ';
   while ((*ptr3 != EMPTY) && isspace(*ptr3)) {
      ptr3++;
   }
   while ((i<length-1) && (*ptr3 != ',') && (*ptr3 != EMPTY)) {
      buffer[i++] = *ptr3++;
   }
   while (i>0) {
      if (isspace(buffer[i-1])) {
         i--;
      } else {
         break;
      }
   }
  
   buffer[i] = EMPTY; 
   length = strlen(buffer);
   contents.setSize(length+1);
   strcpy(contents.getBase(), buffer);
}



///////////////////////////////
//
// calculateQRecord -- assemble the tempo and movement designation
// to be stored on a Q: line.  Traditional tempo markings are:
//      40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 63, 66, 69, 
//      72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120,
//      126, 132, 138, 144, 152, 160, 168, 176, 184, 192, 200, and 208.
//  Round to the nearest of these markings when converting to a
//  base duration other than quarter notes.  Probably should have a
//  user flag which prevents this rounding from occurring if an
//  explicit non-standard tempo marking is desired. (or allow 
//  decimal digits in the tempo as well?)
//

void calculateQRecord(Array<char>& QRecord, double tempo, 
      const char* omdstring, int top, int bot) {

   char ch;
   int i;
   char tempostring[1024] = {0};
   QRecord.setSize(1024);
   QRecord.setSize(0);
   char textbuffer[1024] = {0};
   int length;

   // probably check the omdstring for quotes and remove/handle them
   length = strlen(omdstring);
   if (strlen(omdstring) > 0) {
      strcpy(textbuffer, "\"");
      strcat(textbuffer, omdstring);
      strcat(textbuffer, "\"");
      translateSpecialCharacters(QRecord, textbuffer);
      QRecord.setSize(QRecord.getSize()-1); // remove string terminator
      if (tempo > 0) {
         ch = ' ';
         QRecord.append(ch);
      }
   }

   int temportop = 1;
   int temporbot = 4;

   double fraction = (double)bot / (double)temporbot;
   temporbot = bot;
   tempo = tempo * fraction;

   if (top % 3 == 0) {
      // check for compound tempo display
      if (tempo >= 180) {
         tempo = tempo / 3.0;
         temportop = temportop * 3;
      }
   } else {
      // duple type meters
      if (tempo < 40) {
         // don't print a ridiculously slow tempo
         // this is for a duple meter, also do for triple meters...
         tempo *= 2;
         temporbot *= 2;
      } else if (tempo > 208) {
         // don't print a ridiculously fast tempo
         // this is for a duple meter, also do for triple meters...
         tempo /= 2;
         temportop *= 2;
      }
	        
   }

   simplifyFraction(temportop, temporbot);

   int tempoint;
   tempoint = roundTempoToNearestStandard(tempo);
   // tempoint = int(tempo + 0.5);



   sprintf(tempostring, "%d/%d=%d", temportop, temporbot, tempoint);

   if ((!notempoQ) && (tempoint > 0)) {
      length = strlen(tempostring);
      for (i=0; i<length; i++) {
         ch = tempostring[i];
         QRecord.append(ch);
      }
   }
   
   ch = EMPTY;
   QRecord.append(ch);
}


//////////////////////////////
//
// roundTempoToNearestStandard -- round to one of the values:
//      40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 63, 66, 69, 
//      72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120,
//      126, 132, 138, 144, 152, 160, 168, 176, 184, 192, 200, and 208.
// (from the original Maezel Metronome device).
//


int roundTempoToNearestStandard(double tempo) {
   if (tempo < 39.0) {
      return int(tempo+0.5);
   }
   if (tempo > 209.0) {
      return int(tempo+0.5);
   }

   Array<int> standardtempos;
   standardtempos.setSize(100);

   int stm;  // a standard tempo mark
   standardtempos.setSize(0);
   stm = 40; standardtempos.append(stm);
   stm = 42; standardtempos.append(stm);
   stm = 44; standardtempos.append(stm);
   stm = 46; standardtempos.append(stm);
   stm = 48; standardtempos.append(stm);
   stm = 50; standardtempos.append(stm);
   stm = 52; standardtempos.append(stm);
   stm = 54; standardtempos.append(stm);
   stm = 56; standardtempos.append(stm);
   stm = 58; standardtempos.append(stm);
   stm = 60; standardtempos.append(stm);
   stm = 63; standardtempos.append(stm);
   stm = 66; standardtempos.append(stm);
   stm = 69; standardtempos.append(stm);
   stm = 72; standardtempos.append(stm);
   stm = 76; standardtempos.append(stm);
   stm = 80; standardtempos.append(stm);
   stm = 84; standardtempos.append(stm);
   stm = 88; standardtempos.append(stm);
   stm = 92; standardtempos.append(stm);
   stm = 96; standardtempos.append(stm);
   stm = 100; standardtempos.append(stm);
   stm = 104; standardtempos.append(stm);
   stm = 108; standardtempos.append(stm);
   stm = 112; standardtempos.append(stm);
   stm = 116; standardtempos.append(stm);
   stm = 120; standardtempos.append(stm);
   stm = 126; standardtempos.append(stm);
   stm = 132; standardtempos.append(stm);
   stm = 138; standardtempos.append(stm);
   stm = 144; standardtempos.append(stm);
   stm = 152; standardtempos.append(stm);
   stm = 160; standardtempos.append(stm);
   stm = 168; standardtempos.append(stm);
   stm = 176; standardtempos.append(stm);
   stm = 184; standardtempos.append(stm);
   stm = 192; standardtempos.append(stm);
   stm = 200; standardtempos.append(stm);
   stm = 208; standardtempos.append(stm);

   Array<int>& ST = standardtempos;

   double diff1;
   double diff2;

   int i;
   for (i=1; i<ST.getSize(); i++) {
      diff1 = ST[i] - tempo;
      if (diff1 < 0) {
         continue;
      }
      if (diff1 == 0.0) {
         return ST[i];
      }
      diff2 = tempo - ST[i-1];
      if (diff1 < diff2) {
         return ST[i];
      } else {
         return ST[i-1];
      }
   }

   return int(tempo+0.5);
}



//////////////////////////////
//
// translateSpecialCharacters -- convert HTML accents (or other
//     accented character formats) into ABC accents.
//
// "To typeset a macron on a letter x, type \=x. To typeset an ogonek,
// type \;x. To typeset a caron, type \vx. To typeset a breve, type \ux. To
// typeset a long Hungarian umlaut, type \:x. Finally, to typeset a dotted
// letter, type \.x.
// http://abc.sourceforge.net/standard/abc2-draft.html#Full%20table%20of%20accented%20letters
//

void translateSpecialCharacters(Array<char>& output, const char* input) {
   int length = strlen(input);
   output.setSize(int(length * 1.5));
   output.setGrowth(output.getSize());
   output.setSize(0);
   char ch;

   int i;
   for (i=0; i<length; i++) {
      // if (input[i] == '&') { // now checking for many types of 
      // special characters... not just starting with &

      // lower-case grave accented vowels:
      if (strncmp(&(input[i]), "&agrave;", strlen("&agrave;")) == 0) {
         ch = '\\'; output.append(ch); ch = '`'; output.append(ch);
         ch = 'a'; output.append(ch);  i += strlen("&agrave;");
      } else if (strncmp(&(input[i]), "&egrave;", strlen("&egrave;")) == 0) {
         ch = '\\'; output.append(ch); ch = '`'; output.append(ch);
         ch = 'e'; output.append(ch);  i += strlen("&egrave;");
      } else if (strncmp(&(input[i]), "&igrave;", strlen("&igrave;")) == 0) {
         ch = '\\'; output.append(ch); ch = '`'; output.append(ch);
         ch = 'i'; output.append(ch);  i += strlen("&igrave;");
      } else if (strncmp(&(input[i]), "&ograve;", strlen("&ograve;")) == 0) {
         ch = '\\'; output.append(ch); ch = '`'; output.append(ch);
         ch = 'o'; output.append(ch);  i += strlen("&ograve;");
      } else if (strncmp(&(input[i]), "&ugrave;", strlen("&ugrave;")) == 0) {
         ch = '\\'; output.append(ch); ch = '`'; output.append(ch);
         ch = 'u'; output.append(ch);  i += strlen("&ugrave;");

      // upper-case grave accented vowels:
      } else if (strncmp(&(input[i]), "&Agrave;", strlen("&Agrave;")) == 0) {
         ch = '\\'; output.append(ch); ch = '`'; output.append(ch);
         ch = 'A'; output.append(ch);  i += strlen("&Agrave;");
      } else if (strncmp(&(input[i]), "&Egrave;", strlen("&Egrave;")) == 0) {
         ch = '\\'; output.append(ch); ch = '`'; output.append(ch);
         ch = 'E'; output.append(ch);  i += strlen("&Egrave;");
      } else if (strncmp(&(input[i]), "&Igrave;", strlen("&Igrave;")) == 0) {
         ch = '\\'; output.append(ch); ch = '`'; output.append(ch);
         ch = 'I'; output.append(ch);  i += strlen("&Igrave;");
      } else if (strncmp(&(input[i]), "&Ograve;", strlen("&Ograve;")) == 0) {
         ch = '\\'; output.append(ch); ch = '`'; output.append(ch);
         ch = 'O'; output.append(ch);  i += strlen("&Ograve;");
      } else if (strncmp(&(input[i]), "&Ugrave;", strlen("&Ugrave;")) == 0) {
         ch = '\\'; output.append(ch); ch = '`'; output.append(ch);
         ch = 'U'; output.append(ch);  i += strlen("&Ugrave;");

      // lower-case acute accented vowels:
      } else if (strncmp(&(input[i]), "&aacute;", strlen("&aacute;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\''; output.append(ch);
         ch = 'a'; output.append(ch);  i += strlen("&aacute;");
      } else if (strncmp(&(input[i]), "&eacute;", strlen("&eacute;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\''; output.append(ch);
         ch = 'e'; output.append(ch);  i += strlen("&eacute;");
      } else if (strncmp(&(input[i]), "&iacute;", strlen("&iacute;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\''; output.append(ch);
         ch = 'i'; output.append(ch);  i += strlen("&iacute;");
      } else if (strncmp(&(input[i]), "&oacute;", strlen("&oacute;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\''; output.append(ch);
         ch = 'o'; output.append(ch);  i += strlen("&oacute;");
      } else if (strncmp(&(input[i]), "&uacute;", strlen("&uacute;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\''; output.append(ch);
         ch = 'u'; output.append(ch);  i += strlen("&uacute;");

      // upper-case acute accented vowels:
      } else if (strncmp(&(input[i]), "&Aacute;", strlen("&Aacute;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\''; output.append(ch);
         ch = 'A'; output.append(ch);  i += strlen("&Aacute;");
      } else if (strncmp(&(input[i]), "&Eacute;", strlen("&Eacute;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\''; output.append(ch);
         ch = 'E'; output.append(ch);  i += strlen("&Eacute;");
      } else if (strncmp(&(input[i]), "&Iacute;", strlen("&Iacute;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\''; output.append(ch);
         ch = 'I'; output.append(ch);  i += strlen("&Iacute;");
      } else if (strncmp(&(input[i]), "&Oacute;", strlen("&Oacute;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\''; output.append(ch);
         ch = 'O'; output.append(ch);  i += strlen("&Oacute;");
      } else if (strncmp(&(input[i]), "&Uacute;", strlen("&Uacute;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\''; output.append(ch);
         ch = 'U'; output.append(ch);  i += strlen("&Uacute;");

      // lower-case umlaut accented vowels:
      } else if (strncmp(&(input[i]), "&auml;", strlen("&auml;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\"'; output.append(ch);
         ch = 'a'; output.append(ch);  i += strlen("&auml;");
      } else if (strncmp(&(input[i]), "&euml;", strlen("&euml;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\"'; output.append(ch);
         ch = 'e'; output.append(ch);  i += strlen("&euml;");
      } else if (strncmp(&(input[i]), "&iuml;", strlen("&iuml;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\"'; output.append(ch);
         ch = 'i'; output.append(ch);  i += strlen("&iuml;");
      } else if (strncmp(&(input[i]), "&ouml;", strlen("&ouml;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\"'; output.append(ch);
         ch = 'o'; output.append(ch);  i += strlen("&ouml;");
      } else if (strncmp(&(input[i]), "&uuml;", strlen("&uuml;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\"'; output.append(ch);
         ch = 'u'; output.append(ch);  i += strlen("&uuml;");

      // upper-case umlaut accented vowels:
      } else if (strncmp(&(input[i]), "&Auml;", strlen("&Auml;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\''; output.append(ch);
         ch = 'A'; output.append(ch);  i += strlen("&Auml;");
      } else if (strncmp(&(input[i]), "&Euml;", strlen("&Euml;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\''; output.append(ch);
         ch = 'E'; output.append(ch);  i += strlen("&Euml;");
      } else if (strncmp(&(input[i]), "&Iuml;", strlen("&Iuml;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\''; output.append(ch);
         ch = 'I'; output.append(ch);  i += strlen("&Iuml;");
      } else if (strncmp(&(input[i]), "&Ouml;", strlen("&Ouml;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\''; output.append(ch);
         ch = 'O'; output.append(ch);  i += strlen("&Ouml;");
      } else if (strncmp(&(input[i]), "&Uuml;", strlen("&Uuml;")) == 0) {
         ch = '\\'; output.append(ch); ch = '\''; output.append(ch);
         ch = 'U'; output.append(ch);  i += strlen("&Uuml;");
      
      // other types of accented characters ////////////////////////////

      // aring : a with a circle above it (for Nordic langauges):
      } else if (strncmp(&(input[i]), "&aring;", strlen("&aring;")) == 0) {
         ch = '\\'; output.append(ch); ch = 'a'; output.append(ch);
         ch = 'a'; output.append(ch);  i += strlen("&aring;");
      } else if (strncmp(&(input[i]), "&Aring;", strlen("&Aring;")) == 0) {
         ch = '\\'; output.append(ch); ch = 'a'; output.append(ch);
         ch = 'a'; output.append(ch);  i += strlen("&Aring;");


      // accidental symbols in titles ///////////////////////////////////
      // \b --> flat sign does not work...
      } else if (strncmp(&(input[i]), "\\n", strlen("\\n")) == 0) {
         ch = '\n'; output.append(ch); i += strlen("\\n");
      } else if (strncmp(&(input[i]), "-flat", strlen("-flat")) == 0) {
         ch = '\\'; output.append(ch); ch = 'b'; output.append(ch);
                                       i += strlen("-flat");
      // \# --> sharp sign in titles does work.
      } else if (strncmp(&(input[i]), "-sharp", strlen("-sharp")) == 0) {
         ch = '\\'; output.append(ch); ch = '#'; output.append(ch);
                                       i += strlen("-sharp");
      // Haven't tested if \= --> natural sign works.
      } else if (strncmp(&(input[i]), "-natural", strlen("-natural")) == 0) {
         ch = '\\'; output.append(ch); ch = '='; output.append(ch);
                                       i += strlen("-natural");


      // other accented characters
      } else if (strncmp(&(input[i]), "&ccedil;", strlen("&ccedil;")) == 0) {
         ch = '\\'; output.append(ch); ch = 'c'; output.append(ch);
         ch = 'c'; output.append(ch);  i += strlen("&ccedil;");
      } else if (strncmp(&(input[i]), "&Ccdeil;", strlen("&Ccdeil;")) == 0) {
         ch = '\\'; output.append(ch); ch = 'C'; output.append(ch);
         ch = 'C'; output.append(ch);  i += strlen("&Ccdeil;");
      } else if (strncmp(&(input[i]), "&szlig;", strlen("&szlig;")) == 0) {
         ch = '\\'; output.append(ch); ch = 's'; output.append(ch);
         ch = 's'; output.append(ch);  i += strlen("&szlig;");


      }
   
      // other accented characters which map to non-number strings:
      /* Need to add these
       * Ntilde = \~N, ntilde = \~n
       * oslash = \/o, Oslash = \/O
       * ocirc = \^o, Ocirc = \^O
       * aelig = \ae, AElig = \AE, OElig = \OE, oelig = \oelig
       * Also, others less common ones like thorn, Atilde, etc.
       * -sharp -> sharp sign, likewise for -flat and -natural
       */
         
      ch = input[i];
      output.append(ch);
   }

   ch = EMPTY;
   output.append(ch);
}



//////////////////////////////
//
// storeHeaderRecord --
//

typedef char const * charstring;

void storeHeaderRecord(Array<char*>& header, char recordletter, 
      const char* inputstring) {

   Array<char> parsedstring;
   translateSpecialCharacters(parsedstring, inputstring);

   int length = strlen(parsedstring.getBase());
   
   int index = recordletter - 'A';

   int startindex = 0;
   while (startindex<length) {
      if (isspace(parsedstring[startindex])) {
         startindex++;
      } else {
         break;
      }
   }

   int newlength = length - startindex;

   delete [] header[index];
   header[index] = new char[newlength+1];
   strcpy(header[index], parsedstring.getBase()+startindex);

   length = strlen(header[index]);
   int i;
   for (i=length-1; i>0; i--) {
      if (isspace(header[index][i])) {
         header[index][i] = EMPTY;
      } else {
         break;
      }
   }
}



//////////////////////////////
//
// getBibPieces --
//

void getBibPieces(const char* string, const char* string2, 
      Array<char>& marker, Array<char>& contents) {

   char bufftemp[strlen(string) + strlen(string2)];
   strcpy(bufftemp, string);
   strcat(bufftemp, string2);

   char ch;
   char empty = EMPTY;

   marker.setSize(1);
   contents.setSize(1);
   marker[0]   = EMPTY;
   contents[0] = EMPTY;
   marker.setSize(0);
   contents.setSize(0);

   int length = strlen(bufftemp);
   if (length < 4) {
      return;
   }
   
   int i = 3;
   while ((i<length) && (bufftemp[i] != '\0') && (bufftemp[i] != ':')) {
      ch = bufftemp[i];
      marker.append(ch);
      i++;
   }
   i++;
   marker.append(empty);

   while ((i<length) && (bufftemp[i] != '\0') && isspace(bufftemp[i])) {
      i++;
   }
   while ((i < length) && (bufftemp[i] != '\0')) {
      ch = bufftemp[i];
      contents.append(ch);
      i++;
   }

   
   for (i=contents.getSize()-1; i>0; i--) {
      if (isspace(contents[i])) {
         contents[i] = empty;
	 contents.setSize(i);
      } else if (contents[i] == 0x0d) {
         contents[i] = empty;
	 contents.setSize(i);
      } else if (contents[i] == 0x0a) {
         contents[i] = empty;
	 contents.setSize(i);
      } else if (contents[i] == EMPTY) {
         contents[i] = empty;
	 contents.setSize(i);
      } else {
         break;
      }
   }
   contents.append(empty);

   char buffer[1024];
   strcpy(buffer, contents.getBase());
   translateSpecialCharacters(contents, buffer);


}



//////////////////////////////
//
// calculateBestRhythmUnit --
//

void calculateBestRhythmUnit(HumdrumFile& infile, int& Ltop, int& Lbot) {
   int i, j;
   int tcount;
   char buffer[128] = {0};
   Array<int> rhythms;
   rhythms.setSize(17);
   rhythms.allowGrowth(0);
   rhythms.setAll(0);
   int rhythm;
   double duration;

   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() != E_humrec_data) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (infile[i].getExInterpNum(j) != E_KERN_EXINT) {
            continue;
         }
         if (strcmp(infile[i][j], ".") == 0) {
            continue;  // ignore null tokens
         }
         tcount = infile[i].getTokenCount(j);
         if (tcount > 1) {
            infile[i].getToken(buffer, j, 0);
	    if (norhythm(buffer)) {
               duration = 1.0;
            } else {
               duration = Convert::kernToDuration(buffer);
            }
         } else {
            if (norhythm(buffer)) {
               duration = 1.0;
            } else {
               duration = Convert::kernToDuration(infile[i][j]);
            }
         }
         Convert::durationToKernRhythm(buffer, duration);
         if (sscanf(buffer, "%d", &rhythm) == 1) {
            if (rhythm >= 16) {
               rhythm = 16;
            } else if (rhythm >= 8) {
               rhythm = 8;
            } else if (rhythm >= 4) {
               rhythm = 4;
            } else {
               rhythm = 0;
            }
            rhythms[rhythm] += tcount;
         }
      }
   }

   int maxx = 8;
   if (rhythms[4] > rhythms[8]) {
      maxx = 4;
   }
   if (rhythms[16] > rhythms[maxx]) {
      maxx = 16;
   }
   if ((maxx == 4) && (rhythms[4] < rhythms[8] + rhythms[16])) {
      maxx = 8;
   }

   Ltop = 1;
   Lbot = maxx;
}



//////////////////////////////
//
// base40ToAbcPitch -- convert a base40 pitch to ABC format
//    C=c4, c=c5, c'=c6, C,=c3, C,,=c2, c''=c7
// The pitch is really ambiguous, since in a bass clef, c can
//    be c3 rather than c5.  
//

char* base40ToAbcPitch(char* buffer, int base40) {

   if (base40 < 0) {
      strcpy(buffer, "z");
      return buffer;
   }
	  
   int octave = base40 / 40;
   int diatonic = Convert::base40ToDiatonic(base40);

   // int accidental = Convert::base40ToAccidental(
   // the accidental will not be printed with this
   // function, you will have to decide from the
   // musical context if the accidental should be
   // displayed or suppressed based on the key
   // signature;

   char dianame[2];
   strcpy(dianame, "x");

   switch (diatonic) {
      case 0:  strcpy(dianame, "c"); break;
      case 1:  strcpy(dianame, "d"); break;
      case 2:  strcpy(dianame, "e"); break;
      case 3:  strcpy(dianame, "f"); break;
      case 4:  strcpy(dianame, "g"); break;
      case 5:  strcpy(dianame, "a"); break;
      case 6:  strcpy(dianame, "b"); break;
   }
   if (octave < 5) {
      dianame[0] = toupper(dianame[0]);
   }
   
   strcpy(buffer, dianame);
 
   switch (octave) {
      case 6: strcat(buffer, "'");    break;
      case 7: strcat(buffer, "''");   break;
      case 8: strcat(buffer, "'''");  break;
      case 9: strcat(buffer, "''''"); break;
      case 3: strcat(buffer, ",");    break;
      case 2: strcat(buffer, ",,");   break;
      case 1: strcat(buffer, ",,,");  break;
      case 0: strcat(buffer, ",,,,"); break;
   }

   return buffer;
}



//////////////////////////////
//
// storeOptionSet --
//

void storeOptionSet(Options& opts) {
   opts.define("debug=b",    "Print extract debugging statements");
   opts.define("m|measures-per-line=i:1", "Number of measures to print on a text line");
   opts.define("header=s:", "Command string for printing headers on pages");
   opts.define("f|footer=s:", "Command string for printing footers on pages");
   opts.define("label=b",    "Explicitly write measure number for every bar");
   opts.define("dir|directory=s", "directory for reading files from");
   opts.define("mask=s:.krn",  "filemask for reading from a directory");
   opts.define("no-invisible=b", "Print all invisible items as visible");
   opts.define("no-veritas=b", "Don't calculate veritas data");
   opts.define("filenum=s:",   "Prepend a filenumber value infront of title");
   opts.define("filetitle=b",  "Print filename at start of title field");
   opts.define("TT=s:",        "Title expansion");
   opts.define("no-grace=b",   "Suppress gracespace redefinition");
   opts.define("no-tempo=b",   "Suppress tempo marking at start of music");
   opts.define("nn|no-auto-natural=b",   "Suppress automatic naturals");
   opts.define("linebreak=b",   "Break lines at !linebreak tokens");

   // abcm2ps parameter settings
   opts.define("box=b",           "Put boxes around measure numbers");
   opts.define("no-autoformat=b", "Linewrap music same as in text");
   opts.define("spacing=d:1.141", "Note spacing factor");
   opts.define("n|barnums=i:0",   "Measure numbering control");
   opts.define("s|scale=d:0.75",  "Set the music scaling factor");
   opts.define("landscape=b",     "Rotate the music 90 degrees");
   opts.define("p=s:",            "Parameter strings to echo into header");
   opts.define("no-indent=b",     "Not actually used directly");

   opts.define("A=s",   "A header record (Area -- origin inside of country)");
   opts.define("B=s",   "B header record (Book -- source publication)");
   opts.define("C=s",   "C header record (Composer)");
   opts.define("D=s",   "D header record (Discography)");
   opts.define("E=s",   "E header record");
   opts.define("F=s",   "F header record (Filename)");
   opts.define("G=s",   "G header record (Group -- instrumentation)");
   opts.define("H=s",   "H header record (History -- source notes)");
   opts.define("I=s",   "I header record (Information)");
   opts.define("J=s",   "J header record");
   opts.define("K=s",   "K header record (Key signature)");
   opts.define("L=s",   "L header record");
   opts.define("M=s",   "M header record (Meter)");
   opts.define("N=s",   "N header record (Notes)");
   opts.define("O=s",   "O header record (Origin -- country of origin)");
   opts.define("P=s",   "P header record");
   opts.define("Q=s",   "Q header record");
   opts.define("q=b",   "don't display tempo, only OMD record");
   opts.define("R=s",   "R header record (Rhythm)");
   opts.define("S=s",   "S header record (Source -- source of data)");
   opts.define("T=s",   "T header record (Title)");
   opts.define("U=s",   "U header record");
   opts.define("V=s",   "V header record");
   opts.define("W=s",   "W header record");
   opts.define("X=s",   "X header record (tune serial number)");
   opts.define("Y=s",   "Y header record");
   opts.define("Z=s",   "Z header record (Transcription note)");

   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("h|help=b",  "short description");

}



//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[], int fcount,
      HumdrumFile& cinfile) {
   opts.reset();
   storeOptionSet(opts);
   opts.setOptions(argc, argv);
   opts.process();
   debugQ = opts.getBoolean("debug"); // needed before final processing
   int tdirectoryQ = opts.getBoolean("directory");

   HumdrumFile tempfile;
   if (opts.getArgumentCount() == 0) {
      if (!tdirectoryQ) {
         cinfile.read(std::cin);
         tempfile = cinfile;
      }
   } else {
      tempfile.read(opts.getArg(fcount));
   }

   int i;
   Array<char> marker;
   Array<char> contents;

   opts.reset();
   storeOptionSet(opts);

   opts.setOptions(1, argv); // store only the command name

   if (!tdirectoryQ) {
      for (i=0; i<tempfile.getNumLines(); i++) {
         if (tempfile[i].getType() == E_humrec_bibliography) {
            if (strncmp(tempfile[i][0], "!!!hum2abc:", strlen("!!!hum2abc:"))==0) {
               if (tempfile[i].getFieldCount() > 1) {	
                  getBibPieces(tempfile[i][0], tempfile[i][1], marker,  contents);
               } else {
                  getBibPieces(tempfile[i][0], "", marker,  contents);
               }
	        

               if (debugQ) {
                  cout << "%Command-line addition: " << marker.getBase()
                       << " :: " << contents.getBase() << endl;
               }
               opts.appendOptions(contents.getBase());
            }
         }
      }
   }
   opts.appendOptions(argc-1, argv+1); // exclude the command name
   opts.process();

   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, October 2008" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 11 October 2008" << endl;
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
   
   // Array<char*> header;
   
   Header.setSize(26);
   for (i=0; i<Header.getSize(); i++) {
      Header[i] = new char[1];
      Header[i][0] = EMPTY;
   }

   // store any changes user input into the header records
   char optchar[2] = {0};
   int length;
   for (i=0; i<26; i++) {
      optchar[0] = char('A'+i);
      length = strlen(opts.getString(optchar));
      if (length > 0) {
         delete [] Header[i];
         Header[i] = new char[length+1];
      }
      storeHeaderRecord(Header, char('A'+i), opts.getString(optchar));
   }

   debugQ = opts.getBoolean("debug");
   linemeasure = opts.getInteger("measures-per-line");
   if (linemeasure < 0) {
      linemeasure = 1;
   }

   barnumberingstyle = opts.getInteger("barnums");
   if (strcmp(opts.getString("barnums"), "none") == 0) {
      barnumberingstyle = -1;
   }

   continueQ = !opts.getBoolean("no-autoformat");
   footerQ   = opts.getBoolean("footer");
   headerQ   = opts.getBoolean("header");
   if (footerQ) {
      footer = opts.getString("footer");
   }
   if (headerQ) {
      header = opts.getString("header");
   }

   labelQ    = opts.getBoolean("label");
   graceQ    = !opts.getBoolean("no-grace");
   notempoQ  = opts.getBoolean("no-tempo");
   veritasQ  = !opts.getBoolean("no-veritas");
   boxQ      = opts.getBoolean("box");
   if (strchr(opts.getString("barnums"), 'b') != NULL) {
      // the presence of a b after the measure number option
      // indicates that the user wants the bar numbers enclosed
      // in a box.
      boxQ = 1;
   }

   notespacingQ = opts.getBoolean("spacing");
   notespacing  = opts.getDouble("spacing");
   invisibleQ   = !opts.getBoolean("no-invisible");
   if (!invisibleQ) {
      invisiblerest = "z";
   }

   directoryQ      = opts.getBoolean("directory");
   directoryname   = opts.getString("directory");
   filemask        = opts.getString("mask");

   landscapeQ      = opts.getBoolean("landscape");
   musicscaleQ     = opts.getBoolean("scale");
   musicscale      = opts.getDouble("scale");
   parameterstring = opts.getString("p");
   if (strstr(parameterstring, "nostems") != NULL) {
      stemlessQ = 1;
   }
   filenumQ        = opts.getBoolean("filenum");
   filenumstring   = opts.getString("filenum");
   filenametitleQ  = opts.getBoolean("filetitle");
   titleexpansionQ = opts.getBoolean("TT");
   titleexpansion  = opts.getString("TT");
   nonaturalQ      = opts.getBoolean("no-auto-natural");

   linebreakQ      = opts.getBoolean("linebreak");
   if (linebreakQ) {
      continueQ = 0;      // don't allow abcm2ps to reformat the lines of music
      linemeasure = 10000;  // set measures per line very high
   }
          
}


///////////////////////////////
//
// string_cksum -- returns the same as the command-line cksum program
// (just the first number, not the size in bytes of the input string).
//

unsigned long string_cksum(const char* buf, int length) {
   static long int const crctab[256] = {
      0x00000000,
      0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
      0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6,
      0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
      0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac,
      0x5bd4b01b, 0x569796c2, 0x52568b75, 0x6a1936c8, 0x6ed82b7f,
      0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a,
      0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
      0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58,
      0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033,
      0xa4ad16ea, 0xa06c0b5d, 0xd4326d90, 0xd0f37027, 0xddb056fe,
      0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
      0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4,
      0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
      0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5,
      0x2ac12072, 0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
      0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca, 0x7897ab07,
      0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c,
      0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1,
      0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
      0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b,
      0xbb60adfc, 0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698,
      0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d,
      0x94ea7b2a, 0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
      0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2, 0xc6bcf05f,
      0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
      0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80,
      0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
      0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a,
      0x58c1663d, 0x558240e4, 0x51435d53, 0x251d3b9e, 0x21dc2629,
      0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c,
      0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
      0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e,
      0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65,
      0xeba91bbc, 0xef68060b, 0xd727bbb6, 0xd3e6a601, 0xdea580d8,
      0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
      0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2,
      0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
      0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74,
      0x857130c3, 0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
      0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c, 0x7b827d21,
      0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a,
      0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e, 0x18197087,
      0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
      0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d,
      0x2056cd3a, 0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce,
      0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb,
      0xdbee767c, 0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
      0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4, 0x89b8fd09,
      0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
      0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf,
      0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
   };

   unsigned long crc = 0;
   for (int i=0; i<length; i++) {
      crc = (crc << 8) ^ crctab[((crc >> 24) ^ *buf++) & 0xFF];
   }
   for (; length; length >>= 8) {
      crc = (crc << 8) ^ crctab[((crc >> 24) ^ length) & 0xFF];
   }
   crc = ~crc & 0xFFFFFFFF;
   return crc;

/*
int main (void) {
   char* mystring = "This is a test.\n";
   int length = strlen(mystring);
	   
   long value = string_cksum(mystring, length);
   printf("Checksum = %u\n", value);

   exit(0);
}
*/

}



//////////////////////////////
//
// example -- example function calls to the program.
//

void example(void) {


}



//////////////////////////////
//
// usage -- command-line usage description and brief summary
//

void usage(const char* command) {

}





// md5sum: 406377c7277b7911841fa5a0440789fe hum2abc.cpp [20090909]

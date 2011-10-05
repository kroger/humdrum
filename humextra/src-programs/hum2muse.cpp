//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Jun  2 23:30:21 PDT 2010
// Last Modified: Fri Dec 24 16:48:57 PST 2010 continued implementation
// Last Modified: Sat Feb  5 14:57:09 PST 2011 added text for lyrics
// Last Modified: Mon Feb 28 06:30:23 PST 2011 added --footer
// Last Modified: Mon Mar  7 19:19:44 PST 2011 added LO:N:smx/Y;bmx/Y
// Last Modified: Fri Mar 25 11:37:46 PDT 2011 macro expansion adjustments
// Last Modified: Fri Apr  1 07:54:48 PDT 2011 hanging tie endings 
// Last Modified: Wed Apr  6 19:54:37 PDT 2011 added --vz
// Last Modified: Thu Apr 14 21:44:11 PDT 2011 added *rscale processing
// Last Modified: Thu Aug 18 14:43:25 PDT 2011 added segno sign on barlines
// Last Modified: Wed Aug 31 16:59:11 PDT 2011 added \+ infront of # text
// Last Modified: Sat Sep 24 19:21:17 PDT 2011 added -x option
// Filename:      ...sig/examples/all/hum2muse.cpp 
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/hum2muse.cpp
// Syntax:        C++; museinfo
//
// Description:   Converts Humdrum files into MuseData files.
//

#include <math.h>
#include <time.h>    /* for current time/date */

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
      #include <strstrea.h>
   #else
      #include <strstream.h>
   #endif
   #define SSTREAM strstream
   #define CSTRING str()
#endif

#include "string.h"

#include "humdrum.h"
#include "PerlRegularExpression.h"
#include "MuseData.h"
#include "MuseDataSet.h"
#include "CheckSum.h"
#include "SigString.h"


//////////////////////////////////////////////////////////////////////////
//
// Layout codes
//

#define LO_WEDGE_START "HP"
#define LO_WEDGE_END   "HPZ"


//////////////////////////////////////////////////////////////////////////

class Coord {
   public:
      Coord(void) { i = j = -1; }
      int i;
      int j;
};


//////////////////////////////////////////////////////////////////////////

class LayoutParameters {
   public:
      LayoutParameters           (void) { clear(); }
      void         clear         (void);
      void         parseLayout   (HumdrumFile& infile, Array<Coord>& layout);
      int          getParameter  (int codeindex, const char* searchkey);

      Array<Array<char> >          code;
      Array<Array<Array<char> > >  key;
      Array<Array<Array<char> > >  value;

   protected:
      void prepareCode(const char* xcode, const char* params);
};


void LayoutParameters::clear(void) {
   code.setSize(0);
   key.setSize(0);
   value.setSize(0);
}

void LayoutParameters::parseLayout(HumdrumFile& infile, Array<Coord>& layout) {
   PerlRegularExpression pre;
   int i;
   int ii;
   int jj;
   clear();
   char codename[128] = {0};
   for (i=0; i<layout.getSize(); i++) {
      ii = layout[i].i;
      jj = layout[i].j;
      if ((ii < 0) || (jj < 0)) {
         continue;
      }
      if (!pre.search(infile[ii][jj], "^!+LO:([^:]+):?(.*)", "")) {
         continue;
      }
      strcpy(codename, pre.getSubmatch(1));
      prepareCode(codename, pre.getSubmatch(2));
   }
}

int LayoutParameters::getParameter(int codeindex, const char* searchkey) {
   int output = -1;
   int j;
   for (j=0; j<key[codeindex].getSize(); j++) {
      if (strcmp(key[codeindex][j].getBase(), searchkey) == 0) {
         output = j;
         break;
      }
   }
   return output;
}

void LayoutParameters::prepareCode(const char* xcode, const char* params) {
   code.increase(1);
   code.last().setSize(strlen(xcode)+1);
   strcpy(code.last().getBase(), xcode);

   PerlRegularExpression pre;
   Array<Array<char> > tokens;
   pre.getTokens(tokens, ":", params);

   key.increase(1);
   value.increase(1);
   key.last().setSize(tokens.getSize());
   value.last().setSize(tokens.getSize());

   PerlRegularExpression pre2;

   int i;
   for (i=0; i<tokens.getSize(); i++) {
      if (pre2.search(tokens[i], "^([^=]+)=?(.*)", "")) {
         key.last()[i].setSize(strlen(pre2.getSubmatch(1))+1);
         strcpy(key.last()[i].getBase(), pre2.getSubmatch());
         value.last()[i].setSize(strlen(pre2.getSubmatch(2))+1);
         strcpy(value.last()[i].getBase(), pre2.getSubmatch());
      } else {
         key.last()[i].setSize(1);
         key.last()[i][0] = '\0';
         value.last()[i].setSize(1);
         value.last()[i][0] = '\0';
      }
   }
}


//////////////////////////////////////////////////////////////////////////

// function declarations:
void  checkOptions             (Options& opts, int argc, char** argv);
void  example                  (void);
void  usage                    (const char* command);
void  convertData              (Array<MuseData*>& outfiles, 
                                HumdrumFile& infile);
void  getKernTracks            (Array<int>& tracks, HumdrumFile& infile);
void  convertTrackToMuseData   (MuseData& musedata, int track, 
                                HumdrumFile& infile, Array<int>& tpq, 
                                int& tickpos, int counter, int total, 
                                int printFirstMeasureQ);
void  getMeasureInfo           (Array<int>& measures, HumdrumFile& infile);
void  getMeasureData           (MuseData& tempdata, int track, 
                                HumdrumFile& infile, int startline, 
                                int stopline, int tpq, int& tickpos,
                                int& measuresuppress, int& founddataQ);
int   getMaxVoices             (HumdrumFile& infile, int startline, 
                                int stopline, int track);
void  addMeasureEntry          (MuseData& tempdata, HumdrumFile& infile, 
                                int line, int spine);
void  processVoice             (MuseData& tempdata, HumdrumFile& infile, 
                                int startline, int stopline, int track, 
                                int voice, int tpq, int& tickpos,
                                int& measuresuppress, int& founddataQ,
                                int starttick, int stoptick);
int   getGlobalTicksPerQuarter   (HumdrumFile& infile);
int   getTick                  (int tpq, HumdrumFile& infile, int line);
int   addNoteToEntry           (MuseData& tempdata, HumdrumFile& infile, 
                                int row, int col, int tpq, int voice);
int   getTickDur               (int tpq, HumdrumFile& infile, int row, 
                                int col);
void  addBackup                (MuseData& tempdata, int backticks, int tpq);
void  checkColor               (Array<char>& colorchar, MuseRecord& arecord, 
                                const char* token, Array<char>& colorout);
void  setColorCharacters       (HumdrumFile& infile, Array<char>& colorchar,
                                Array<char>& colorout);
void  setNoteheadShape         (MuseRecord& arecord, const char* token);
void  insertHeaderRecords      (HumdrumFile& infile, MuseData& tempdata, 
                                int track, int counter, int total);
void  addCopyrightLine         (HumdrumFile& infile, MuseData& tempdata);
void  addControlNumber         (HumdrumFile& infile, MuseData& tempdata);
void  addTimeStamp             (MuseData& tempdata);
void  addDateAndEncoder        (HumdrumFile& infile, MuseData& tempdata);
void  addWorkNumberInfo        (HumdrumFile& infile, MuseData& tempdata);
void  addSourceRecord          (HumdrumFile& infile, MuseData& tempdata);
void  addWorkTitle             (HumdrumFile& infile, MuseData& tempdata);
void  addMovementTitle         (HumdrumFile& infile, MuseData& tempdata);
void  addPartName              (HumdrumFile& infile, int track, 
                                MuseData& tempdata);
void  insertDollarRecord       (HumdrumFile& infile, int line, 
                                MuseData& musedata, int track, int counter, 
                                int total, int tpq);
int   appendKeySignature       (char* buffer, HumdrumFile& infile, int line, 
                                int track);
int   appendClef               (char* buffer, HumdrumFile& infile, int line, 
                                int track);
int   isBarlineBeforeData      (HumdrumFile& infile, int startindex);
char  getColorCategory         (const char* color);
int   appendTimeSignature      (char* buffer, HumdrumFile& infile, int line, 
                                int track, int tpq);
void  printMuse2PsOptions      (HumdrumFile& infile);
void  getPartNames             (HumdrumFile& infile, 
                                Array<Array<char> >& PartNames);
void  addChordLevelArtic       (MuseData& tempdata, MuseRecord&  arecord, 
                                MuseRecord& printsuggestion, 
                                HumdrumFile& infile, int row, int col, 
                                int voice);
void  addNoteLevelArtic        (MuseRecord&  arecord, HumdrumFile& infile, 
                                int row, int col, int tnum);
void  setupMusicaFictaVariables(HumdrumFile& infile);
void  getBeamState             (Array<Array<Array<char> > >& beams, 
                                Array<Array<Array<Coord> > >& layout, 
                                Array<Array<Coord> >& glayout,
                                Array<Array<Coord> >& clefs,
                                HumdrumFile& infile);
void  countBeamStuff           (const char* token, int& start, int& stop, 
                                int& flagr, int& flagl);
void  getMeasureDuration       (HumdrumFile& infile, 
                                Array<RationalNumber>& rns);
int   isPowerOfTwo             (RationalNumber& rn);
void  getTupletState           (Array<int>& hasTuplet, 
                                Array<Array<char> >& TupletState, 
                                Array<Array<int> >& TupletTopNum, 
                                Array<Array<int> >& TupletBotNum, 
                                HumdrumFile& infile);
void  primeFactorization       (Array<int>& factors, int input);
char  getBase36                (int value);
int   getTupletTop             (HumdrumFile& infile, int row, int col);
void  storeLayoutInformation   (HumdrumFile& infile, int line, 
                                Array<Array<Array<Coord> > >& laystate, 
                                Array<Array<Array<Coord> > >& layout,
                                int initializeQ);
void  addLayoutInfo            (HumdrumFile& infile, int line, 
                                Array<Array<Array<Coord> > > &laystate);
void  handleLayoutInfoChord    (MuseData& tempdata, HumdrumFile& infile, 
                                int row, int col, LayoutParameters& lp, 
                                int voice);
void  insertSlurUpPrintSug     (MuseData& data, MuseRecord& prec);
void  insertSlurDownPrintSug   (MuseData& data, MuseRecord& prec);
void  insertStaccatoSuggestion (MuseData& indata, MuseRecord& prec, 
                                int direction);
void  getPitches               (Array<int>& pitches, HumdrumFile& infile, 
                                int row, int col);
int   numbersort               (const void* A, const void* B);
int   numberRsort              (const void* A, const void* B);
void  getChordMapping          (Array<int>& chordmapping, HumdrumFile& infile, 
                                int row, int col);
void  getDynamicsAssignment    (HumdrumFile& infile, 
                                Array<int>& DynamicsAssignment, int& dynamicsQ);
void  addDynamics              (HumdrumFile& infile, int row, int col, 
                                MuseData& indata, MuseRecord& prec, 
                                Array<int>& DynamicsAssignment,
                                LayoutParameters& lp);
void  addMovementDesignation   (char* buffer, HumdrumFile& infile, int line);
void  convertKernLODYtoMusePS  (char* buffer, Array<Array<char> >& keys,
                                Array<Array<char> >& values);
void  processDynamicsLayout    (int loc, MuseRecord& prec, 
                                LayoutParameters& lp);
int   rnsort                   (const void* A, const void* B);
void  getRhythms               (Array<RationalNumber>& rns, HumdrumFile& infile,
                                int startindex, int stopindex);
void  prepareLocalTickChanges  (HumdrumFile& infile, Array<int>& ticks);
int   LCM                      (Array<int>& rhythms);
int   GCD                      (int a, int b);
int   getTPQByRhythmSet        (Array<RationalNumber>& rhys);
void  adjustClefState          (HumdrumFile& infile, int line, 
                                Array<Coord>& clefstate);
void  storeClefInformation     (HumdrumFile& infile, int line, 
                                Array<Coord>& clefstate, 
                                Array<Array<Coord> >& clefs);
void  insertArpeggio           (MuseData& tempdata, HumdrumFile& infile, 
                                int row, int col);
int   getScoreStaffVerticalPos (int note, int line, int row, 
                                Array<Array<Coord> >& clefs, 
                                HumdrumFile& infile);
void  processMeasureLayout     (MuseData& tempdata, HumdrumFile& infile, 
                                int line, int col, 
                                LayoutParameters& lp,
                                LayoutParameters& glp);
void  storeGlobalLayoutInfo    (HumdrumFile& infile, int line, 
                                Array<Coord>& glaystate, 
                                Array<Array<Coord> >& glayout);
void  addGlobalLayoutInfo      (HumdrumFile& infile, int line, 
                                Array<Coord>& glaystate);
void  handleMeasureLayoutParam (MuseData& tempdata, HumdrumFile& infile,
                                LayoutParameters& lp, int index);
void  addHairpinStops          (MuseData& tempdata, HumdrumFile& infile, 
                                int row, int col);
void  addHairpinStarts         (MuseData& tempdata, HumdrumFile& infile, 
                                int row, int col);
void  addDecrescendoStop       (MuseData& tempdata, LayoutParameters& lp);
void  addCrescendoStop         (MuseData& tempdata, LayoutParameters& lp);
void  addCrescendoStart        (MuseData& tempdata, LayoutParameters& lp);
void  addDecrescendoStart      (MuseData& tempdata, LayoutParameters& lp);
void  getXxYy                  (Array<int>& vals, Array<int>& states, 
                                Array<Array<char> >& keys,
                                Array<Array<char> >& values);
void  addPositionInfo          (MuseData& tempdata, int column, 
                                LayoutParameters lp, const char* code);
void  addCrescText             (MuseData& tempdata, HumdrumFile& infile, 
                                int row, int col, int dcol, 
                                LayoutParameters& lp, const char* text);
void  addPositionParameters    (MuseData& tempdata, int column, 
                                Array<Array<char> >& keys, 
                                Array<Array<char> >& values);
void  addTextLayout            (MuseData& tempdata, HumdrumFile& infile, 
                                int row, int col, LayoutParameters& lp, 
                                const char* code);
void  addText                  (MuseData& tempdata, Array<Array<char> >& keys, 
                                Array<Array<char> >& values);
int   addDashing               (MuseData& tempdata, int column, int track, 
                                LayoutParameters& lp);
void  addUnDash                (MuseData& tempdata, HumdrumFile& infile, 
                                int row, int col, int dcol, 
                                LayoutParameters& lpd);
void  setupTextAssignments     (HumdrumFile& infile, int& textQ, 
                                Array<Array<int> >& TextAssignment,
                                const char* textspines);
void  track2column             (Array<int>& trackcol, HumdrumFile& infile, 
                                int row);
void  addLyrics                (MuseRecord& arecord, HumdrumFile& infile, 
                                int row, int col, 
                                Array<Array<int> >& TextAssignment);
void  convertHumdrumTextToMuseData(Array<char> & text);
void  convertHtmlTextToMuseData(Array<char> & text);
void  getWorkAndMovement       (Array<char>& work, Array<char>& movment, 
                                HumdrumFile& infile);
void  printWithMd5sum          (MuseData& datafile);
void  appendReferenceRecords   (MuseData& musedata, HumdrumFile& infile);
RationalNumber getDuration     (const char* input, const char* def);
RationalNumber getDurationNoDots(const char* input, const char* def);
void  processLyricsSpines      (Array<Array<int> >& spineinfo, 
                                HumdrumFile& infile, const char* data);
void  verifyMuseDataFile       (const char* filename);
void  verifyMuseDataFile       (istream& input);
void  verifyMuseDataFiles      (Options& options);
int   verifyPart               (MuseData& part);
void  updateMuseDataFileTimeStamps(Options& options);
void  removeOldTimeStamps      (MuseData& part);
int   getTimeStampLine         (MuseRecord& arecord, MuseData& part);
void  getNewLine               (Array<char>& newline, MuseData& part);
void  updateFileTimeStamps     (const char* filename,  Options& options);
void  updateFileTimeStamps     (istream& input, Options& options);
void  doUpdateWork             (MuseDataSet& mds);
void  updatePart               (MuseData& part);
void  getCheckSum              (Array<char>& checksum, MuseData& part, 
                                Array<char>& newline);
void  appendToEndOfPart        (MuseData& md, MuseRecord& arecord);
void  addHumdrumVeritas        (MuseData& musedata, HumdrumFile& infile);
void  setGracenoteVisualRhythm (MuseRecord& arecord, const char* token);
void  convertKernLONtoMusePS   (char* buffer, Array<Array<char> >& keys,
                                Array<Array<char> >& values, const char* token);
void  printFooter              (MuseData& musedata, HumdrumFile& infile, 
                                const char* footertext);
void  cleanFooterField         (Array<char>& footerline, HumdrumFile& infile);
void  substituteReferenceRecord(Array<char>& string, const char* refstring, 
                                const char* extension, HumdrumFile& infile);
void  getReferenceValue        (Array<char>& value, Array<char>& refkey, 
                                HumdrumFile& infile);
void  hideNotesAfterLong       (HumdrumFile& infile, int row, int col);
void  analyzeTieConditions     (Array<Array<Array<char> > >& tiestate, 
                                HumdrumFile& infile);
void  flipNameComma            (Array<char>& value);
void  getRscaleState           (HumdrumFile& infile, 
                                Array<Array<RationalNumber> >& rscale);
void  filterOptions            (Array<char>& options, const char* filter);


Array<Array<Array<char> > > TieConditions;

// User interface variables:
Options options;
int    debugQ       = 0;          // used with --debug option
int    roundQ       = 0;          // used with --round option
int    slurQ        = 1;          // used with --no-slurs option
int    dynamicsQ    = 1;          // used with --no-dynamics option
int    sfzQ         = 1;          // used with --no-dynamics option
int    textQ        = 1;          // used with --no-text option
const char* TextSpines = "";      // used with --text option
int    metQ         = 1;          // used with --no-met option
int    verselimit   = 5;          // used with --vl option
int    mensuralQ    = 0;          // used with --mensural option
int    mensural2Q   = 0;          // used with --mensural2 option
int    referenceQ   = 1;          // used with -R option
int    beamQ        = 1;          // used with --no-beams option
int    tieQ         = 1;          // used with --no-ties option
int    excludeQ     = 0;          // used with -x option
const char* excludeString = "";   // used with -x option
int    tupletQ      = 1;          // used with -no-tuplets option
int    vzQ          = 0;          // used with --vz option
int    sepbracketQ  = 0;          // used with --sepbracket option
int    ignoreTickError = 0;       // used with --dd option
int    footerQ    = 0;            // used with --footer option
const char* footertext = "";      // used with --footer option
const char* defaultDur  = "4";    // used with --dd option
const char* LyricSpines = "";     // used with --ls option
const char* Encoder = "";         // used with --encoder option
int encoderQ = 0;                 // used with --encoder option
int copyrightQ = 0;               // used with --copyright option
const char* Copyright = "";       // used with --copyright option
Array<char> Colorchar;            // charcter in **kern data which causes color
Array<char> Colorout;             // converted charcter in col 14 of MuseData
Array<Array<char> > PartNames;
int hasFictaQ = 0;                // used with !!!RDF**kern: i=musica ficta
char FictaChar = 'i';
int hasLongQ = 0;                 // used with !!!RDF**kern: l=long note
char LongChar = 'l';
Array<Array<Array<char> > > BeamState;
Array<int> hasTuplet;
Array<Array<char> > TupletState;
Array<Array<int> > TupletTopNum;
Array<Array<int> > TupletBotNum;
Array<RationalNumber> MeasureDur;  // used to decide centered whole rest
Array<Array<Array<Coord> > > LayoutInfo;
Array<Array<Coord> > GlobalLayoutInfo;
int LastTPQPrinted;                // used for tpqs that change
const char* muse2psoptionstring = "";  // used with --mo option
Array<char> NEWLINE;               // used to control the newline style

Array<Array<int> > TextAssignment;  // used to keep track of lyics by staff

Array<int> DynamicsAssignment;    
Array<Array<Coord> > ClefState;     // which clef is being used for every note
Array<int> DashState;               // used for turning off dashed lines
Array<Array<RationalNumber> > RscaleState;  // used for *rscale processing


// muse2ps =k options: Display alternative options
unsigned int kflag = 0;
#define k_active   (1)
#define k_sfz      (1 << 1)   // on = display as sfz
#define k_subedit  (1 << 2)   // on = editorial data looks like regular data
#define k_noedit   (1 << 3)   // on = hide editorla data from print
#define k_roman    (1 << 4)   // on = editorial marks in times-roman font
#define k_ligature (1 << 5)   // on = text ligatures
#define k_fbass    (1 << 6)   // on = figured bass above staff
#define k_lrbar    (1 << 7)   // on =:|!|:, off =:!!:
#define k_augdot   (1 << 8)   // on = don't share aug. dots between voices
#define k_newkey   (1 << 9)   // on = allow same key sig to repeat on staff
#define k_chord    (1 << 10)  // on = mixed rhythm chords
#define k_hidekey  (1 << 11)  // on = hide key signatures
#define k_clef     (1 << 12)  // on = clef changes are normal size not cue size
  

//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   NEWLINE.setSize(3);
   NEWLINE[0] = (char)0x0d;
   NEWLINE[1] = (char)0x0a;
   NEWLINE[2] = '\0';

   HumdrumFile infile;

   // initial processing of the command-line options
   checkOptions(options, argc, argv);

   if (options.getArgCount() < 1) {
      infile.read(cin);
   } else {
      infile.read(options.getArg(1));
   }
   infile.analyzeRhythm("4");
   setColorCharacters(infile, Colorchar, Colorout);
   DashState.setSize(infile.getMaxTracks()+1);
   DashState.setAll(0);

   setupTextAssignments(infile, textQ, TextAssignment, TextSpines);
   setupMusicaFictaVariables(infile);
   getBeamState(BeamState, LayoutInfo, GlobalLayoutInfo, ClefState, infile);
   getTupletState(hasTuplet, TupletState, TupletTopNum, TupletBotNum, infile);
   getMeasureDuration(infile, MeasureDur); 
   Array<MuseData*> outfiles;
   outfiles.setSize(0);
   getRscaleState(infile, RscaleState);

   if (dynamicsQ) {
      getDynamicsAssignment(infile, DynamicsAssignment, dynamicsQ);
      // dynamicsQ may become false if no dynamics found
   }

   getPartNames(infile, PartNames);
   analyzeTieConditions(TieConditions, infile);
   convertData(outfiles, infile);
   printMuse2PsOptions(infile);     // must come after convertData()
   // eventual the footer should go here, but it is not currently
   // echoed with =M option.
   //if (footerQ) {
   //   printFooter(infile, footertext);
   //}

   int i;
   for (i=0; i<outfiles.getSize(); i++) {
      outfiles[i]->cleanLineEndings();
      printWithMd5sum(*(outfiles[i]));
      // cout << *(outfiles[i]);
      delete outfiles[i];
      outfiles[i] = NULL;
      cout << "/eof" << NEWLINE << flush; 
   }
   // end of all data file marker (two slashes on a line by themselves):
   cout << "//" << NEWLINE << flush; 
   outfiles.setSize(0);

   return 0;
}


//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// getRscaleState --  Does not work on sub-tracks.  Only powers of two will
//     give predictable results at the moment.
//

void getRscaleState(HumdrumFile& infile, 
    Array<Array<RationalNumber> >& rscale) {

   int tracks = infile.getMaxTracks();
   rscale.setSize(infile.getNumLines());
   int i, j;
   Array<RationalNumber> current;
   current.setSize(tracks+1);
   current.setAll(1);
   
   PerlRegularExpression pre;
   RationalNumber rnum;
   int track;

   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isInterpretation()) {
         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (pre.search(infile[i][j], "^\\*rscale\\s*:\\s*(\\d+)/(\\d+)", 
                  "")) {
               rnum = atoi(pre.getSubmatch(1));
               rnum /= atoi(pre.getSubmatch(2));
               track = infile[i].getPrimaryTrack(j);
               current[track] = rnum;
            } else if (pre.search(infile[i][j], 
                  "^\\*rscale\\s*:\\s*(\\d+)",  "")) {
               rnum = atoi(pre.getSubmatch(1));
               track = infile[i].getPrimaryTrack(j);
               current[track] = rnum;
            }
         }
      }
      if (!infile[i].isData()) {
         continue;
      }
      rscale[i].setSize(infile[i].getFieldCount());
      for (j=0; j<infile[i].getFieldCount(); j++) {
         track = infile[i].getPrimaryTrack(j);
         rscale[i][j] = current[track];
      }
   }
}



//////////////////////////////
//
// analyzeTieConditions -- check to see if a tie has a matching start/end.
//    If the tie is not ended at the end of the music add codes to the
//    output data to draw a hanging tie.
//
// Here is a complicated example with two layers on the staff, with one
// of the voices containing a chord:
//
// measure 3
// C3    24      1 w     d
// F3    24-     1 w     d        -
// G3    24-     1 w     d        -
// *               X       F3
// *               X       G3
// back  48
// G3    24      1 w     u
// A3    24-     1 w     u        -
// *               X       A3
// measure         &
//
// Place a * record after the note or all notes in the chord which have a
// hanging tie.  In column 17, place an 'X'. In column 25, place the pitch
// of the tie.  On the measure where the tie will end, place a "&" character
// in column 17.
//

void analyzeTieConditions(Array<Array<Array<char> > >& tiestate, 
      HumdrumFile& infile) {
   tiestate.setSize(infile.getNumLines());

   int i, j, k;
   int tcount;
   char buffer[128] = {0};
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isMeasure()) {
      }
      if (!infile[i].isData()) {
         tiestate[i].setSize(0);
         continue;
      }
      tiestate[i].setSize(infile[i].getFieldCount());
      for (j=0; j<infile[i].getFieldCount(); j++) {
         tiestate[i][j].setSize(0);
         if (!infile[i].isExInterp(j, "**kern")) {
            continue;
         }
         if (strcmp(infile[i][j], ".") == 0) {
            continue;
         }
         if (strchr(infile[i][j], 'r') != NULL) {
            continue;
         }
         tcount = infile[i].getTokenCount(j);
         tiestate[i][j].setSize(tcount);
         tiestate[i][j].setAll(0);
         for (k=0; k<tcount; k++) {
            infile[i].getToken(buffer, j, k);
            if (strchr(buffer, '[') == NULL) {
               continue;               
            }
            // search for the end of the tie
//ggg            tiestate[i][j][k] = infile.hasTieEnding(i, j, k);
         }
      }
   }

}



//////////////////////////////
//
// printFooter --
//

void printFooter(MuseData& musedata, HumdrumFile& infile, const char* footertext) {
   if (strlen(footertext) < 1) {
      return;
   }
   PerlRegularExpression pre;
   MuseRecord arecord;

   Array<char> separator;
   separator.setSize(2);
   separator[0] = footertext[0];
   separator[1] = '\0';

   pre.sar(separator, "\\$", "\\$", "");   // escape $ character
   pre.sar(separator, "\\^", "\\^", "");   // escape ^ character
   pre.sar(separator, "\\*", "\\*", "");   // escape * character

   Array<Array<char> > tokens;
   pre.getTokensWithEmpties(tokens, separator.getBase(), footertext+1);

   int i;
   for (i=0; i<tokens.getSize(); i++) {
      cleanFooterField(tokens[i], infile);
   }

   arecord.clear();
   arecord.appendString("@START:\tFOOTER");
   musedata.append(arecord); arecord.clear();

   arecord.appendString("@FOOT1: \t");
   if (tokens.getSize() >= 3) { arecord.appendString(tokens[2].getBase()); }
   musedata.append(arecord); arecord.clear();

   arecord.appendString("@FOOT2: \t");
   if (tokens.getSize() >= 2) { arecord.appendString(tokens[1].getBase()); }
   musedata.append(arecord); arecord.clear();

   arecord.appendString("@FOOT3: \t");
   if (tokens.getSize() >= 1) { arecord.appendString(tokens[0].getBase()); }
   musedata.append(arecord); arecord.clear();

   arecord.appendString("@FOOT1R:\t");
   if (tokens.getSize() >= 6) { arecord.appendString(tokens[5].getBase()); }
   musedata.append(arecord); arecord.clear();

   arecord.appendString("@FOOT2R:\t");
   if (tokens.getSize() >= 5) { arecord.appendString(tokens[4].getBase()); }
   musedata.append(arecord); arecord.clear();

   arecord.appendString("@FOOT3R:\t");
   if (tokens.getSize() >= 4) { arecord.appendString(tokens[3].getBase()); }
   musedata.append(arecord); arecord.clear();

   // extra control parameters, should start with @:
   if (tokens.getSize() >= 7) { 
      arecord.appendString(tokens[6].getBase()); 
      musedata.append(arecord); arecord.clear();
   }

   arecord.appendString("@END:\tFOOTER");
   musedata.append(arecord);
}



//////////////////////////////
//
// cleanFooterField --
//

void cleanFooterField(Array<char>& footerline, HumdrumFile& infile) {
   PerlRegularExpression pre;
   pre.sar(footerline, "^\\s+", "", "");  // remove leading spaces
   pre.sar(footerline, "\\s+$", "", "");  // remove trailing spaces
   Array<char> buf1;
 
   while (pre.search(footerline, "(@\\{[^}]+\\})(\\{[^}]*\\})?", "")) {
      buf1.setSize(strlen(pre.getSubmatch(1)+1));
      strcpy(buf1.getBase(), pre.getSubmatch(1));
      substituteReferenceRecord(footerline, buf1.getBase(), 
            pre.getSubmatch(2), infile);
   }
}



//////////////////////////////
//
// substituteReferenceRecord --
//

void substituteReferenceRecord(Array<char>& string, const char* refstring, 
      const char* extension, HumdrumFile& infile) {

   Array<char> refkey;
   refkey.setSize(strlen(refstring)+1);
   strcpy(refkey.getBase(), refstring);
   PerlRegularExpression pre;
   pre.sar(refkey, "^@\\{", "", "");
   pre.sar(refkey, "\\}$", "", "");
   Array<char> value;
   value.setSize(1);
   value[0] = '\0';
   getReferenceValue(value, refkey, infile);
   pre.sar(value, "^\\s+", "", "");
   pre.sar(value, "\\s+$", "", "");

   Array<char> newref;
   newref.setSize(strlen(refstring)+1);
   strcpy(newref.getBase(), refstring);

   Array<char> tref;
   tref.setSize(strlen(refstring)+1);
   strcpy(tref.getBase(), refstring);
   PerlRegularExpression pre2;

   if (pre.search(value, "^(20\\d\\d)/0?(\\d+?)\\/0?(\\d+?)\\/?$", "")) {
      char buffer[1024] = {0};
      strcpy(buffer, pre.getSubmatch(3));
      int month = atoi(pre.getSubmatch(2));
      switch (month) {
         case  1:  strcat(buffer, " Jan"); break;
         case  2:  strcat(buffer, " Feb"); break;
         case  3:  strcat(buffer, " Mar"); break;
         case  4:  strcat(buffer, " Apr"); break;
         case  5:  strcat(buffer, " May"); break;
         case  6:  strcat(buffer, " Jun"); break;
         case  7:  strcat(buffer, " Jul"); break;
         case  8:  strcat(buffer, " Aug"); break;
         case  9:  strcat(buffer, " Sep"); break;
         case 10:  strcat(buffer, " Oct"); break;
         case 11:  strcat(buffer, " Nov"); break;
         case 12:  strcat(buffer, " Dec"); break;
      }
      strcat(buffer, " ");
      strcat(buffer, pre.getSubmatch(1));
      pre.sar(value, "^.*$", buffer, "");
   }

   if (pre.search(value, "^\\s*$", "")) {
      // reference record is empty
      // remove any conditional text after @{} operator:
      pre.sar(newref, "$", "\\{[^}]*\\}", "");
      char empty = '\0';
      newref.append(empty);
      pre.sar(string, newref.getBase(), value.getBase(), "");
      // get rid of marker since it does not exist in bib records.
      pre.sar(string, refstring, "", "");
      // get rid of extra text marker as well
      pre.sar(string, extension, "", "");
   } else {
      // found reference record 
      // also insert conditional text
      pre.sar(tref, "$", "\\{([^}]+)\\}", "");
      if (pre.search(string, tref.getBase(), "")) {
         pre2.sar(value, "$", pre.getSubmatch(1), "");
         pre2.sar(newref, "$", "\\{[^}]*\\}", "");
      }
      pre.sar(string, newref.getBase(), value.getBase(), "");
   }
}



//////////////////////////////
//
// getReferenceValue -- Flip COM & COA around if there is a comma
//      in the name.
//

void getReferenceValue(Array<char>& value, Array<char>& refkey, 
      HumdrumFile& infile) {
   int i;
   char buffer[1024] = {0};
   value.setSize(1);
   value = '\0';

   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) {
         continue;
      }
      if (strcmp(infile[i].getBibKey(buffer), refkey.getBase()) != 0) {
         continue; 
      }
      infile[i].getBibValue(buffer);
      value.setSize(strlen(buffer)+1);
      strcpy(value.getBase(), buffer);
      if (strncmp(refkey.getBase(), "COM", 3) == 0) {
         flipNameComma(value);
      } else if (strncmp(refkey.getBase(), "COA", 3) == 0) {
         flipNameComma(value);
      }
      return;
   }

   //  record was not found, search without any @ qualifiers:
  
   Array<char> refkey2 = refkey;  
   PerlRegularExpression pre;
   pre.sar(refkey2, "@.*", "");

   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) {
         continue;
      }
      if (!pre.search(infile[i].getBibKey(buffer), refkey2.getBase(), "")) {
         continue; 
      }
      infile[i].getBibValue(buffer);
      value.setSize(strlen(buffer)+1);
      strcpy(value.getBase(), buffer);
      if (strncmp(refkey.getBase(), "COM", 3) == 0) {
         flipNameComma(value);
      } else if (strncmp(refkey.getBase(), "COA", 3) == 0) {
         flipNameComma(value);
      }
      return;
   }

}



//////////////////////////////
//
// flipNameComma -- reverse last and first name
//

void flipNameComma(Array<char>& value) {
   PerlRegularExpression pre;
   char buffer[1024] = {0};
   if (pre.search(value, "^([^,]+)\\s*,\\s*([^,]+)$", "")) {
      strcpy(buffer, pre.getSubmatch(2));
      strcat(buffer, " ");
      strcat(buffer, pre.getSubmatch(1));
      value.setSize(strlen(buffer)+1);
      strcpy(value.getBase(), buffer);
   }
}



//////////////////////////////
//
// appendReferenceRecords --
// 

void appendReferenceRecords(MuseData& musedata, HumdrumFile& infile) {
   int i;
   /*  Always print reference records if being printed: if no VTS, then
    *  generate one.
    *
    
   int hasbib = 0;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isBibliographic()) {
         hasbib = 1;
         break;
      }
   }

   if (hasbib == 0) {
      return;
   }
   */

   MuseRecord arecord;

   arecord.clear();
   arecord.insertString(1, "@@START: HUMDRUMREFERENCE");
   musedata.append(arecord);

   RationalNumber lastabs(-1, 1);
   RationalNumber testabs(0, 1);

   arecord.clear();
   arecord.insertString(1, "@@TOP");
   int startfound = 0;
   int endfound = 0;
   musedata.append(arecord);
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) {
         if (infile[i].isInterpretation()) {
            if (strncmp(infile[i][0], "**", 2) == 0) {
               startfound++;
            }
            if (strcmp(infile[i][0], "*-") == 0) {
               endfound = 1;
               arecord.clear();
               arecord.insertString(1, "@@BOTTOM");
               musedata.append(arecord);
            }
         }
         continue;
      }

      if (!endfound && startfound) {
         testabs = infile[i].getAbsBeatR(); 
         if (testabs != lastabs) {
            arecord.clear();
            arecord.append("sr", "@@POSITION:", &testabs);
            musedata.append(arecord);

            lastabs = testabs;
         }
      }
      
      arecord.clear();
      arecord.append("ss", "@", infile[i][0]);
      musedata.append(arecord);
    
   }

   addHumdrumVeritas(musedata, infile);

   arecord.clear();
   arecord.insertString(1, "@@END: HUMDRUMREFERENCE");
   musedata.append(arecord);
}



//////////////////////////////
//
// addHudrumVeritas --
//

void addHumdrumVeritas(MuseData& musedata, HumdrumFile& infile) {
   int hasVts = 0;
   int hasVtsData = 0;
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) { 
         continue;
      }
      if (strncmp(infile[i][0], "!!!VTS:", strlen("!!!VTS:")) == 0) {
         hasVts = 1;
      } else if (strncmp(infile[i][0], "!!!VTS-data:", 
            strlen("!!!VTS-data:")) == 0) {
         hasVtsData = 1;
      }
   }

   if (hasVts && hasVtsData) {
      return;
   }

   MuseRecord arecord;
   arecord.insertString(1, "@@EXTRA");
   musedata.append(arecord);
   if (!hasVts) {
      Array<char> vts;
      infile.makeVts(vts);
      arecord.clear();
      arecord.append("ss", "@", vts.getBase());
      musedata.append(arecord);
   }
   if (!hasVtsData) {
      Array<char> vts;
      infile.makeVtsData(vts);
      arecord.clear();
      arecord.append("ss", "@", vts.getBase());
      musedata.append(arecord);
   }
}



//////////////////////////////
//
// printWithMd5sum -- add an MD5sum for each file on the third header
//     record line.
//

void printWithMd5sum(MuseData& datafile) {
   SSTREAM tempstream;
   int i;
   for (i=0; i<datafile.getLineCount(); i++) {
      tempstream << datafile[i] << NEWLINE;
   }
   // tempstream << datafile;
   tempstream << ends;
   Array<char> data;
   data.setSize(strlen(tempstream.CSTRING)+1);
   strcpy(data.getBase(), tempstream.CSTRING);

   Array<char> md5sum;
   CheckSum::getMD5Sum(md5sum, data);
   PerlRegularExpression pre;
   // 0d0a added to indicate the the md5sum was calculated with DOS newlines.
   char newlinestring[1024] = {0};
   char buffer[32] = {0};
   for (i=0; i<NEWLINE.getSize(); i++) {
      if ((i==NEWLINE.getSize()-1) && (NEWLINE[i] == '\0')) { 
         break;
      }
      sprintf(buffer, "%02x", (int)NEWLINE[i]);
      strcat(newlinestring, buffer);
   }

   pre.sar(md5sum, "^", "[md5sum:XXXXXXXXXX:", "");
   pre.sar(md5sum, "XXXXXXXXXX", newlinestring, "");
   pre.sar(md5sum, "$", "]", "");
   pre.sar(data, "\\[\\]", md5sum.getBase(), "");
   cout << data;
}



//////////////////////////////
//
// setupTextAssignments --  Currently attach **text spines to the 
//   first **kern spine found on their left.  The order of the spines
//   indiate which verse the **text belongs to, left most spine is the
//   first verse for a **kern spine.
//

void setupTextAssignments(HumdrumFile& infile, int& textQ, 
      Array<Array<int> >& TextAssignment, const char* textspines) {
   int i, j, track;
   TextAssignment.setSize(infile.getMaxTracks()+1);
   for (i=0; i<TextAssignment.getSize(); i++) {
      TextAssignment[i].setSize(0);
   }
   if (textQ == 0) {
      // user request to not print text
      return;
   }
   int foundtext = 0;

   SigString exinterp;
   PerlRegularExpression pre;


   // print **text spines if --ls is not used and textspines is empty.
   if (strlen(textspines) == 0) {
      textspines = "**text";
   }

   if (strlen(LyricSpines) > 0) {
      textQ = 1;
      Array<Array<int> > lyricspines;
      processLyricsSpines(lyricspines, infile, LyricSpines);
      Array<int> kerntracks;
      getKernTracks(kerntracks, infile);

      for (i=0; i<kerntracks.getSize(); i++) {
         for (j=0; j<lyricspines[kerntracks[i]].getSize(); j++) {
            TextAssignment[kerntracks[i]].append(lyricspines[kerntracks[i]][j]);
            foundtext++;
         }
      }

   } else {

      int lastkern = 0;
      for (i=0; i<infile.getNumLines(); i++) {
         if (!infile[i].isInterpretation()) {
            continue;
         }
         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (infile[i].isExInterp(j, "**kern")) {
               lastkern = infile[i].getPrimaryTrack(j);
               // continue;
            }
            exinterp = "\\b";
            exinterp += (infile[i].getExInterp(j)+2);
            exinterp += "\\b";
            if (pre.search(textspines, exinterp.getBase(), "")) {
               track = infile[i].getPrimaryTrack(j);
               foundtext = 1;
               TextAssignment[lastkern].append(track);
            }
         }
         break;
      }
   }
   
   if (foundtext && textQ) {
      textQ = 1;
   } else {
      textQ = 0;
   }
}



//////////////////////////////
//
// getDynamicsAssignment -- map **dynam spines to the **kern spines
//     with which they should be printed.  Currently a simple mapping
//     model.  If a **dynam spine occurs immediately after a **kern spine
//     then it will be assigned to that **kern spine.  Only one **dynam
//     spine to a **kern spine.  In the future *staff tandem interpretations
//     should be processed for more complicated **dynam assignments.
//     Also think later about sub-spine assingment of **dynam/**kern data.
//

void getDynamicsAssignment(HumdrumFile& infile, Array<int>& DynamicsAssignment,       int& dynamicsQ) {
   DynamicsAssignment.setSize(infile.getMaxTracks()+1);
   DynamicsAssignment.setAll(0);
   Array<int>& da = DynamicsAssignment;
   dynamicsQ = 0;
   int i, j;
   int lastkerntrack = -1;
   int track;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isInterpretation()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         track = infile[i].getPrimaryTrack(j);
         if (infile[i].isExInterp(j, "**kern")) {
            lastkerntrack = track;
         } else if (infile[i].isExInterp(j, "**dynam")) {
            if ((lastkerntrack > 0) && (da[lastkerntrack] <= 0)) {
               da[lastkerntrack] = track;
               dynamicsQ = 1;
               if (debugQ) {
                  cout << "ASSIGNING DYNAM TRACK " << track
                       << " TO KERN TRACK " << lastkerntrack << NEWLINE;
               }
            }
         }
      }
      break;
   }
}



//////////////////////////////
//
// getMeasureDuration -- for each line, report the duration of the
// current measure.  The algorithm only considers one meter for all
// parts, and would have to be done by part if the meter is different in
// different parts...
//

void getMeasureDuration(HumdrumFile& infile, Array<RationalNumber>& rns) {
   PerlRegularExpression pre;
   int i, j;
   int top;
   int bot;
   RationalNumber current(0,1);
   rns.setSize(infile.getNumLines());
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isInterpretation()) {
         rns[i] = current;
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (!infile[i].isExInterp(j, "**kern")) {
            continue;
         }
         if (pre.search(infile[i][j], "^\\*M(\\d+)/(\\d+)", "")) {
            top = atoi(pre.getSubmatch(1));
            bot = atoi(pre.getSubmatch(2));
            current = top * 4;
            current /= bot;
            break;
         }
      }
      rns[i] = current;
   }
}



//////////////////////////////
//
// setupMusicaFictaVariables --
//
// !!!RDF**kern: i=musica ficta
//

void setupMusicaFictaVariables(HumdrumFile& infile) {
   int i;
   PerlRegularExpression pre;
   for (i=infile.getNumLines()-1; i>=0; i--) {
      if (!infile[i].isBibliographic()) {
         continue;
      }       
      if (pre.search(infile[i][0], 
            "^!!!RDF\\*\\*kern\\s*:\\s*([^\\s=])\\s*=.*musica ficta", "i")) {
         hasFictaQ = 1;
         FictaChar = pre.getSubmatch(1)[0];
      }
      if (pre.search(infile[i][0], 
            "^!!!RDF\\*\\*kern\\s*:\\s*([^\\s=])\\s*=.*long", "i")) {
         hasLongQ = 1;
         LongChar = pre.getSubmatch(1)[0];
      }
   }
}



//////////////////////////////
//
// getPartNames --
//

void getPartNames(HumdrumFile& infile, Array<Array<char> >& PartNames) {
   int i, j;
   PartNames.setSize(infile.getMaxTracks()+1);  //  0 = unused
   for (i=0; i<PartNames.getSize(); i++) {
      PartNames[i].setSize(1);
      PartNames[i][0]= '\0';
   }

   Array<int> ignore;
   ignore.setSize(infile.getMaxTracks()+1);
   ignore.setAll(0);

   PerlRegularExpression pre;
   int track;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isData()) {
         // stop looking when the first data line is found
         break;
      }
      if (!infile[i].isInterpretation()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strcmp(infile[i][j], "*^") == 0) {
            // don't search for names after spine splits (there might
            // be two names, and one of them will be ignored).
            ignore[infile[i].getPrimaryTrack(j)] = 1;
         }
         if (ignore[infile[i].getPrimaryTrack(j)]) {
            continue;
         }
         if (pre.search(infile[i][j], "^\\*I\"\\s*(.*)\\s*$", "")) {
            track = infile[i].getPrimaryTrack(j);
            PartNames[track].setSize(strlen(pre.getSubmatch(1))+1);
            strcpy(PartNames[track].getBase(), pre.getSubmatch());
         }
      }
   }
   
   // if no part name, set to "part name" (for debugging purposes):
   //for (i=1; i<=infile.getMaxTracks(); i++) {
   //   if (strcmp(PartNames[i].getBase(), "") == 0) {
   //      PartNames[i].setSize(strlen("part name")+1);
   //      strcpy(PartNames[i].getBase(), "part name");
   //   }
   // }

}



//////////////////////////////
//
// filterOptions -- removed specified options from an embedded option string.
//     can only handle single-character options.  Will need to be adjusted 
//     when/if double-letter parameters are allowed.
//

void filterOptions(Array<char>& options, const char* filter) {
   int olength = strlen(options.getBase());
   Array<Array<char> > singles;
   singles.setSize(100);
   singles.setSize(0);
   char empty = '\0';

   int i, j, k;
   int startindex = 0;
   for (i=0; i<options.getSize(); i++) {
      if (i == ':') {
         i++;
         while ((i<options.getSize()) && isspace(options[i])) {
            i++;
         }
         startindex = i;
         break;
      }
   }
 
   // splits the options string into separate components.
   for (i=startindex; i<olength; i++) {
      singles.setSize(singles.getSize()+1);
      singles.last().setSize(128);
      singles.last().setSize(0);
      singles.last().append(options[i]);
      if (i >= olength - 1) {
         break;
      }
      if ((i < olength - 1) && (options[i+1] == '^')) {
         // parameter for option is a string.  keep reading option string
         // until another ^ character is found.
         i++;
         singles.last().append(options[i++]);
         while ((i < olength) && (options[i] != '^')) {
            singles.last().append(options[i]);
         }
         if ((i<olength) && (options[i] == '^')) {
            singles.last().append(options[i++]);
         }
      } else if (isdigit(options[i+1])) {
         // read one or more integers, separated by commas
         i++;
         while ((i < olength) && (isdigit(options[i]) || (options[i] == ','))) {
            singles.last().append(options[i++]);
         }
         i--;
      }
   }

   //for (i=0; i<singles.getSize(); i++) {
   //   singles[i].append(empty);
   //}

   Array<char> temp;
   temp.setSize(options.getSize() + 32);
   temp.setSize(0);


   for (i=0; i<startindex; i++) {
      temp.append(options[i]);
   }
  
   int flength = strlen(filter);
   int ignore = 0;
   
   for (i=0; i<singles.getSize(); i++) {
      ignore = 0;
      for (j=0; j<flength; j++) {
         if (filter[j] == singles[i][0]) {
            ignore = 1;
            break;
         }
      }
      if (ignore) {
         continue;
      }

      // the option should not be filtered, so store in temporary output
      for (k=0; k<singles[i].getSize(); k++) {
         if (singles[i][k] == '\0') {
            continue;
         }
         temp.append(singles[i][k]);
      }
   }
   temp.append(empty);
   options.setSize(temp.getSize());
   strcpy(options.getBase(), temp.getBase());

}



//////////////////////////////
//
// printMuse2PsOptions -- print any lines in the Humdrum file
// which have the form !!!muse2ps:, !!muse2ps:, !!!muse2psv1:, !!muse2psv1:
// 
// Always converts v\d+ to v1. May need to change in future.
//

void printMuse2PsOptions(HumdrumFile& infile) {
   int i;
   PerlRegularExpression pre;
   PerlRegularExpression pre2;
   int titleline = -1;
   int composerline = -1;
   int dateline = -1;
   int hastitle = 0;
   int hascomposer = 0;

   SSTREAM globaldefaults;
   SSTREAM localdefaults;
   Array<char> tempdata;
   Array<char> temp2;

   for (i=0; i<infile.getNumLines(); i++) {
      if (!(infile[i].isGlobalComment() || infile[i].isBibliographic())) {
         continue;
      }
      
      tempdata.setSize(strlen(infile[i][0])+1);
      strcpy(tempdata.getBase(), infile[i][0]);
      if (excludeQ) {
         filterOptions(tempdata, excludeString);
      }
      cleanFooterField(tempdata, infile);
      if (pre.search(tempdata, 
            "^!!!?\\s*muse2ps(?:v\\d+)?\\s*:\\s*['\"]*=?(.*)\\s*['\"]*\\s*$", 
            "i")) {
         if (pre2.search(pre.getSubmatch(1), "^=", "")) {
           // a global option
           temp2.setSize(strlen(pre.getSubmatch())+1);
           strcpy(temp2.getBase(), pre.getSubmatch());
           convertHtmlTextToMuseData(temp2);
           globaldefaults << "@muse2psv1=" << temp2 << NEWLINE;
         } else if (pre2.search(pre.getSubmatch(1), "=", "")) {
           // a local option
           temp2.setSize(strlen(pre.getSubmatch())+1);
           strcpy(temp2.getBase(), pre.getSubmatch());
           convertHtmlTextToMuseData(temp2);
           localdefaults << "@muse2psv1=" << temp2 << NEWLINE;
         } else {
           // a global option
           temp2.setSize(strlen(pre.getSubmatch())+1);
           strcpy(temp2.getBase(), pre.getSubmatch());
           convertHtmlTextToMuseData(temp2);
           globaldefaults << "@muse2psv1==" << temp2 << NEWLINE;
         }
         if (pre2.search(tempdata, "T^")) {
            // already a title so don't try to add one
            hastitle = 1;
         }
         if (pre2.search(tempdata, "C^")) {
            // already a title so don't try to add one
            hascomposer = 1;
         }
      } else if ((titleline < 0) && pre.search(tempdata, "^!!!OTL[^:]*:", "")) {
         titleline = i;
      } else if ((composerline < 0) && pre.search(tempdata, "^!!!COM[^:]*:", "")) {
         composerline = i;
      } else if ((dateline < 0) && pre.search(tempdata, "^!!!CDT[^:]*:", "")) {
         dateline = i;
      }
   }


   // Print title if not found in muse2ps options, but found in file.
   // Think about !!!title: and !!title: markers (from Themefinder system).
   if ((!hastitle) && (titleline >= 0)) {
      if (pre.search(infile[titleline][0], "^!!!OTL[^:]*:\\s*(.*)\\s*$", "")) {
         Array<char> title;
         title.setSize(strlen(pre.getSubmatch(1))+1);
         strcpy(title.getBase(), pre.getSubmatch());
         // deal with HTML accented characters converted into MuseData 
         // equivalents here...
         
         // title cannot contain ^ character
         pre2.sar(title, "\\^", "", "g");
         convertHtmlTextToMuseData(title);

         cout << "@muse2psv1==T^" << title << "^" << NEWLINE;
      }
   }


   // Print composer if not found in muse2ps options, but found in file.
   if ((!hascomposer) && (composerline >= 0)) {
      if (pre.search(infile[composerline][0], 
            "^!!!COM[^:]*:\\s*(.*)\\s*$", "")) {
         Array<char> composer;
         composer.setSize(strlen(pre.getSubmatch(1))+1);
         strcpy(composer.getBase(), pre.getSubmatch());

         // reverse order of names if there is a comma in the name
         if (pre.search(composer, "^\\s*(.*?)\\s*,\\s*(.*)", "")) {
            char buffer[1024] = {0};
            strcpy(buffer, pre.getSubmatch(2));
            strcat(buffer, " ");
            strcat(buffer, pre.getSubmatch(1));
            composer.setSize(strlen(buffer)+1);
            strcpy(composer.getBase(), buffer);
         }

         // abbreviate the names of famous composers
         pre2.sar(composer, "Johann Sebastian Bach", "J.S. Bach", "");
         pre2.sar(composer, "Wolfgang Amadeus Mozart", "W.A. Mozart", "");
         pre2.sar(composer, "Ludwig van Beethoven", "L. van Beethoven", "");
         convertHtmlTextToMuseData(composer);

         // deal with HTML accented characters converted into MuseData 
         // equivalents here...
         
         // composer cannot contain ^ character
         pre2.sar(composer, "\\^", "", "g");

         cout << "@muse2psv1==C^" << composer << "^" << NEWLINE;
      }
   }

   Array<int> kerntracks;
   getKernTracks(kerntracks, infile);

   Array<char> systemspine;
   systemspine.setSize(1024);
   systemspine.setSize(1);
   systemspine[0] = '\0';
   if (sepbracketQ) {
      pre.sar(systemspine, "$", "s^[", "");
      for (i=0; i<kerntracks.getSize(); i++) {
         pre.sar(systemspine, "$", "(.)", "");
      }
      pre.sar(systemspine, "$", "]^", "");
      cout << "@muse2psv1==" << systemspine << NEWLINE;
   } else {
      // hack to have curly braces on piano solo part:
      // This is now a default behavior of muse2ps.
      //if (kerntracks.getSize() == 2) {
      //   cout << "@muse2psv1==s^{(..)}^" << NEWLINE;
      //}
      // hack to have bracket on SATB parts:
      if (kerntracks.getSize() == 4) {
         cout << "@muse2psv1==s^[(....)]^" << NEWLINE;
      }
   }

   Array<char> ostring;
   ostring.setSize(1);
   ostring[0] = '\0';
   if (strlen(muse2psoptionstring) > 0) {
      ostring.setSize(strlen(muse2psoptionstring) + 1);
      strcpy(ostring.getBase(), muse2psoptionstring);
   }
   if (kflag > 0) {
      char buffer[32] = {0};
      if (kflag < 0xff) {
         sprintf(buffer, "k^0x%02x^", kflag);
      } else if (kflag < 0xffff) {
         sprintf(buffer, "k^0x%04x^", kflag);
      } else if (kflag < 0xffffff) {
         sprintf(buffer, "k^0x%06x^", kflag);
      } else {
         sprintf(buffer, "k^0x%08x^", kflag);
      }
      pre.sar(ostring, "$", buffer, "");
   }
   if (mensuralQ) {
      pre.sar(ostring, "$", "W1", "");  // thin barlines
   }
   if (strlen(ostring.getBase()) > 0) {
      if (!pre.search(ostring, "^=", "")) {
         // add muse2ps option marker at start of option string.
         pre.sar(ostring, "^", "=", "");
      }
      if (!pre.search(ostring, "^=[^=]*=", "")) {
         // mark as a default type of option.
         pre.sar(ostring, "^", "=", "");
      }
      cout << "@muse2psv1" << ostring << NEWLINE;
   }
   

   // print global default options
   globaldefaults << ends;
   cout << globaldefaults.CSTRING;

   // print local default options
   localdefaults << ends;
   cout << localdefaults.CSTRING;

}



//////////////////////////////
//
// setColorCharacters -- If the Humdrum file has a record in the form:
//    !!!RDF**kern:\s*([^\s])\s*=\s*match
//    Then $1 is the marker in **kern data to indicate a match result
//    from a search red is the default color.  If there is a color code
//    in the RDF marker, the the most dominant color will be used
//    to mark the note in MuseData.
//
//    This note will be colored red in muse2ps, indicated
//    by placing "R" in column 14 of the note record, "G" for green,
//    and "B" for blue.
//

void setColorCharacters(HumdrumFile& infile, Array<char>& colorchar,
      Array<char>& colorout) {
   PerlRegularExpression pre;
   colorchar.setSize(0);
   colorout.setSize(0);
   char value;
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) {
         continue;
      }
      // !!!RDF**kern: N= mark color="#ff0000", root
      if (pre.search(infile[i].getLine(), 
            "^!!!RDF\\*\\*kern:\\s*([^\\s])\\s*=\\s*match", "i") ||
          pre.search(infile[i].getLine(), 
            "^!!!RDF\\*\\*kern:\\s*([^\\s])\\s*=\\s*mark", "i")
         ) {
         value = pre.getSubmatch(1)[0];
         colorchar.append(value);
         if (pre.search(infile[i].getLine(), 
               "color\\s*=\\s*\"?#([a-f0-9]{6})\"?", "i")) {
            value = getColorCategory(pre.getSubmatch(1));
            colorout.append(value);
         } else {
            value = 'R';
            colorout.append(value);
         }
      }
   }

   if (debugQ) {
      if (colorchar.getSize() > 0) {
         cout << "COLOR MARKERS:\n";
         for (i=0; i<colorchar.getSize(); i++) {
            cout << "\t\"" << colorchar[i] << "\" ==> ";
            cout << "\t\"" << colorout[i] << "\"";
            cout << NEWLINE;
         }
      }
   }
}



//////////////////////////////
//
// getColorCategory --
//

char getColorCategory(const char* color) {
   int len = strlen(color);
   if (len != 6) {
      return ' ';
   }

   char buffer[4] = {0};
   int num[3];

   buffer[0] = color[0];
   buffer[1] = color[1];
   num[0] = strtol(buffer, NULL, 16);

   buffer[0] = color[2];
   buffer[1] = color[3];
   num[1] = strtol(buffer, NULL, 16);

   buffer[0] = color[4];
   buffer[1] = color[5];
   num[2] = strtol(buffer, NULL, 16);

   if ((num[0] == num[1]) && (num[1] == num[2])) {
      // grayscale, so print black
      return ' ';
   }

   int max = 0;
   if (num[1] > num[0]) {
      max = 1;
   }
   if (num[2] > num[max]) {
      max = 2;
   }

   switch (max) {
      case 0:  return 'R';   // red channel is most dominant
      case 1:  return 'G';   // green channel is most dominant
      case 2:  return 'B';   // blue channel is most dominant
   }

   return ' ';
}



//////////////////////////////
//
// isBarlineBeforeData -- returns true if there is a barline found
//    before a dataline.
//

int isBarlineBeforeData(HumdrumFile& infile, int startindex) {
   int i;
   for (i=startindex; i<infile.getNumLines(); i++) {
      if (infile[i].isData()) {
         return 0;
      }
      if (infile[i].isMeasure()) {
         return 1;
      }
   }
   return 0;
}



//////////////////////////////
//
// convertData -- Extract all spines of **kern data into separate 
//     musedata files which are serially appended to the program output.
//

void convertData(Array<MuseData*>& outfiles, HumdrumFile& infile) {
   Array<int> kerntracks;
   getKernTracks(kerntracks, infile);
   int i;
   int tickpos; // tick location within score.
   outfiles.setSize(kerntracks.getSize());
   int printfirstmeasureQ = isBarlineBeforeData(infile, 0);
   int reversei;

   Array<int> ticksperquarter;
   prepareLocalTickChanges(infile, ticksperquarter);

   MuseRecord endrecord;
   for (i=0; i<kerntracks.getSize(); i++) {
      reversei = kerntracks.getSize() - i - 1;
      if (debugQ) {
         cout << "Extracting data for track: " << kerntracks[i]+1 << NEWLINE;
      }
      outfiles[reversei] = new MuseData;
      tickpos = 0;
      convertTrackToMuseData(*(outfiles[reversei]), 
            kerntracks[i], infile, ticksperquarter, tickpos, i+1, 
            kerntracks.getSize(), printfirstmeasureQ);

      // add an "/END" record to indicate the end of the file (there can 
      // be free-form comments after this
      if (referenceQ) {
         appendReferenceRecords(*outfiles[reversei], infile);
      }
      if (footerQ) {
         printFooter(*outfiles[reversei], infile, footertext);
      }
      // place abouve appendReferenceRecord when the =M preserves post-END comments
      endrecord.clear();
      endrecord.insertString(1, "/END");
      outfiles[reversei]->append(endrecord);
   }
}



//////////////////////////////
//
// prepareLocalTickChanges -- check to make sure that ticks never
//    exceed 999 for rhythmic durations.  If they do, then try to change
//    the Q: record (ticks per quarter) based on measures, If the global
//    tick setting will not work.  If measure-based doesn't work, then
//    generalized this function to allow Q: to change within measures
//    (but this is difficult to generalize).
//
//    If this algorithm fails, next thing to try is to encode tpq
//    for parts separately.  If that generalization fails, then
//    adjust tpq within measures.  If that generalization fails, then
//    too bad.
//

void prepareLocalTickChanges(HumdrumFile& infile, Array<int>& ticks) {
   int ticksperquarter = getGlobalTicksPerQuarter(infile);

   // maybe make measurelines an input parameter since it is also
   // calculated elsewhere.  But it is calculated in an inconvenient
   // location, so doing it here.
   Array<int> measurelines;
   getMeasureInfo(measurelines, infile);

   // handle case where there are no measure lines in the data:
   if (measurelines.getSize() == 0) {
      measurelines.setSize(2);
      measurelines[0] = 0;
      measurelines[1] = infile.getNumLines()-1;
   }

   Array<RationalNumber> rhys;
   getRhythms(rhys, infile, 0, -1);
   if (rhys.getSize() == 0) {
      // something funny: no durations in file, so don't try to do anything
      ticks.setSize(measurelines.getSize());
      ticks.setAll(ticksperquarter);
      return;
   }
   RationalNumber maxtick;
   // the first rhythm is the largest.
   maxtick = rhys[0].getInversion() * ticksperquarter;

   if (maxtick < 1000) {
      // there will never be a tick overflow in the converted data.
      ticks.setSize(measurelines.getSize());
      ticks.setAll(ticksperquarter);
      return;
   }

   // there will be an overflow in the converted data.
   // try to adjust the ticks by each measure to localize
   // tuplets or extreme rhythms which are causing the problem.

   Array<int> mtpq;
   mtpq.setSize(measurelines.getSize());
   mtpq.setAll(0);
   Array<RationalNumber> mticks;
   mticks.setSize(measurelines.getSize());
   mticks.setAll(0);
   int i;
   for (i=0; i<measurelines.getSize()-1; i++) {
      getRhythms(rhys, infile, measurelines[i], measurelines[i+1]);
      if (rhys.getSize() > 0) {
         mtpq[i] = getTPQByRhythmSet(rhys);
         mticks[i] = rhys[0].getInversion() * mtpq[i];
      } else {
         mticks[i] = 0;
         if (i > 0) {
            mtpq[i] = mtpq[i-1];
         }
      }

   }

   if (mtpq.getSize() == 0) {
      // no barlines, too difficult to deal with.
      ticks.setSize(measurelines.getSize());
      ticks.setAll(ticksperquarter);
      return;
   }
   if (mtpq[mtpq.getSize()-2] == 0) {
      // no non-zero mtpq values, strange, so just give up.
      ticks.setSize(measurelines.getSize());
      ticks.setAll(ticksperquarter);
      return;
   }

   // group measures by their ability to not exceed durations of 999 ticks
   // for the maximum rhythmic duration in the range.
   Array<int> newtpq;
   newtpq.setSize(measurelines.getSize());
   newtpq.setAll(0);

   int curtpq = mtpq[0];
   int lcm;
   RationalNumber maxdur = mticks[0];  // keep track of the maximum rhy value
   maxdur /= mtpq[0];                 
   RationalNumber tmax;
   RationalNumber tdur;                // test new maximum rhy duration
   Array<int> twonum(2);

   for (i=1; i<mtpq.getSize(); i++) {
      twonum[0] = curtpq;
      twonum[1] = mtpq[i];
      lcm = LCM(twonum);

      // get the non-tick version of the new possible maximum.
      tdur = mticks[i];
      tdur /= mtpq[i];
      if (tdur > maxdur) {
         tmax = tdur;
      } else {
         tmax = maxdur;
      }
      // check to see if the test maximum does not exceed the new
      // test ticks per quarter setting:
      if (tmax * lcm < 1000) {
         // the new measure can be added to the previous grouping
         // so store the new values (and erase previous setting.
         maxdur = tmax;
         curtpq = lcm;
         newtpq[i] = lcm;
         if (i < measurelines.getSize()-2) {
            newtpq[i-1] = 0;
         }
         continue;
      }

      // the new tpq would be too large to accomodate the largest
      // duration in the range. So restart the calculation on the
      // current measure.

      maxdur = tdur;
      curtpq = mtpq[i];
      newtpq[i] = curtpq;
   }


   // travel backwards in newtpq, setting any values that are 0 to the 
   // next higher index value which is non-zero.
   for (i=newtpq.getSize()-2; i>=0; i--) {
      if (newtpq[i] == 0) {
         newtpq[i] = newtpq[i+1];
      }
   }

   

   if (debugQ) {
      RationalNumber rn;
      for (i=0; i<newtpq.getSize()-1; i++) {
         cout << "START:" << measurelines[i] << "\tSTOP:" << measurelines[i+1] 
              << "\ttpq = " << mtpq[i]  << "(" << newtpq[i] << ")";
         cout << "\tmaxticks = " << mticks[i];
         rn = mticks[i];
         rn /= mtpq[i];
         rn *= newtpq[i];
         cout << "(" << rn << ")";
         //cout << "\trhys: ";
         //int j;
         //for (j=0; j<rhys.getSize(); j++) {
         //   cout << rhys[j] * 4 << " ";
         //}
         cout << NEWLINE;
      }
   }

   ticks.setSize(measurelines.getSize());
   for (i=0; i<ticks.getSize(); i++) {
      ticks[i] = newtpq[i];
   }

}



//////////////////////////////
//
// getTPQByRhythmSet --
//

int getTPQByRhythmSet(Array<RationalNumber>& rhys) {
   Array<int> intrhy;
   intrhy.setSize(rhys.getSize());
   int i;
   for (i=0; i<rhys.getSize(); i++) {
      intrhy[i] = rhys[i].getNumerator();
   }
   return LCM(intrhy);
}



//////////////////////////////
//
// getGlobalTicksPerQuarter -- Returns the number of ticks per quarter
//     which would be the result of all rhythms found in the file.
//     If the ticks per quarter will generate a note duration longer than 
//     999 ticks, then a local tick system needs to be done.
//

int getGlobalTicksPerQuarter(HumdrumFile& infile) {
   RationalNumber rn(1,1);
   rn = infile.getMinTimeBaseR();
   rn /= 4;

   if (rn.getDenominator() != 1) {
      // tuplets at a higher level than a quarter note most likely,
      // so put the 4 back in place.
      rn *= rn.getDenominator();
   }

   // hacky case where the minrhy is 0 and there is a defaultDur
   if (rn == 0) {
      rn = getDuration("", defaultDur);
      rn.invert();
      Array<int> rhythms(2);
      rhythms[0] = rn.getNumerator();
      rhythms[1] = 4;
      int returnval = LCM(rhythms);
      ignoreTickError = 1;
      return returnval;
   }

   if (debugQ) {
      cout << "Ticks per quarter: " << rn << NEWLINE;
   }
   
   // should be simplified automatically.
   return rn.getNumerator() * rn.getDenominator();
}



//////////////////////////////
//
// convertTrackToMuseData -- Convert a particular track of **kern data into
//     a MuseData file.  Successive measures are processed one at a time.
//

void convertTrackToMuseData(MuseData& musedata, int track, 
      HumdrumFile& infile, Array<int>& tpq, int& tickpos, int counter, 
      int total, int printFirstMeasureQ) {
   Array<int> measures;

   // should probably move to calling function, but keeping here for now
   // in case the more generalized solution of finding the barlines in each
   // individual track should be done instead of the global barlines.
   // The HumdrumFile class likes barlines to be global...
   getMeasureInfo(measures, infile);
   int i;
   MuseData tempdata;
   insertHeaderRecords(infile, musedata, track, counter, total);
   LastTPQPrinted = tpq[0];
   insertDollarRecord(infile, 0, musedata, track, counter, total, tpq[0]);
   
   if (hasTuplet[track]) {
      // add a style which places no tuplet slur on beamed notes
      MuseRecord arecord;
      arecord.insertString(1, "P C0:t2");
      musedata.append(arecord);
   }

   int founddataQ = 0;  // needed to suppress first $ record from being
                       // reprinted (the first one was needed above
                       // with the Q: field added).

   for (i=0; i<measures.getSize()-1; i++) {
      tempdata.clear();
      if (debugQ) {
         cout << "Extracting data for measure starting at line: " 
              << measures[i] << NEWLINE;
      }
      getMeasureData(tempdata, track, infile, measures[i], measures[i+1], 
            tpq[i], tickpos, printFirstMeasureQ, founddataQ);
      musedata.append(tempdata);
   }

}



//////////////////////////////
//
//
// insertDollarRecord -- Musical Attributes Record
//
// Example:
//    $  K:0   Q:4   T:1/1   C1:4   C2:22
// Column 1: $
// Column 2: level number (optional)
// Column 3: footnote column
// Columns 4-80 attributes
//
// K: key
// Q: divisions per quarter note (required in first $ record in part)
// T: time signature, such as T:4/4
// C: clef (for all staves in part)
// C1: clef for staff 1 in part
// C2: clef for staff 2 in part
// X: transposing part
// S: number of staves in part
// I: number of instruments in part
// D: directive (last field on data line)
//

void insertDollarRecord(HumdrumFile& infile, int line, MuseData& musedata, 
      int track, int counter, int total, int tpq) {

   MuseRecord arecord;
   arecord.insertString(1, "$  ");
   char buffer[1024] = {0};
   char tempbuf[16] = {0};

   if (line != 0) {
      // don't know why this is being done, maybe remove, or always
      // place extra space?
      strcat(buffer, " ");
   }
   
   // key signature
   if (appendKeySignature(buffer, infile, line, track)) {
      strcat(buffer, "   ");
   }
   // ticks
   if (tpq > 0) {
      sprintf(tempbuf, "Q:%d   ", tpq);
      strcat(buffer, tempbuf);
   }

   // time signature
   if (appendTimeSignature(buffer, infile, line, track, tpq)) {
         strcat(buffer, "   ");
   }

   // clefs
   if (appendClef(buffer, infile, line, track)) {
      strcat(buffer, "   ");
   } else {
      // a clef must be present in the first musical attributes line
      // for a part, so force one here.  Make it a treble clef...
      if (tpq > 0) {
         // only print if Q: recrod is being printed.  Not quite
         // right, but rare to print Q: record twice in part, so good
         // enought for now.
         strcat(buffer, "C:4   ");
      }
   }

   // add a movement designation if this is the first
   // one (will have to fix so that movement designations which
   // occur later can be also printed...
   // if (tpq > 0) {
      addMovementDesignation(buffer, infile, line);
   // } 

   arecord.insertString(4, buffer);
   musedata.append(arecord);
}



//////////////////////////////
//
// AddMovementDesignation --
//

void addMovementDesignation(char* buffer, HumdrumFile& infile, int line) {
   int omdline = -1;
   int i;
   PerlRegularExpression pre;

   for (i=line; i<infile.getNumLines(); i++) {
      if (infile[i].isData()) {
         break;
      }
      if (!infile[i].isBibliographic()) {
         continue;
      }
      if (pre.search(infile[i][0], "^!!!OMD[^:]*:\\s*(.*)\\s*$", "")) {
         omdline = i;
         break;
      }
   }


   if (omdline < 0) {
      for (i=line-1; i>=0; i--) {
         if (infile[i].isData()) {
            break;
         }
         if (!infile[i].isBibliographic()) {
            continue;
         }
         if (pre.search(infile[i][0], "^!!!OMD[^:]*:\\s*(.*)\\s*$", "")) {
            omdline = i;
            break;
         }
      }
   }

   if (omdline >= 0) {
      strcat(buffer, "D:");
      strcat(buffer, pre.getSubmatch(1));
      strcat(buffer, "   ");
   }

}



//////////////////////////////
//
// appendClef --  Append a clef marker to a Music Attribute
// record if there is a key signature intepretation somewhere between the 
// current line and the first data line found after that line -- in the 
// specified primary track.
//

int appendClef(char* buffer, HumdrumFile& infile, int line, int track) {
   int i, j;

   int row = -1;  // row where key signature was found;
   int col = -1;  // col where key signature was found;

   for (i=line; i<infile.getNumLines(); i++) {
      if (infile[i].isData()) {
         break;
      }
      if (!infile[i].isInterpretation()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (infile[i].getPrimaryTrack(j) != track) {
            continue;
         }
         if (strncmp("*clef", infile[i][j], 5) == 0) {
            row = i;
            col = j;
         }
      }
   }

   if ((row < 0) || (col < 0)) {
      return 0;
   }

   if (strcmp(infile[row][col], "*clefG1") == 0) {  // French violin clef
      strcat(buffer, "C:5");
   } else if (strcmp(infile[row][col], "*clefG2") == 0) {  // treble clef
      strcat(buffer, "C:4");
   } else if (strcmp(infile[row][col], "*clefG3") == 0) {
      strcat(buffer, "C:3");
   } else if (strcmp(infile[row][col], "*clefG4") == 0) {
      strcat(buffer, "C:2");
   } else if (strcmp(infile[row][col], "*clefG5") == 0) {
      strcat(buffer, "C:1");
   } else if (strcmp(infile[row][col], "*clefC1") == 0) { // soprano
      strcat(buffer, "C:15");
   } else if (strcmp(infile[row][col], "*clefC2") == 0) { // mezzo-soprano
      strcat(buffer, "C:14");
   } else if (strcmp(infile[row][col], "*clefC3") == 0) { // alto
      strcat(buffer, "C:13");
   } else if (strcmp(infile[row][col], "*clefC4") == 0) { // tenor
      strcat(buffer, "C:12");
   } else if (strcmp(infile[row][col], "*clefC5") == 0) { // baritone
      strcat(buffer, "C:11");
   } else if (strcmp(infile[row][col], "*clefF1") == 0) {
      strcat(buffer, "C:25");
   } else if (strcmp(infile[row][col], "*clefF2") == 0) {
      strcat(buffer, "C:24");
   } else if (strcmp(infile[row][col], "*clefF3") == 0) {
      strcat(buffer, "C:23");
   } else if (strcmp(infile[row][col], "*clefF4") == 0) { // bass clef
      strcat(buffer, "C:22");
   } else if (strcmp(infile[row][col], "*clefF5") == 0) {
      strcat(buffer, "C:21");
   } else if (strcmp(infile[row][col], "*clefGv1") == 0) {
      strcat(buffer, "C:35");
   } else if (strcmp(infile[row][col], "*clefGv2") == 0) { // vocal tenor clef
      strcat(buffer, "C:34");
   } else if (strcmp(infile[row][col], "*clefGv3") == 0) {
      strcat(buffer, "C:33");
   } else if (strcmp(infile[row][col], "*clefGv4") == 0) {
      strcat(buffer, "C:32");
   } else if (strcmp(infile[row][col], "*clefGv5") == 0) {
      strcat(buffer, "C:31");
   } else {
      strcat(buffer, "C:4"); // use default of treble clef
      // although auto-detecting the range of the data in the spine 
      // might be better (so that bass clef might be selected automatically).
   }

   return 1;
}



//////////////////////////////
//
// appendTimeSignature --  Append a time signature marker to a Music Attribute
// record if there is a time signature intepretation somewhere between the 
// current line and the first data line found after that line -- in the 
// specified primary track.  If there is a *met() code, then use that instead
// of the *M attribute.
//

int appendTimeSignature(char* buffer, HumdrumFile& infile, int line, 
      int track, int tpq) {
   int metrow = -1;
   int metcol = -1;
   int timerow = -1;
   int timecol = -1;

   PerlRegularExpression pre;
   int i, j;
   for (i=line; i<infile.getNumLines(); i++) {
      if (infile[i].isData()) {
         break;
      }
      if (!infile[i].isInterpretation()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (track != infile[i].getPrimaryTrack(j)) {
            continue;
         }
         if (pre.search(infile[i][j], "^\\*M\\d+/\\d+", "")) {
            timerow = i;
            timecol = j;
         } else if (pre.search(infile[i][j], "^\\*met\\([^)]*\\)", "")) {
            if (metQ) {
               metrow = i;
               metcol = j;
            }
         }
         // only look at first layer of track on a line.
         break;
      }
   }

   if (metrow >= 0) {
      // a met code has been found, so use that to print a time signature
      if (strcmp(infile[metrow][metcol], "*met(O)") == 0) {
         strcat(buffer, "T:11/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(c)") == 0) {
         strcat(buffer, "T:1/1");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(com)") == 0) {
         strcat(buffer, "T:1/1");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(common)") == 0) {
         strcat(buffer, "T:1/1");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(c|)") == 0) {
         strcat(buffer, "T:0/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(cut)") == 0) {
         strcat(buffer, "T:0/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(O:)") == 0) {
         strcat(buffer, "T:12/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(O.)") == 0) {
         strcat(buffer, "T:21/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(O:.)") == 0) {
         strcat(buffer, "T:22/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(O;)") == 0) {
         // preferred alternate for *met(O;)
         strcat(buffer, "T:22/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(C)") == 0) {
         strcat(buffer, "T:31/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(C.)") == 0) {
         strcat(buffer, "T:41/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(Cr)") == 0) {
         strcat(buffer, "T:51/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(C|)") == 0) {
         strcat(buffer, "T:61/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(C2)") == 0) {
         strcat(buffer, "T:71/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(O2)") == 0) {
         strcat(buffer, "T:81/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(O|)") == 0) {
         strcat(buffer, "T:91/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(C|3)") == 0) {
         strcat(buffer, "T:101/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(3)") == 0) {
         strcat(buffer, "T:102/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(O/3)") == 0) {
         // eventually O/3 should generate a new mensural sign in
         // musedata.  O/3 has the same function as 3.
         strcat(buffer, "T:102/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(3/2)") == 0) {
         strcat(buffer, "T:103/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(C|2)") == 0) {
         strcat(buffer, "T:111/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(2)") == 0) {
         strcat(buffer, "T:112/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met(Oo)") == 0) {
         strcat(buffer, "T:121/0");   return 1;
      } else if (strcmp(infile[metrow][metcol], "*met()") == 0) {
         // hide the time signature
         // strcat(buffer, "");   return 1;
         return 0;
      } else {
         // an unknown metric symbol, so try your luck with time signature
         // data below.
      }
   }

   if (timerow >= 0) {
      if (!pre.search(infile[timerow][timecol], "^\\*M(\\d+)/(\\d+)")) {
         // something funny happened...
         return 0;
      }
      int timetop = atoi(pre.getSubmatch(1));
      int timebot = atoi(pre.getSubmatch(2));
      if ((timebot == 0) && (timetop < 9)) {
         // Currently reserving 9/0 for a hidden time signature
         // meters larger than 10/0 are used for mensural signatures
         //
         // For time signatures smaller than 9/0, note that
         // MuseData cannot handle a breve as the beat, so adjust
         // by shifting the bottom number from 0 to 1 and double the
         // top number in the time signature.
         timebot = 1;
         timetop *= 2;
      }
      SSTREAM temps;
      temps << "T:";
      temps << timetop << "/" << timebot;
      temps << ends;
      strcat(buffer, temps.CSTRING);
   } else {
      // no time signature found in interpretation region.
      // currently print the time signature 9/0 which functions
      // as an invisible time signature, although this will change
      // later when the muse2ps program no longer requires a time
      // signature at the start of the music.
      if (tpq >= 0) {
         // no longer needed since muse2ps can now handle an empty time sig.
         // strcat(buffer, "T:9/0");
      }
      // don't print hidden time signature if no Q record.
      // return 0;
   }

   // *met(.*) or *M\d+/\d+ found in interpretation region.
   return 1;
}




//////////////////////////////
//
// appendKeySignature --  Append a key signature marker to a Music Attribute
// record if there is a key signature intepretation somewhere between the 
// current line and the first data line found after that line -- in the 
// specified primary track.
//

int appendKeySignature(char* buffer, HumdrumFile& infile, int line, int track) {
   int i, j;
   PerlRegularExpression pre;

   int row = -1;  // row where key signature was found;
   int col = -1;  // col where key signature was found;

   for (i=line; i<infile.getNumLines(); i++) {
      if (infile[i].isData()) {
         break;
      }
      if (!infile[i].isInterpretation()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (infile[i].getPrimaryTrack(j) != track) {
            continue;
         }
         if (pre.search(infile[i][j], "^\\*k\\[.*\\]", "")) {
            row = i;
            col = j;
         }
      }
   }

   if ((row < 0) || (col < 0)) {
      return 0;
   }

   int keynumber = Convert::kernKeyToNumber(infile[row][col]);
   char keybuf[16] = {0};
   sprintf(keybuf, "K:%d", keynumber);
   strcat(buffer, keybuf);
   return 1;
}



//////////////////////////////
//
// insertHeaderRecords --
//

void insertHeaderRecords(HumdrumFile& infile, MuseData& tempdata, 
     int track, int counter, int total) {

   MuseRecord arecord;

   // Record 1: 		Copyright notice
   addCopyrightLine(infile, tempdata);
 
   // Record 2: 		Control number
   addControlNumber(infile, tempdata);

   // Record 3: 		Timestamp
   addTimeStamp(tempdata);
   
   // Record 4: 		<date> <name of encoder>
   addDateAndEncoder(infile, tempdata);

   // Record 5: 		WKn:<work number> MVn:<movement number>
   addWorkNumberInfo(infile, tempdata);
   
   // Record 6: 		<source>
   addSourceRecord(infile, tempdata);
   
   // Record 7: 		<work title>
   addWorkTitle(infile, tempdata);
   
   // Record 8: 		<movement title>
   addMovementTitle(infile, tempdata);
   
   // Record 9: 		<name of part>
   addPartName(infile, track, tempdata);
   
   // Record 10:                miscellaneous designations 
   // such as          [mode], [movement type] and [voice]
   arecord.clear();
   arecord.insertString(1, "Header Record 10");
   tempdata.append(arecord);
   
   // Record 11: 		group memberships: <name1> <name2> . . .
   arecord.clear();
   arecord.insertString(1, "Group memberships: score");
   tempdata.append(arecord);

   // Record 12: 		<name1>: part <x> of <number in group>
   SSTREAM partnum;
   partnum << "score: part " << (total - counter)+1 << " of " << total;
   partnum << ends;
   arecord.clear();
   arecord.insertString(1, partnum.CSTRING);
   tempdata.append(arecord);
}



//////////////////////////////
//
// addCopyrightLine -- add fixed header record 1
//


void addCopyrightLine(HumdrumFile& infile, MuseData& tempdata) {
   MuseRecord arecord;
   arecord.insertString(1, "Header Record 1: optional copyright notice");

   char buffer [1024] = {0};
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) {
         continue;
      }
      if (strncmp("YEC", infile[i].getBibKey(buffer, 1000), 3) == 0) {
         arecord.clear();
         arecord.insertString(1, infile[i].getBibValue(buffer, 80));
         break;
      }
   }

   if (copyrightQ) {
      arecord.clear();
      arecord.insertString(1, Copyright);
   }

   tempdata.append(arecord);
}



//////////////////////////////
//
// addControlNumber -- add fixed header record 2
//

void addControlNumber(HumdrumFile& infile, MuseData& tempdata) {
   MuseRecord arecord;
   arecord.insertString(1, "Header Record 2: optional file identification");
   tempdata.append(arecord);
}



//////////////////////////////
//
// addTimeStamp -- add fixed header record 3
//

void addTimeStamp(MuseData& tempdata) {
   MuseRecord arecord;
   // arecord.insertString(1, "Header Record 3: optional timestamp/checksum");
   struct tm *current;
   time_t now;
   time(&now);
   current = localtime(&now);
   int year    = current->tm_year + 1900;
   int month   = current->tm_mon + 1;
   int day     = current->tm_mday;
   const char* ptr = "JAN";
   switch (month) {
      case  1:  ptr = "JAN"; break;
      case  2:  ptr = "FEB"; break;
      case  3:  ptr = "MAR"; break;
      case  4:  ptr = "APR"; break;
      case  5:  ptr = "MAY"; break;
      case  6:  ptr = "JUN"; break;
      case  7:  ptr = "JUL"; break;
      case  8:  ptr = "AUG"; break;
      case  9:  ptr = "SEP"; break;
      case 10:  ptr = "OCT"; break;
      case 11:  ptr = "NOV"; break;
      case 12:  ptr = "DEC"; break;
   }
   const char* dptr = "";
   if (day < 10) {
      dptr = "0";
   }
   arecord.append("ssssisis", "TIMESTAMP: ", ptr, "/", dptr, day, 
         "/", year, " []");
   tempdata.append(arecord);
}



//////////////////////////////
//
// addDateAndEncoder -- add fixed header record 4
//

void addDateAndEncoder(HumdrumFile& infile, MuseData& tempdata) {
   MuseRecord arecord;
   int encline = -1;
   int eedline = -1;
   int endline = -1;
   int eevline = -1;
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()){ 
         continue;
      }
      if (strncmp(infile[i][0], "!!!END:", 7) == 0) {  // encoding date
         endline = i;
      }
      if (strncmp(infile[i][0], "!!!ENC:", 7) == 0) { // encoder's name
         encline = i;
      }
      if (strncmp(infile[i][0], "!!!EED:", 7) == 0) { // edition editor
         eedline = i;
      }
      if (strncmp(infile[i][0], "!!!EEV:", 7) == 0) { // edition date
         eevline = i;
      }
   }

   int year  = -1;
   int month = -1;
   int day   = -1;

   PerlRegularExpression pre;
   if (endline >= 0) {

      if (pre.search(infile[endline][0], "(\\d{4})/0*(\\d+)/0*(\\d+)", "")) {
         year  = atoi(pre.getSubmatch(1));
         month = atoi(pre.getSubmatch(2)); 
         day   = atoi(pre.getSubmatch(3));
      } else if (pre.search(infile[endline][0], "(\\d{4})/0*(\\d+)", "")) {
         year  = atoi(pre.getSubmatch(1));
         month = atoi(pre.getSubmatch(2)); 
         day   = 0;
      } else if (pre.search(infile[endline][0], "(\\d{4})", "")) {
         year  = atoi(pre.getSubmatch(1));
         month = 0;
         day   = 0;
      }

   } else if (eevline >= 0) {

      if (pre.search(infile[eevline][0], "(\\d{4})/0*(\\d+)/0*(\\d+)", "")) {
         year  = atoi(pre.getSubmatch(1));
         month = atoi(pre.getSubmatch(2)); 
         day   = atoi(pre.getSubmatch(3));
      } else if (pre.search(infile[eevline][0], "(\\d{4})/0*(\\d+)", "")) {
         year  = atoi(pre.getSubmatch(1));
         month = atoi(pre.getSubmatch(2)); 
         day   = 0;
      } else if (pre.search(infile[eevline][0], "(\\d{4})", "")) {
         year  = atoi(pre.getSubmatch(1));
         month = 0;
         day   = 0;
      }

   }

   char encoder[1024] = {0};

   if (year < 0) {
      // Use today's date:
      struct tm *current;
      time_t now;
      time(&now);
      current = localtime(&now);
      year = current->tm_year + 1900;
      month = current->tm_mon + 1;
      day = current->tm_mday;
   }

   char datebuf[1024] = {0};
   char daybuf[16] = {0};
   if (day < 10) {
      sprintf(daybuf, "0%d", day);
   } else {
      sprintf(daybuf, "%d", day);
   }
   char monthbuf[16] = {0};
   if (month < 10) {
      sprintf(monthbuf, "0%d", month);
   } else {
      sprintf(monthbuf, "%d", month);
   }
   sprintf(datebuf, "%s/%s/%d ", monthbuf, daybuf, year);

   if (encline >= 0) {
      if (pre.search(infile[encline][0], "^!!!ENC:\\s*(.*)\\s*$", "")) {
         strcpy(encoder, pre.getSubmatch(1));
      }
   }  else if (eedline >= 0) {
      if (pre.search(infile[eedline][0], "^!!!EED:\\s*(.*)\\s*$", "")) {
         strcpy(encoder, pre.getSubmatch(1));
      }
   }
   if (encoderQ) {
      strcpy(encoder, Encoder);
   } else if (strlen(encoder) == 0) {
      strcpy(encoder, "hum2muse");
   }

   arecord.insertString(1, datebuf);
   arecord.appendString(encoder);

   tempdata.append(arecord);
}



//////////////////////////////
//
// addWorkNumberInfo -- add fixed header record 5.  This line must start
//    with "WK#" in order for muse2ps to process the file.
//

void addWorkNumberInfo(HumdrumFile& infile, MuseData& tempdata) {
   MuseRecord arecord;
   Array<char> work;
   Array<char> movement;
   getWorkAndMovement(work, movement, infile);
   arecord.append("ssss", "WK#:", work.getBase(), " MV#:", movement.getBase());
   tempdata.append(arecord);
}



//////////////////////////////
//
// getWorkAndMovement --
//

void getWorkAndMovement(Array<char>& work, Array<char>& movement, 
      HumdrumFile& infile) {

   movement.setSize(2);
   work.setSize(2);
   strcpy(movement.getBase(), "1");
   strcpy(work.getBase(),     "1");

   int omvline = -1;
   int sctline = -1;
   int opsline = -1;
   int onmline = -1;

   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) {
         continue;
      }
      if (strncmp(infile[i][0], "!!!OMV", 6) == 0) {
         omvline = i;
      }
      if (strncmp(infile[i][0], "!!!SCT", 6) == 0) {
         sctline = i;
      }
      if (strncmp(infile[i][0], "!!!OPS", 6) == 0) {
         opsline = i;
      }
      if (strncmp(infile[i][0], "!!!ONM", 6) == 0) {
         onmline = i;
      }
   }

   PerlRegularExpression pre;
   PerlRegularExpression pre2;
   if (omvline >= 0) {
      pre.search(infile[omvline][0], "^!!!OMV[^:]*:\\s*(.*)\\s*$", "");
      movement.setSize(strlen(pre.getSubmatch(1)) +1);
      strcpy(movement.getBase(), pre.getSubmatch());
      pre.sar(movement, "^[^\\d]+", "", "");
      pre.sar(movement, "\\s*.*", "", "");
      if (strlen(movement.getBase()) == 0) {
         movement.setSize(2);
         strcpy(movement.getBase(), "1");
      }
   }

   // if there is a BWV in SCT, then use that as the work # and any
   // slash after the first number after BWV as the movment number.
   if ((sctline >= 0) && pre.search(infile[sctline][0], 
         "^!!!SCT[^:]*:\\s*(.+)\\s*$", "")) {

      if (pre2.search(pre.getSubmatch(1), "BWV\\s+(\\d+)/(\\d+)", "i")) {
         work.setSize(strlen(pre2.getSubmatch(1)) + 1);
         strcpy(work.getBase(), pre2.getSubmatch());
         movement.setSize(strlen(pre2.getSubmatch(2)) + 1);
         strcpy(movement.getBase(), pre2.getSubmatch());
         return;
      }

      if (pre2.search(pre.getSubmatch(1), "BWV\\s*(\\d+[^\\s]*)", "i")) {
         work.setSize(strlen(pre2.getSubmatch(1)) + 1);
         strcpy(work.getBase(), pre2.getSubmatch());
         return;
      }

   }

   // if there is an opus number, then use that as the work number
   // handle onm line later...
   if ((opsline >= 0) && pre.search("^!!!OPS[^:]*:\\s*(\\d[^\\s]*)", "")) {
      work.setSize(strlen(pre2.getSubmatch(1)) + 1);
      strcpy(work.getBase(), pre2.getSubmatch());
      return;
   }

}



//////////////////////////////
//
// addSourceRecord -- add fixed header record 6, which is the 
// original source for this particular digital encoding.
//

void addSourceRecord(HumdrumFile& infile, MuseData& tempdata) {
   MuseRecord arecord;

   PerlRegularExpression pre;
   int smsline = -1;
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) {
         continue;
      }
      if (pre.search(infile[i][0], "^!!!SMS[^:]*:\\s(.*)\\s*$", "")) {
         smsline = i;
         break;
      }
      
   }
   if (smsline < 0) {
      arecord.insertString(1, "Header Record 6: source");
   } else {
      Array<char> sms;
      sms.setSize(strlen(pre.getSubmatch(1)) + 1);
      strcpy(sms.getBase(), pre.getSubmatch());
      convertHtmlTextToMuseData(sms);
      arecord.insertString(1, sms.getBase());
   }

   tempdata.append(arecord);
}



//////////////////////////////
//
// addWorkTitle -- add fixed header record 7 which is the title of the work
// (of which this data may be a particular movment).
//

void addWorkTitle(HumdrumFile& infile, MuseData& tempdata) {
   MuseRecord arecord;

   PerlRegularExpression pre;
   int otlline = -1;
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) {
         continue;
      }
      if (pre.search(infile[i][0], "^!!!OTL[^:]*:\\s(.*)\\s*$", "")) {
         otlline = i;
         break;
      }
      
   }
   if (otlline < 0) {
      arecord.insertString(1, "Header Record 7: work title");
   } else {
      Array<char> strang(strlen(pre.getSubmatch(1))+1);
      strcpy(strang.getBase(), pre.getSubmatch(1));
      convertHtmlTextToMuseData(strang);
      arecord.insertString(1, strang.getBase());
   }

   tempdata.append(arecord);
}



//////////////////////////////
//
// addMovementTitle -- add fixed header record 8, which is the movement
// name.  Change to OMV: OMD, instead of OMD.
//

void addMovementTitle(HumdrumFile& infile, MuseData& tempdata) {
   MuseRecord arecord;

   PerlRegularExpression pre;
   int omdline = -1;
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isBibliographic()) {
         continue;
      }
      if (pre.search(infile[i][0], "^!!!OMD[^:]*:\\s(.*)\\s*$", "")) {
         omdline = i;
         break;
      }
      
   }
   if (omdline < 0) {
      arecord.insertString(1, "Header Record 8: movement title");
   } else {
      Array<char> strang(strlen(pre.getSubmatch(1))+1);
      strcpy(strang.getBase(), pre.getSubmatch(1));
      convertHtmlTextToMuseData(strang);
      arecord.insertString(1, strang.getBase());
   }

   tempdata.append(arecord);
}



//////////////////////////////
//
// addPartName -- add fixed header record 9 which is the instrumental
// name of the part.  This name can be set explicitly with *I" tandem
// interpretations in the data.  Otherwise, the part name will be
// (eventually) extracted automatically from the *I instrument 
// code names.
//

void addPartName(HumdrumFile& infile, int track, MuseData& tempdata) {
   MuseRecord arecord;
   if (strcmp(PartNames[track].getBase(), "") != 0) {
      arecord.insertString(1, PartNames[track].getBase());
   } else {
      arecord.insertString(1, "                                  ");
   }
   tempdata.append(arecord);
}



//////////////////////////////
//
// getMeasureData --  Get MuseData for a particular measure in a particular
//     part (track).
//

void getMeasureData(MuseData& tempdata, int track, HumdrumFile& infile, 
      int startline, int stopline, int tpq, int& tickpos, 
      int& measuresuppress, int& founddataQ) {
   int maxvoice = getMaxVoices(infile, startline, stopline, track);
   if (debugQ) {
      cout << "For line range " << startline+1 << " to " << stopline+1
           << " in track " << track << " maxvoices = " << maxvoice << NEWLINE;
   }

   if (maxvoice == 0) {
      // measure has no data
      maxvoice = 1;
   }

   int starttick;
   int stoptick;
   starttick = getTick(tpq, infile, startline);
   stoptick  = getTick(tpq, infile, stopline);

   int voice;
   for (voice=0; voice<maxvoice; voice++) {
      processVoice(tempdata, infile, startline, stopline, track, voice+1, 
            tpq, tickpos, measuresuppress, founddataQ, starttick, stoptick);
   }
}



//////////////////////////////
//
// processVoice --
//

void processVoice(MuseData& tempdata, HumdrumFile& infile, int startline, 
      int stopline, int track, int voice, int tpq, int& tickpos, 
      int& measuresuppress, int& founddataQ, int starttick, int stoptick) {

   int tpqtest = -1;
   if (LastTPQPrinted != tpq) {
      tpqtest = tpq;
      LastTPQPrinted = tpq;
      // hack for variable tpq:
      tickpos = -1;    
   }

   PerlRegularExpression pre;

   // int starttick = getTick(tpq, infile, startline);
   // int stoptick  = getTick(tpq, infile, stopline);
   if (debugQ) {
      cout << "STARTTICK = " << starttick 
           << "\tSTOPTICK = " << stoptick << NEWLINE;
   }

   if (tickpos < 0) {
      // hack for variable tpq
      tickpos = starttick;
   }
   int backQ = 0;

   // emit a back command to go back to the start of the measure
   // if this is not the first voice to process...
   if (voice > 1) {
      addBackup(tempdata, stoptick - starttick, tpq);
      tickpos -= stoptick - starttick;
      backQ = 1;
   }

   if ((!ignoreTickError) && (tickpos != starttick)) {
      cerr << "Error: tick mismatch: " << tickpos << " " << starttick << NEWLINE;
      cerr << "on line " << startline + 1 << NEWLINE;
      exit(1);
   }

   int dollarprint = 0;

   int curvoice = 0;
   int i, j;
   for (i=startline; i<stopline; i++) {
      if (debugQ) {
         cout << "INPUT LINE: " << infile[i] << endl;
      }
      curvoice = 0;
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (debugQ) {
            cout << "J = " << j << endl;
         }
         if (track == infile[i].getPrimaryTrack(j)) {
            curvoice++;
         } else {
            if ((!backQ) && (curvoice == 0) && (!dollarprint) &&
               infile[i].isBibliographic()) {
               if (strncmp(infile[i][0], "!!!OMD", 6) == 0) {
                  dollarprint = 1;
                  insertDollarRecord(infile, i, tempdata, track, 0, 0, tpqtest);
                  tpqtest = -1;
               }
            }

            continue;
         }
         if (curvoice != voice) {
            continue;
         }

         // print the barline for this segment of data
         if ((curvoice == 1) && infile[i].isMeasure()) {
            if (!measuresuppress) {
               addMeasureEntry(tempdata, infile, i, j);
            } else {
               // turn off the suppression (of the first measure)
               measuresuppress = !measuresuppress;
            }
            continue;
         }

         if (founddataQ) {
            if ((curvoice == 1) && (!dollarprint) && 
                  infile[i].isInterpretation()) {
               if (pre.search(infile[i][j], "^\\*M\\d+/\\d+")) {
                  dollarprint = 1;
                  insertDollarRecord(infile, i, tempdata, track, 0, 0, tpqtest);
                  tpqtest = -1;
               }
               if (pre.search(infile[i][j], "^\\*MM\\d+")) {
                  dollarprint = 1;
                  insertDollarRecord(infile, i, tempdata, track, 0, 0, tpqtest);
                  tpqtest = -1;
               }
               if (pre.search(infile[i][j], "^\\*clef")) {
                  dollarprint = 1;
                  insertDollarRecord(infile, i, tempdata, track, 0, 0, tpqtest);
                  tpqtest = -1;
               }
               if (pre.search(infile[i][j], "^\\*k\\[.*\\]")) {
                  dollarprint = 1;
                  insertDollarRecord(infile, i, tempdata, track, 0, 0, tpqtest);
                  tpqtest = -1;
               }
            }
         }

         if (!infile[i].isData()) {
            continue;
         }
         founddataQ = 1;
         dollarprint = 0;
      
         // if tpqtest is not -1, then a dollar record with the updated
         // ticks per quarter must be printed now.
         if (tpqtest > 0) {
            MuseRecord dchange;
            char buffer[128] = {0};
            sprintf(buffer, "$ Q:%d", tpqtest);
            dchange.insertString(1, buffer);
            tempdata.append(dchange);
            tpqtest = -1;
         }
 
         int currtick  = getTick(tpq, infile, i);
         if (currtick > tickpos) {
            // a voice which does not start at the beginning of the measure.
            MuseRecord forward;
            forward.setPitch("irst");   // also "irest" can be used
            forward.setTicks(currtick-tickpos);
            tempdata.append(forward);
            tickpos += (currtick - tickpos);
         }

         // print a note in the voice.  Keeping track of the
         // time that the next note is expected, and emit a
         // forward marker if needed.
         if (strcmp(infile[i][j], ".") != 0) {
            tickpos += addNoteToEntry(tempdata, infile, i, j, tpq, voice);
         }
      }

   }

   if (debugQ) { cout << "GOT HERE AAA" << endl; }
   // emit a forward to get to end of measure in current voice
   if (tickpos < stoptick) {
      if (debugQ) { cout << "GOT HERE BBB" << endl; }
      // cout << "FORWARD " << stoptick - tickpos << NEWLINE;
      MuseRecord forward;
      forward.setPitch("irst");   // also "irest" can be used
      forward.setTicks(stoptick-tickpos);
      tempdata.append(forward);
      tickpos += stoptick - tickpos;
   } else if ((!ignoreTickError) && (tickpos > stoptick)) {
      cerr << "Error: duration of music is too large on line " 
           << startline+1 << NEWLINE;
      cerr << "Tickpos = " << tickpos << "\tStoptick = " << stoptick << NEWLINE;
      exit(1);
   }

   if (debugQ) { cout << "GOT HERE CCC" << endl; }
}



//////////////////////////////
//
// addBackup -- insert a backup command.  If the tick size is greater than
//     999, it will have to be split into multiple entries.
//

void addBackup(MuseData& tempdata, int backticks, int tpq) {
   if (backticks <= 0) {
      cerr << "Error with ticks in addBackup: " << backticks << NEWLINE;
   }
   MuseRecord arecord;
   if (backticks < 1000) {
      arecord.setBack(backticks);
      tempdata.append(arecord);
      return;
   }

   int increment = tpq * 4;
   if (increment >= 1000) {
      increment = 999;
   }
   while (backticks > 0) {
      arecord.clear();
      if (backticks <= increment) {
         arecord.setBack(backticks);
         tempdata.append(arecord);
         break;
      }
      backticks -= increment;
      arecord.setBack(increment);
      tempdata.append(arecord);
   }
}



//////////////////////////////
//
// getTick -- return the tick position of the given line of music in
//     the Humdrum score.
//

int getTick(int tpq, HumdrumFile& infile, int line) {
   RationalNumber tique = infile[line].getAbsBeatR();
   tique *= tpq;
   if (tique.getDenominator() != 1) {
      cerr << "Error at line " << line+1 << " in file.\n";
      cerr << "Tick position is not an integer: " << tique << NEWLINE;
      exit(1);
   }

   return tique.getNumerator();
}



//////////////////////////////
//
// getDuration -- return the duration of the note/chord/rest.  If the
//    note is not a grace note (indicated by a q or Q in the record),
//    and the note does not have a duration, then use the default duration
//    which can be set by the --dd option (the default duration if none
//    exists is a quarter note).  This is used to display music with no
//    durations (primarily monophonic, but also polyphonic as long as 
//    all parts have the same rhythmn).
//

RationalNumber getDuration(const char* input, const char* def) {
   Array<char> strang;
   strang.setSize(strlen(input)+1);
   strcpy(strang.getBase(), input);
   PerlRegularExpression pre;
   pre.sar(strang, "\\s.*", "", "");
   if (pre.search(strang, "\\d", "")) {
      // has some sort of numeric value so don't add default
      return Convert::kernToDurationR(strang.getBase());
   }
   if (pre.search(strang, "q", "i")) {
      // is a grace note (0 duration, but let kernToDurationR decide)
      return Convert::kernToDurationR(strang.getBase());
   }
   pre.sar(strang, "$", def);
   return Convert::kernToDurationR(strang.getBase());
}



//////////////////////////////
//
// getDurationNoDots -- 
//

RationalNumber getDurationNoDots(const char* input, const char* def) {
   Array<char> strang;
   strang.setSize(strlen(input)+1);
   strcpy(strang.getBase(), input);
   PerlRegularExpression pre;
   pre.sar(strang, "\\s.*", "", "");


   
   if (pre.search(strang, "(\\d+)%(\\d+)", "")) {
      // handle a rational value in the rhythm
      RationalNumber rn;
      rn = atoi(pre.getSubmatch(2));
      rn /= atoi(pre.getSubmatch(1));
      rn *= 4;  // convert from whole-note to quarter-note units.
      return rn;
   }


   if (pre.search(strang, "\\d", "")) {
      // has some sort of numeric value so don't add default
      return Convert::kernToDurationNoDotsR(strang.getBase());
   }

   if (pre.search(strang, "q", "i")) {
      // is a grace note (0 duration, but let kernToDurationR decide)
      return Convert::kernToDurationNoDotsR(strang.getBase());
   }

   pre.sar(strang, "$", def);
   return Convert::kernToDurationNoDotsR(strang.getBase());

}



/////////////////////////////
//
// addNoteToEntry --  Add a note or a chord to the data. 
//

int addNoteToEntry(MuseData& tempdata, HumdrumFile& infile, int row, int col,
      int tpq, int voice) {
   int& i = row;
   int& j = col;
   MuseRecord arecord;
   MuseRecord psuggestion;
   if (roundQ) {
      arecord.setRoundedBreve();
   }
   RationalNumber rn = getDuration(infile[row][col], defaultDur);
   int  tokencount   = infile[row].getTokenCount(col);
   char buffer[128]  = {0};
   int tickdur = getTickDur(tpq, infile, row, col);
   int k, kk;
   int hidetieQ = 0;
   int hidetie  = 0;

   LayoutParameters lp;
   lp.parseLayout(infile, LayoutInfo[row][col]);

   const char* visual_display = "";
   char visbuffer[128] = {0};

   // if RscaleState[i][j] is not one, then apply a new visual display
   if (RscaleState[i][j] != 1) {
      RationalNumber visrn;
      visrn = rn * RscaleState[i][j];
      Convert::durationRToKernRhythm(visbuffer, visrn);
      visual_display = visbuffer;
   }

   int m;
   for (m=0; m<lp.code.getSize(); m++) {
      if (strcmp(lp.code[m].getBase(), "N") != 0) {
         continue;
      }
      int ind = lp.getParameter(m, "vis");
      if (ind >= 0) {
         visual_display = lp.value[m][ind].getBase();
      }
      if (strcmp(visual_display, "dot") == 0) {
         // vis=dot case handled in print suggestion do not needed here
         // vis=dot means replace the noteheat/stem with a dot.
         visual_display = "";
      }

      // only use the first visual_display setting found, and ignore any other
      break;
   }

   if (mensural2Q) {
      // if the note is tied, and the second note is 1/2 the duration
      // of the first note, then make the tie invisible, and make the
      // second note a dot, only considering the first note listed
      // in a chord.
      char tbuffer[32] = {0};
      infile[row].getToken(tbuffer, col, 0);
      if (strchr(tbuffer, '[') != NULL) {
         // start of tied note: see if the duration of the note is 2/3 of the
         // tied duration.  If so, then hide the tie.  A dot will replace
         // the ending note of tie, so keep the same visual appearance.
         RationalNumber dur;
         RationalNumber tdur;
         dur = Convert::kernToDurationR(tbuffer);
         tdur = infile.getTiedDuration(row, col, 0);
         if (dur * 3 / 2 == tdur) {
            hidetieQ = 1;
            hidetie  = 1;
         }
      } else if (strchr(tbuffer, ']') != NULL) {
         // end of tied note: see if the duration of the note is 1/3 of the
         // tied duration.  If so, then convert the note to a dot.
         RationalNumber dur;
         RationalNumber tdur;
         dur = Convert::kernToDurationR(tbuffer);
         tdur = infile.getTotalTiedDurationR(row, col, 0);
         if (dur * 3  == tdur) {
            visual_display = "";  // turn off any existing visual instruction
            // add faked LO:N:vis=dot layout directive
            lp.code.increase(1);
            lp.key.increase(1);
            lp.value.increase(1);
            lp.code.last().setSize(2);
            strcpy(lp.code.last().getBase(), "N");
            lp.key.last().setSize(1);
            lp.key.last()[0].setSize(strlen("vis")+1);
            strcpy(lp.key.last()[0].getBase(), "vis");
            lp.value.last().setSize(1);
            lp.value.last()[0].setSize(strlen("dot")+1);
            strcpy(lp.value.last()[0].getBase(), "dot");
         }
      }
   }
   
   if (voice == 1) {
      addHairpinStarts(tempdata, infile, row, col);
   }

   // have global layout text codes add above/below the system.
   //if (voice == 1) {
   //   addTextLayout(tempdata, infile, row, col, glp, "TX");
   //}
   addTextLayout(tempdata, infile, row, col, lp, "TX");

   Array<int> chordmapping;
   chordmapping.setSize(tokencount);
   if (tokencount > 1) {
      getChordMapping(chordmapping, infile, row, col);
   } else {
      chordmapping.setAll(0);
   }

   for (kk=0; kk<tokencount; kk++) {
      k = chordmapping[kk];
      infile[row].getToken(buffer, col, k);
      arecord.clear();
      if ((voice > 0) && (voice < 10)) {
         // set the track number
         arecord.getColumn(15) = '0' + voice;
      }
      checkColor(Colorchar, arecord, buffer, Colorout);
      if (tickdur > 0) {
         arecord.setTicks(tickdur);
      } else {
         arecord.setTypeGraceNote();
      }
      if (strchr(buffer, 'r') != NULL) {
         if ((strchr(buffer, ';') != NULL) && (strstr(buffer, ";y") == NULL)) {
            if (voice != 2) {
               arecord.addAdditionalNotation('F');
            } else{
               // put fermata underneath if rest is in the second voice
               arecord.addAdditionalNotation('E');
            }
         }
         if (strstr(buffer, "ry") != NULL) {
            // the rest should not be printed
            // also don't provide a shape for the rest.
            arecord.setPitch("irst");   // also "irest" can be used
         } else {
            if ((rn == MeasureDur[row]) && (RscaleState[i][j] == 1)) {
               // the duration of the rest matches the duration of the measure,
               // so make the rest a centered whole rest.  If the duration
               // is greated than 4 quarter notes, maybe don't center?
               // To make a centered whole note shaped rests, put a space
               // in column 17:
               arecord.getColumn(17) = ' ';
            } else {
               if (strlen(visual_display) > 0) {
                  setNoteheadShape(arecord, visual_display);
               } else {
                  setNoteheadShape(arecord, buffer);
               }
            }
            arecord.setPitch("rest");
         }

         // added 20110815 so that column 20 tuplet info is filled in for 
         // rests:
         addNoteLevelArtic(arecord, infile, row, col, k);

         // added 20110815 so that rests can start/end tuplet brackets
         addChordLevelArtic(tempdata, arecord, psuggestion, 
               infile, row, col, voice);

         tempdata.append(arecord);
         if (kk == 0) {
            // also handles dynamics which are likely to have layout
            // information associated with them, and it is easier
            // to keep track of them in this function:
            handleLayoutInfoChord(tempdata, infile, row, col, lp, voice);
         }

         continue;
      }
      if (strlen(visual_display) > 0) {
         setNoteheadShape(arecord, visual_display);
      } else {
         setNoteheadShape(arecord, buffer);
      }

      if (hasLongQ) {
         // Longa notehead shape
         if (strchr(infile[row][col], LongChar) != NULL) {
            arecord.setNoteheadLong();
            // current usage of the longa will not desire an
            // augmentation dot.  If it is ever needed, then 
            // there should be an option added to suppress
            // the dot in other cases.  Ideally, the addition of
            // an augmentation dot for longs should be an option
            // and the default behavior left as is.
            arecord.getColumn(18) = ' ';
            hideNotesAfterLong(infile, row, col);
         }
      }

      if (strchr(buffer, '/') != NULL) {   // stem up
         arecord.setStemUp();    
      }
      if (strchr(buffer, '\\') != NULL) {   // stem down
         arecord.setStemDown();    
      }

      if (!(hasLongQ && (strchr(infile[row][col], LongChar) != NULL))) {
         if (strchr(buffer, '[') != NULL) {
            // tie start
            if ((strstr(buffer, "yy") != NULL) || (strstr(buffer, "[y") != NULL)) {
               arecord.setTie(1);
            } else {
               if (hidetieQ) {
                  arecord.setTie(hidetie);
               } else {
                  arecord.setTie(!tieQ);
               }
            }
         }
      }
      if (strchr(buffer, '_') != NULL) {
         // tie continuation
         if ((strstr(buffer, "yy") != NULL) || (strstr(buffer, "_y") != NULL)) {
            arecord.setTie(1);
         } else {
            if (hidetieQ) { 
               arecord.setTie(hidetie);
            } else {
               arecord.setTie(!tieQ);
            }
         }
      }

      if (kk == 0) {
         if (beamQ) {
            arecord.setBeamInfo(BeamState[i][j]);
         }
      }

      if (kk == 0) {
         // handle artculations
         addChordLevelArtic(tempdata, arecord, psuggestion, 
               infile, row, col, voice);
      }

      addNoteLevelArtic(arecord, infile, row, col, k);
      arecord.setPitch(Convert::kernToBase40(buffer), kk);
      if (kk == 0) {
         addLyrics(arecord, infile, row, col, TextAssignment);
      }
      tempdata.append(arecord);
      if (!psuggestion.isEmpty()) {
         tempdata.append(psuggestion);
      }
      if (kk == 0) {
         // also handles dynamics which are likely to have layout
         // information associated with them, and it is easier
         // to keep track of them in this function:
         handleLayoutInfoChord(tempdata, infile, row, col, lp, voice);
      }
   }

   if (voice == 1) {
      addHairpinStops(tempdata, infile, row, col);
   }
   
   return tickdur;
}



//////////////////////////////
//
// hidNotesAfterLong -- hides all notes tied after a long note.
//    also hides the barlines of any barlines which are found while removing.
//    currently presumes that each track does not have subspines.
//

void hideNotesAfterLong(HumdrumFile& infile, int row, int col) {
   if (strchr(infile[row][col], '[') == NULL) {
      // no ties on the long, so nothing to hide later.
      return;
   }
   int track = infile[row].getPrimaryTrack(col);
   RationalNumber endtime = infile[row].getAbsBeatR();
   endtime += infile.getTiedDurationR(row,col);
 
   int j;
   int i = row+1;
   char buffer[1024] = {0}; 
   while ((i<infile.getNumLines()) && (endtime >= infile[i].getAbsBeatR())) {
      if (infile[i].isMeasure()) {
         // hide measure
         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (track != infile[i].getPrimaryTrack(j)) {
               continue;
            }
            strcpy(buffer, infile[i][j]);
            strcat(buffer, "yy");
            infile[i].changeField(j, buffer);
         }
      }
      if (!infile[i].isData()) {
         i++;
         continue; 
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (track != infile[i].getPrimaryTrack(j)) {
            continue;
         }

         if (strcmp(infile[i][j], ".") == 0) {
            continue;
         }

         if ((strchr(infile[i][j], ']') != NULL) && 
             (strchr(infile[i][j], '_') != NULL)) {
            // no tie on note for some reason
            continue;
         }
         strcpy(buffer, infile[i][j]);
         strcat(buffer, "yy");
         infile[i].changeField(j, buffer);
         if (strchr(infile[i][j], ']') != NULL) {
            // this is the end of the tied group, so stop
            // making things invisible.
            return;
         }
         break;
      }

      i++;
   }


}



//////////////////////////////
//
// addLyrics --
//

void addLyrics(MuseRecord& arecord, HumdrumFile& infile, int row, int col, 
      Array<Array<int> >& TextAssignment) {
   int track = infile[row].getPrimaryTrack(col);
   int versecount = TextAssignment[track].getSize();
   if (versecount == 0) {
      // no text to print so return
      return;
   }

   // Muse2ps cannot handle more than 6 verses
   // but limiting to 5 since MuseData cannot handle more than 80 columns
   if (versecount > verselimit) {
      versecount = verselimit;
   }

   Array<int> trackcol;
   track2column(trackcol, infile, row);

   Array<Array<char> > verses;
   verses.setSize(versecount);
   int i;
   int texttrack;
   const char* ptr;
   char buffer[1024] = {0};
   for (i=0; i<versecount; i++) {
      verses[i].setSize(1);
      verses[i][0] = '\0';
      ptr = infile[row][trackcol[TextAssignment[track][i]]];
      // if (strcmp(ptr, ".") == 0) {
      //    continue;
      // }
      verses[i].setSize(strlen(ptr)+1);
      strcpy(verses[i].getBase(), ptr);
      texttrack = TextAssignment[track][i];
      if (infile[row].isExInterp(trackcol[texttrack], "**text")) {
         convertHumdrumTextToMuseData(verses[i]);
         convertHtmlTextToMuseData(verses[i]);

         // if the verse starts with a digit, then it will not be
         // printed by default in MuseData muse2ps program.  Adding
         // the string "\+" without quote in front of the number will
         // allow the number to be printed.
         if (isdigit(verses[i].getBase()[0])) {
            strcat(buffer, "\\+");
         }
         strcat(buffer, verses[i].getBase());
         if (i<versecount-1) {
            strcat(buffer, "|");
         }
      } else {
         // treat non **text spines as lyrics:
         // (Don't apply convertHumdrumTextToMuseData)
         convertHtmlTextToMuseData(verses[i]);
         PerlRegularExpression pre3;
         // cannot print backslashes. (don't know why, \\ doesn't work)

         pre3.sar(verses[i], "\\\\", "", "g");
         // cannot print pipes (because it is a verse separator).
         pre3.sar(verses[i], "\\|", "", "g"); 
         pre3.sar(verses[i], "^\\s*$", "\\+", "");
         pre3.sar(verses[i], "^\\.$", "\\+", "");
 
         // if the verse starts with a digit, then it will not be
         // printed by default in MuseData muse2ps program.  Adding
         // the string "\+" without quote in front of the number will
         // allow the number to be printed.
         if (isdigit(verses[i].getBase()[0])) {
            strcat(buffer, "\\+");
         }
         strcat(buffer, verses[i].getBase());
         if (i<versecount-1) {
            strcat(buffer, "|");
         }
      }
   }

   if (strlen(buffer) > 0) {
      arecord.insertString(44, buffer);
   }

}



//////////////////////////////
//
// convertHumdrumTextToMuseData -- convert text in the **text format into
//    text for MuseData lyrics.
//

void convertHumdrumTextToMuseData(Array<char> & text) {
   PerlRegularExpression pre;
   PerlRegularExpression pre2;

   if (pre.sar(text, "^\\s*$", "\\+", "")) {
      // a blank syllable
      return;
   }
 
   pre.sar(text, " \\*", ".", "g"); // convert period      " *" -> "."
   pre.sar(text, " ,",   ",", "g"); // convert comma       " ," -> ","
   pre.sar(text, " \\?", "?", "g"); // convert question    " ?" -> "?"
   pre.sar(text, " !",   "!", "g"); // convert exclamation " !" -> "!"
   pre.sar(text, " :",   ":", "g"); // convert colon       " :" -> ":"
   pre.sar(text, " ;",   ";", "g"); // convert semi-colon  " ;" -> ";"
   pre.sar(text, "~",    "-", "g"); // word hypen          "~"  -> "-"

   pre.sar(text, "^-", "", "");                 // remove staring hyphen marker

   pre.sar(text, "2a", "\\===a===3", "g");     // umlauts
   pre.sar(text, "2e", "\\===e===3", "g");     // umlauts
   pre.sar(text, "2i", "\\===i===3", "g");     // umlauts
   pre.sar(text, "2o", "\\===o===3", "g");     // umlauts
   pre.sar(text, "2u", "\\===u===3", "g");     // umlauts
   pre.sar(text, "2A", "\\===A===3", "g");     // umlauts
   pre.sar(text, "2E", "\\===E===3", "g");     // umlauts
   pre.sar(text, "2I", "\\===I===3", "g");     // umlauts
   pre.sar(text, "2O", "\\===O===3", "g");     // umlauts
   pre.sar(text, "2U", "\\===U===3", "g");     // umlauts
   pre.sar(text, "2y", "\\===y===3", "g");     // umlauts
   pre.sar(text, "2Y", "\\===Y===3", "g");     // umlauts

   pre.sar(text, "7n", "\\===n===1", "g");     // tildes
   pre.sar(text, "7N", "\\===N===1", "g");     // tildes
   pre.sar(text, "7a", "\\===a===1", "g");     // tildes
   pre.sar(text, "7e", "\\===e===1", "g");     // tildes
   pre.sar(text, "7i", "\\===i===1", "g");     // tildes
   pre.sar(text, "7o", "\\===o===1", "g");     // tildes
   pre.sar(text, "7u", "\\===u===1", "g");     // tildes
   pre.sar(text, "7A", "\\===A===1", "g");     // tildes
   pre.sar(text, "7E", "\\===E===1", "g");     // tildes
   pre.sar(text, "7I", "\\===I===1", "g");     // tildes
   pre.sar(text, "7O", "\\===O===1", "g");     // tildes
   pre.sar(text, "7U", "\\===U===1", "g");     // tildes
   pre.sar(text, "7y", "\\===y===1", "g");     // tildes
   pre.sar(text, "7Y", "\\===Y===1", "g");     // tildes

   pre.sar(text, "/a", "\\===a===7", "g");     // acute
   pre.sar(text, "/e", "\\===e===7", "g");     // acute
   pre.sar(text, "/i", "\\===i===7", "g");     // acute
   pre.sar(text, "/o", "\\===o===7", "g");     // acute
   pre.sar(text, "/u", "\\===u===7", "g");     // acute
   pre.sar(text, "/A", "\\===A===7", "g");     // acute
   pre.sar(text, "/E", "\\===E===7", "g");     // acute
   pre.sar(text, "/I", "\\===I===7", "g");     // acute
   pre.sar(text, "/O", "\\===O===7", "g");     // acute
   pre.sar(text, "/U", "\\===U===7", "g");     // acute
   pre.sar(text, "/y", "\\===y===7", "g");     // acute
   pre.sar(text, "/Y", "\\===Y===7", "g");     // acute

   pre.sar(text, "\\\\a", "\\===a===8", "g");   // grave
   pre.sar(text, "\\\\e", "\\===e===8", "g");   // grave
   pre.sar(text, "\\\\i", "\\===i===8", "g");   // grave
   pre.sar(text, "\\\\o", "\\===o===8", "g");   // grave
   pre.sar(text, "\\\\u", "\\===u===8", "g");   // grave
   pre.sar(text, "\\\\A", "\\===A===8", "g");   // grave
   pre.sar(text, "\\\\E", "\\===E===8", "g");   // grave
   pre.sar(text, "\\\\I", "\\===I===8", "g");   // grave
   pre.sar(text, "\\\\O", "\\===O===8", "g");   // grave
   pre.sar(text, "\\\\U", "\\===U===8", "g");   // grave
   pre.sar(text, "\\\\y", "\\===y===8", "g");   // grave
   pre.sar(text, "\\\\Y", "\\===Y===8", "g");   // grave

   pre.sar(text, "\\^a", "\\===a===9", "g");    // circumflex
   pre.sar(text, "\\^e", "\\===e===9", "g");    // circumflex
   pre.sar(text, "\\^i", "\\===i===9", "g");    // circumflex
   pre.sar(text, "\\^o", "\\===o===9", "g");    // circumflex
   pre.sar(text, "\\^u", "\\===u===9", "g");    // circumflex
   pre.sar(text, "\\^A", "\\===A===9", "g");    // circumflex
   pre.sar(text, "\\^E", "\\===E===9", "g");    // circumflex
   pre.sar(text, "\\^I", "\\===I===9", "g");    // circumflex
   pre.sar(text, "\\^O", "\\===O===9", "g");    // circumflex
   pre.sar(text, "\\^U", "\\===U===9", "g");    // circumflex
   pre.sar(text, "\\^y", "\\===y===9", "g");    // circumflex
   pre.sar(text, "\\^Y", "\\===Y===9", "g");    // circumflex

   pre.sar(text, "===", "", "g"); // get rid of buffer characters

   // macron: 1
   // cedilla: 5

   pre.sar(text, "\\s+", "\\0", "g");           // word elision character
 
   pre.sar(text, "\\|", "", "g");  // disable dashes for now.

}

void convertHtmlTextToMuseData(Array<char> & text) {
   PerlRegularExpression pre;
   
   pre.sar(text, "&auml;", "\\a3", "g");   // umlaut
   pre.sar(text, "&euml;", "\\e3", "g");   // umlaut
   pre.sar(text, "&iuml;", "\\i3", "g");   // umlaut
   pre.sar(text, "&ouml;", "\\o3", "g");   // umlaut
   pre.sar(text, "&uuml;", "\\u3", "g");   // umlaut
   pre.sar(text, "&Auml;", "\\A3", "g");   // umlaut
   pre.sar(text, "&Euml;", "\\E3", "g");   // umlaut
   pre.sar(text, "&Iuml;", "\\I3", "g");   // umlaut
   pre.sar(text, "&Ouml;", "\\O3", "g");   // umlaut
   pre.sar(text, "&Uuml;", "\\U3", "g");   // umlaut
   pre.sar(text, "&yuml;", "\\y3", "g");   // umlaut
   pre.sar(text, "&Yuml;", "\\Y3", "g");   // umlaut
   pre.sar(text, "&ntilde;", "\\n1", "g"); // tilde
   pre.sar(text, "&Ntilde;", "\\N1", "g"); // tilde
   pre.sar(text, "&atilde;", "\\a1", "g"); // tilde
   pre.sar(text, "&etilde;", "\\e1", "g"); // tilde
   pre.sar(text, "&itilde;", "\\i1", "g"); // tilde
   pre.sar(text, "&otilde;", "\\o1", "g"); // tilde
   pre.sar(text, "&utilde;", "\\u1", "g"); // tilde
   pre.sar(text, "&Atilde;", "\\A1", "g"); // tilde
   pre.sar(text, "&Etilde;", "\\E1", "g"); // tilde
   pre.sar(text, "&Itilde;", "\\I1", "g"); // tilde
   pre.sar(text, "&Otilde;", "\\O1", "g"); // tilde
   pre.sar(text, "&Utilde;", "\\U1", "g"); // tilde
   pre.sar(text, "&ytilde;", "\\y1", "g"); // tilde
   pre.sar(text, "&Ytilde;", "\\Y1", "g"); // tilde

   pre.sar(text, "&agrave;", "\\a8", "g"); // grave
   pre.sar(text, "&egrave;", "\\e8", "g"); // grave
   pre.sar(text, "&igrave;", "\\i8", "g"); // grave
   pre.sar(text, "&ograve;", "\\o8", "g"); // grave
   pre.sar(text, "&ugrave;", "\\u8", "g"); // grave
   pre.sar(text, "&Agrave;", "\\A8", "g"); // grave
   pre.sar(text, "&Egrave;", "\\E8", "g"); // grave
   pre.sar(text, "&Igrave;", "\\I8", "g"); // grave
   pre.sar(text, "&Ograve;", "\\O8", "g"); // grave
   pre.sar(text, "&Ugrave;", "\\U8", "g"); // grave
   pre.sar(text, "&ygrave;", "\\y8", "g"); // grave
   pre.sar(text, "&Ygrave;", "\\Y8", "g"); // grave

   pre.sar(text, "&aacute;", "\\a7", "g"); // acute
   pre.sar(text, "&eacute;", "\\e7", "g"); // acute
   pre.sar(text, "&iacute;", "\\i7", "g"); // acute
   pre.sar(text, "&oacute;", "\\o7", "g"); // acute
   pre.sar(text, "&uacute;", "\\u7", "g"); // acute
   pre.sar(text, "&Aacute;", "\\A7", "g"); // acute
   pre.sar(text, "&Eacute;", "\\E7", "g"); // acute
   pre.sar(text, "&Iacute;", "\\I7", "g"); // acute
   pre.sar(text, "&Oacute;", "\\O7", "g"); // acute
   pre.sar(text, "&Uacute;", "\\U7", "g"); // acute
   pre.sar(text, "&yacute;", "\\y7", "g"); // acute
   pre.sar(text, "&Yacute;", "\\Y7", "g"); // acute

   pre.sar(text, "&acirc;", "\\a9", "g"); // circumflex
   pre.sar(text, "&ecirc;", "\\e9", "g"); // circumflex
   pre.sar(text, "&icirc;", "\\i9", "g"); // circumflex
   pre.sar(text, "&ocirc;", "\\o9", "g"); // circumflex
   pre.sar(text, "&ucirc;", "\\u9", "g"); // circumflex
   pre.sar(text, "&Acirc;", "\\A9", "g"); // circumflex
   pre.sar(text, "&Ecirc;", "\\E9", "g"); // circumflex
   pre.sar(text, "&Icirc;", "\\I9", "g"); // circumflex
   pre.sar(text, "&Ocirc;", "\\O9", "g"); // circumflex
   pre.sar(text, "&Ucirc;", "\\U9", "g"); // circumflex
   pre.sar(text, "&ycirc;", "\\y9", "g"); // circumflex
   pre.sar(text, "&Ycirc;", "\\Y9", "g"); // circumflex

   pre.sar(text, "&szlig;", "\\s2", "g"); // ss
   pre.sar(text, "&ccedil;", "\\c2", "g"); // c-cedilla
   pre.sar(text, "&oslash;", "\\o2", "g"); // o-slash

   pre.sar(text, "&aring;", "\\a4", "g"); // ring
   pre.sar(text, "&ering;", "\\e4", "g"); // ring
   pre.sar(text, "&iring;", "\\i4", "g"); // ring
   pre.sar(text, "&oring;", "\\o4", "g"); // ring
   pre.sar(text, "&uring;", "\\u4", "g"); // ring
   pre.sar(text, "&Aring;", "\\A4", "g"); // ring
   pre.sar(text, "&Ering;", "\\E4", "g"); // ring
   pre.sar(text, "&Iring;", "\\I4", "g"); // ring
   pre.sar(text, "&Oring;", "\\O4", "g"); // ring
   pre.sar(text, "&Uring;", "\\U4", "g"); // ring

   // 5's are v (hachek) accent (\\s5)
   // 6's?

   pre.sar(text, "&colon;", ":", "g");     // colon (:)
   pre.sar(text, "&eqals;", "=", "g");     // equals sign (=)
   pre.sar(text, "&amp;", "&", "g");       // ampersand (&)

}


//////////////////////////////
//
// track2column --
//

void track2column(Array<int>& trackcol, HumdrumFile& infile, int row) {
   trackcol.setSize(infile.getMaxTracks()+1);
   trackcol.setAll(-1);
   int j, track;
   for (j=0; j<infile[row].getFieldCount(); j++) {
      track = infile[row].getPrimaryTrack(j);
      if (trackcol[track] < 0) {
         // don't process sub-spines other than the first
         trackcol[track] = j; 
       }
   }
}



//////////////////////////////
//
// addTextLayout -- add some free-form text to the music
//

void addTextLayout(MuseData& tempdata, HumdrumFile& infile, int row, int col, 
      LayoutParameters& lp, const char* code) {
   int i;
   for (i=0; i<lp.code.getSize(); i++) {
      if (strcmp(lp.code[i].getBase(), code) != 0) {
         continue;
      }
      addText(tempdata, lp.key[i], lp.value[i]);
      // don't break, add more text if it is found
   }
}



//////////////////////////////
//
// addText -- add text from a layout parameter list.
//
// i = italic
// b = bold
// t = text
//

void addText(MuseData& tempdata, Array<Array<char> >& keys, 
      Array<Array<char> >& values) {

   int i;
   int italicQ = 0;
   int boldQ = 0;
   const char* text = "";

   char justification = 'D';  // left justified
   //justification = 'C';     // center justified
   //justification = 'B';     // right justified

   int dsize = 0;

   for (i=0; i<keys.getSize(); i++) {
      if (strcmp(keys[i].getBase(), "i") == 0) {
         italicQ = 1;
      } else if (strcmp(keys[i].getBase(), "b") == 0) {
         boldQ = 1;
      } else if (strcmp(keys[i].getBase(), "bi") == 0) {
         boldQ = 1;
         italicQ = 1;
      } else if (strcmp(keys[i].getBase(), "ib") == 0) {
         boldQ = 1;
         italicQ = 1;
      } else if (strcmp(keys[i].getBase(), "t") == 0) {
         text = values[i].getBase();
      } else if (strcmp(keys[i].getBase(), "rj") == 0) {
         justification = 'B';
      } else if (strcmp(keys[i].getBase(), "cj") == 0) {
         justification = 'C';
      } else if (strcmp(keys[i].getBase(), "hide") == 0) {
         // don't print text, it is turned off for some reason
         return;  
      } else if (strcmp(keys[i].getBase(), "sz") == 0) {
         if (strcmp(values[i].getBase(), "l") == 0) {
            dsize += 4;
         } else if (strcmp(values[i].getBase(), "large") == 0) {
            dsize += 4;
         } else if (strcmp(values[i].getBase(), "s") == 0) {
            dsize -= 4;
         } else if (strcmp(values[i].getBase(), "small") == 0) {
            dsize -= 4;
         } else if (strcmp(values[i].getBase(), "t") == 0) {
            dsize -= 8;
         } else if (strcmp(values[i].getBase(), "tiny") == 0) {
            dsize -= 8;
         }
      }
   }
   dsize = 0;   // font sizes don't seem to be all available
                // turning off for now.

   if (strlen(text) == 0) {
      // no text to print...
      return;
   }
   Array<char> textstring;
   textstring.setSize(strlen(text)+1);
   strcpy(textstring.getBase(), text);
   convertHtmlTextToMuseData(textstring);

   int fontnumber = 31;   // roman, regular size
   if (boldQ && !italicQ) {
      fontnumber = 32;    // bold, regular size
   } else if (italicQ && !boldQ) {
      fontnumber = 33;    // italic, regular size
   } else if (italicQ && boldQ) {
      // can't do both italic and bold at the same time? Setting to italic
      fontnumber = 33;    // italic & bold, regular size
   }
   fontnumber += dsize;

   int column = 17;
   MuseRecord arecord;
   arecord.appendString("*");
   arecord.getColumn(column) = justification;
   arecord.insertString(25, textstring.getBase());
   tempdata.append(arecord);

   // add font info
   arecord.clear();
   if ((dsize != 0) || italicQ || boldQ) {
      arecord.append("sisi", "P C", column, ":f" , fontnumber);
      tempdata.append(arecord);
   }

   addPositionParameters(tempdata, column, keys, values);
}



//////////////////////////////
//
// addHairpinStarts -- place cresc and decresc hairpins in music.  In theory
//     the dynamic markings can be added to the start/stop of the
//     hairpin, but these are handled in handleLayoutInfoChord.
//

void addHairpinStarts(MuseData& tempdata, HumdrumFile& infile, 
      int row, int col) {


   PerlRegularExpression pre;

   Array<int>& da = DynamicsAssignment;
   int j; 
   int track = infile[row].getPrimaryTrack(col);
   int targettrack = da[track];
   if (targettrack <= 0) {
      // this **kern track does not have a dynamics assignment.
      return;
   }
   int dcol = -1;
   for (j=0; j<infile[row].getFieldCount(); j++) {
      track = infile[row].getPrimaryTrack(j);
      if (track != targettrack) {
         continue;
      }
      dcol = j;
      break;
   }
   if (dcol < 0) {
      // could not find **dynam track for some reason...
      return;
   }
   if (strcmp(infile[row][dcol], ".") == 0) {
      // dynamics column only contains a null token, nothing interesting.
      return;
   }
   if (strcmp(infile[row][dcol], "(") == 0) {
      // crescendo continuation marker, not encoded in MuseData
      return;
   }
   if (strcmp(infile[row][dcol], ")") == 0) {
      // decrescendo continuation marker, not encoded in MuseData
      return;
   }

   // only one dynamic in a **dynam token expected, not checking for more.
   // but don't print any dynamic which has "yy" after it which means
   // the dynamic is invisible.
   if (pre.search(infile[row][dcol], "yy", "")) {
      // ignore entire string if yy is found.  Fix later so that
      // multiple subtokens can be processed, with perhaps one with yy
      // and another without.
      return;
   }
   if (!pre.search(infile[row][dcol], "([<>])(?!yy)", "")) {
      return;
   }

   LayoutParameters lpd;
   lpd.parseLayout(infile, LayoutInfo[row][dcol]);

   // If there is an X appended to the < or >, then it should also not
   // be printed as a hairpin since it is a written word.

   PerlRegularExpression pre2;
   if (pre2.search(infile[row][dcol], "([\\[\\]]).*[<>]X?", "")) {
      // there is a stoping crescendo/decrescendo occuring before
      // the next one start, so stop it before the new one starts.
      if (strcmp(pre2.getSubmatch(1), "[") == 0) {
         addCrescendoStop(tempdata, lpd);
      } else if (strcmp(pre2.getSubmatch(), "]") == 0) {
         addDecrescendoStop(tempdata, lpd);
      }
   }

   if (!pre.search(infile[row][dcol], "([<>]X?)", "")) {
      return;
   }

   // The "X" character after a crescendo or decrescendo start mark
   // indidates that a text version of the symbol should be printed.
   // (note that the crescento or descrescendo stop mark needs to
   // be suppresed in these cases, so also add an "X" to the end 
   // of the cresc/decresc (otherwise muse2ps will currently 
   // quit without producing any output).
   if (strcmp(">", pre.getSubmatch(1)) == 0) {
      addDecrescendoStart(tempdata, lpd);
   } else if (strcmp("<", pre.getSubmatch(1)) == 0) {
      addCrescendoStart(tempdata, lpd);
   } else if (strcmp(">X", pre.getSubmatch(1)) == 0) {
      addCrescText(tempdata, infile, row, col, dcol, lpd, "decresc.");
   } else if (strcmp("<X", pre.getSubmatch(1)) == 0) {
      addCrescText(tempdata, infile, row, col, dcol, lpd, "cresc.");
   }
}



//////////////////////////////
//
// addUnDash -- turn off a dashing which was previously turned on.
//

void addUnDash(MuseData& tempdata, HumdrumFile& infile, int row, int col, 
      int dcol, LayoutParameters& lpd) {
   int track = infile[row].getPrimaryTrack(col);
   if (!DashState[track]) {
      // dashing is not turned on for this part
      return;
   }
   MuseRecord arecord;
   int column = 17;
   arecord.getColumn(1) = '*';
   arecord.getColumn(column) = 'J';   // turns off dashing
   tempdata.append(arecord);

   // turn off the dashing state
   DashState[track] = 0;
}



//////////////////////////////
//
// addCrescText -- place the word "cresc." in the music (instead of a 
//    crescendo symbol).  If the layout code contains the parameter
//    "dash", then add a dash, and keep track of it for later turning
//    off with the paired [X.  The "text" input parameter is the
//    default text to display, unless there is some alternate text 
//    in the "t" layout parameters.
//

void addCrescText(MuseData& tempdata, HumdrumFile& infile, int row, int col, 
      int dcol, LayoutParameters& lp, const char* text) {

   int i;
   int pind;
   for (i=0; i<lp.code.getSize(); i++) {
      if (strcmp("HP", lp.code[i].getBase()) != 0) {
         continue;
      }
      pind  = lp.getParameter(i, "t");
      if (pind >= 0) {
         text = lp.value[i][pind].getBase();
      }
   }

   MuseRecord arecord;
   arecord.getColumn(1) = '*';
   int column = 17;
   // allow all these later:
   // D = left-justified
   // C = centered
   // B = right-justified
   arecord.getColumn(column) = 'D';
   arecord.insertString(25, text);
   tempdata.append(arecord); 
   int dQ = addDashing(tempdata,column+1, infile[row].getPrimaryTrack(col), lp);

   int fontnumber = 33;  // 33 = italic
   arecord.clear();
   arecord.append("sisi", "P C", column, ":f", fontnumber);
   tempdata.append(arecord);

   addPositionInfo(tempdata, column, lp, "TX");
   if (dQ) {
      addPositionInfo(tempdata, column+1, lp, "TX");
   }
}



//////////////////////////////
//
// addDashing --
//

int addDashing(MuseData& tempdata, int column, int track, 
      LayoutParameters& lp) {
   // check for a "DY" code with a "dash" key
   int dashQ = 0;
   int i, j;
   for (i=0; i<lp.code.getSize(); i++) {
      if ((strcmp(lp.code[i].getBase(), "DY") != 0) && 
          (strcmp(lp.code[i].getBase(), "HP") != 0)
            ) {
         continue;
      }
      for (j=0; j<lp.key[i].getSize(); j++) {
         if (strcmp(lp.key[i][j].getBase(), "dash") != 0) {
            continue;
         }
         dashQ = 1;
         break;
      }
      if (dashQ) {
         break;
      }
   }

   if (!dashQ) {
      return 0;
   }

   // add "H" in column of last element in tempdata
   MuseRecord& arecord = tempdata[tempdata.getNumLines()-1];
   if (arecord.getColumn(1) != '*') {
      cerr << "Error: line expected to be a * record: " << NEWLINE;
      cerr << arecord << NEWLINE;
   }
   if (DashState[track]) {
      cerr << "Error dashed line in track " << track 
           << " while another is being started " << NEWLINE;
      exit(1);
   }

   arecord.getColumn(column) = 'H';

   // turn on dashing state which will control turning off later
   DashState[track] = 1;
   return 1;

}



//////////////////////////////
//
// addDecrescendoStart -- Add a decrescendo hairpin start.  Also will
//       process layout information.
//

void addDecrescendoStart(MuseData& tempdata, LayoutParameters& lp) {
   MuseRecord arecord;
   char buffer[32] = {0};
   int  spread = 12;
   int  column = 18;

   arecord.getColumn(1) = '*';
   arecord.getColumn(column) = 'E';
   sprintf(buffer, "%d", spread);
   arecord.insertStringRight(23, buffer);
   tempdata.append(arecord);

   addPositionInfo(tempdata, column, lp, "HP");
}



//////////////////////////////
//
// addPositionInfo --
//

void addPositionInfo(MuseData& tempdata, int column, LayoutParameters lp, 
      const char* code) {

   int i;
   for (i=0; i<lp.code.getSize(); i++) {
      if (strcmp(lp.code[i].getBase(), code) != 0) {
         continue;
      }

      addPositionParameters(tempdata, column, lp.key[i], lp.value[i]);

      // Only process the first directive that matches.
      // Think about multiple ones later?
      break;
   }
}



//////////////////////////////
//
// addPositionParameters  -- final step from AddPostionInfo, or from
// elsewhere if parameter list from layout message has been extracted.
//

void addPositionParameters(MuseData& tempdata, int column, 
      Array<Array<char> >& keys, Array<Array<char> >& values) {

   Array<int> vals;
   Array<int> states;
   getXxYy(vals, states, keys, values);

   int& X = vals[0];    int& x = vals[1];
   int& Y = vals[2];    int& y = vals[3];
   int& XQ = states[0]; int& xQ = states[1];
   int& YQ = states[2]; int& yQ = states[3];

   char buffer[128] = {0};
   char nbuff[32] = {0};

   if (!(xQ || XQ || yQ || YQ)) {
      // no position information so don't do anything
      return;
   }
  
   if (xQ) {
      sprintf(nbuff, "%d", x); 
      strcat(buffer, "x"); 
      strcat(buffer, nbuff);
   } else if (XQ) {
      sprintf(nbuff, "%d", X);
      strcat(buffer, "x");     // X does not seem to work, to remap to x
      strcat(buffer, nbuff);
   }
  
   if (yQ) {
      sprintf(nbuff, "%d", y); strcat(buffer, "y"); strcat(buffer, nbuff);
   } else if (YQ) {
      sprintf(nbuff, "%d", Y); strcat(buffer, "Y"); strcat(buffer, nbuff);
   }

   if (strlen(buffer) == 0) {
      // something strange happend, don't print anything
      return;
   }

   char buffer2[128] = {0};
   sprintf(buffer2, " C%d:%s", column, buffer);
   
   // if there is already a print suggestion line, then the new
   // print suggestion must occur on the same line (appended to end).
   if ((!tempdata.isEmpty()) && (tempdata.last().getColumn(1) == 'P')) {
      MuseRecord& arecord = tempdata.last();
      arecord.appendString(buffer2);
   } else {
      MuseRecord arecord;
      arecord.appendString("P");
      arecord.appendString(buffer2);
      tempdata.append(arecord);
   }
}



//////////////////////////////
//
// addCrescendoStart -- Add a crescendo hairpin start.  Also will
//       process layout information.
//

void addCrescendoStart(MuseData& tempdata, LayoutParameters& lp) {
   MuseRecord arecord;
   char buffer[32] = {0};
   int  spread = 0;
   int  column = 18;

   arecord.getColumn(1) = '*';
   arecord.getColumn(column) = 'E';
   sprintf(buffer, "%d", spread);
   arecord.insertStringRight(23, buffer);
   tempdata.append(arecord);

   addPositionInfo(tempdata, column, lp, "HP");
}



//////////////////////////////
//
// addDecrescendoStop -- Add a descrescendo hairpin stop.  Also will
//       process layout information.
//

void addDecrescendoStop(MuseData& tempdata, LayoutParameters& lp) {
   MuseRecord arecord;
   char buffer[32] = {0};
   int  spread = 0;
   int  column = 17;  // can also be 18, but reserve for optional dynamic

   arecord.getColumn(1) = '*';
   arecord.getColumn(column) = 'F';
   sprintf(buffer, "%d", spread);
   arecord.insertStringRight(23, buffer);
   tempdata.append(arecord);

   // Note that the MuseData printing system ignores Y adjustments
   // to the ends of dyanmics hairpins.
   addPositionInfo(tempdata, column, lp, LO_WEDGE_END);
}



//////////////////////////////
//
// addCrescendoStop -- Add a screscendo hairpin stop.  Also will
//       process layout information.
//

void addCrescendoStop(MuseData& tempdata, LayoutParameters& lp) {
   MuseRecord arecord;
   char buffer[32] = {0};
   int  spread = 12;
   int  column = 17;  // can also be 18, but reserve for optional dynamic

   arecord.getColumn(1) = '*';
   arecord.getColumn(column) = 'F';
   sprintf(buffer, "%d", spread);
   arecord.insertStringRight(23, buffer);
   tempdata.append(arecord);

   // Note that the MuseData printing system ignores Y adjustments
   // to the ends of dyanmics hairpins.
   addPositionInfo(tempdata, column, lp, LO_WEDGE_END);
}



//////////////////////////////
//
// addHairpinStops -- place cresc and decresc hairpins in music.  In theory
//     the dynamic markings can be added to the start/stop of the
//     hairpin, but these are handled in handleLayoutInfoChord.
//

void addHairpinStops(MuseData& tempdata, HumdrumFile& infile, int row, 
      int col) {
   PerlRegularExpression pre;

   Array<int>& da = DynamicsAssignment;
   int j; 
   int track = infile[row].getPrimaryTrack(col);
   int targettrack = da[track];
   if (targettrack <= 0) {
      // this **kern track does not have a dynamics assignment.
      return;
   }
   int dcol = -1;
   for (j=0; j<infile[row].getFieldCount(); j++) {
      track = infile[row].getPrimaryTrack(j);
      if (track != targettrack) {
         continue;
      }
      dcol = j;
      break;
   }
   if (dcol < 0) {
      // could not find **dynam track for some reason...
      return;
   }
   if (strcmp(infile[row][dcol], ".") == 0) {
      // dynamics column only contains a null token, nothing interesting.
      return;
   }
   if (strcmp(infile[row][dcol], "(") == 0) {
      // crescendo continuation marker, not encoded in MuseData
      return;
   }
   if (strcmp(infile[row][dcol], ")") == 0) {
      // decrescendo continuation marker, not encoded in MuseData
      return;
   }

   // only one dynamic in a **dynam token expected, not checking for more.
   // but don't print any dynamic which has "yy" after it which means
   // the dynamic is invisible.
   if (pre.search(infile[row][dcol], "yy", "")) {
      // ignore entire string if yy is found.  Fix later so that
      // multiple subtokens can be processed, with perhaps one with yy
      // and another without.
      return;
   }
   if (!pre.search(infile[row][dcol], "([\\[\\]])(?!yy)", "")) {
      return;
   }

   PerlRegularExpression pre2;
   if (pre2.search(infile[row][dcol], "([\\[\\]]).*[<>]", "")) {
      // the stop is for the previous crescendo, and a new one is
      // starting at this note, so don't handle the stop, since it
      // was handled before the start was printed.  However, there
      // is a possible case which is currently being missed:
      // if there are two stops, and one is after start, then
      // need to print the second stop.  Add that case later...
      return;
   }

   if (!pre.search(infile[row][dcol], "([\\[\\]]X?)", "")) {
      return;
   }

   LayoutParameters lpd;
   lpd.parseLayout(infile, LayoutInfo[row][dcol]);

   // If there is an X appended to the < or >, then it should not
   // be printed as a hairpin since it is a written word.
   if (strcmp("[", pre.getSubmatch(1)) == 0) {
      addCrescendoStop(tempdata, lpd);
   } else if (strcmp("]", pre.getSubmatch(1)) == 0) {
      addDecrescendoStop(tempdata, lpd);
   } else if (strcmp("]X", pre.getSubmatch(1)) == 0) {
      // suppress printing a * record.
      addUnDash(tempdata, infile, row, col, dcol, lpd);
   } else if (strcmp("[X", pre.getSubmatch(1)) == 0) {
      // suppress printing a * record.
      addUnDash(tempdata, infile, row, col, dcol, lpd);
   }
}



//////////////////////////////
//
// getChordMapping -- chords are displayed with the note furthest from
//    the stem displayed first, then the other notes, with the last
//    note in the chord list being the one on the stem side of the chord.
//

void getChordMapping(Array<int>& chordmapping, HumdrumFile& infile, int row, 
      int col) {
  
   Array<int> pitches;
   getPitches(pitches, infile, row, col);

   int stemdir = +1;
   if (strchr(infile[row][col], '\\') != NULL) {
      stemdir = -1;
   }
   // do automatic analysis for whole notes here, but this will involve 
   // knowing the clef which the notes are placed...
   
   Array<Coord> tempp;
   tempp.setSize(pitches.getSize());
   int i;
   for (i=0; i<tempp.getSize(); i++) {
      tempp[i].i = i;
      tempp[i].j = pitches[i];
   }

   if (stemdir > 0) {
      // stemdir is up, so sort notes from low to high
      qsort(tempp.getBase(), tempp.getSize(), sizeof(Coord), numbersort);
   } else {
      // stemdir is down, so sort notes from high to low
      qsort(tempp.getBase(), tempp.getSize(), sizeof(Coord), numberRsort);
   }

   chordmapping.setSize(pitches.getSize());
   for (i=0; i<chordmapping.getSize(); i++) {
      chordmapping[i] = tempp[i].i;
   }

}



//////////////////////////////
//
// numberRsort -- sort items largest first.
//

int numberRsort(const void* A, const void* B) {
   Coord valuea = *((Coord*)A);
   Coord valueb = *((Coord*)B);
   if (valuea.j > valueb.j) {
      return -1;
   } else if (valuea.j < valueb.j) {
      return +1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// numbersort -- sort smallest largest first.
//

int numbersort(const void* A, const void* B) {
   Coord valuea = *((Coord*)A);
   Coord valueb = *((Coord*)B);
   if (valuea.j > valueb.j) {
      return +1;
   } else if (valuea.j < valueb.j) {
      return -1;
   } else {
      return 0;
   }
}



//////////////////////////////
//
// getPitches --
//

void getPitches(Array<int>& pitches, HumdrumFile& infile, int row, int col) {
   int k;
   int tokencount = infile[row].getTokenCount(col);
   pitches.setSize(tokencount);
   pitches.setAll(0);
   char buffer[128] = {0};
   for (k=0; k<tokencount; k++) {
      infile[row].getToken(buffer, col, k, 100);
      pitches[k] = Convert::kernToBase40(buffer);
   }
}



//////////////////////////////
//
// handleLayoutInfoChord --  Generate print suggestions which are related
//     only to the first note in a chord.  The chord note in question is
//     currently the last item in the tempdata.
//

void handleLayoutInfoChord(MuseData& indata, HumdrumFile& infile, int row, 
      int col, LayoutParameters& lp, int voice) {

   PerlRegularExpression pre;
   PerlRegularExpression pre2;
   Array<Array<char> > pfields;
   MuseRecord prec;
   prec.setLine("P");
   char notepsbuffer[128] = {0};
   char tbuffer[128] = {0};
   infile[row].getToken(tbuffer, col, 0);

   int i, j;
   if (lp.code.getSize() != 0 ) {
      for (i=0; i<lp.code.getSize(); i++) {

         if (slurQ && (strcmp(lp.code[i].getBase(), "S") == 0)) {
            // slur layout code. check for "a" or "b"
            for (j=0; j<lp.key[i].getSize(); j++) {
               if (strcmp(lp.key[i][j].getBase(), "a") == 0)  {
                  // add a slur-above print suggestion;
                  insertSlurUpPrintSug(indata, prec);
               } else if (strcmp(lp.key[i][j].getBase(), "b") == 0)  {
                  // add a slur-below print suggestion;
                  insertSlurDownPrintSug(indata, prec);
               }
            }
         }

         else if (strcmp(lp.code[i].getBase(), "SC") == 0)  {
            // staccato layout code (change to articulation?)
            for (j=0; j<lp.key[i].getSize(); j++) {
               if (strcmp(lp.key[i][j].getBase(), "a") == 0)  {
                  // add a staccato-above print suggestion;
                  insertStaccatoSuggestion(indata, prec, +1);
               } else if (strcmp(lp.key[i][j].getBase(), "b") == 0)  {
                  // add a staccato-below print suggestion;
                  insertStaccatoSuggestion(indata, prec, -1);
               }
            }
         }

         else if (strcmp(lp.code[i].getBase(), "N") == 0)  {
            convertKernLONtoMusePS(notepsbuffer, lp.key[i],lp.value[i], tbuffer);
         }
      }
   }

   if (strstr(tbuffer, "yy") != NULL) {
      strcat(notepsbuffer, "p1");   // make note invisible
   }
   if (strlen(notepsbuffer) > 0) {
      prec.append("ss", "  C1:", notepsbuffer);
   }
    
   if ((voice == 1) && dynamicsQ) {
      addDynamics(infile, row, col, indata, prec, DynamicsAssignment, lp);
   }

   if (strlen(prec.getLine()) > 1) {
      // if data was added to the P record, then store it.
      indata.append(prec);
   }

}



//////////////////////////////
//
// addDynamics -- addDynamics to the current note if it has a **dynam
//   dynamic on the current line according to the DynamicsAssignment.
//

void addDynamics(HumdrumFile& infile, int row, int col, MuseData& indata, 
      MuseRecord& prec, Array<int>& DynamicsAssignment, 
      LayoutParameters& lp) {

   PerlRegularExpression pre;
   MuseRecord& last = indata[indata.getNumLines()-1];
   int loc;

   // first handle "z" and "zz" marks in the **kern data.  If the **kern
   // data has a "z", then it maps to "sf" dynamic.  If the **kern data
   // has a "zz", then it maps to sfz.
   // This could should perhaps go outside of the addDynamics function,
   // since the sforzandos will not be printed if there is not a **dynam
   // spine for the part. (use the sfzQ variable for that).
   if (pre.search(infile[row][col], "zz(?!y)")) {
      // produce an sfz dynamic accent
      // don't know how to do that yet, to add sf mark instead:
      loc = last.addAdditionalNotation("Z");
      // handle layout information next.
      processDynamicsLayout(loc, prec, lp);
   } else if (pre.search(infile[row][col], "z(?!y)")) {
      // produce an sf dynamic accent
      loc = last.addAdditionalNotation("Z");
      // handle layout information next.
      processDynamicsLayout(loc, prec, lp);
   }

   Array<int>& da = DynamicsAssignment;
   int j; 
   int track = infile[row].getPrimaryTrack(col);
   int targettrack = da[track];
   if (targettrack <= 0) {
      // this **kern track does not have a dynamics assignment.
      return;
   }
   int dcol = -1;
   for (j=0; j<infile[row].getFieldCount(); j++) {
      track = infile[row].getPrimaryTrack(j);
      if (track != targettrack) {
         continue;
      }
      dcol = j;
      break;
   }
   if (dcol < 0) {
      // could not find **dynam track for some reason...
      return;
   }
   if (strcmp(infile[row][dcol], ".") == 0) {
      // dynamics column only contains a null token, nothing interesting.
      return;
   }
   if (strcmp(infile[row][dcol], "(") == 0) {
      // crescendo continuation marker, not encoded in MuseData
      return;
   }
   if (strcmp(infile[row][dcol], ")") == 0) {
      // decrescendo continuation marker, not encoded in MuseData
      return;
   }

   LayoutParameters lpd; // parameters for **dynam spines
   lpd.parseLayout(infile, LayoutInfo[row][dcol]);

   // only one dynamic in a **dynam token expected, not checking for more.
   // but don't print any dynamic which has "yy" after it which means
   // the dynamic is invisible.
   if (pre.search(infile[row][dcol], "yy", "")) {
      // ignore entire string if yy is found.  Fix later so that
      // multiple subtokens can be processed, with perhaps one with yy
      // and another without.
      return;
   }
   if (!pre.search(infile[row][dcol], "([mpfrsz]+)(?!yy)", "")) {
      return;
   }

   Array<char> value;
   value.setSize(strlen(pre.getSubmatch(1))+1);
   strcpy(value.getBase(), pre.getSubmatch());
   pre.sar(value, "^sf$", "Z", "g");
   // sfz?
   pre.sar(value, "sfp", "Zp", "g");
   pre.sar(value, "rfz", "R", "g");

   // loc stores the column number where the dynamic was stored.
   loc = last.addAdditionalNotation(value.getBase());
   processDynamicsLayout(loc, prec, lpd);
}



/////////////////////////////
//
// processDynamicsLayout --
//

void processDynamicsLayout(int loc, MuseRecord& prec, LayoutParameters& lp) {
   char buffer[128]  = {0};
   int i;
   for (i=0; i<lp.code.getSize(); i++) {
      if (strcmp(lp.code[i].getBase(), "DY") != 0) {
         continue;
      }
      convertKernLODYtoMusePS(buffer, lp.key[i], lp.value[i]);
      if (strlen(buffer) > 0) {
         prec.append("siss", " C", loc, ":", buffer);
      }
      break;
   }
}



//////////////////////////////
//
// getXxYy -- return the X, x, Y, and y MuseData parameters from 
//      the Humdrum Layut paramters, X, x, Y, y, Z, z
//
//      X=1:y=-54:z=56.34
//
//      X = 1
//      y = -54
//      z = 56
//

void getXxYy(Array<int>& vals, Array<int>& states, Array<Array<char> >& keys,
      Array<Array<char> >& values) {

   vals.setSize(4);   vals.setAll(0);
   states.setSize(4); states.setAll(0);

   int& X = vals[0];    int& x = vals[1];
   int& Y = vals[2];    int& y = vals[3];
   int& XQ = states[0]; int& xQ = states[1];
   int& YQ = states[2]; int& yQ = states[3];

   double value;
   int i;

   for (i=0; i<keys.getSize(); i++) {
      if ((strcmp(keys[i].getBase(), "x") == 0) && 
            (strlen(values[i].getBase()) > 0)) {
         value = strtod(values[i].getBase(), NULL);
         x = int(value); 
         xQ = 1;
      } else if ((strcmp(keys[i].getBase(), "y") == 0) && 
            (strlen(values[i].getBase()) > 0)) {
         value = strtod(values[i].getBase(), NULL);
         y = int(value); 
         yQ = 1;
      } else if ((strcmp(keys[i].getBase(), "z") == 0) && 
            (strlen(values[i].getBase()) > 0)) {
         value = strtod(values[i].getBase(), NULL);
         y = -1 * int(value); 
         yQ = 1;
      } else if ((strcmp(keys[i].getBase(), "X") == 0) && 
            (strlen(values[i].getBase()) > 0)) {
         value = strtod(values[i].getBase(), NULL);
         X = int(value); 
         XQ = 1;
      } else if ((strcmp(keys[i].getBase(), "Y") == 0) && 
            (strlen(values[i].getBase()) > 0)) {
         value = strtod(values[i].getBase(), NULL);
         Y = int(value) + 40;
         YQ = 1;
      } else if ((strcmp(keys[i].getBase(), "Z") == 0) && 
            (strlen(values[i].getBase()) > 0)) {
         value = strtod(values[i].getBase(), NULL);
         Y = -int(value);
         YQ = 1;
      }
   }
}



//////////////////////////////
//
// convertKernLODYtoMusePS --
//

void convertKernLODYtoMusePS(char* buffer, Array<Array<char> >& keys,
      Array<Array<char> >& values) {
   if (keys.getSize() == 0) {
      return;
   }
   buffer[0] = '\0';
   char buffer2[128] = {0};

   Array<int> vals;
   Array<int> states;
   getXxYy(vals, states, keys, values);

   int& X  = vals[0];    // maps from layout X
   int& x  = vals[1];    // maps from layout x
   int& Y  = vals[2];    // maps from layout y or -z
   int& y  = vals[3];    // maps from layout Y or Z
   int& XQ = states[0]; int& xQ = states[1];
   int& YQ = states[2]; int& yQ = states[3];

   if (xQ) { sprintf(buffer2, "x%d", x); strcat(buffer, buffer2); }
   if (yQ) { sprintf(buffer2, "y%d", y); strcat(buffer, buffer2); }
   if (XQ) { sprintf(buffer2, "X%d", X); strcat(buffer, buffer2); }
   if (YQ) { sprintf(buffer2, "Y%d", Y); strcat(buffer, buffer2); }
}



//////////////////////////////
//
// convertKernLONtoMusePS -- Note layout suggestion
//

void convertKernLONtoMusePS(char* buffer, Array<Array<char> >& keys,
      Array<Array<char> >& values, const char* token) {
   if (keys.getSize() == 0) {
      return;
   }
   buffer[0] = '\0';
   char buffer2[128] = {0};

   Array<int> vals;
   Array<int> states;
   getXxYy(vals, states, keys, values);

   int& X  = vals[0];    // maps from layout X
   int& x  = vals[1];    // maps from layout x
   int& Y  = vals[2];    // maps from layout y or -z
   int& y  = vals[3];    // maps from layout Y or Z
   int& XQ = states[0]; int& xQ = states[1];
   int& YQ = states[2]; int& yQ = states[3];

   if (xQ) { sprintf(buffer2, "x%d", x); strcat(buffer, buffer2); }
   if (yQ) { sprintf(buffer2, "y%d", y); strcat(buffer, buffer2); }
   if (XQ) { sprintf(buffer2, "X%d", X); strcat(buffer, buffer2); }
   if (YQ) { sprintf(buffer2, "Y%d", Y); strcat(buffer, buffer2); }

   // check for LO:N:vis=dot layout directive:
   int i;

   if (strstr(token, "yy") != NULL) {
      // note should be hidden.  Don't do anything now, it will be
      // processed later.
   } else {
      // check for LO:N:vis=dot
      for (i=0; i<keys.getSize(); i++) {
         if (strcmp(keys[i].getBase(), "vis") != 0) {
            continue;
         }
         if (strcmp(values[i].getBase(), "dot") == 0) {
            strcat(buffer, "p2");
            break;
         }
         if (strcmp(values[i].getBase(), "x") == 0) {
            strcat(buffer, "s1");   // make an x notehead shape
            break;
         }
      }

      int value;
      // check for stem and beam length code
      for (i=0; i<keys.getSize(); i++) {
         if (strcmp(keys[i].getBase(), "smy") == 0) {
            value = atoi(values[i].getBase());
            sprintf(buffer2, " C23:y%d", value); strcat(buffer, buffer2);
         } else if (strcmp(keys[i].getBase(), "smY") == 0) {
            value = atoi(values[i].getBase());
            sprintf(buffer2, " C23:Y%d", value); strcat(buffer, buffer2);
         } else if (strcmp(keys[i].getBase(), "bmy") == 0) {
            value = atoi(values[i].getBase());
            sprintf(buffer2, " C26:y%d", value); strcat(buffer, buffer2);
         } else if (strcmp(keys[i].getBase(), "bmY") == 0) {
            value = atoi(values[i].getBase());
            sprintf(buffer2, " C26:y%d", value); strcat(buffer, buffer2);
         }
      }
  
   }

}



//////////////////////////////
//
// insertStaccatoSuggestion --
//

void insertStaccatoSuggestion(MuseData& data, MuseRecord& prec, 
      int direction) {
   int last = data.getLineCount() - 1;

   // last line should be the note which has the staccato
   int staccatoColumn = data[last].findField('.', 32, 43);
   if (staccatoColumn < 0) {
      return;
   }
   char buffer[32] = {0};
   if (direction < 0) {
      sprintf(buffer, " C%d:b", staccatoColumn);
   } else {
      sprintf(buffer, " C%d:a", staccatoColumn);
   }
   prec.appendString(buffer);
}



//////////////////////////////
//
// insertSlurUpPrintSug --
//

void insertSlurUpPrintSug(MuseData& data, MuseRecord& prec) {
   int last = data.getLineCount() - 1;

   // last line should be the note which has the slur
   int slurcolumn = data[last].getSlurStartColumn();
   if (slurcolumn < 0) {
      return;
   }
   char buffer[32] = {0};
   sprintf(buffer, " C%d:o", slurcolumn);
   prec.appendString(buffer);
}



//////////////////////////////
//
// insertSlurDownPrintSug --
//

void insertSlurDownPrintSug(MuseData& data, MuseRecord& prec) {
   int last = data.getLineCount() - 1;

   // last line should be the note which has the slur
   int slurcolumn = data[last].getSlurStartColumn();
   if (slurcolumn < 0) {
      return;
   }
   char buffer[32] = {0};
   sprintf(buffer, " C%d:u", slurcolumn);
   prec.appendString(buffer);
}



//////////////////////////////
//
// getBase36 --
//

char getBase36(int value) {
   if (value > 35) {
      cerr << "Error cannot value  " << value << NEWLINE;
      exit(1);
   }
   char output;
   if (value < 10) {
      output = '0' + value;
   } else {
      output = 'A' + value - 10;
   }
   return output;
}



//////////////////////////////
//
// addNoteLevelArtic -- might not make senese always in a chord note,
// but print anyway...
// 
//

void addNoteLevelArtic(MuseRecord&  arecord, HumdrumFile& infile, 
      int row, int col, int tnum) {
   const char* token = infile[row][col];

   char buffer[128] = {0};
   infile[row].getToken(buffer, col, tnum, 100);

   // if (TupletState[row][col] != ' ') {
   if (TupletTopNum[row][col] != 0) {
      char value = getBase36(TupletTopNum[row][col]);
      // rests are not processing the next line: 20110815
      arecord.getColumn(20) = value;
      // clear out rest of tuplet field (in case something is there):
      arecord.getColumn(21) = ' ';
      arecord.getColumn(22) = ' ';
   }

   if (hasFictaQ) {
      // musical ficta markings.
      if (strchr(token, FictaChar) != NULL) {
         arecord.addAdditionalNotation('^');
         arecord.addAdditionalNotation('+');  // always force display of acc.
      }
   }

   if (strstr(buffer, "-X") != NULL) {
      // explicitly written out flat (or double flat)
      arecord.addAdditionalNotation('+');
   } else if (strstr(buffer, "#X") != NULL) {
      // explicitly written out sharp (or double sharp)
      arecord.addAdditionalNotation('+');
   } else if (strstr(buffer, "nX") != NULL) {
      // doubly explicit written natural sign
      arecord.addAdditionalNotation('+');
   } else if (strchr(buffer, 'n') != NULL) {
      // written natural sign
      arecord.getColumn(19) = 'n';
   }

   if (strchr(token, ':') != NULL) {
      // arpeggio
      // and arpeggio is handled as a chord-level articulation 
      // (as a dummy grace note), Adding an 'S' to the 
      // additional notations section of the note.  But this 
      // does not actually print an arpeggio in the muse2ps program
      // output, so can't really use.
      // arecord.addAdditionalNotation('S');
   }

}



//////////////////////////////
//
// addChordLevelArtic --
//

void addChordLevelArtic(MuseData& tempdata, MuseRecord&  arecord, 
      MuseRecord& printsuggestion, HumdrumFile& infile, 
      int row, int col, int voice) {
   const char* token = infile[row][col];
   printsuggestion.clear();

   if (tupletQ) {
      if (TupletState[row][col] == '*') {
         int tloc = arecord.addAdditionalNotation('*');  // start of tuplet
         // if the note does not have a beam, then place
         // a bracket.  This will not work in the generalized case
         // where there may be multiple beamed/unbeamed notes under
         // the same tuplet mark.
         //
         // To add a tuplet bracket, place a print suggestion after
         // the note (or is it chord? which will have to be checked later).
         // The print suggestion for  bracket is ':'.  Exmaple
         // P C32::
         RationalNumber rn;
         rn = getDurationNoDots(infile[row][col], "4");
         RationalNumber note8th(1,2);
         if (rn > note8th) {
            printsuggestion.append("sis", "P C", tloc, "::");
         }

      } else if (TupletState[row][col] == '!') {
         arecord.addAdditionalNotation('!');  // end of tuplet
      }
   } else if (vzQ) {
      if (strchr(infile[row][col], 'V') != NULL) {
         int tloc = arecord.addAdditionalNotation('*');  // start of tuplet
         RationalNumber rn;
         rn = getDurationNoDots(infile[row][col], "4");
         RationalNumber note8th(1,2);
         if (rn > note8th) {
            printsuggestion.append("sis", "P C", tloc, "::");
         }
      } else if (strchr(infile[row][col], 'Z') != NULL) {
         arecord.addAdditionalNotation('!');  // end of tuplet
      }
   }

   if (strchr(token, ':') != NULL) {
      // insert an arpeggio
      insertArpeggio(tempdata, infile, row, col);
   }

   if ((strchr(token, ';') != NULL) && (strstr(token, ";y") == NULL)) {
      // F = fermata above staff
      // E = fermata below staff (not considered here).
      if (voice != 2) {
         arecord.addAdditionalNotation('F');
      } else {
         arecord.addAdditionalNotation('E');
      }
   }

   if ((strchr(token, '`') != NULL) || (strstr(token, "\'\'") != NULL)) {
      // staccatissmo/spiccato
      arecord.addAdditionalNotation('i');
   } else if (strchr(token, '\'') != NULL) {
      if (strchr(token, '~') != NULL) {
         // tenuto tenuto
         arecord.addAdditionalNotation('=');
      } else {
         // staccato
         arecord.addAdditionalNotation('.');
      }
   }

   if (strchr(token, '~') != NULL) {
      if (strchr(token, '\'') == NULL) {
         // tenuto
         arecord.addAdditionalNotation('_');
      }
   }

   if (strchr(token, ',') != NULL) {
      // breat hmark (does not work in muse2ps?)
      arecord.addAdditionalNotation(',');
   }

   if (strchr(token, 'o') != NULL) {
      // harmonic
      arecord.addAdditionalNotation('o');
   }

   if (strchr(token, 'v') != NULL) {
      // up-bow (does not work in muse2ps?)
      arecord.addAdditionalNotation('v');
   }

   if (strchr(token, 'u') != NULL) {
      // down-bow
      arecord.addAdditionalNotation('n');
   }

   if (strchr(token, '^') != NULL) {
      // accent
      arecord.addAdditionalNotation('>');
   }

   if (slurQ) {
     
      // have to keep track of slurs when occur between 
      // two separate levels...

      if (strchr(token, ')') != NULL) {
         // slur end, only considering a single level at the moment...
         // place slurs in each layer in a separate slur level:
         switch (voice % 4) {
            case 1: arecord.addAdditionalNotation(')'); break;
            case 2: arecord.addAdditionalNotation(']'); break;
            case 3: arecord.addAdditionalNotation('}'); break;
            case 0: arecord.addAdditionalNotation('x'); break;
        }
      }
      if (strchr(token, '(') != NULL) {
         // slur end, only considering a single level at the moment...
         // place slurs in each layer in a separate slur level:
         switch (voice %4) {
             case 1: arecord.addAdditionalNotation('('); break;
             case 2: arecord.addAdditionalNotation('['); break;
             case 3: arecord.addAdditionalNotation('{'); break;
             case 0: arecord.addAdditionalNotation('z'); break;
         }
      }
   }

   if (strchr(token, 't') != NULL) {
      // trill (half-tone)
      arecord.addAdditionalNotation('t');
   } else if (strchr(token, 'T') != NULL) {
      // trill (whole-tone)
      arecord.addAdditionalNotation('t');
   }

}


//////////////////////////////
//
// insertArpeggio -- and arpeggio mark is a grace note before the
//    note(s) to which is applies.  The pitch of the grace note is arbitrary
//    and is only used to position the arpeggio sign.  If the arpeggio 
//    is to be placed in normal spacing, place the grace note pitch outside
//    stem region of the grace note  (place above highest pitch in chord if
//    the stem is down, or below the lowest pitch if the stem is down). This
//    function currently does not arpeggiate multiple voices (or staves in
//    a grand staff).
//

void insertArpeggio(MuseData& tempdata, HumdrumFile& infile, int row, int col) {
   PerlRegularExpression pre;
   int stemdir = +1;
   int& i = row;
   int& j = col;
   if (pre.search(infile[i][j], "^[^ ]*\\\\", "")) {
      // if the first note listed in the chord contains a down stem, then
      // switch the direction of the stem.
      stemdir = -1;
   }
   Array<int> pitches;
   getPitches(pitches, infile, row, col);
   Array<int> chordmapping;

   // the first chord in chordmapping is furthest from the stem
   // side of the note.
   getChordMapping(chordmapping, infile, row, col);

   if (pitches.getSize() == 0) {
      // can't arpeggiate empty chord, so don't attempt anything
      return;
   }
   if (chordmapping.getSize() == 0) {
      // can't arpeggiate empty chord, so don't attempt anything
      return;
   }

   int firstpitch = pitches[chordmapping[0]];
   int lastpitch  = pitches[chordmapping.last()];

   int firstvpos; // vertical position of the note on the staff, using
                  // diatonic steps from bottom line of staff (E4 in
                  // treble clef).
   int lastvpos;  // vertical postion of note closest  to the stem side
                  // of the chord.

   firstvpos = getScoreStaffVerticalPos(firstpitch, i, j, 
         ClefState, infile);
   lastvpos = getScoreStaffVerticalPos(lastpitch, i, j, 
         ClefState, infile);

   int gracepitch;   // pitch of dummy notes to mark arpeggio
   int posbot;  // staff position of the bottom of the arpeggio
   int postop;  // staff position of the bottom of the arpeggio
   double pbot;
   double ptop;

   if (firstvpos > lastvpos) {
      // chord is stem down
      
      // set grace note pitch above top (first) note in chord.
      gracepitch = firstpitch + 23;

      if (((firstvpos + 1000) % 2) == 0) { // note is on a space
         ptop = (12-firstvpos-2) / 2.0;    // 1 space higher than note
      } else {                             // note is on a line
         ptop = (12-firstvpos-1) / 2.0;    // 1/2 space higher than note
      }

      if (((lastvpos + 1000) % 2) == 0) {  // note is on a space
         pbot = (12-lastvpos+1) / 2.0;     // 1/2 space lower than note
      } else {                             // note is on a line
         pbot = (12-lastvpos+1) / 2.0;     // 1/2 space lower than note
      }
 
      if      (pbot > 0) { posbot = int(pbot+0.5); }
      else if (pbot < 0) { posbot = int(pbot); }
      else               { posbot = 0; }
   
      if      (ptop > 0) { postop = int(ptop+0.5); }
      else if (ptop < 0) { postop = int(ptop); }
      else               { postop = 0; }

   } else {
      // chord is stem up (or is a single note, or multiple
      // notes at the same diatonic pitch level, which may
      // cause problems).
      
      // set grace note pitch below bottom (first) note in chord.
      gracepitch = firstpitch - 23;
      pbot = int(12-firstvpos+1) / 2.0;
      ptop = int(12-lastvpos-1)  / 2.0;

      if (((lastvpos + 1000) % 2) == 0) { // note is on a space
         ptop = (12-lastvpos-2) / 2.0;    // 1 space higher than note
      } else {                             // note is on a line
         ptop = (12-lastvpos-1) / 2.0;    // 1/2 space higher than note
      }

      if (((firstvpos + 1000) % 2) == 0) {  // note is on a space
         pbot = (12-firstvpos+1) / 2.0;     // 1/2 space lower than note
      } else {                             // note is on a line
         pbot = (12-firstvpos+1) / 2.0;     // 1/2 space lower than note
      }

      if      (pbot > 0) { posbot = int(pbot+0.5); }
      else if (pbot < 0) { posbot = int(pbot); }
      else               { posbot = 0; }
   
      if      (ptop > 0) { postop = int(ptop+0.5); }
      else if (ptop < 0) { postop = int(ptop); }
      else               { postop = 0; }
   }

   // MuseData bottom position cannot be greater than two digits
   // including the negative sign.
   if (posbot >= 100) {  posbot = 99; }
   if (postop >= 100) {  postop = 99; }
   if (posbot <= -10) {  posbot = -9; }
   if (postop <= -10) {  postop = -9; }

   // the arpeggio positions are always from the highest to lowest
   // positions, so print postop in column 17/18 and posbot in column 20/21

   // the posbot (actually start of arpeggio) is stored in columns 17/18
   // the postop (actually end  of arpeggio) is stored in columns 20/21
   // if the posbot or postop are only 1 digit (1-9), then placing in
   // column 17/20 works (maybe also column 18/21).

   char buffer[32] = {0};
   
   MuseRecord arecord;
   // column 1: "g" for grace note:
   arecord.getColumn(1) = 'g'; 
   // columns 2-5: pitch (used to space arpeggio, not a real note)
   arecord.insertString(2, Convert::base40ToMuse(gracepitch, buffer));
   // columns 6-7 blank
   // columns 8: "X" which means grace-note is a dummy for arpeggios
   arecord.getColumn(8) = 'X';   
   // columns 9-12 blank
   // columns 13-15: footnote, level, track number
   // columns 16 blank
   // columns 17-21 vertical parameters of arpeggio:
   // column 17-18 are
   sprintf(buffer, "%d", postop);
   arecord.insertString(17, buffer);
   // column 19 grand staff marker (not set by this algorithm)
   // column 20-21: ending level of arpeggio.
   sprintf(buffer, "%d", posbot);
   arecord.insertString(20, buffer);
   // column 22-23 blank
   // column 24 = staff number (space is 1).
   // columns 25 to end: not used
   
   tempdata.append(arecord);
}



//////////////////////////////
//
// getScoreStaffVerticalPos --
// 

int getScoreStaffVerticalPos(int note, int line, int row, 
      Array<Array<Coord> >& clefs, HumdrumFile& infile) {

   const char* clef = "*clefG2";  // default clef if none specified.
   int i = clefs[line][row].i;
   int j = clefs[line][row].j;
   
   if ((i < 0) || (j < 0)) {
      // use the default clef
   } else {
      clef = infile[i][j];
   }

   int sclef = 0;
   if (strncmp(clef, "*clefG", 5) == 0) {
      sclef = 0;
   } else if (strncmp(clef, "*clefF", 5) == 0) {
      sclef = 1;
   } else if (strncmp(clef, "*clefC", 5) == 0) {
      sclef = 2;
   }
   int position = Convert::base40ToScoreVPos(note, sclef);

   if (strstr(clef, "vv") != NULL) {
      // transpose up two octaves for printed pitch
      position += 14;
   } else if (strchr(clef, 'v') != NULL) {
      // transpose up one octave for printed pitch
      position += 7;
   } else if (strstr(clef, "^^") != NULL) {
      // transpose down two octaves for printed pitch
      position += -14;
   } else if (strchr(clef, '^') != NULL) {
      // transpose down one octaves for printed pitch
      position += -7;
   }

   PerlRegularExpression pre;
   // make adjustments for some other more exotice clefs
   if (pre.search(clef, "^\\*clefC.*?(\\d)", "")) {
      if (strcmp(pre.getSubmatch(1), "2") == 0) {        // mezzo-soprano clef
         position += -2;
      } else if (strcmp(pre.getSubmatch(1), "1") == 0) { // soprano clef
         position += -4;
      } else if (strcmp(pre.getSubmatch(1), "4") == 0) { // tenor clef
         position += +2;
      } else if (strcmp(pre.getSubmatch(1), "5") == 0) { // baritone clef
         position += +4;
      }
   } else if (pre.search(clef, "^\\*clefG.*?(\\d)", "")) {
      if (strcmp(pre.getSubmatch(1), "1") == 0) {        // violin treble clef
         position += -2;
      }
   }
   return position;
}



/////////////////////////////
//
// setNoteheadShape --
//

void setNoteheadShape(MuseRecord& arecord, const char* token) {
   int dotcount = 0;
   int i;
   PerlRegularExpression pre;
   if ((!pre.search(token, "\\d", "")) && (!pre.search(token, "q", "i"))) {
      // look at the default duration for a dot.
      token = defaultDur;
   }
   int len = strlen(token);

   for (i=0; i<len; i++) {
      if (token[i] == ' ') {
         break;
      }
      if (token[i] == '.') { // presuming no null token input.
         dotcount++;
      }
   }

   arecord.setDots(dotcount);


   RationalNumber rn;
   rn = getDurationNoDots(token, defaultDur);
   //if (dotcount == 1) {
   //   rn *= 2;
   //   rn /= 3;
   //} else if (dotcount == 2) {
   //   rn *= 4;
   //   rn /= 7;
   //}

   RationalNumber zeroR(0,1);

   if (rn != zeroR) {
      if (!mensuralQ) {
         arecord.setNoteheadShape(rn);
      } else {
         arecord.setNoteheadShapeMensural(rn);
      }
   } else {
      setGracenoteVisualRhythm(arecord, token);
   }

}



//////////////////////////////
//
// setGracenoteVisualRhythm -- set the grace note as a quarter, eighth, 
//     sixteenth, etc. 
//
//     q = slash on note
//     Q = no slash on note (probably a appogiatura)
//

void setGracenoteVisualRhythm(MuseRecord& arecord, const char* token) {
   PerlRegularExpression pre;
   RationalNumber eighth(1,2);
   Array<char> strang;
   strang.setSize(strlen(token) + 1);
   strcpy(strang.getBase(), token);
   pre.sar(strang, "[^0-9]", "", "g");
   RationalNumber rn;
   if (strlen(strang.getBase()) == 0) {
      rn = 1;
      rn /= 2;   // 1/2 = eighth note
   } else {
      rn = Convert::kernToDurationR(strang.getBase());
   }
   if (rn == 0) { 
     rn = 1;
     rn /= 8;   // 1/2 = eighth note
   }
   if ((rn == eighth) && (pre.search(token, "q", ""))) {
      // eighth note shaped grace note with a q should have a slash.
      arecord.getColumn(8) = '0';
      // all 'q' notes should have a slash, but muse2ps will only
      // print on an eighth note.
   } else {
      arecord.setNoteheadShape(rn);
   }
}



//////////////////////////////
//
// checkColor --
//

void checkColor(Array<char>& colorchar, MuseRecord& arecord, 
      const char* token, Array<char>& colorout) {
   int i;
   for (i=0; i<colorchar.getSize(); i++) {
      if (strchr(token, colorchar[i]) != NULL) {
         arecord.getColumn(14) = colorout[i];
         break;
      }
   }
}



///////////////////////////////
//
// getTickDur --
//

int getTickDur(int tpq, HumdrumFile& infile, int row, int col) {
   RationalNumber rn = getDuration(infile[row][col], defaultDur);
   rn *= tpq;

   if (rn.getNumerator() == 0) {
      return 0;
   }

   if (rn.getDenominator() != 1) {
      cerr << "Error in token: " << infile[row][col] << NEWLINE;
      cerr << "Tick value = " << rn << NEWLINE;
      cerr << "Ticks per quarter: " << tpq << NEWLINE;
      cerr << "Original rhythmic duration: " << (rn / tpq) << NEWLINE;
      exit(1);
   }

   return rn.getNumerator();
}



//////////////////////////////
//
// addMeasureEntry --  print a measure, and print suggestions immediately
//    afterwards.
//

void addMeasureEntry(MuseData& tempdata, HumdrumFile& infile, int line, 
      int col) {

   MuseRecord arecord;
   char buffer[128] = {0};
   strcpy(buffer, "measure");
   PerlRegularExpression pre;
   int measurenum = -1;

   if (pre.search(infile[line][col], "-", "")) {
      strcpy(buffer, "msilent");
   } else if (pre.search(infile[line][col], "\\.", "")) {
      strcpy(buffer, "mdotted");
   } else if (pre.search(infile[line][col], "^==", "")) {
      strcpy(buffer, "mheavy2");
   } 

   if (pre.search(infile[line][col], "^=.*?(\\d+)", "")) {
      measurenum = atoi(pre.getSubmatch(1));
      if (measurenum >= 0) {
        strcat(buffer, " ");
        strcat(buffer, pre.getSubmatch());
      }
   } 

   arecord.insertString(1, buffer);

   // #define MDOUBLE "measure"
   #define MDOUBLE "mdouble"
   // process measure style
   if (pre.search(infile[line][col], "yy")) {
      arecord.insertString(1, "msilent");
   } else if (pre.search(infile[line][col], ":\\|!\\|:", "")) {
      arecord.addMeasureFlag(":||:");
      kflag |= k_lrbar;
      arecord.insertString(1, "mheavy4");
   } else if (pre.search(infile[line][col], ":!!:", "")) {
      arecord.addMeasureFlag(":||:");
      arecord.insertString(1, "mheavy4");
   } else if (pre.search(infile[line][col], ":\\|\\|:", "")) {
      arecord.addMeasureFlag(":||:");
      arecord.insertString(1, MDOUBLE);
   } else if (pre.search(infile[line][col], ":\\|!", "")) {
      arecord.addMeasureFlag(":|");
      arecord.insertString(1, "mheavy2");
   } else if (pre.search(infile[line][col], ":\\|\\|", "")) {
      arecord.addMeasureFlag(":|");
      arecord.insertString(1, MDOUBLE);
   } else if (pre.search(infile[line][col], ":\\|", "")) {
      arecord.addMeasureFlag(":|");
   } else if (pre.search(infile[line][col], "!\\|:", "")) {
      arecord.addMeasureFlag("|:");
      arecord.insertString(1, "mheavy3");
   } else if (pre.search(infile[line][col], "\\|\\|:", "")) {
      arecord.insertString(1, MDOUBLE);
      arecord.addMeasureFlag("|:");
   } else if (pre.search(infile[line][col], "\\|\\|", "")) {
      arecord.insertString(1, MDOUBLE);
   } else if (pre.search(infile[line][col], "\\|:", "")) {
      arecord.addMeasureFlag("|:");
   }
   LayoutParameters lp;
   LayoutParameters glp; // global layout parameters
   if (col < LayoutInfo[line].getSize()) {
      lp.parseLayout(infile, LayoutInfo[line][col]);
   }
   glp.parseLayout(infile, GlobalLayoutInfo[line]);

   // print any text associated with the barline
   addTextLayout(tempdata, infile, line, col, lp, "TX");

   tempdata.append(arecord);
   processMeasureLayout(tempdata, infile, line, col, lp, glp);

}


//////////////////////////////
//
// processMeasureLayout -- The last line of tempdata should contain the 
//    barline.
//

void processMeasureLayout(MuseData& tempdata, HumdrumFile& infile, int line, 
      int col, LayoutParameters& lp, LayoutParameters& glp) {

   int i;
   for (i=0; i<lp.code.getSize(); i++) {
      handleMeasureLayoutParam(tempdata, infile, lp, i);
   }

   // search the global layout parameters for items (particularly
   // line/system breaks).
   for (i=0; i<glp.code.getSize(); i++) {
      handleMeasureLayoutParam(tempdata, infile, glp, i);
   }


   // search for segno sign parameter (only searching global parameters)
   for (i=0; i<glp.code.getSize(); i++) {
      if (strcmp(glp.code[i].getBase(), "SEGNO") == 0) {
         // currently LO:SEGNO has no parameters, so don't check
         MuseRecord segnoline;
         segnoline.getColumn(1) = '*';
         segnoline.getColumn(17) = 'A';   // segno musical directive
         segnoline.getColumn(19) = '+';   // above the staff
         // don't know what will happen if a linebreak and a segno
         // happen on the same barline...
         tempdata.append(segnoline);
      }
   }

}



//////////////////////////////
//
// handleMeasureLayoutParam -- do final generation of print suggestions
//     for Measures (this function called by processMeasureLayout).
//

void handleMeasureLayoutParam(MuseData& tempdata, HumdrumFile& infile,
      LayoutParameters& lp, int index) {

   int linebreakQ = 0;
   int pagebreakQ = 0;

   if (strcmp(lp.code[index].getBase(), "LB") == 0) {
      linebreakQ = 1;
   } else if (strcmp(lp.code[index].getBase(), "PB") == 0) {
      pagebreakQ = 1;
      linebreakQ = 1;
   }

   // only processing line (system) and page breaks
   // can only handle line breaks at the moment.
   if (!(linebreakQ || pagebreakQ)) {
      return;
   }

   PerlRegularExpression pre;
   int index2 = lp.getParameter(index, "g");
   if (index2 < 0) {
      // cannot find a group parameter, and not allowing a global
      // grouping (at least at the moment).
      return;
   }

   Array<Array<char> > tokens;
   pre.getTokens(tokens, "\\s,\\s", lp.value[index][index2].getBase());

   // true if a part instruction
   int partQ = pre.search(lp.value[index][index2], "\\bP\\b", "");

   // currently only printing z groups which are used to control
   // the directions for a particular music size.
   MuseRecord arecord;
   int i;
   int musicsize=6;
   for (i=0; i<tokens.getSize(); i++) {
      if (!pre.search(tokens[i].getBase(), "^z(\\d*)", "")) {
         continue;
      }
      if (strlen(pre.getSubmatch(1)) > 0) {
         // for a specific size
         musicsize = atoi(pre.getSubmatch(1));
         arecord.clear();
         arecord.appendString("P");
         if (partQ) {
            arecord.appendString("p");
         } else {
            arecord.appendString("a");
         }
         arecord.append("sis", "#", musicsize, " C1:]");
         // have to check for multiple print suggestions and combine them.
         tempdata.append(arecord);
      } else {
         // for any MuseData size
         musicsize = atoi(pre.getSubmatch(1));
         arecord.clear();
         arecord.appendString("P");
         if (partQ) {
            arecord.appendString("p");
         } else {
            arecord.appendString("a");
         }
         arecord.appendString(" C1:]");
         // have to check for multiple print suggestions and combine them.
         tempdata.append(arecord);
      }
      break;
   }
}



//////////////////////////////
//
// getMaxVoices -- the maximum number of spines with the same primary track
//    found on the line.
//

int getMaxVoices(HumdrumFile& infile, int startline, int stopline, int track) {
   int tcount;
   int i, j;
   int output = 0;
   for (i=startline; i<stopline; i++) {
      if (!infile[i].isData()) {
         continue;
      }
      tcount = 0;
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (infile[i].getPrimaryTrack(j) == track) {
            tcount++;
         }      
      }
      if (output < tcount) {
         output = tcount;
      }
   }
   return output;
}



//////////////////////////////
//
// getMeasureInfo -- returns a list of the measures in the score listed
//     as line indexes in the Humdurm score.
//

void getMeasureInfo(Array<int>& measures, HumdrumFile& infile) {
   measures.setSize(infile.getNumLines());
   measures.setSize(0);
   int i;
   int datafound = 0;
   int exinterpline = -1;
   for (i=0; i<infile.getNumLines(); i++) {
      if ((exinterpline < 0) && (strncmp(infile[i][0], "**", 2) == 0)) {
         exinterpline = i;
         continue;
      }
      if (infile[i].isData()) {
         datafound = 1;
         continue;
      }
      if (infile[i].isMeasure()) {
         if (measures.getSize() == 0) {
            if (datafound) {
               measures.append(exinterpline);
            }
         }
         measures.append(i);
      }
   }


   // determine if there is data after the last measure
   // in the file.

   datafound = 0;
   int lastline = -1;
   for (i=infile.getNumLines()-1; i>0; i--) {
      if (strcmp(infile[i][0], "*-") == 0) {
         lastline = i;
         continue;
      }
      if (infile[i].isData()) {
         datafound = 1;
         continue;
      }
      if (infile[i].isMeasure()) {
         if (datafound) {
            measures.append(lastline);
         }
      }
   }

   // remove duplicate endings
   for (i=measures.getSize()-1; i>=1; i--) {
      if (measures[i] == measures[i-1]) {
         measures.setSize(measures.getSize()-1);
      } else {
         break;
      }
   }

   if (debugQ) {
      cout << "MEASURE LINES:\n";
      for (i=0; i<measures.getSize(); i++) {
         cout << "\t" << measures[i] << NEWLINE;
      }
   }
}



//////////////////////////////
//
// getKernTracks --  Return a list of the **kern primary tracks found
//     in the Humdrum data.  Currently all tracks are independent parts.
//     No grand staff parts are considered if the staves are separated 
//     into two separate spines.
//
//

void getKernTracks(Array<int>& tracks, HumdrumFile& infile) {
   tracks.setSize(infile.getMaxTracks());
   tracks.setSize(0);
   int i;
   for (i=1; i<=infile.getMaxTracks(); i++) {
      if (strcmp(infile.getTrackExInterp(i), "**kern") == 0) {
         tracks.append(i);
      }
   }

   if (debugQ) {
      cout << "\t**kern tracks:\n";
      for (i=0; i<tracks.getSize(); i++) {
         cout << "\t" << tracks[i] << NEWLINE; 
      }
   }
}



//////////////////////////////
//
// getTupletState -- mark basic start/stop of tuplets.  Not perfect,
// and does not segment tuplets into smaller tuplet groups in a generalized
// manner
//

void getTupletState(Array<int>& hasTuplet, Array<Array<char> >& TupletState, 
      Array<Array<int> >& TupletTopNum, 
      Array<Array<int> >& TupletBotNum, HumdrumFile& infile) {
   char tupletstart = '*';
   // char tupletstop = '!';
   Array<Array<int> > tstate;
   Array<Array<int> > lastrow;
   Array<Array<int> > lastcol;

   hasTuplet.setSize(infile.getMaxTracks()+1);
   hasTuplet.setAll(0);

   int i, j, ii, lasti, lastj;
   tstate.setSize(infile.getMaxTracks()+1);
   lastrow.setSize(infile.getMaxTracks()+1);
   lastcol.setSize(infile.getMaxTracks()+1);
   for (i=0; i<tstate.getSize(); i++) {
      tstate[i].setSize(100);
      tstate[i].setAll(-1);
      lastrow[i].setSize(100);
      lastrow[i].setAll(-1);
      lastcol[i].setSize(100);
      lastcol[i].setAll(-1);
   }

   TupletState.setSize(infile.getNumLines());
   TupletTopNum.setSize(infile.getNumLines());
   TupletBotNum.setSize(infile.getNumLines());
   Array<int> curlayer;
   curlayer.setSize(infile.getMaxTracks() + 1);
   int layer;
   int track;
   RationalNumber rn;
   
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isMeasure()) {
         // don't allow tuplets across barlines.  
         for (ii=0; ii<tstate.getSize(); ii++) {
            layer = 1;
            lasti = lastrow[ii][layer];
            lastj = lastcol[ii][layer];
            if ((tstate[ii][layer] != ' ') && (lasti >= 0) && (lastj >= 0)) {
               TupletState[lasti][lastj] = '!';
            }
            tstate[ii].setAll(' ');
         }
         continue;
      }
      if (!infile[i].isData()) {
         continue;
      }

      TupletState[i].setSize(infile[i].getFieldCount());
      TupletState[i].setAll(0);

      TupletTopNum[i].setSize(infile[i].getFieldCount());
      TupletBotNum[i].setSize(infile[i].getFieldCount());

      TupletTopNum[i].setAll(0);
      TupletBotNum[i].setAll(0);

      curlayer.setAll(0);

      for (j=0; j<infile[i].getFieldCount(); j++) {
         track = infile[i].getPrimaryTrack(j);
         curlayer[track]++;
         layer = curlayer[track];
         if (!infile[i].isExInterp(j, "**kern")) {
            continue;
         }
         if (strcmp(infile[i][j], ".") == 0) {
            continue;
         }
         rn = getDurationNoDots(infile[i][j], defaultDur);
         if (!isPowerOfTwo(rn)) {
            hasTuplet[track] = 1;
            // The note is a tuplet
            if (tstate[track][layer] == 't') {
               TupletState[i][j] = ' ';
            } else {
               TupletState[i][j] = tupletstart;
            }
            TupletTopNum[i][j] = getTupletTop(infile, i, j);
            // TupletBotNum always set to 0 for now.
            // to calculate the TupletBotNum, need to know beaming.
            tstate[track][layer] = 't';
         } else {
            // The note is a not a tuplet
            lasti = lastrow[track][layer];
            lastj = lastcol[track][layer];
            if ((tstate[track][layer] != ' ') && (lasti >= 0) && (lastj >= 0)) {
               TupletState[lasti][lastj] = '!';
            }
            tstate[track][layer] = ' ';
         }
         lastrow[track][layer] = i;
         lastcol[track][layer] = j;
      }
   }
}



//////////////////////////////
//
// getTupletTop --
//

int getTupletTop(HumdrumFile& infile, int row, int col) {
   RationalNumber rn;
   rn = getDurationNoDots(infile[row][col], defaultDur);
   // basic algorithm when works for most simple tuplets:
   // extract all prime factors which are not 2.
   

   if (rn.getNumerator() != 1) {
      if (rn.isEqualTo(16,3)) {
         // triplet breve note
         return 3;
      }
      if (rn.isEqualTo(8,3)) {
         // triplet whole note
         return 3;
      }
      if (rn.isEqualTo(4,3)) {
         // triplet half note
         return 3;
      }
      if (rn.isEqualTo(2,3)) {
         // triplet quarter note
         return 3;
      }
      cerr << "Error: Cannot handle exotic tuplets " << rn << NEWLINE;
      exit(1);
   }

   Array<int> factors;
   primeFactorization(factors, rn.getDenominator());
   int product = 1;
   int i;
   for (i=factors.getSize()-1; i>=0; i--) {
      if (factors[i] == 2) {
         break;
      }
      product *= factors[i];
   }

   return product;
}



//////////////////////////////
//
// getBeamState -- Analyze structure of beams and store note layout
//      directives at the same time.
//
// Type          Humdrum     MuseData
// start          L           [
// continue                   =
// end            J           ]
// forward hook   K           /
// backward hook  k           \   x
//
// Column 26-31 are for beams
//    26   is for 8th-note level
//    27   is for 16th-note level
//    28   is for 32th-note level
//    29   is for 64th-note level
//    30   is for 128th-note level
//    31   is for 256th-note level
//
// LayoutInfo and ClefState are also filled in this function since
// it is a convenient place to do so.  Clefstate is currently set
// by track rather than subtrack, so clef changes in subtokens which
// differ will cause problems.
//
// The GlobalLayout (glayout) is for layout information stored in global
// comments.  These get stored at the next line in the file which contains 
// data or measures.
//

void getBeamState(Array<Array<Array<char> > >& beams, 
    Array<Array<Array<Coord> > >& layout,  Array<Array<Coord> >& glayout,
    Array<Array<Coord> >& clefs, HumdrumFile& infile) {
   int i, j, t;
   int ii;
   int len;
   int contin;
   int start;
   int stop;
   int flagr;
   int flagl;
   int track;
   int layoutisinitQ = 0;
   RationalNumber rn;

   Array<Coord> clefstate;
   Array<Array<Array<Coord> > > laystate;  // state of layout info

   Array<Coord> glaystate; // global layout temporary storage

   Array<Array<int> > beamstate;   // state of beams in tracks/layers
   Array<Array<int> > gracestate;  // independents state for grace notes

   Array<char> gbinfo;
   gbinfo.setSize(100);
   gbinfo.allowGrowth(0);

   beamstate.setSize(infile.getMaxTracks() + 1);
   gracestate.setSize(infile.getMaxTracks() + 1);
   laystate.setSize(infile.getMaxTracks() + 1);
   clefstate.setSize(infile.getMaxTracks() + 1);
   beamstate.allowGrowth(0);
   gracestate.allowGrowth(0);
   laystate.allowGrowth(0);
   clefstate.allowGrowth(0);
   for (i=0; i<beamstate.getSize(); i++) {
      clefstate[i].i = -1;  // should already by -1, but just in case
      clefstate[i].j = -1;  // should already by -1, but just in case
      beamstate[i].setSize(100);     // maximum of 100 layers in each track...
      gracestate[i].setSize(100);    // maximum of 100 layers in each track...
      beamstate[i].allowGrowth(0);
      gracestate[i].allowGrowth(0);
      beamstate[i].setAll(0);
      gracestate[i].setAll(0);
   }
   glaystate.setSize(0);

   beams.setSize(infile.getNumLines());
   layout.setSize(infile.getNumLines());
   glayout.setSize(infile.getNumLines());
   clefs.setSize(infile.getNumLines());
   Array<int> curlayer;
   curlayer.setSize(infile.getMaxTracks() + 1);
   Array<int> laycounter;

   for (i=0; i<infile.getNumLines(); i++) {
      layout[i].setSize(infile[i].getFieldCount());

      if (infile[i].isMeasure()) {
         // don't allow beams across barlines.  Mostly for 
         // preventing buggy beams from propagating...
         for (t=1; t<=infile.getMaxTracks(); t++) {
            beamstate[t].setAll(0);
            gracestate[t].setAll(0);
         }
         // but do allow for measures to posses layout information
         storeLayoutInformation(infile, i, laystate, layout, layoutisinitQ++);
      }

      if (infile[i].isGlobalComment()) {
         addGlobalLayoutInfo(infile, i, glaystate);
      }

      if (infile[i].isLocalComment()) {
         addLayoutInfo(infile, i, laystate);
      }

      if (infile[i].isInterpretation()) {
         adjustClefState(infile, i, clefstate);
      }

      if (!infile[i].isData() && !infile[i].isMeasure()) {
         continue;
      }

      // store any layout information collected since the last note/measure
      storeLayoutInformation(infile, i, laystate, layout, layoutisinitQ++);
      storeGlobalLayoutInfo(infile, i, glaystate, glayout);

      if (!infile[i].isData()) {
         continue;
      }

      storeClefInformation(infile, i, clefstate, clefs);

      beams[i].setSize(infile[i].getFieldCount());
      for (j=0; j<beams[i].getSize(); j++) {
         beams[i][j].setSize(1);
         beams[i][j][0] = '\0';
      }

      curlayer.setAll(0);
      for (j=0; j<infile[i].getFieldCount(); j++) {
         track = infile[i].getPrimaryTrack(j);
         curlayer[track]++;
         if (strcmp(infile[i][j], ".") == 0) {
            // ignore null tokens
            continue;
         }         
         if (strchr(infile[i][j], 'r') != NULL) {
            // ignore rests.  Might be useful to not ignore
            // rests if beams extent over rests...
            continue;
         }         
         rn = Convert::kernToDurationR(infile[i][j]);
         if (rn >= 1) {
            beamstate[track][curlayer[track]] = 0;
            continue;
         }
         if (rn == 0) {

            // grace notes;
            countBeamStuff(infile[i][j], start, stop, flagr, flagl);
            if ((start != 0) && (stop != 0)) {
               cerr << "Funny error in grace note beam calculation" << NEWLINE;
               exit(1);
            }
            if (start > 7) {
               cerr << "Too many beam starts" << NEWLINE;
            }
            if (stop > 7) {
               cerr << "Too many beam ends" << NEWLINE;
            }
            if (flagr > 7) {
               cerr << "Too many beam flagright" << NEWLINE;
            }
            if (flagl > 7) {
               cerr << "Too many beam flagleft" << NEWLINE;
            }
            contin = gracestate[track][curlayer[track]];
            contin -= stop;
            gbinfo.setAll(0);
            for (ii=0; ii<contin; ii++) { 
               gbinfo[ii] = '=';
            }
            if (start > 0) {
               for (ii=0; ii<start; ii++) {
                  strcat(gbinfo.getBase(), "[");
               }
            } else if (stop > 0) {
               for (ii=0; ii<stop; ii++) {
                  strcat(gbinfo.getBase(), "]");
               }
            }
            for (ii=0; ii<flagr; ii++) {
               strcat(gbinfo.getBase(), "/");
            }
            for (ii=0; ii<flagl; ii++) {
               strcat(gbinfo.getBase(), "\\");
            }
            len = strlen(gbinfo.getBase());
            if (len > 6) {
               cerr << "Error too many grace note beams" << NEWLINE;
               exit(1);
            }
            beams[i][j].setSize(len+1);
            strcpy(beams[i][j].getBase(), gbinfo.getBase());
            gracestate[track][curlayer[track]] = contin;
            gracestate[track][curlayer[track]] += start;

         } else {
            // regular notes which are shorter than a quarter note
            // (including tuplet quarter notes which should be removed):

            countBeamStuff(infile[i][j], start, stop, flagr, flagl);
            if ((start != 0) && (stop != 0)) {
               cerr << "Funny error in note beam calculation" << NEWLINE;
               exit(1);
            }
            if (start > 7) {
               cerr << "Too many beam starts" << NEWLINE;
            }
            if (stop > 7) {
               cerr << "Too many beam ends" << NEWLINE;
            }
            if (flagr > 7) {
               cerr << "Too many beam flagright" << NEWLINE;
            }
            if (flagl > 7) {
               cerr << "Too many beam flagleft" << NEWLINE;
            }
            contin = beamstate[track][curlayer[track]];
            contin -= stop;
            gbinfo.setAll(0);
            for (ii=0; ii<contin; ii++) { 
               gbinfo[ii] = '=';
            }
            if (start > 0) {
               for (ii=0; ii<start; ii++) {
                  strcat(gbinfo.getBase(), "[");
               }
            } else if (stop > 0) {
               for (ii=0; ii<stop; ii++) {
                  strcat(gbinfo.getBase(), "]");
               }
            }
            for (ii=0; ii<flagr; ii++) {
               strcat(gbinfo.getBase(), "/");
            }
            for (ii=0; ii<flagl; ii++) {
               strcat(gbinfo.getBase(), "\\");
            }
            len = strlen(gbinfo.getBase());
            if (len > 6) {
               cerr << "Error too many grace note beams" << NEWLINE;
               exit(1);
            }
            beams[i][j].setSize(len+1);
            strcpy(beams[i][j].getBase(), gbinfo.getBase());
            beamstate[track][curlayer[track]] = contin;
            beamstate[track][curlayer[track]] += start;

         }
      }
   }
}



//////////////////////////////
//
// addGlobalLayoutInfo --
//

void addGlobalLayoutInfo(HumdrumFile& infile, int line, 
      Array<Coord>& glaystate) {
   if (strncmp(infile[line][0], "!!LO:", 5 != 0)) {
      return;
   }
   glaystate.setSize(glaystate.getSize()+1);
   glaystate.last().i = line;
   glaystate.last().j = 0;
}



//////////////////////////////
//
// storeGlobalLayoutInfo --
//

void storeGlobalLayoutInfo(HumdrumFile& infile, int line, 
   Array<Coord>& glaystate, Array<Array<Coord> >& glayout) {
   int i;
   glayout[line].setSize(glaystate.getSize());
   for (i=0; i<glaystate.getSize(); i++) {
      glayout[line][i].i = glaystate[i].i;
      glayout[line][i].j = glaystate[i].j;
   }
   glaystate.setSize(0);
}



//////////////////////////////
//
// storeClefInformation --
//

void storeClefInformation(HumdrumFile& infile, int line, 
      Array<Coord>& clefstate, Array<Array<Coord> >& clefs) {

   int j;
   int track;
   clefs[line].setSize(infile[line].getFieldCount());
   for (j=0; j<infile[line].getFieldCount(); j++) {
      track = infile[line].getPrimaryTrack(j);
      clefs[line][j].i = clefstate[track].i;
      clefs[line][j].j = clefstate[track].j;
   }
}



//////////////////////////////
//
// adjustClefState --
//

void adjustClefState(HumdrumFile& infile, int line, 
      Array<Coord>& clefstate) {
   int j;
   int track;
   for (j=0; j<infile[line].getFieldCount(); j++) {
      if (strncmp(infile[line][j], "*clef", 5) == 0) {
         track = infile[line].getPrimaryTrack(j);
         clefstate[track].i = line;
         clefstate[track].j = j;
      }
   }
}



//////////////////////////////
//
// addLayoutInfo -- accumulate layot info for a note, rest or measure token
//    into the current layout buffer.  storeLayoutInfo() function actually 
//    stores it in the final data structure.
//

void addLayoutInfo(HumdrumFile& infile, int line, 
      Array<Array<Array<Coord> > > &laystate) {

   int track;
   int layer;
   Array<int> layerinfo;
   layerinfo.setSize(infile.getMaxTracks()+1);
   layerinfo.setAll(0);
   Coord xy;

   int j;
   for (j=0; j<infile[line].getFieldCount(); j++) {
      track = infile[line].getPrimaryTrack(j);
      layerinfo[track]++;
      layer = layerinfo[track];
      if (strncmp(infile[line][j], "!LO:", 4) == 0) {
         xy.i = line;
         xy.j = j;
         laystate[track][layer].append(xy);
      }
   }

}



/////////////////////////////
//
// storeLayoutInformation --  Global layout information
//    gets processed into track 0.
//

void storeLayoutInformation(HumdrumFile& infile, int line, 
      Array<Array<Array<Coord> > >& laystate, 
      Array<Array<Array<Coord> > >& layout,
      int layoutisinitQ) {

   Array<int> layerinfo;
   layerinfo.setSize(infile.getMaxTracks()+1);
   layerinfo.setAll(0);

   if (!layoutisinitQ) {
      layout[line].setSize(infile[line].getFieldCount());
   }

   Coord xy;
   int j, k;
   int track;
   int layer;
   for (j=0; j<infile[line].getFieldCount(); j++) {
      track = infile[line].getPrimaryTrack(j);
      layerinfo[track]++;
      if (strcmp(infile[line][j], ".") == 0) {
         // null token: don't store any layout data
         continue;
      }
      layer = layerinfo[track];
      if (!layoutisinitQ) {
         layout[line][j].setSize(0);
      }
      for (k=0; k<laystate[track][layer].getSize(); k++) {
         xy = laystate[track][layer][k];
         if ((xy.i < 0) || (xy.j < 0)) {
            continue;
         }

         layout[line][j].append(xy);
      }
      // clear the current layer information
      laystate[track][layer].setSize(0);
   }


}



//////////////////////////////
//
// countBeamStuff --
//

void countBeamStuff(const char* token, int& start, int& stop, int& flagr, 
      int& flagl) {
   int i = 0;
   start = stop = flagr = flagl = 0;
   while (token[i] != '\0') {
      switch (token[i]) {
         case 'L': start++;  break;
         case 'J': stop++;   break;
         case 'K': flagr++;  break;
         case 'k': flagl++;  break;
      }
      i++;
   }
}



//////////////////////////////
//
// verifyMuseDataFiles --
//

void verifyMuseDataFiles(Options& options) {
   int i;

   if (options.getArgCount() == 0) {
      verifyMuseDataFile(std::cin);
   } else {
      for (i=0; i<options.getArgCount(); i++) {
         verifyMuseDataFile(options.getArg(i+1));
      }
   }
}



//////////////////////////////
//
// verifyMuseDataFile -- standard input version
//

void verifyMuseDataFile(istream& input) {
   MuseDataSet mds;
   mds.read(input);
   int i;
   int vp;
   for (i=0; i<mds.getPartCount(); i++) {
      vp = verifyPart(mds[i]);
      if (vp) {
         cout << ".\tpart_" << i+1 << ":\t" << "verified" << endl;
      } else {
         cout << ".\tpart_" << i+1 << ":\t" << "modified" << endl;
      }
   }
}



//////////////////////////////
//
// verifyMuseDataFile --
//

void verifyMuseDataFile(const char* filename) {
   MuseDataSet mds;
   mds.read(filename);
   int i;
   int vp;
   for (i=0; i<mds.getPartCount(); i++) {
      vp = verifyPart(mds[i]);
      if (vp) {
         cout << filename << "\tpart_" << i+1 << ":\t" << "verified" << endl;
      } else {
         cout << filename << "\tpart_" << i+1 << ":\t" << "modified" << endl;
      }
   }
}



//////////////////////////////
//
// verifyPart -- check to see if the MD5sum matches the part
//

int verifyPart(MuseData& part) {
   int output = 0;
   int startline = -1;
   int stopline = -1;
   int i;
   int commentstate = 0;
   for (i=0; i<part.getNumLines(); i++) {
      if (part[i][0] == '@') {
         // single line comment
         continue;
      }
      if (part[i][0] == '&') {
         commentstate = !commentstate;
         continue;
      }
      if (commentstate) {
         continue;
      }
      startline = i;
      break;
   }

   for (i=part.getNumLines()-1; i>=0; i--) {
      if (strncmp(part[i].getLine(), "/eof", 4) == 0) {
         stopline = i-1;
         break;
      }
      if (strncmp(part[i].getLine(), "/END", 4) == 0) {
         stopline = part.getNumLines()-1;
         break;
      }
      if (strncmp(part[i].getLine(), "/FINE", 4) == 0) {
         stopline = part.getNumLines()-1;
         break;
      }
   }

   if (debugQ) {
      cout << "-============================================" << endl;
      cout << part;
      cout << "-============================================" << endl;
   }

   if ((startline < 0) || (stopline < 0)) {
      cerr << "Syntax error in part" << endl;
      exit(1);
   }
   if (startline >= stopline) {
      cerr << "Strange Syntax error in part" << endl;
      exit(1);
   }

   int timestampindex = -1;
   for (i=startline; i<=stopline; i++) {
      if (strncmp(part[i].getLine(), "TIMESTAMP", strlen("TIMESTAMP")) == 0) {
         timestampindex = i;
         break;
      }
   }

   if (timestampindex < 0) {
      cerr << "Cannot find timestamp index" << endl;
      exit(1);
   }

   PerlRegularExpression pre;
   if (!pre.search(part[timestampindex].getLine(), "^(.*\\[)(.*)(\\].*)$", "")) {
      cerr << "Cannot find checksum on timestamp line:" << endl;
      cerr << part[timestampindex] << endl;
      exit(1);
   }

   Array<char> checksum(strlen(pre.getSubmatch(2))+1);
   strcpy(checksum.getBase(), pre.getSubmatch(2));

   PerlRegularExpression pre2;
   if (!pre2.search(checksum, ":?([^:]+)$", "")) {
      return 0;
      cerr << "Cannot find checksum on timestamp line, here is the checksum: " 
           <<  checksum << endl;
   }
   Array<char> oldmd5sum(strlen(pre2.getSubmatch(1))+1);
   strcpy(oldmd5sum.getBase(), pre2.getSubmatch());

   // currently only presuming md5sum as the checksum type.

   // default is for MS-DOS
   Array<char> LNEWLINE(3);
   LNEWLINE[0] = (char)0x0d;
   LNEWLINE[1] = (char)0x0a;
   LNEWLINE[2] = '\0';

   if (pre2.search(checksum, ":([^:]+):", "")) {
      int len = strlen(pre2.getSubmatch(1));
      int value;
      int value2;
      if (len == 1) {
         if (sscanf(pre2.getSubmatch(), "%1x", &value)) {
            LNEWLINE[0] = (char)value;
            LNEWLINE[1] = '\0';
         }
      } else if (len == 2) {
         if (sscanf(pre2.getSubmatch(), "%2x", &value)) {
            LNEWLINE[0] = (char)value;
            LNEWLINE[1] = '\0';
         }
      } else if (len == 4) {
         if (sscanf(pre2.getSubmatch(), "%2x%2x", &value, &value2)) {
            LNEWLINE[0] = (char)value;
            LNEWLINE[1] = (char)value2;
            LNEWLINE[2] = '\0';
         }
      } // ignore if longer than 4 characters
   } else if (pre2.search(checksum, "::", "")) {
      // no newlines at all when calculating checksum
      LNEWLINE[0] = '\0';
   }

   part[timestampindex].clear();
   part[timestampindex].appendString(pre.getSubmatch(1));
   part[timestampindex].appendString(pre.getSubmatch(3));
    
   SSTREAM tstream;
   for (i=startline; i<=stopline; i++) {
      tstream << part[i] << LNEWLINE;
   }
   tstream << ends;
   Array<char> partdata(strlen(tstream.CSTRING)+1);
   strcpy(partdata.getBase(), tstream.CSTRING);

   Array<char> newmd5sum;
   CheckSum::getMD5Sum(newmd5sum, partdata);

   if (debugQ) {
      cout << "OLD MD5SUM: " << oldmd5sum << endl;
      cout << "NEW MD5SUM: " << newmd5sum << endl;
      cout << "PART ===================================" << endl;
      cout << partdata;
      cout << "==========================================" << endl;
   }

   if (strcmp(newmd5sum.getBase(), oldmd5sum.getBase()) == 0) {
      output = 1;
   } else {
      output = 0;
   }

   return output;
}



//////////////////////////////
//
// updateMuseDataFileTimeStamps -- Update timestamps if the part
//     has been changed according to the timestamp.
//

void updateMuseDataFileTimeStamps(Options& options) {
   if (options.getArgCount() == 0) {
      updateFileTimeStamps(std::cin, options);
   } else {
      int i;
      for (i=0; i<options.getArgCount(); i++) {
         updateFileTimeStamps(options.getArg(i+1), options);
      }
   }
}



//////////////////////////////
//
// removeOldTimeStamps -- remove old timestamps (and reasons) at the bottom
// of the file.
//

void removeOldTimeStamps(MuseData& part) {
   int i;
   int atend = 0;
   SSTREAM tstream;
   for (i=0; i<part.getLineCount(); i++) {
      if (strncmp(part[i].getLine(), "/END", 4) == 0) {
         atend = 1;
      }
      if (strncmp(part[i].getLine(), "/FINE", 4) == 0) {
         atend = 1;
      }
      if (!atend) {
         tstream << part[i] << "\n";  // newline version does not matter
         continue;
      }
      if (strncmp(part[i].getLine(), "@SUPERSEDES:", 
            strlen("@SUPERSEDES:")) == 0) {
         continue;
      }
      if (strncmp(part[i].getLine(), "@REASON:", 
            strlen("@REASON:")) == 0) {
         continue;
      }
      tstream << part[i] << "\n";
   }

   tstream << ends;
   part.clear();
   part.read(tstream);

}



//////////////////////////////
//
// getTimeStampLine -- return the line in the header which starts with 
//     "TIMESTAMP".  Returns -1 if there is no timestamp.
//

int getTimeStampLine(MuseRecord& arecord, MuseData& part) {
   int i;
   arecord.clear();
   for (i=0; i<part.getLineCount(); i++) {
      if (strncmp(part[i].getLine(), "TIMESTAMP", strlen("TIMESTAMP")) == 0) {
         arecord = part[i];
         return i;
      }
   }
   return -1;
}



//////////////////////////////
//
// updateFileTimeStamps -- standard input version
//

void updateFileTimeStamps(istream& input, Options& options) {
   MuseDataSet mds;
   mds.read(input);
   doUpdateWork(mds);

   // print result to standard output
   Array<char> newline;
   int i, j;
   for (i=0; i<mds.getPartCount(); i++) {
      getNewLine(newline, mds[i]);
      for (j=0; j<mds[i].getLineCount(); j++) {
         cout << mds[i][j] << newline;
      }
   }

}


void updateFileTimeStamps(const char* filename,  Options& options) {
   MuseDataSet mds;
   mds.read(filename);
   doUpdateWork(mds);

   // print new contents to file.
   ofstream outfile;
   outfile.open(filename);
   if (!outfile.is_open()) {
      cerr << "Error: could not open " << filename << " for rewriting" << endl;
      exit(1);
   }

   Array<char> newline;
   int i, j;
   for (i=0; i<mds.getPartCount(); i++) {
      getNewLine(newline, mds[i]);
      for (j=0; j<mds[i].getLineCount(); j++) {
         outfile << mds[i][j] << newline;
      }
   }
   outfile.close();

}


//////////////////////////////
//
// doUpdateWork -- Update the timestams in a MuseData multi-part file.
//

void doUpdateWork(MuseDataSet& mds) {

   int i;
   int vp;
   int tindex;
   MuseRecord trecord;
   MuseRecord arecord;
   MuseRecord rrecord;
   if (options.getBoolean("reason")) {
      rrecord.append("ss", "@REASON: ", options.getString("reason"));
   }

   for (i=0; i<mds.getPartCount(); i++) {
      if (options.getBoolean("remove-old")) {
         removeOldTimeStamps(mds[i]);
      }
      tindex = getTimeStampLine(trecord, mds[i]);
      if (tindex < 0) {
         cerr << "ERROR: part " << i+1 << " does not have a timestamp" << endl;
      }
      vp = verifyPart(mds[i]);
      if (vp) {
         // do nothing: the part is up-to-date.
      } else {
         arecord.clear();
         // need to keep track of /eof and // lines and insert before them.
         // just appending for now.
         if ((!options.getBoolean("remove-old")) && 
                  (!options.getBoolean("no-append"))) {

            arecord.append("ss", "@SUPERSEDES: ", trecord.getLine());
            appendToEndOfPart(mds[i], arecord);
            if (options.getBoolean("reason")) {
               appendToEndOfPart(mds[i], rrecord);
            }
         }
         updatePart(mds[i]);
      }
   }
}



//////////////////////////////
//
// appendToEndOfPart -- place record at end of part file, but before
// any /eof or // lines.
//

void appendToEndOfPart(MuseData& md, MuseRecord& arecord) {
   int i;
   int ending = -1;
   MuseData mend;
   for (i=md.getLineCount()-1; i>=0; i--) {
      if (strncmp(md[i].getLine(), "//", 2) == 0) {
         mend.append(md[i]);
         continue;
      } 
      if (strncmp(md[i].getLine(), "/eof", 2) == 0) {
         mend.append(md[i]);
         continue;
      }
      ending = i;
      break;
   }
   if (ending < 0) {
      md.append(arecord);
      return;
   }
   if (ending == md.getLineCount() - 1) {
      md.append(arecord);
      return;
   }

   md.append(arecord);  // add a dummy record to increase by one
   md[ending+1] = arecord;
   for (i=0; i<mend.getNumLines(); i++) {
      md[ending+2+i] = mend[mend.getNumLines()-i-1];
   }
}



//////////////////////////////
//
// updatePart -- change the timestamp date to today and calculate
// a new checksum.
//

void updatePart(MuseData& part) {
   Array<char> newline;
   getNewLine(newline, part);

   // make timestamp record with current date
   MuseData timestamp;
   addTimeStamp(timestamp);

   // put new time stamp in part
   MuseRecord arecord;
   int tline;
   tline = getTimeStampLine(arecord, part);
   if (tline < 0) {
      cerr << "Error: no timestamp line to update" << endl;
      exit(1);
   }
   part[tline] = timestamp[0];

   // calculate the new checksum and store in file
   Array<char> checksum;
   getCheckSum(checksum, part, newline);

   Array<char> tdata;
   tdata.setSize(strlen(part[tline].getLine()) + 1);
   strcpy(tdata.getBase(), part[tline].getLine());
   PerlRegularExpression pre;
   pre.sar(tdata, "\\[\\]", "[md5sum:AAYYYBB:AAZZZBB]", "");
   char newlinestring[128] = {0};
   char buffer[16] = {0};
   int i;
   for (i=0; i<newline.getSize()-1; i++) {
      sprintf(buffer, "%02x", (int)newline[i]);
      strcat(newlinestring, buffer);
   }
   pre.sar(tdata, "AAYYYBB", newlinestring, "");
   pre.sar(tdata, "AAZZZBB", checksum.getBase(), "");
   part[tline].clear();
   part[tline].insertString(1, tdata.getBase());
}



//////////////////////////////
//
// getCheckSum -- get the checksum for the given part (which should
// not contain anything within the [] on the TIMESAMP line.
//

void getCheckSum(Array<char>& checksum, MuseData& part, Array<char>& newline) {
   int startline = -1;
   int stopline = -1;
   int i;
   int commentstate = 0;
   for (i=0; i<part.getNumLines(); i++) {
      if (part[i][0] == '@') {
         // single line comment
         continue;
      }
      if (part[i][0] == '&') {
         commentstate = !commentstate;
         continue;
      }
      if (commentstate) {
         continue;
      }
      startline = i;
      break;
   }

   for (i=part.getNumLines()-1; i>=0; i--) {
      if (strncmp(part[i].getLine(), "/eof", 4) == 0) {
         stopline = i-1;
         break;
      }
      if (strncmp(part[i].getLine(), "/END", 4) == 0) {
         stopline = part.getNumLines()-1;
         break;
      }
      if (strncmp(part[i].getLine(), "/FINE", 4) == 0) {
         stopline = part.getNumLines()-1;
         break;
      }
   }

   if (debugQ) {
      cout << "-============================================" << endl;
      cout << part;
      cout << "-============================================" << endl;
   }

   if ((startline < 0) || (stopline < 0)) {
      cerr << "Syntax error in part" << endl;
      exit(1);
   }
   if (startline >= stopline) {
      cerr << "Strange Syntax error in part" << endl;
      exit(1);
   }

   Array<char>& LNEWLINE = newline;

   SSTREAM tstream;
   for (i=startline; i<=stopline; i++) {
      tstream << part[i] << LNEWLINE;
   }
   tstream << ends;
   Array<char> partdata(strlen(tstream.CSTRING)+1);
   strcpy(partdata.getBase(), tstream.CSTRING);

   Array<char>& newmd5sum = checksum;
   CheckSum::getMD5Sum(newmd5sum, partdata);

   if (debugQ) {
      cout << "NEW MD5SUM: " << newmd5sum << endl;
      cout << "PART ===================================" << endl;
      cout << partdata;
      cout << "==========================================" << endl;
   }
}



//////////////////////////////
//
// getNewLine -- return the newline convention used in the file.
//    Using MS-DOS convention if none found.
//

void getNewLine(Array<char>& newline, MuseData& part) {
  
   // default style:
   newline.setSize(3);
   newline[0] = (char)0x0d;
   newline[1] = (char)0x0a;
   newline[2] = '\0';

   int timestampindex = -1;
   int i;
   for (i=0; i<part.getLineCount(); i++) {
      if (strncmp(part[i].getLine(), "TIMESTAMP", strlen("TIMESTAMP")) == 0) {
         timestampindex = i;
         break;
      }
   }
   if (timestampindex < 0) {
      return;
   }

   PerlRegularExpression pre;
   if (!pre.search(part[timestampindex].getLine(), 
         "^.*\\[[^:]*?:([^:]+):.*\\].*$", "")) {
      return;
   }

   int len = strlen(pre.getSubmatch(1));
   int value;
   int value2;
   if (len == 1) {
      if (sscanf(pre.getSubmatch(), "%1x", &value)) {
         newline[0] = (char)value;
         newline[1] = '\0';
      }
   } else if (len == 2) {
      if (sscanf(pre.getSubmatch(), "%2x", &value)) {
         newline[0] = (char)value;
         newline[1] = '\0';
      }
   } else if (len == 4) {
      if (sscanf(pre.getSubmatch(), "%2x%2x", &value, &value2)) {
         newline[0] = (char)value;
         newline[1] = (char)value2;
         newline[2] = '\0';
      }
   } // ignore if longer than 4 characters

}



//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char** argv) {
   opts.define("v|verify=b",  "Verify input MuseData file(s)");
   opts.define("d|debug=b",    "Debugging information");
   opts.define("ns|no-slur|no-slurs|noslur|noslurs=b",  "do not convert slurs");
   // tuplet-bracket no longer being used:
   opts.define("tb|tuplet-bracket=b", "print brackets on tuplet");
   opts.define("vz=b", "only display explicit tuplet marks");
   opts.define("nm|nomet|nomen|no-met|no-men=b",  "do not print metrical symbols instead of time signatures");
   opts.define("nd|no-dynamics|no-dynamic|no-dyn|nodynamics|nodyn|nodynamic=b", "do not convert dynamics");
   opts.define("nb|no-beams|no-beam|nobeam|nobeams=b", "do not convert beams");
   opts.define("no-tie|no-ties|notie|noties=b", "do not encode ties");
   opts.define("no-tuplet|no-tuplets|notuplet|notuplets|no-triplet|no-triplets|notriplet|notriplets=b", "do not encode tuplet groupings");
   opts.define("round=b", "rounded breves (double whole notes)");
   opts.define("nt|no-text|notext=b", "do not convert unknown spines to text");
   opts.define("text=s:", "list of spine types to treat as lyrics");
   opts.define("x|exclude=s:", "list of parameters to exclude");
   opts.define("U|unix|linux=b", "use Unix/Linux newlines");
   opts.define("MAC=b", "use Apple Macintosh old-style newlines");
   opts.define("men|mensural=b", "display in quasi-mensural notation");
   opts.define("men2|mensural2=b", "display in quasi-mensural notation, model 2");
   opts.define("R|no-reference-records|norr|nor|norefernce|no-bib|nobib|no-bibliographic|nobibliographic=b", "do not add reference records");
   opts.define("mono=i:0", "display rhythms in mono-spacing");
   opts.define("vl|verse-limit=i:5", "maximum number of verses to display");
   opts.define("update=b", "update timestamps in MuseData file(s)");
   opts.define("f|foot|footer=s", "Add footer information");
   opts.define("sepbracket=b", "Bracket all staves, with separate barlines");
   opts.define("reason=s", "reason for updating a timestamp");
   opts.define("ro|removeoold|remove-old=b", "remove old timestamps");
   opts.define("na|noappend|no-append=b", "do not append old timestamp");
   opts.define("ls|lyrics-spines=s:", "spines to display as lyrics");
   opts.define("dd|dr|default-duration|default-rhythm=s:4", 
       "rhythm to use if note does not have a rhythm and is not a grace note");
   opts.define("mo|muse2ps-options=s:", "muse2ps option string to append to data");
   opts.define("encoder=s:", "Person to place on line fixed header line 4");
   opts.define("copyright|copy|cr=s:", "Copyright to place on line 1 of fixed header");

   opts.define("author=b",    "Program author");
   opts.define("version=b",   "Program version");
   opts.define("example=b",   "Program examples");
   opts.define("h|help=b",    "Short description");
   opts.process(argc, argv);

   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, June 2010" << NEWLINE;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 2 June 2010" << NEWLINE;
      cout << "compiled: " << __DATE__ << NEWLINE;
      cout << MUSEINFO_VERSION << NEWLINE;
      exit(0);
   } else if (opts.getBoolean("help")) {
      usage(opts.getCommand());
      exit(0);
   } else if (opts.getBoolean("example")) {
      example();
      exit(0);
   }

   if (opts.getBoolean("verify")) {
      verifyMuseDataFiles(options);
      exit(0);
   } else if (opts.getBoolean("update")) {
      updateMuseDataFileTimeStamps(options);
      exit(0);
   }

   debugQ       =  opts.getBoolean("debug");
   roundQ       =  opts.getBoolean("round");
   slurQ        = !opts.getBoolean("no-slur");
   dynamicsQ    = !opts.getBoolean("no-dynamics");
   mensuralQ    =  opts.getBoolean("mensural");
   metQ         = !opts.getBoolean("no-met");
   sepbracketQ  =  opts.getBoolean("sepbracket");
   mensural2Q   =  opts.getBoolean("mensural2");
   if (mensural2Q) {
      mensuralQ = 1;
   }
   referenceQ = !opts.getBoolean("no-reference-records");
   textQ      = !opts.getBoolean("no-text");
   TextSpines =  opts.getString("text");
   beamQ      = !opts.getBoolean("no-beams");
   tieQ       = !opts.getBoolean("no-ties");
   excludeQ   =  opts.getBoolean("exclude");
   excludeString = opts.getString("exclude");

   footerQ    =  opts.getBoolean("footer");
   if (footerQ) {
      footertext = opts.getString("footer");
   }
   tupletQ    = !opts.getBoolean("no-tuplets");
   vzQ        =  opts.getBoolean("vz");
   if (vzQ) {
      // only draw explicit tuplets
      tupletQ = 0;
   }
   defaultDur =  opts.getString("default-duration");
   verselimit =  opts.getInteger("verse-limit");
   if (opts.getBoolean("encoder")) {
      encoderQ = 1;
      Encoder  = opts.getString("encoder");
   }
   if (opts.getBoolean("copyright")) {
      copyrightQ = 1;
      Copyright = opts.getString("copyright");
   }
   sfzQ = dynamicsQ;
   muse2psoptionstring = opts.getString("muse2ps-options");
   LyricSpines  = opts.getString("lyrics-spines");

   if (opts.getBoolean("unix")) {
      NEWLINE.setSize(2);
      NEWLINE[0] = (char)0x0a;
      NEWLINE[1] = '\0';
   } else if (opts.getBoolean("MAC")) {
      NEWLINE.setSize(2);
      NEWLINE[0] = (char)0x0d;
      NEWLINE[1] = '\0';
   } // otherwise default of 0x0d 0x0a (MS-DOS) will be used.

}



//////////////////////////////
//
// procesSLyricsSpines -- Generate a list of spines which should be printed
//     as lyrics.
//

void processLyricsSpines(Array<Array<int> >& spineinfo, 
      HumdrumFile& infile, const char* data) {
   spineinfo.setSize(infile.getMaxTracks()+1);
   int i, j;
   for (i=0; i<spineinfo.getSize(); i++) {
      spineinfo[i].setSize(0);
   }

   Array<int> kerntracks;
   getKernTracks(kerntracks, infile);

   Array<char> adata(strlen(data) + 1);
   strcpy(adata.getBase(), data);
   PerlRegularExpression pre;
   PerlRegularExpression pre2;
   pre.sar(adata, "\\s+", ",", "gi");
   pre.sar(adata, ",+", ",", "gi");
   pre.sar(adata, "^[^s]*s", "s", "i");
   Array<Array<char> > tokens;
   int staff, spine;
   Array<Array<char> > subtokens;
   pre.getTokens(tokens, ",?s", adata.getBase());

   for (i=0; i<tokens.getSize(); i++) {
      if (!pre.search(tokens[i], "^(\\d+):(.*)", "")) {
         continue;
      }
      staff = abs(atoi(pre.getSubmatch(1)));
      if (staff > infile.getMaxTracks()) {
         continue;
      }
      pre2.getTokens(subtokens, ",", pre.getSubmatch(2));
      for (j=0; j<subtokens.getSize(); j++) {
         spine = abs(atoi(subtokens[j].getBase()));
         if (spine > infile.getMaxTracks()) {
            continue;
         }
         spineinfo[kerntracks[kerntracks.getSize()-staff-1+1]].append(spine);
      }
   }
}


//////////////////////////////
//
// GCD -- greatest common divisor
//
 
int GCD(int a, int b) {
   if (b == 0) {
      return a;
   }
   int z = a % b;
   a = b;
   b = z;
   int output = GCD(a, b);
   if (debugQ) {  
      cout << "GCD of " << a << " and " << b << " is " << output << NEWLINE;
   }
   return output;
}    



//////////////////////////////
//
// LCM -- find the least common multiple between rhythms
//

int LCM(Array<int>& rhythms) {
   if (rhythms.getSize() == 0) {
      return 0;
   }
   int output = rhythms[0];
   for (int i=1; i<rhythms.getSize(); i++) {
      output = output * rhythms[i] / GCD(output, rhythms[i]);
   }

   return output;
}



//////////////////////////////
//
// isPowerOfTwo --
//

int isPowerOfTwo(RationalNumber& rn) {
   if (rn == 0) {
      return 1;
   }
   double val1 = log2((double)rn.getNumerator());
   double val2 = log2((double)rn.getDenominator());
   const double roundofferr  = 0.00001;
   const double roundofferr2 = 0.000001;
   if ((val1 - (int)(val1 + roundofferr2)) < roundofferr) {
      if ((val2 - (int)(val2 + roundofferr2)) < roundofferr) {
         return 1;
      }
   }
   return 0;
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
// getRhythms -- returns a list of rhythms found in the given range
// in the score.  The output is sorted by largest duration rhythm first, 
// towards the shortest.  zero-length notes are ignored (grace notes).  
// A quarter note is always reported to be present, since this function 
// is intended to be used with the Q: field of $ records in MuseData files.
//
// If the stop index is -1, then use the last line of the file ast tne
// stopping index.
//

void getRhythms(Array<RationalNumber>& rns, HumdrumFile& infile, 
       int startindex, int stopindex) {
   if (stopindex < 0) {
      stopindex = infile.getNumLines() - 1;
   }
   rns.setSize(100);
   rns.setSize(0);
   RationalNumber rn;
   rn = 4;
   int found;
   // rns.append(rn);   // stick a quarter note in the list
   int i, j, k;
   for (i=startindex; i<=stopindex; i++) {
      if (!infile[i].isData()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (!infile[i].isExInterp(j, "**kern")) {
            continue;
         }
         if (strcmp(infile[i][j], ".") == 0) {
            // ignore null records
            continue;
         }
         if (strchr(infile[i][j], 'q') != NULL) {
            // ignore grace notes with a slash
            continue;
         }
         if (strchr(infile[i][j], 'Q') != NULL) {
            // ignore grace notes without a slash
            continue;
         }
         rn = Convert::kernToDurationR(infile[i][j]);
         if (rn == 0) {
            continue;
         }
         found = 0;
         for (k=0; k<rns.getSize(); k++) {
            if (rns[k] == rn) {
               found = 1;
               break;
            }
         }
         if (found == 0) {
            rns.append(rn);
         }
      }
   }

   qsort(rns.getBase(), rns.getSize(), sizeof(RationalNumber), rnsort);

   for (i=0; i<rns.getSize(); i++) {
      rns[i].invert();
   }
}



//////////////////////////////
//
// rnsort -- sort Rational numbers, largest first.
//

int rnsort(const void* A, const void* B) {
   RationalNumber valuea = *((RationalNumber*)A);
   RationalNumber valueb = *((RationalNumber*)B);
   if (valuea > valueb) {
      return -1;
   } else if (valuea < valueb) {
      return +1;
   } else {
      return 0;
   }
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





/* Function hierarchy:
 *
main:
	CALLS:
		checkOptions
		convertData
		getBeamState
		getDynamicsAssignment
		getMeasureDuration
		getPartNames
		getTupletState
		printMuse2PsOptions
		printWithMd5sum
		setColorCharacters
		setupMusicaFictaVariables
		setupTextAssignments
checkOptions:
	CALLS:
		example
		updateMuseDataFileTimeStamps
		usage
		verifyMuseDataFiles
	CALLED BY:
		main
example:
	CALLED BY:
		checkOptions
updateMuseDataFileTimeStamps:
	CALLS:
		updateFileTimeStamps
	CALLED BY:
		checkOptions
updateFileTimeStamps:
	CALLS:
		doUpdateWork
	CALLED BY:
		updateMuseDataFileTimeStamps
doUpdateWork:
	CALLS:
		getTimeStampLine
		removeOldTimeStamps
		updatePart
		verifyPart
	CALLED BY:
		updateFileTimeStamps
getTimeStampLine:
	CALLED BY:
		doUpdateWork
removeOldTimeStamps:
	CALLED BY:
		doUpdateWork
updatePart:
	CALLED BY:
		doUpdateWork
verifyPart:
	CALLS:
		getMD5Sum
	CALLED BY:
		doUpdateWork
		verifyMuseDataFile
getMD5Sum:
	CALLS:
		MD5Final
		MD5Init
		MD5Update
	CALLED BY:
		printWithMd5sum
		verifyPart
MD5Final:
	CALLS:
		Encode
		MD5Update
	CALLED BY:
		getMD5Sum
Encode:
	CALLED BY:
		MD5Final
MD5Update:
	CALLS:
		MD5Transform
	CALLED BY:
		MD5Final
		getMD5Sum
MD5Transform:
	CALLS:
		Decode
	CALLED BY:
		MD5Update
Decode:
	CALLED BY:
		MD5Transform
MD5Init:
	CALLED BY:
		getMD5Sum
usage:
	CALLED BY:
		checkOptions
verifyMuseDataFiles:
	CALLS:
		verifyMuseDataFile
	CALLED BY:
		checkOptions
verifyMuseDataFile:
	CALLS:
		verifyPart
	CALLED BY:
		verifyMuseDataFiles
convertData:
	CALLS:
		convertTrackToMuseData
		getKernTracks
		isBarlineBeforeData
		prepareLocalTickChanges
	CALLED BY:
		main
convertTrackToMuseData:
	CALLS:
		appendReferenceRecords
		getMeasureData
		getMeasureInfo
		insertDollarRecord
		insertHeaderRecords
	CALLED BY:
		convertData
appendReferenceRecords:
	CALLED BY:
		convertTrackToMuseData
getMeasureData:
	CALLS:
		getMaxVoices
		processVoice
	CALLED BY:
		convertTrackToMuseData
getMaxVoices:
	CALLED BY:
		getMeasureData
processVoice:
	CALLS:
		addBackup
		addMeasureEntry
		addNoteToEntry
		getTick
		insertDollarRecord
	CALLED BY:
		getMeasureData
addBackup:
	CALLED BY:
		processVoice
addMeasureEntry:
	CALLS:
		addTextLayout
		processMeasureLayout
	CALLED BY:
		processVoice
addTextLayout:
	CALLS:
		addText
	CALLED BY:
		addMeasureEntry
		addNoteToEntry
addText:
	CALLS:
		addPositionParameters
		convertHtmlTextToMuseData
	CALLED BY:
		addTextLayout
addPositionParameters:
	CALLS:
		getXxYy
	CALLED BY:
		addPositionInfo
		addText
getXxYy:
	CALLED BY:
		addPositionParameters
		convertKernLODYtoMusePS
convertHtmlTextToMuseData:
	CALLED BY:
		addLyrics
		addMovementTitle
		addSourceRecord
		addText
		addWorkTitle
		printMuse2PsOptions
processMeasureLayout:
	CALLS:
		handleMeasureLayoutParam
	CALLED BY:
		addMeasureEntry
handleMeasureLayoutParam:
	CALLED BY:
		processMeasureLayout
addNoteToEntry:
	CALLS:
		addChordLevelArtic
		addHairpinStarts
		addHairpinStops
		addLyrics
		addNoteLevelArtic
		addTextLayout
		checkColor
		getChordMapping
		getDuration
		getTickDur
		handleLayoutInfoChord
		setNoteheadShape
	CALLED BY:
		processVoice
addChordLevelArtic:
	CALLS:
		insertArpeggio
	CALLED BY:
		addNoteToEntry
insertArpeggio:
	CALLS:
		getChordMapping
		getPitches
		getScoreStaffVerticalPos
	CALLED BY:
		addChordLevelArtic
getChordMapping:
	CALLS:
		getPitches
		numberRsort
		numbersort
	CALLED BY:
		addNoteToEntry
		insertArpeggio
getPitches:
	CALLED BY:
		getChordMapping
		insertArpeggio
numberRsort:
	CALLED BY:
		getChordMapping
numbersort:
	CALLED BY:
		getChordMapping
getScoreStaffVerticalPos:
	CALLED BY:
		insertArpeggio
addHairpinStarts:
	CALLS:
		addCrescText
		addCrescendoStart
		addCrescendoStop
		addDecrescendoStart
		addDecrescendoStop
	CALLED BY:
		addNoteToEntry
addCrescText:
	CALLS:
		addDashing
		addPositionInfo
	CALLED BY:
		addHairpinStarts
addDashing:
	CALLED BY:
		addCrescText
addPositionInfo:
	CALLS:
		addPositionParameters
	CALLED BY:
		addCrescText
		addCrescendoStart
		addCrescendoStop
		addDecrescendoStart
		addDecrescendoStop
addCrescendoStart:
	CALLS:
		addPositionInfo
	CALLED BY:
		addHairpinStarts
addCrescendoStop:
	CALLS:
		addPositionInfo
	CALLED BY:
		addHairpinStarts
		addHairpinStops
addDecrescendoStart:
	CALLS:
		addPositionInfo
	CALLED BY:
		addHairpinStarts
addDecrescendoStop:
	CALLS:
		addPositionInfo
	CALLED BY:
		addHairpinStarts
		addHairpinStops
addHairpinStops:
	CALLS:
		addCrescendoStop
		addDecrescendoStop
		addUnDash
	CALLED BY:
		addNoteToEntry
addUnDash:
	CALLED BY:
		addHairpinStops
addLyrics:
	CALLS:
		convertHtmlTextToMuseData
		convertHumdrumTextToMuseData
		track2column
	CALLED BY:
		addNoteToEntry
convertHumdrumTextToMuseData:
	CALLED BY:
		addLyrics
track2column:
	CALLED BY:
		addLyrics
addNoteLevelArtic:
	CALLS:
		getBase36
	CALLED BY:
		addNoteToEntry
getBase36:
	CALLED BY:
		addNoteLevelArtic
checkColor:
	CALLED BY:
		addNoteToEntry
getDuration:
	CALLED BY:
		addNoteToEntry
		getGlobalTicksPerQuarter
		getTickDur
		setNoteheadShape
getTickDur:
	CALLS:
		getDuration
	CALLED BY:
		addNoteToEntry
handleLayoutInfoChord:
	CALLS:
		addDynamics
		insertSlurDownPrintSug
		insertSlurUpPrintSug
		insertStaccatoSuggestion
	CALLED BY:
		addNoteToEntry
addDynamics:
	CALLS:
		processDynamicsLayout
	CALLED BY:
		handleLayoutInfoChord
processDynamicsLayout:
	CALLS:
		convertKernLODYtoMusePS
	CALLED BY:
		addDynamics
convertKernLODYtoMusePS:
	CALLS:
		getXxYy
	CALLED BY:
		processDynamicsLayout
insertSlurDownPrintSug:
	CALLED BY:
		handleLayoutInfoChord
insertSlurUpPrintSug:
	CALLED BY:
		handleLayoutInfoChord
insertStaccatoSuggestion:
	CALLED BY:
		handleLayoutInfoChord
setNoteheadShape:
	CALLS:
		getDuration
		setNoteheadShape
	CALLED BY:
		addNoteToEntry
		setNoteheadShape
getTick:
	CALLED BY:
		processVoice
insertDollarRecord:
	CALLS:
		addMovementDesignation
		appendClef
		appendKeySignature
		appendTimeSignature
	CALLED BY:
		convertTrackToMuseData
		processVoice
addMovementDesignation:
	CALLED BY:
		insertDollarRecord
appendClef:
	CALLED BY:
		insertDollarRecord
appendKeySignature:
	CALLED BY:
		insertDollarRecord
appendTimeSignature:
	CALLED BY:
		insertDollarRecord
getMeasureInfo:
	CALLED BY:
		convertTrackToMuseData
		prepareLocalTickChanges
insertHeaderRecords:
	CALLS:
		addControlNumber
		addCopyrightLine
		addDateAndEncoder
		addMovementTitle
		addPartName
		addSourceRecord
		addTimeStamp
		addWorkNumberInfo
		addWorkTitle
	CALLED BY:
		convertTrackToMuseData
addControlNumber:
	CALLED BY:
		insertHeaderRecords
addCopyrightLine:
	CALLED BY:
		insertHeaderRecords
addDateAndEncoder:
	CALLED BY:
		insertHeaderRecords
addMovementTitle:
	CALLS:
		convertHtmlTextToMuseData
	CALLED BY:
		insertHeaderRecords
addPartName:
	CALLED BY:
		insertHeaderRecords
addSourceRecord:
	CALLS:
		convertHtmlTextToMuseData
	CALLED BY:
		insertHeaderRecords
addTimeStamp:
	CALLED BY:
		insertHeaderRecords
addWorkNumberInfo:
	CALLS:
		getWorkAndMovement
	CALLED BY:
		insertHeaderRecords
getWorkAndMovement:
	CALLED BY:
		addWorkNumberInfo
addWorkTitle:
	CALLS:
		convertHtmlTextToMuseData
	CALLED BY:
		insertHeaderRecords
getKernTracks:
	CALLED BY:
		convertData
		processLyricsSpines
		setupTextAssignments
isBarlineBeforeData:
	CALLED BY:
		convertData
prepareLocalTickChanges:
	CALLS:
		LCM
		getGlobalTicksPerQuarter
		getMeasureInfo
		getRhythms
		getTPQByRhythmSet
	CALLED BY:
		convertData
LCM:
	CALLS:
		GCD
	CALLED BY:
		getGlobalTicksPerQuarter
		getTPQByRhythmSet
		prepareLocalTickChanges
GCD:
	CALLS:
		GCD
	CALLED BY:
		GCD
		LCM
getGlobalTicksPerQuarter:
	CALLS:
		LCM
		getDuration
	CALLED BY:
		prepareLocalTickChanges
getRhythms:
	CALLS:
		rnsort
	CALLED BY:
		prepareLocalTickChanges
rnsort:
	CALLED BY:
		getRhythms
getTPQByRhythmSet:
	CALLS:
		LCM
	CALLED BY:
		prepareLocalTickChanges
getBeamState:
	CALLS:
		addGlobalLayoutInfo
		addLayoutInfo
		adjustClefState
		countBeamStuff
		storeClefInformation
		storeGlobalLayoutInfo
		storeLayoutInformation
	CALLED BY:
		main
addGlobalLayoutInfo:
	CALLED BY:
		getBeamState
addLayoutInfo:
	CALLED BY:
		getBeamState
adjustClefState:
	CALLED BY:
		getBeamState
countBeamStuff:
	CALLED BY:
		getBeamState
storeClefInformation:
	CALLED BY:
		getBeamState
storeGlobalLayoutInfo:
	CALLED BY:
		getBeamState
storeLayoutInformation:
	CALLED BY:
		getBeamState
getDynamicsAssignment:
	CALLED BY:
		main
getMeasureDuration:
	CALLED BY:
		main
getPartNames:
	CALLED BY:
		main
getTupletState:
	CALLS:
		getDurationNoDots
		getTupletTop
		isPowerOfTwo
	CALLED BY:
		main
getDurationNoDots:
	CALLED BY:
		getTupletState
		getTupletTop
getTupletTop:
	CALLS:
		getDurationNoDots
		primeFactorization
	CALLED BY:
		getTupletState
primeFactorization:
	CALLED BY:
		getTupletTop
isPowerOfTwo:
	CALLED BY:
		getTupletState
printMuse2PsOptions:
	CALLS:
		convertHtmlTextToMuseData
	CALLED BY:
		main
printWithMd5sum:
	CALLS:
		getMD5Sum
	CALLED BY:
		main
setColorCharacters:
	CALLS:
		getColorCategory
	CALLED BY:
		main
getColorCategory:
	CALLED BY:
		setColorCharacters
setupMusicaFictaVariables:
	CALLED BY:
		main
setupTextAssignments:
	CALLS:
		getKernTracks
		processLyricsSpines
	CALLED BY:
		main
processLyricsSpines:
	CALLS:
		getKernTracks
	CALLED BY:
		setupTextAssignments
getNewLine:
	ORPHAN

*/

// md5sum: e35f22b41a2deaa7210e4154e6429bdb hum2muse.cpp [20110918]

//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun  5 13:48:27 PDT 2001
// Last Modified: Tue Jun  5 13:48:29 PDT 2001
// Last Modified: Fri Feb 25 00:09:19 PST 2005 (balanced <measure> markers)
// Last Modified: Tue Dec 12 20:23:42 PST 2006 (handling > and < as text)
// Last Modified: Tue Nov 22 07:08:10 PST 2011 (fixed grace noteshapes)
// Last Modified: Tue Jun 26 15:13:50 PDT 2012 (added <time-modification> for tuplets)
// Last Modified: Tue Jun 26 17:06:38 PDT 2012 (some bug fixes for multi-voice treatment)
// Filename:      ...sig/examples/all/hum2xml.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/hum2xml.cpp
// Syntax:        C++; museinfo
//
// Description:   Converts a Humdrum file into MusicXML.
//

#include "humdrum.h"
#include "PerlRegularExpression.h"
#include "RationalNumber.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif

// function declarations:
void      checkOptions          (Options& opts, int argc, char** argv);
void      example               (void);
void      convertToMusicXML     (HumdrumFile& infile);
int       makePartList          (HumdrumFile& infile);
void      makePart              (HumdrumFile& infile, int start, int col,
                                 int count);
void      generatePartInfo      (HumdrumFile& infile, int start, int col,
                                 int count);
void      convertDataToMusicXML (HumdrumFile& infile, int line, int col, 
                                 int voice);
void      convertMeasureToXML   (HumdrumFile& infile, int line, int col, 
                                 int voice);
void      convertAttributeToXML (HumdrumFile& infile, int line, int col, 
                                 int voice);
void      convertNoteToXML      (HumdrumFile& infile, int line, int col, 
                                 int voice);
void      pline                 (int level, const char* string);
void      usage                 (const char* command);
double    convertNoteEntryToXML (HumdrumFile& infile, int line, int col,
                                 const char* buffer, int chord, int vlevel, 
                                 int voice);
void      checkMeasure          (void);
void      printDurationType     (const char* durstring);
void      printDots             (const char* durstring);
void      adjustKey             (int keyinfo);
void      checkAccidentals      (int diatonic, int alter, int chord);
void      updateAccidentals     (void);
void      printGlobalComments   (HumdrumFile& infile, int direction);
void      printGlobalComment    (HumdrumFile& infile, int line);
void      printBibliography     (HumdrumFile& infile, int line);
void      checkbackup           (int currenttick, int targettick);
void      processTextUnderlay   (HumdrumFile& infile, int line, int col);
void      displayUnknownTextType(HumdrumFile& infile, int line, int col, 
                                 int verse);
void      displayLyrics         (HumdrumFile& infile, int line, int col, 
                                 int verse);
void      displayHTMLText       (const char* buffer);
void      processBeams          (HumdrumFile& infile, int line, int col, 
                                 const char* buffer, int vlevel);
void      setLineTicks          (Array<int>& LineTick, HumdrumFile& infile, int divisions);
void      getBarlines           (Array<int>& barlines, HumdrumFile& infile);
void      convertMeasureToMusicXML(HumdrumFile& infile, int track, 
                                 int startbar, int endbar);
int       getMaxVoice           (HumdrumFile& infile, int track, 
                                 int startline, int endline);
void      convertVoice          (HumdrumFile& infile, int track, int startbar, 
                                 int endbar, int voice);

// User interface variables:
Options   options;
int       debugQ = 0;          // used with the --debug option
int       cautionaryQ = 1;     // used with the --nocaution option
int       reverseQ = 0;        // reverse the ordering of the parts


int       lev = 0;             // indentation level in output text
int       minit = 0;           // at start of each part: if measure is active
double    smallestdur = 1.0;   // smallest durational component in the music
int       divisions = 1;       // number of divisions per beat
int       measurestate = 0;    // 0 = measure is not open, 1 = measure open


int v1lastmeasure[7] = {0};    // for explicit accidentals ABCDEFG
int v1states[7] = {0};         // for explicit accidentals ABCDEFG
int v1key[7] = {0};            // for explicit accidentals ABCDEFG
int v1chord[7] = {0};          // for explicit accidentals ABCDEFG
int v1prechordstates[7] = {0}; // for explicit accidentals ABCDEFG

int AbsTick = 0;                // for <backup> <forward> commands
Array<int> LineTick;            // for <backup> and <forward> commands
int beamlevel[128] = {0};       // for beaming information
int musicstart = 0;             // for suppressing bars before music starts
// int attributes = 0;             // for suppressing multiple attributes entries

Array<int> Barlines;


//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   HumdrumFile infile;
   for (int i=0; i<numinputs || i==0; i++) {
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
      } else {
         infile.read(options.getArg(i+1));
      }

      infile.analyzeRhythm("4");
      getBarlines(Barlines, infile);
      divisions = infile.getMinTimeBase();
      if (divisions % 4 == 0) {
         divisions = divisions/4;
      } else {
         // don't know what this case may be
      }
      setLineTicks(LineTick, infile, divisions);
      convertToMusicXML(infile);

   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// getBarlines --
//

void getBarlines(Array<int>& barlines, HumdrumFile& infile) {
   barlines.setSize(infile.getNumLines());
   barlines.setSize(0);
   int i;
   int datafoundQ = 0;
   int exinterpline = 0;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isData()) {
         datafoundQ = 1;
      }
      if (strncmp(infile[i][0], "**", 2) == 0) {
         exinterpline = i;
         barlines.append(i);
      }
      if (!infile[i].isMeasure()) {
         continue;
      }
      if (!datafoundQ) {
         continue;
      }
      barlines.append(i);
   }
   i = infile.getNumLines() - 1;
   barlines.append(i);
}



//////////////////////////////
//
// setLineTicks --
//

void setLineTicks(Array<int>& LineTick, HumdrumFile& infile, int divisions) {
   LineTick.setSize(infile.getNumLines());
   LineTick.allowGrowth(0);
   RationalNumber rat;
   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      rat = infile[i].getAbsBeatR();
      rat *= divisions;
      if (rat.getDenominator() != 1) {
         cerr << "Divisions error on line " << i+1 << ": " << infile[i] << endl;
         cerr << "GetAbsBeat = " << rat << endl;
         exit(1);
      }
      LineTick[i] = rat.getNumerator();
   }
}



//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("debug=b",  "print debug information"); 
   opts.define("nocaution=b",  "print cautionary accidentals"); 
   opts.define("r|reverse=b",  "reverse the order of the parts"); 

   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("h|help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, May 1998" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 3 July 1998" << endl;
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
   cautionaryQ = !opts.getBoolean("nocaution");
   reverseQ    = opts.getBoolean("reverse");
}



//////////////////////////////
//
// example --
//

void example(void) {


}



//////////////////////////////
//
// convertToMusicXML -- 
//
// MusicXML 0.6a header:
// <?xml version="1.0" standalone="no"?>
// <!DOCTYPE score-partwise PUBLIC "-//Recordare//DTD MusicXML 0.6a Partwise//EN" "//C:/Program Files/Finale 2003/Component Files/partwise.dtd">
//
// MusicXML 0.3 (alpha) header:
// <?xml version=\"1.0\" standalone=\"no\"?>
// <!DOCTYPE score-partwise SYSTEM \"/musicxml/partwise.dtd\">
//   <!-- DOCTYPE score-partwise PUBLIC
// \"-//Recordare//DTD MusicXML 0.3 Partwise//EN\"
// \"http://www.musicxml.org/dtds/partwise.dtd\" -->
//

void convertToMusicXML(HumdrumFile& infile) {
   pline (lev, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
   pline (lev, "<!DOCTYPE score-partwise PUBLIC \"-//Recordare//DTD MusicXML 2.0 Partwise//EN\" \"http://www.musicxml.org/dtds/partwise.dtd\">\n");

   // previous xml declaration:
   // pline(lev, "<?xml version=\"1.0\" standalone=\"no\"?>\n");
   // pline(lev, "<!DOCTYPE score-partwise PUBLIC \"-//Recordare//DTD MusicXML 0.6a Partwise//EN\" \"//C:/Program Files/Finale 2003/Component Files/partwise.dtd\">\n");

   printGlobalComments(infile, 1);
   // pline(lev, "<score-partwise version="2.0">\n");
   pline(lev, "<score-partwise version=\"2.0\">\n");

   Array<int> kerntracks;
   infile.getTracksByExInterp(kerntracks, "**kern");

   int count = makePartList(infile);

   int start = 0;  // should be the start of exclusive interpretation line  
                   // (but setting to zero).
   int gcount = 0;
   int i;
   if (!reverseQ) {
      // print parts in reverse order because Humdrum scores are printed lowest 
      // part first going towards highest, but MusicXML is from highest to lowest.
      for (i=kerntracks.getSize()-1; i>=0 ;i--) {
         makePart(infile, start, kerntracks[i], ++gcount);
      }
   } else {
      // print reverse because Humdrum score is probably reversed 
      for (i=0; i<kerntracks.getSize(); i++) {
         makePart(infile, start, kerntracks[i], ++gcount);
      }
   }

   lev = 0;

   pline(lev, "</score-partwise>\n");
   printGlobalComments(infile, -1);
   if (count != kerntracks.getSize()) {
      cerr << "Error in generating parts: number of parts has changed: " << count << endl;
   }
}



//////////////////////////////
//
// printGlobalComments
//

void printGlobalComments(HumdrumFile& infile, int direction) {
   int start = 0;
   int stop = infile.getNumLines();
   int i;

   if (direction == 1) {
      start = 0;
      i = 0;
      while (i < infile.getNumLines()) {
         if (strncmp(infile[i][0], "**", 2) == 0) {
            stop = i;
            break;
         }
         i++;
      }
      
   } else {
      stop = infile.getNumLines() - 1;
      i = stop;
      while (i >= 0) {
         if (strcmp(infile[i][0], "*-") == 0) {
            start = i;
            break;
         }
         i--;
      }
   }

   for (i=start; i<=stop; i++) {
      switch (infile[i].getType()) {
         case E_humrec_global_comment:
            printGlobalComment(infile, i);
            break;
         case E_humrec_bibliography:
            printBibliography(infile, i);
            break;
      }

   }

}
 

//////////////////////////////
//
// printGlobalComment --
//

void printGlobalComment(HumdrumFile& infile, int line) {
   pline(lev, "<!-- GCOMMENT value=\"");
   cout << &(infile[line][0][2]) << "\" -->\n";
}



//////////////////////////////
//
// printBibliography --
//

void printBibliography(HumdrumFile& infile, int line) {
   char buffer[1024] = {0};
   strcpy(buffer, infile[line][0]);
   int i = 0;
   while (buffer[i] != '\0') {
      if (buffer[i] == ':') {
         buffer[i] = '\0';
         i++;
         if (buffer[i] == ' ') {
            buffer[i] = '\0';
            i++;
         }
         break;
      }
      i++;
   }
   if (i == 0) return;

   // Handle stupid XML parsers which find "--" within a comment and choke:
   Array<char> newvalue;
   newvalue.setSize(strlen(&(buffer[i]))+1);
   strcpy(newvalue.getBase(), &(buffer[i]));
   PerlRegularExpression pre;
   pre.sar(newvalue, "--", "&#45;&#45;", "g");

   pline(lev, "<!-- INFO key=\"");
   cout << &(buffer[3]) << "\" value=\"" << newvalue << "\" -->\n";

}


//////////////////////////////
//
// makePart -- ggg
//

void makePart(HumdrumFile& infile, int start, int track, int count) {
// spine == track ?
   cout << "\n";
   pline(lev, "<part id=\"P");
   cout << count << "\">\n";
   lev++;
   minit = 0;  
   musicstart = 0;
   AbsTick = 0;

   checkMeasure();
   lev++;
   pline(lev, "<attributes>\n");
   //   attributes = 1;
   lev++;
   pline(lev, "<divisions>");
   cout << divisions << "</divisions>\n";
   lev--;

   pline(lev, "</attributes>\n");  // att
   lev--;                          // att

   int i;

   for (i=0; i<Barlines.getSize()-1; i++) {
      convertMeasureToMusicXML(infile, track, Barlines[i], Barlines[i+1]);
   }

/*
   int i;
   int j;
   int testtrack;
   for (i=start+1; i<infile.getNumLines(); i++) {
      if (infile[i].isComment() || infile[i].isEmpty()) {
         continue;
      }

      for (j=0; j<infile[i].getFieldCount(); j++) {
         testtrack = infile[i].getPrimaryTrack(j);
         if (testtrack != track) {
            continue;
         }
         convertDataToMusicXML(infile, i, j);
      }
   }
*/

   if (measurestate == 1) {
      pline(lev, "</measure>\n");
      measurestate = 0;
   }
   lev--;
   pline(lev, "</part>\n");
}



//////////////////////////////
//
// convertMeasureToMusicXML --
//

void convertMeasureToMusicXML(HumdrumFile& infile, int track, int startbar,
      int endbar) {

   int maxVoice = getMaxVoice(infile, track, startbar, endbar);

//cout << "<!-- Track = " << track << "\tstart = " 
//      << startbar << "\tend = " << endbar 
//      << "\tvoices = " << maxVoice
//      << " -->" << endl;

   int voice;
   for (voice=1; voice<=maxVoice; voice++) {
      convertVoice(infile, track, startbar, endbar, voice);
   }
}


//////////////////////////////
//
// convertVoice --
//

void convertVoice(HumdrumFile& infile, int track, int startbar, int endbar, 
      int voice) {

   int i, j, t, v;

   int start = startbar;
   if (voice > 1) {
      // don't print measure information for secondary voices
      start++;
   }
   
   for (i=start; i<endbar; i++) {
      v = 0;
      for (j=0; j<infile[i].getFieldCount(); j++) {
         t = infile[i].getPrimaryTrack(j);
         if (t != track) { 
            continue;
         }
         v++;
         if (v != voice) {
            continue;
         }
         convertDataToMusicXML(infile, i, j, v);
         break;
      }
   }
}



//////////////////////////////
//
// getMaxVoice -- return the maximum voice for the particular track
//      within the given line range (excluding the endpoints).
//

int getMaxVoice(HumdrumFile& infile, int track, int startline, int endline) {
   int i, j;
   int t;
   int maxv = 0;

   int count;
   for (i=startline+1; i<endline; i++) {
      if (!infile[i].isData()) {
         continue;
      }
      count = 0;
      for (j=0; j<infile[i].getFieldCount(); j++) {
         t = infile[i].getPrimaryTrack(j);
         if (t == track) {
            count++;
         }
      }
      if (count > maxv) {
         maxv = count;
      }
   }

   return maxv;
}



//////////////////////////////
//
// convertDataToMusicXML --
//

void convertDataToMusicXML(HumdrumFile& infile, int line, int col, int voice) {
   if (debugQ) {
      cout << "<!-- PROCESSING  " << infile[line][col] << "-->\n";
   }
   switch(infile[line].getType()) {
      case E_humrec_none:
      case E_humrec_empty:
      case E_humrec_global_comment:
      case E_humrec_bibliography:
      case E_humrec_data_comment:
         break;
      case E_humrec_data_kern_measure:
         convertMeasureToXML(infile, line, col, voice);
         break;
      case E_humrec_interpretation:
         convertAttributeToXML(infile, line, col, voice);
         break;
      case E_humrec_data:
         musicstart = 1;
         convertNoteToXML(infile, line, col, voice);
         break;
      default:
         break;
   }
}

   

//////////////////////////////
//
// updateAccidentals -- restore accidentals at a measure boundary.
// 

void updateAccidentals(void) {
   // copy v1states to v1lastmeasure
   // and fill v1states with v1key
   int i;
   for (i=0; i<7; i++) {
      v1lastmeasure[i] = v1states[i];
      v1states[i] = v1key[i];
   }
}



//////////////////////////////
//
// convertMeasureToXML --
//

void convertMeasureToXML(HumdrumFile& infile, int line, int col, int voice) { 
   int measureno = -1;

   if (!musicstart) {
      return;
   }

   updateAccidentals();
   const char *ptr;

   if (strcmp(infile[line][col], "==") == 0
       || strcmp(infile[line+1][0], "*-") == 0) {
    
      checkbackup(AbsTick, LineTick[line]);

      lev++;
      pline(lev, "<barline>\n"); 
      lev++;
      pline(lev, "<bar-style>light-heavy</bar-style>\n");
      lev--;
      pline(lev, "</barline>\n"); 
      lev--;
      pline(lev, "</measure>\n");
      measurestate = 0;

   } else if ((ptr = strchr(infile[line][col], ':')) != NULL) {
      if ((ptr+1)[0] == '|' || (ptr+1)[0] == '!') {
         lev++;
         pline(lev, "<barline>\n"); 
         lev++;
         if (strstr(infile[line][col], ":|!") != NULL) {
            pline(lev, "<bar-style>light-heavy</bar-style>\n");
         } else if (strstr(infile[line][col], ":||") != NULL) {
            pline(lev, "<bar-style>light-light</bar-style>\n");
         } else if (strstr(infile[line][col], ":!") != NULL) {
            pline(lev, "<bar-style>heavy</bar-style>\n");
         } else if (strstr(infile[line][col], ":|") != NULL) {
            pline(lev, "<bar-style>light</bar-style>\n");
         }
         pline(lev, "<repeat direction=\"backward\"/>\n");
         lev--;
         pline(lev, "</barline>\n"); 
         lev--;
      }

   } else if (strstr(infile[line][col], "||") != NULL) {
      checkbackup(AbsTick, LineTick[line]);

      lev++;
      pline(lev, "<barline>\n"); 
      lev++;
      pline(lev, "<bar-style>light-light</bar-style>\n");
      lev--;
      pline(lev, "</barline>\n"); 
      lev--;

   } 


   if (strcmp(infile[line+1][0], "*-") == 0) {
      return;
   }

   checkbackup(AbsTick, LineTick[line]);

   if (minit != 0) {
      if (measurestate != 0) {
         pline(lev, "</measure>\n");
         measurestate = 0;
      }
   } 
   minit++;

   if (sscanf(infile[line][col], "=%d", &measureno)) {
      pline(lev, "<measure number=\"");
      cout << minit << "\">\n";
      measurestate = 1;
   } else if (strncmp(infile[line][col], "=", 1) == 0) {
      if (infile.getTotalDuration() > infile[line].getAbsBeat()) {
         // don't start a new measure if we are at the end of the music
         pline(lev, "<measure number=\"");
         cout << minit << "\">\n";
         measurestate = 1;
      }
   }

   if ((ptr = strchr(infile[line][col], ':')) != NULL) {
      if ((ptr-1)[0] == '|' || (ptr-1)[0] == '!') {
         lev++;
         pline(lev, "<barline>\n"); 
         lev++;
         if (strstr(infile[line][col], ":|!") != NULL) {
            pline(lev, "<bar-style>light-heavy</bar-style>\n");
         } else if (strstr(infile[line][col], ":||") != NULL) {
            pline(lev, "<bar-style>light-light</bar-style>\n");
         } else if (strstr(infile[line][col], ":!") != NULL) {
            pline(lev, "<bar-style>heavy</bar-style>\n");
         } else if (strstr(infile[line][col], ":|") != NULL) {
            pline(lev, "<bar-style>light</bar-style>\n");
         }
         pline(lev, "<repeat direction=\"forward\"/>\n");
         lev--;
         pline(lev, "</barline>\n"); 
         lev--;
      }
   }
}



//////////////////////////////
//
// checkMeasure -- 
//

void checkMeasure(void) {
   if (minit == 0) {
      minit++;
      pline(lev, "<measure number=\"");
      cout << minit << "\">\n";
      measurestate = 1;
   } 
}


//////////////////////////////
//
// adjustKey -- replace the old key signature with a new one
//

void adjustKey(int keyinfo) {
   int i;
   for (i=0; i<7; i++) {
      v1key[i] = 0;
   }

   if (keyinfo < 0) {
      v1key['B' - 'A'] = -1;
      if (keyinfo < -1) v1key['E' - 'A'] = -1;
      if (keyinfo < -2) v1key['A' - 'A'] = -1;
      if (keyinfo < -3) v1key['D' - 'A'] = -1;
      if (keyinfo < -4) v1key['G' - 'A'] = -1;
      if (keyinfo < -5) v1key['C' - 'A'] = -1;
      if (keyinfo < -6) v1key['F' - 'A'] = -1;
   }

   if (keyinfo > 0) {
      v1key['F' - 'A'] = 1;
      if (keyinfo > 1) v1key['C' - 'A'] = 1;
      if (keyinfo > 2) v1key['G' - 'A'] = 1;
      if (keyinfo > 3) v1key['D' - 'A'] = 1;
      if (keyinfo > 4) v1key['A' - 'A'] = 1;
      if (keyinfo > 5) v1key['E' - 'A'] = 1;
      if (keyinfo > 6) v1key['B' - 'A'] = 1;
   }

   // erase previous measure accidentals:
   for (i=0; i<7; i++) {
      v1lastmeasure[i] = v1key[i];
      v1states[i] = v1key[i];
   }

}





//////////////////////////////
//
// convertAttributeToXML --
//

void convertAttributeToXML(HumdrumFile& infile, int line, int col, int voice) {
   int length = strlen(infile[line][col]);

   if (strncmp(infile[line][col], "*M", 2) == 0 &&
         strchr(infile[line][col], '/') != NULL) {
      // meter marking
      int top = 0;
      int bottom = 0;

      int flag = sscanf(infile[line][col], "*M%d/%d", &top, &bottom);
      if (flag == 2) {
         checkMeasure();
         lev++;
//         if (!attributes) {
            pline(lev, "<attributes>\n");
            lev++;
//          attributes = 1;
//         }
         pline(lev, "<time>\n");
         lev++;

         pline(lev, "<beats>");
         cout << top << "</beats>\n";
         pline(lev, "<beat-type>");
         cout << bottom << "</beat-type>\n";

         lev--;
         pline(lev, "</time>\n");
         lev--;
         pline(lev, "</attributes>\n");  // att
         lev--;                          // att
      }
   } else if (strncmp(infile[line][col], "*k[", 3) == 0 &&
         infile[line][col][length-1] == ']') {
      // key signature
      int pitch = 0;
      if (length > 4) {
         pitch = Convert::kernToBase40(&infile[line][col][length-3]);
         pitch = pitch % 40;
      } else {
         pitch = E_muse_c;
      }
      int keyinfo = 0;
      switch (pitch) {
         case E_muse_c:   keyinfo = 0;   break;
         case E_muse_fs:  keyinfo = 1;   break;
         case E_muse_cs:  keyinfo = 2;   break;
         case E_muse_gs:  keyinfo = 3;   break;
         case E_muse_ds:  keyinfo = 4;   break;
         case E_muse_as:  keyinfo = 5;   break;
         case E_muse_es:  keyinfo = 6;   break;
         case E_muse_bs:  keyinfo = 7;   break;
         case E_muse_bf:  keyinfo = -1;  break;
         case E_muse_ef:  keyinfo = -2;  break;
         case E_muse_af:  keyinfo = -3;  break;
         case E_muse_df:  keyinfo = -4;  break;
         case E_muse_cf:  keyinfo = -5;  break;
         case E_muse_gf:  keyinfo = -6;  break;
         case E_muse_ff:  keyinfo = -7;  break;
      }

      adjustKey(keyinfo);

      checkMeasure();
      lev++;
//      if (!attributes) {
         pline(lev, "<attributes>\n");
         lev++;
//         attributes = 1;
//      }
      pline(lev, "<key>\n");
      lev++;
      pline(lev, "<fifths>");
      cout << keyinfo << "</fifths>\n";
      lev--;
      pline(lev, "</key>\n");
      lev--;
      pline(lev, "</attributes>\n"); // att
      lev--;                         // att

   } else if (strcmp(infile[line][col], "*clefF4") == 0) {
      checkMeasure();
      lev++;
//      if (!attributes) {
         pline(lev, "<attributes>\n");
         lev++;
//         attributes = 1;
//      }
      pline(lev, "<clef>\n");
      lev++;
      pline(lev, "<sign>F</sign>\n");
      pline(lev, "<line>4</line>\n");
      lev--;
      pline(lev, "</clef>\n");
      lev--;
      pline(lev, "</attributes>\n");  // att
      lev--;                          // att

   } else if (strcmp(infile[line][col], "*clefG2") == 0) {
      checkMeasure();
      lev++;
//      if (!attributes) {
         pline(lev, "<attributes>\n");
         lev++;
//         attributes = 1;
//      }
      pline(lev, "<clef>\n");
      lev++;
      pline(lev, "<sign>G</sign>\n");
      pline(lev, "<line>2</line>\n");
      lev--;
      pline(lev, "</clef>\n");
      lev--;
      pline(lev, "</attributes>\n");  // att
      lev--;                          // att
   }

}



//////////////////////////////
//
// convertNoteToXML --
//

void convertNoteToXML(HumdrumFile& infile, int line, int col, int voice) {
   static char buffer[128] = {0};
   int i;

   if (!infile[line].isExInterp(col, "**kern")) {
      return;
   }

   musicstart = 1;

   // copy current accidentals into chord buffer
   for (i=0; i<7; i++) {
      v1chord[i] = v1lastmeasure[i];
      v1prechordstates[i] = v1states[i];
   }

   lev++;

   // double cdur = 0;
   // int xdur = 0;

   int tokencount = infile[line].getTokenCount(col);
   for (i=0; i<tokencount; i++) {
      infile[line].getToken(buffer, col, i);
      // cdur = convertNoteEntryToXML(infile, line, col, buffer, i, 0);
      convertNoteEntryToXML(infile, line, col, buffer, i, 0, voice);
   }

/*
   if ((col+1 < infile[line].getFieldCount()) && 
         (infile[line].getPrimaryTrack(col) == 
         infile[line].getPrimaryTrack(col+1))) {

      xdur = (int)(cdur * divisions);
      // AbsTick += xdur;

      tokencount = infile[line].getTokenCount(col+1);
      for (i=0; i<tokencount; i++) {
         infile[line].getToken(buffer, col+1, i);
         cdur = convertNoteEntryToXML(infile, line, col+1, buffer, i, 1);
      }

      xdur = (int)(cdur * divisions) - 
            (int)(divisions * infile[line].getDuration());
      // AbsTick += xdur;
   }
*/

   lev--;
}



//////////////////////////////
//
// checkbackup -- 
//

void checkbackup (int currenttick, int targettick) {

   int direction = currenttick - targettick;

   if (direction > 0) {
      // backwards
      pline(lev, "<backup>\n");
      lev++;
      pline(lev, "<duration>");
      cout << direction << "</duration>\n";
      lev--;
      pline(lev, "</backup>\n");
   } else if (direction < 0) {
      // forwards
      pline(lev, "<forward>\n");
      lev++;
      pline(lev, "<duration>");
      cout << -direction << "</duration>\n";
      lev--;
      pline(lev, "</forward>\n");
   }

   AbsTick = targettick;
}



//////////////////////////////
//
// convertNoteEntryToXML --
//

double convertNoteEntryToXML(HumdrumFile& infile, int line, int col,
      const char* buffer, int chord, int vlevel, int voice) {

   RationalNumber durationR = Convert::kernToDurationR(buffer);

   if (strcmp(buffer, ".") == 0) {
      // nothing to do
      return 0.0;
   }

   if ((strstr(buffer, "yy") != NULL) && (strchr(buffer, 'r') != NULL)) {
      if (!chord) {
         checkbackup(AbsTick, LineTick[line]);
      }
      // invisible rest: generate a <forward> direction
      RationalNumber rduration = durationR * divisions;
      int tickdur = (int)rduration.getFloat();
      AbsTick += tickdur;
      pline(lev, "<forward>\n");
      pline(lev+1, "<duration>");
      cout << tickdur  << "</duration>\n";
      pline(lev, "</forward>\n");
      return durationR.getFloat();
   }
 
   double output = 0;

   int explicitz = 0;
   int altered = 0;
   int pitch = Convert::kernToBase40(buffer);
   char buff2[64] = {0};
   if (pitch > 0) {
      Convert::base40ToKern(buff2, pitch);
   } else {
      strcpy(buff2, "r");
   }

   if (strstr(buff2, "--") != NULL) {
   } else if (strstr(buff2, "--") != NULL) {
      altered = -2;
   } else if (strstr(buff2, "##") != NULL) {
      altered = 2;
   } else if (strstr(buff2, "-") != NULL) {
      altered = -1;
   } else if (strstr(buff2, "#") != NULL) {
      altered = 1;
   } else if (strstr(buffer, "n") != NULL) {
      altered = 0;
      explicitz = 1;
   } else {
      altered = 0;
      explicitz = 0;
   }
   char diapitch = toupper(buff2[0]);
   int octave = 0;
   if (pitch > 0) {
      octave = pitch / 40;
   }

   if (debugQ) {
      cout << "+++ processing note token: " << buffer << endl;
   }

//   if (attributes) {
//      lev--;
//      pline(lev, "</attributes>\n");
//      lev--;   // att
//      attributes = 0;
//   }

   if (!chord) {
      checkbackup(AbsTick, LineTick[line]);
   }

   pline(lev, "<note>\n");
   lev++;

   int grace = 0;
   if (strchr(buffer, 'q') != NULL || strchr(buffer, 'Q') != NULL) {
      grace = 1;
   }
   if (grace) {
      pline(lev, "<grace/>\n");
   }

   if (chord) {
      pline(lev, "<chord/>\n");
   }
   if (toupper(diapitch) != 'R') {
      pline(lev, "<pitch>\n");
      lev++;
      pline(lev, "<step>");
      cout << diapitch << "</step>\n";
      if (altered != 0) {
         pline(lev, "<alter>");
         cout << altered << "</alter>\n";
      }
      pline(lev, "<octave>");
      cout << octave << "</octave>\n";
      lev--;
      pline(lev, "</pitch>\n");
   } else {
      pline(lev, "<rest/>\n");
   }
   if (!grace) {
      pline(lev, "<duration>");
      RationalNumber ratnum;
      ratnum = durationR * divisions;
      int value = (int)ratnum.getFloat();
      cout << value << "</duration>\n";
      if (!chord) {
         // don't keep track of chord notes, and presume first note of 
         // chord has correct duration for entire chord.
         AbsTick += value;
      }
      output = durationR.getFloat();
   } else {
      output = 0.0;
   }

   int tietype = 0;
   if (strchr(buffer, '[') != NULL) {
      tietype = 1;
   } else if (strchr(buffer, '_') != NULL) {
      tietype = 2;
   } else if (strchr(buffer, ']') != NULL) {
      tietype = 3;
   }
   switch (tietype) {
      case 1:
         pline(lev, "<tie type=\"start\"/>\n");
         break;
      case 2:
         pline(lev, "<tie type=\"stop\"/>\n");
         pline(lev, "<tie type=\"start\"/>\n");
         break;
      case 3:
         pline(lev, "<tie type=\"stop\"/>\n");
         break;
   }

   pline(lev, "<voice>");
   cout << voice << "</voice>\n";
   int vcase = voice;

/*
   int vcase = 0;
   if (strchr(infile[line].getSpineInfo(col), '(') == NULL) {
      vcase = 0;
      pline(lev, "<voice>1</voice>\n");
      pline(lev, "<type>");
   } else if (strchr(infile[line].getSpineInfo(col), 'a') != NULL) {
      vcase = 1;
      pline(lev, "<voice>1</voice>\n");
      pline(lev, "<type>");
   } else if (strchr(infile[line].getSpineInfo(col), 'b') != NULL) {
      vcase = 2;
      pline(lev, "<voice>2</voice>\n");
      pline(lev, "<type>");
   } else {
      vcase = 3;
      pline(lev, "<voice>1</voice>\n");
      pline(lev, "<type>");
   }
*/

   pline(lev, "<type>");

   char durstring[32] = {0};
   PerlRegularExpression pre;
   Array<char> newbuffer;
   newbuffer.setSize(strlen(buffer)+1);
   strcpy(newbuffer.getBase(), buffer);

   Array<char> nodots;
   nodots.setSize(strlen(newbuffer.getBase()) + 1);
   strcpy(nodots.getBase(), newbuffer.getBase());
   PerlRegularExpression removedots;
   removedots.sar(nodots, "\\.", "", "g");

   pre.sar(newbuffer, "q", "", "i");
   if (grace) {
      // if a grace note and no rhythm, set the visual duration to 8th
      if (!pre.search(newbuffer, "\\d")) {
         newbuffer.setSize(2);
         strcpy(newbuffer.getBase(), "8");
      }
   }

   // have to ignore augmentation dots on rhythm?
   RationalNumber rat = Convert::kernToDurationR(nodots.getBase());
   double newduration = rat.getFloat();
   RationalNumber ratout = 1;

   if (newduration > 0.0) {
      double tempval =  log10(newduration)/log10(2);
      if (tempval > 0.0) {
         tempval = int(tempval);
         ratout *= (int)pow(2.0,tempval);
      } else {
         tempval = -int(-tempval);
         ratout /= (int)pow(2.0,-tempval);
      }
      newduration = pow(2.0,tempval);
      Convert::durationToKernRhythm(durstring, newduration);
      printDurationType(durstring);
   } else {
      printDurationType(nodots.getBase());
   }

   cout << "</type>\n";

   // print number of augmentation dots in duration ///////////////////
   // printDots(durstring);
   printDots(newbuffer.getBase());


   /// WRITTEN ACCIDENTALS ////////////////////////////////////////////

   if (toupper(buff2[0]) != 'R') {
      if (strchr(buffer, 'n') != NULL) {
         pline(lev, "<accidental>natural</accidental>\n");
         v1states[toupper(buff2[0]) - 'A'] = 0;
         v1lastmeasure[toupper(buff2[0]) - 'A'] = 0;
      } else {
         checkAccidentals(toupper(buff2[0]), altered, chord);
      }
   }

   // Time modification (tuplet indications) //////////////////////////
   if (!grace) {  // don't do <time-modification> if a grace note
      RationalNumber timemod = rat / ratout;
      if (timemod.getDenominator() != 1) {
         pline(lev, "<time-modification>\n");
         pline(lev+1, "<actual-notes>");
         cout << timemod.getDenominator() << "</actual-notes>\n";
         pline(lev+1, "<normal-notes>");
         cout << timemod.getNumerator() << "</normal-notes>\n";
         pline(lev, "</time-modification>\n");
      }
   }
    
   // Stem direction
   if (strchr(buffer, '/') != NULL) {
      pline(lev, "<stem>up</stem>\n");
   } else if (strchr(buffer, '\\') != NULL) {
      pline(lev, "<stem>down</stem>\n");
   } else if (vcase == 1) {
      pline(lev, "<stem>down</stem>\n");
   } else if (vcase == 2) {
      if (strchr(buffer, '/') == NULL && strchr(buffer, '\\') == NULL) {
         pline(lev, "<stem>up</stem>\n");
      }
   }

   if (durationR.getFloat() >= 1.0) {
      // kill rogue beam marks.
      beamlevel[vlevel] = 0;
   }
   if (!chord) {
      // processBeams(infile, line, col, buffer, vlevel);
      processBeams(infile, line, col, infile[line][col], vlevel);
   }

   /// WRITTEN TIES ///////////////////////////////////////////////////

   if (tietype) {
      pline(lev, "<notations>\n");
      lev++;
      switch (tietype) {
         case 1:
            pline(lev, "<tied type=\"start\"/>\n");
            break;
         case 2:
            pline(lev, "<tied type=\"stop\"/>\n");
            pline(lev, "<tied type=\"start\"/>\n");
            break;
         case 3:
            pline(lev, "<tied type=\"stop\"/>\n");
            break;
      }

      lev--;
      pline(lev, "</notations>\n");
   }

   /// FERMATAS ///////////////////////////////////////////////////////
 
   if (strchr(buffer, ';') != NULL) {
      pline(lev, "<notations>\n");
      lev++;
      pline(lev, "<fermata type=\"upright\"/>\n"); 
      lev--;
      pline(lev, "</notations>\n");
   }

   /// ARTICULATIONS ////////////////////////////////////////////
 
   if (strchr(buffer, '\'') != NULL) {
      pline(lev, "<notations>\n");
      lev++;
      pline(lev, "<articulations>\n");
      lev++;
      pline(lev, "<staccato placement=\"above\"/>\n"); 
      lev--;
      pline(lev, "</articulations>\n");
      lev--;
      pline(lev, "</notations>\n");
   }
 
   if (strchr(buffer, '~') != NULL) {
      pline(lev, "<notations>\n");
      lev++;
      pline(lev, "<articulations>\n");
      lev++;
      pline(lev, "<tenuto placement=\"above\"/>\n"); 
      lev--;
      pline(lev, "</articulations>\n");
      lev--;
      pline(lev, "</notations>\n");
   }
 
   if (strchr(buffer, '^') != NULL) {
      pline(lev, "<notations>\n");
      lev++;
      pline(lev, "<articulations>\n");
      lev++;
      pline(lev, "<accent placement=\"above\"/>\n"); 
      lev--;
      pline(lev, "</articulations>\n");
      lev--;
      pline(lev, "</notations>\n");
   }


   /// LYRICS /////////////////////////////////////////////////////////

   if (!chord) {
      processTextUnderlay(infile, line, col);
   }

   lev--;
   pline(lev, "</note>\n");
   return output;
}



//////////////////////////////
//
// processBeams --
//

void processBeams(HumdrumFile& infile, int line, int col, const char* buffer,
      int vlevel) {

   int backhook = 0;
   int forehook = 0;
   int openbeam = 0;
   int closebeam = 0;

   int i=0; 
   int sum = 0;
   int length = strlen(buffer);
   for (i=0; i<length; i++) {
      switch (buffer[i]) {
         case 'k': backhook++;  sum++; break;
         case 'K': forehook++;  sum++; break;
         case 'L': openbeam++;  sum++; break;
         case 'J': closebeam++; sum++; break;
      }
   }
   if (closebeam && openbeam) {
      openbeam = 0;
   }

   if (forehook && backhook) {
      if (openbeam) {
         backhook = 0;
      } else {
         forehook = 0;
      }
   }

   if (sum == 0) {
      // there is nothing new to do
      for (i=0; i<beamlevel[vlevel]; i++) {
         pline(lev, "<beam number=\"");
         cout << i+1 << "\">continue</beam>\n";
      }
      return;
   }

   // int totalcount = beamlevel[vlevel] + openbeam - closebeam + 
   //       backhook + forehook;
   int rcount = 0;

   if (openbeam != 0) {
      // add any new beams which are starting
 
      // continue any old beams
      for (i=0; i<beamlevel[vlevel]; i++) {
         pline(lev, "<beam number=\"");
         cout << i+1 << "\">continue</beam>\n";
         rcount++;
      }
   
      // add new beams
      for (i=0; i<openbeam; i++) {
         pline(lev, "<beam number=\"");
         cout << beamlevel[vlevel] + i + 1 << "\">begin</beam>\n";
         rcount++;
      }
      beamlevel[vlevel] += openbeam;

      // add any opening beam hook
      for (i=0; i<forehook; i++) {
         pline(lev, "<beam number=\"");
         cout << beamlevel[vlevel] + i + 1 << "\">forward hook</beam>\n";
         rcount++;
      }

   }

   if (closebeam != 0) {
      // close any old beams which are ending

      // continue any old beams
      for (i=0; i<beamlevel[vlevel] - closebeam - backhook; i++) {
         pline(lev, "<beam number=\"");
         cout << i+1 << "\">continue</beam>\n";
         rcount++;
      }
   
      // close old beams
      for (i=0; i<closebeam; i++) {
         // some strange problem required this following if statement to preven end=0
         if ((beamlevel[vlevel] - closebeam + i + 1) > 0) {
            pline(lev, "<beam number=\"");
            cout << beamlevel[vlevel] - closebeam + i + 1 << "\">end</beam>\n";
            rcount++;
         }
      }
      beamlevel[vlevel] -= closebeam;

      // add any ending beam hook
      for (i=0; i<backhook; i++) {
         pline(lev, "<beam number=\"");
         cout << beamlevel[vlevel] + closebeam + i + 1 
              << "\">backward hook</beam>\n";
         rcount++;
      }
   
   }

   if (beamlevel[vlevel] < 0) {
      beamlevel[vlevel] = 0;
   }

}




//////////////////////////////
//
// processTextUnderlay --
//

void processTextUnderlay(HumdrumFile& infile, int line, int col) {
   int fields = infile[line].getFieldCount();
   if (col >= fields-1) {
      return;
   }

   int tcol = col+1;
   int verse = 1;
   while (tcol < fields && 
         strcmp(infile[line].getExInterp(tcol), "**kern") != 0) {
      if (strcmp(infile[line][tcol], ".") == 0) {
         // ignore null tokens
      } else if (strcmp(infile[line].getExInterp(tcol), "**text") == 0) {
         displayLyrics(infile, line, tcol, verse);
      } else {
         displayUnknownTextType(infile, line, tcol, verse);
      }

      tcol++;
      verse++;
   }

}



//////////////////////////////
//
// displayLyrics --
//

void displayLyrics(HumdrumFile& infile, int line, int col, int verse) {
   displayUnknownTextType(infile, line, col, verse);
}



//////////////////////////////
//
// displayUnknownTextType --
//

void displayUnknownTextType(HumdrumFile& infile, int line, int col, 
      int verse) {

   pline(lev, "<lyric number=\"");
   cout << verse << "\" name=\"verse\">\n";
   lev++;
   pline(lev, "<syllabic>single</syllabic>\n");
   pline(lev, "<text>");

   displayHTMLText(infile[line][col]);

   cout << "</text>\n";
   lev--;
   pline(lev, "</lyric>\n");
}



//////////////////////////////
//
// displayHTMLText --
//

void displayHTMLText(const char* buffer) {
   int length = strlen(buffer);

   for (int i=0; i<length; i++) {
      switch (buffer[i]) {
         case 'ä':  cout << "&auml;"; break;
         case 'ë':  cout << "&euml;"; break;
         case 'ï':  cout << "&iuml;"; break;
         case 'ö':  cout << "&ouml;"; break;
         case 'ü':  cout << "&uuml;"; break;
         case '<':  cout << "&lt;"; break;
         case '>':  cout << "&gt;"; break;
         default:   cout << buffer[i];
      }
   }
}



//////////////////////////////
//
// checkAccidentals --
//

void checkAccidentals(int diatonic, int alter, int chord) {
   // if a note is different than the current state
   // in the measure, then add an accidental to make
   // sure that the note is played right

   if (toupper(diatonic) == 'R') {
      return;
   }

   if (!chord) {

      diatonic = toupper(diatonic);

      if (v1states[diatonic - 'A'] != alter) {
   
         pline(lev, "<accidental>");
         switch (alter) {
            case 2:  cout << "double-sharp"; break;
            case 1:  cout << "sharp"; break;
            case 0:  cout << "natural"; break;
            case -1:  cout << "flat"; break;
            case -2:  cout << "double-flat"; break;
         }
         cout << "</accidental>\n";
         v1states[diatonic - 'A'] = alter;
         v1lastmeasure[diatonic - 'A'] = alter;
   
      } else if (cautionaryQ && v1lastmeasure[diatonic - 'A'] != alter) {
         // if a cautionary accidental accross measure, then adjust here
   
         pline(lev, "<accidental>");
         switch (alter) {
            case 2:  cout << "double-sharp"; break;
            case 1:  cout << "sharp"; break;
            case 0:  cout << "natural"; break;
            case -1:  cout << "flat"; break;
            case -2:  cout << "double-flat"; break;
         }
         cout << "</accidental>\n";
         v1states[diatonic - 'A'] = alter;
         v1lastmeasure[diatonic - 'A'] = alter;
      }

   } else {
      // note in a chord

      diatonic = toupper(diatonic);
      if (v1prechordstates[diatonic - 'A'] != alter) {
   
         pline(lev, "<accidental>");
         switch (alter) {
            case 2:  cout << "double-sharp"; break;
            case 1:  cout << "sharp"; break;
            case 0:  cout << "natural"; break;
            case -1:  cout << "flat"; break;
            case -2:  cout << "double-flat"; break;
         }
         cout << "</accidental>\n";
         v1states[diatonic - 'A'] = alter;
         v1lastmeasure[diatonic - 'A'] = alter;
   
      } else if (cautionaryQ && v1chord[diatonic - 'A'] != alter) {
         // if a cautionary accidental accross measure, then adjust here
   
         pline(lev, "<accidental>");
         switch (alter) {
            case 2:  cout << "double-sharp"; break;
            case 1:  cout << "sharp"; break;
            case 0:  cout << "natural"; break;
            case -1:  cout << "flat"; break;
            case -2:  cout << "double-flat"; break;
         }
         cout << "</accidental>\n";
         v1states[diatonic - 'A'] = alter;
         v1lastmeasure[diatonic - 'A'] = alter;
      }

   }


}



//////////////////////////////
//
// printDots --
//

void printDots(const char* durstring) {
   int length = strlen(durstring);
   int i;
   for (i=0; i<length; i++) {
      if (durstring[i] == '.') {
         pline(lev, "<dot/>\n");
      }
   }
}



//////////////////////////////
//
// printDurationType --
//

void printDurationType(const char* durstring) {
   char buffer[32] = {0};
   int length = strlen(durstring);
   strcpy(buffer, durstring);
   int counter = 0;
   for (int i=0; i<length; i++) {
      if (buffer[i] == '0') {
         counter++;
      } else if (isdigit(buffer[i])) {
         counter = -100;
      }
      if (buffer[i] == '.') {
         buffer[i] = '\0';
      }
   }

   int number = atoi(buffer);
   if (counter == 2) {
      cout << "long";
   } else if (number == 0) {
      cout << "breve";
   } else if (number <= 1) {
      cout << "whole";
   } else if (number <= 2) {
      cout << "half";
   } else if (number <= 4) {
      cout << "quarter";
   } else if (number <= 8) {
      cout << "eighth";
   } else if (number <= 16) {
      cout << "16th";
   } else if (number <= 32) {
      cout << "32nd";
   } else if (number <= 64) {
      cout << "64th";
   } else if (number <= 128) {
      cout << "128th";
   } else if (number <= 256) {
      cout << "256th";
   } else if (number <= 512) {
      cout << "512th";
   } else if (number <= 1024) {
      cout << "1024th";
   } else if (number <= 2048) {
      cout << "2048th";
   } else if (number <= 4096) {
      cout << "4096th";
   } else {
      cout << "UNKNOWN";
   }
}



//////////////////////////////
//
// makePartList -- generate a list of the parts in the score
//

int makePartList(HumdrumFile& infile) {
   lev++;
   pline(lev, "<part-list>\n");
   lev++;
   
   // find the start of the spine data and output a part for each
   // spine
   int i = 0;
   int count = 0;   // the number of **kern spines
   int j = 0;
   while (i<infile.getNumLines() && infile[i].getType() != 
         E_humrec_interpretation) {
      i++;
   }
   if (infile[i].getType() == E_humrec_interpretation) {
      if (strncmp("**", infile[i][0], 2) != 0) {
         cout << "Error on line " << i + 1 << " of file: No start of data" 
              << endl;
      }
   } else {
      cout << "Error: no data in file" << endl;
      exit(1);
   }

   if (reverseQ) {
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strcmp(infile[i].getExInterp(j), "**kern") != 0) {
            continue;
         }
         count++;
         generatePartInfo(infile, i, j, count);
      }
   } else {
      // doing parts in reverse order
      for (j=infile[i].getFieldCount()-1; j>=0; j--) {
         if (strcmp(infile[i].getExInterp(j), "**kern") != 0) {
            continue;
         }
         count++;
         generatePartInfo(infile, i, j, count);
      }
   }

   lev--;
   pline(lev, "</part-list>\n");

   return count;
}



//////////////////////////////
//
// generatePart --
//

void generatePartInfo(HumdrumFile& infile, int start, int col, int count) {
   int i = start + 1;
   int j = 0;
   pline(lev, "<score-part id=\"P");
   cout << count << "\">\n";
   lev++;

   int done = 0;
   while (!done && i < infile.getNumLines()) {
      if (infile[i].getType() == E_humrec_interpretation) {
         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (infile[i].getPrimaryTrack(j) == col + 1) {
               if (strncmp(infile[i][j], "*I", 2) == 0 &&
                   strncmp(infile[i][j], "*IC", 3) != 0) {
                  done = 1;
                  break;
               }
            } else {
            }
         }
      }
      if (done) break;
      i++;
   }

   if (done == 1) {
      pline(lev, "<part-name>");
      cout << &(infile[i][j][2]) << "</part-name>\n";
   } else {
      pline(lev, "<part-name>XPart ");
      cout << col << "</part-name>\n";
   }
 
   lev--;
   pline(lev, "</score-part>\n");
}



//////////////////////////////
//
// pline -- print a line of data
//

void pline(int level, const char* string) {
   for (int i=0; i<level; i++) {
      cout << '\t';
   }
   cout << string;
} 



//////////////////////////////
//
// usage --
//

void usage(const char* command) {

}




// md5sum: 8e93554da88296920e7179282a173cfb hum2xml.cpp [20120727]

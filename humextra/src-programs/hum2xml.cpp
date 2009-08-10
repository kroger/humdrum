//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun  5 13:48:27 PDT 2001
// Last Modified: Tue Jun  5 13:48:29 PDT 2001
// Last Modified: Fri Feb 25 00:09:19 PST 2005 (balanced <measure> markers)
// Last Modified: Tue Dec 12 20:23:42 PST 2006 (handling > and < as text)
// Filename:      ...sig/examples/all/hum2xml.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/hum2xml.cpp
// Syntax:        C++; museinfo
//
// Description:   Converts a Humdrum file into MusicXML.
//

#include "humdrum.h"

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
void      convertToMusicXML     (HumdrumFile& hfile);
int       makePartList          (HumdrumFile& hfile);
void      makePart              (HumdrumFile& hfile, int start, int spine,
                                 int count);
void      generatePartInfo      (HumdrumFile& hfile, int start, int spine,
                                 int count);
void      convertDataToMusicXML (HumdrumFile& hfile, int line, int trace);
void      convertMeasureToXML   (HumdrumFile& hfile, int line, int spine);
void      convertAttributeToXML (HumdrumFile& hfile, int line, int spine);
void      convertNoteToXML      (HumdrumFile& hfile, int line, int spine);
void      pline                 (int level, const char* string);
void      usage                 (const char* command);
double    convertNoteEntryToXML (HumdrumFile& hfile, int line, int spine,
                                 const char* buffer, int chord, int vlevel);
void      checkMeasure          (void);
void      printDurationType     (const char* durstring);
void      printDots             (const char* durstring);
void      adjustKey             (int keyinfo);
void      checkAccidentals      (int diatonic, int alter, int chord);
void      updateAccidentals     (void);
void      printGlobalComments   (HumdrumFile& hfile, int direction);
void      printGlobalComment    (HumdrumFile& hfile, int line);
void      printBibliography     (HumdrumFile& hfile, int line);
void      checkbackup           (void);
void      processTextUnderlay   (HumdrumFile& hfile, int line, int spine);
void      displayUnknownTextType(HumdrumFile& hfile, int line, int spine, 
                                 int verse);
void      displayLyrics         (HumdrumFile& hfile, int line, int spine, 
                                 int verse);
void      displayHTMLText       (const char* buffer);
void      processBeams          (HumdrumFile& hfile, int line, int spine, 
                                 const char* buffer, int vlevel);

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

int direction = 0;              // for <backup> <forward> commands
int beamlevel[128] = {0};       // for beaming information
int musicstart = 0;             // for suppressing bars before music starts
// int attributes = 0;             // for suppressing multiple attributes entries


//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   HumdrumFile hfile;
   for (int i=0; i<numinputs || i==0; i++) {
      hfile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         hfile.read(cin);
      } else {
         hfile.read(options.getArg(i+1));
      }

      hfile.analyzeRhythm();
      divisions = hfile.getMinTimeBase();
      if (divisions % 4 == 0) {
         divisions = divisions/4;
      } else {
         // don't know what this case may be
      }
      convertToMusicXML(hfile);

   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////


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

void convertToMusicXML(HumdrumFile& hfile) {
   pline(lev, "<?xml version=\"1.0\" standalone=\"no\"?>\n");
   pline(lev, "<!DOCTYPE score-partwise PUBLIC \"-//Recordare//DTD MusicXML 0.6a Partwise//EN\" \"//C:/Program Files/Finale 2003/Component Files/partwise.dtd\">\n");

   printGlobalComments(hfile, 1);
   pline(lev, "<score-partwise>\n");


   int count = makePartList(hfile);
   int start = 0;
   while (start < hfile.getNumLines() && 
         strncmp(hfile[start][0], "**", 2) != 0) {
      start++;
   }

   int i;
   int gcount = 0;
   if (!reverseQ) {
      for (i=hfile[start].getFieldCount()-1; i>=0; i--) {
         if (strcmp(hfile[start].getExInterp(i), "**kern") != 0) {
            continue;
         }
         gcount++;
         makePart(hfile, start, i, gcount);
         count--;
      }
   } else {
      // doing things in reverse order
      for (i=0; i<hfile[start].getFieldCount(); i++) {
         if (strcmp(hfile[start].getExInterp(i), "**kern") != 0) {
            continue;
         }
         gcount++;
         makePart(hfile, start, i, gcount);
         count--;
      }
   }

   lev = 0;

   pline(lev, "</score-partwise>\n");
   printGlobalComments(hfile, -1);
   if (count != 0) {
      cerr << "Error in generating parts: number of parts has changed" << endl;
   }
}


//////////////////////////////
//
// printGlobalComments
//

void printGlobalComments(HumdrumFile& hfile, int direction) {
   int start = 0;
   int stop = hfile.getNumLines();
   int i;

   if (direction == 1) {
      start = 0;
      i = 0;
      while (i < hfile.getNumLines()) {
         if (strncmp(hfile[i][0], "**", 2) == 0) {
            stop = i;
            break;
         }
         i++;
      }
      
   } else {
      stop = hfile.getNumLines() - 1;
      i = stop;
      while (i >= 0) {
         if (strcmp(hfile[i][0], "*-") == 0) {
            start = i;
            break;
         }
         i--;
      }
   }

   for (i=start; i<=stop; i++) {
      switch (hfile[i].getType()) {
         case E_humrec_global_comment:
            printGlobalComment(hfile, i);
            break;
         case E_humrec_bibliography:
            printBibliography(hfile, i);
            break;
      }

   }

}
 

//////////////////////////////
//
// printGlobalComment --
//

void printGlobalComment(HumdrumFile& hfile, int line) {
   pline(lev, "<!-- GCOMMENT value=\"");
   cout << &(hfile[line][0][2]) << "\" -->\n";
}



//////////////////////////////
//
// printBibliography --
//

void printBibliography(HumdrumFile& hfile, int line) {
   char buffer[1024] = {0};
   strcpy(buffer, hfile[line][0]);
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

   pline(lev, "<!-- INFO key=\"");
   cout << &(buffer[3]) << "\" value=\"" << &(buffer[i]) << "\" -->\n";

}


//////////////////////////////
//
// makePart --
//

void makePart(HumdrumFile& hfile, int start, int spine, int count) {
   cout << "\n";
   pline(lev, "<part id=\"P");
   cout << count << "\">\n";
   lev++;
   minit = 0;  
   musicstart = 0;

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

   int line = 0;
   int trace = 0;
   for (line = start+1; line < hfile.getNumLines(); line++) {
      trace = 0;
      if (strncmp(hfile[line][0], "!!", 2) == 0) {
         continue;
      }
      if (hfile[line].getType() == E_humrec_empty) {
         continue;
      }
      while (trace < hfile[line].getFieldCount() && 
            spine+1 != hfile[line].getPrimaryTrack(trace)) {
         trace++;
      }

      if (strcmp(hfile[line].getExInterp(trace), "**kern") != 0) {
         continue;
      }

      if (trace < hfile[line].getFieldCount()) {
         convertDataToMusicXML(hfile, line, trace);
      } else {
         cout << "Error on line " << line+1 << ": Spine error\n";
      }
   }

   if (measurestate == 1) {
      pline(lev, "</measure>\n");
      measurestate = 0;
   }
   lev--;
   pline(lev, "</part>\n");
}



//////////////////////////////
//
// convertDataToMusicXML --
//

void convertDataToMusicXML(HumdrumFile& hfile, int line, int trace) {
   if (debugQ) {
      cout << "DATA = " << hfile[line][trace] << " \n";
   }
   switch(hfile[line].getType()) {
      case E_humrec_none:
      case E_humrec_empty:
      case E_humrec_global_comment:
      case E_humrec_bibliography:
      case E_humrec_data_comment:
         break;
      case E_humrec_data_kern_measure:
         convertMeasureToXML(hfile, line, trace);
         break;
      case E_humrec_interpretation:
         convertAttributeToXML(hfile, line, trace);
         break;
      case E_humrec_data:
         musicstart = 1;
         convertNoteToXML(hfile, line, trace);
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

void convertMeasureToXML(HumdrumFile& hfile, int line, int spine) { 
   int measureno = -1;

   if (!musicstart) {
      return;
   }

   updateAccidentals();
   const char *ptr;

   if (strcmp(hfile[line][spine], "==") == 0
       || strcmp(hfile[line+1][0], "*-") == 0) {
    
      checkbackup();

      lev++;
      pline(lev, "<barline>\n"); 
      lev++;
      pline(lev, "<bar-style>light-heavy</bar-style>\n");
      lev--;
      pline(lev, "</barline>\n"); 
      lev--;
      pline(lev, "</measure>\n");
      measurestate = 0;

   } else if ((ptr = strchr(hfile[line][spine], ':')) != NULL) {
      if ((ptr+1)[0] == '|' || (ptr+1)[0] == '!') {
         lev++;
         pline(lev, "<barline>\n"); 
         lev++;
         if (strstr(hfile[line][spine], ":|!") != NULL) {
            pline(lev, "<bar-style>light-heavy</bar-style>\n");
         } else if (strstr(hfile[line][spine], ":||") != NULL) {
            pline(lev, "<bar-style>light-light</bar-style>\n");
         } else if (strstr(hfile[line][spine], ":!") != NULL) {
            pline(lev, "<bar-style>heavy</bar-style>\n");
         } else if (strstr(hfile[line][spine], ":|") != NULL) {
            pline(lev, "<bar-style>light</bar-style>\n");
         }
         pline(lev, "<repeat direction=\"backward\"/>\n");
         lev--;
         pline(lev, "</barline>\n"); 
         lev--;
      }

   } else if (strstr(hfile[line][spine], "||") != NULL) {
      checkbackup();

      lev++;
      pline(lev, "<barline>\n"); 
      lev++;
      pline(lev, "<bar-style>light-light</bar-style>\n");
      lev--;
      pline(lev, "</barline>\n"); 
      lev--;

   } 


   if (strcmp(hfile[line+1][0], "*-") == 0) {
      return;
   }

   checkbackup();

   if (minit != 0) {
      if (measurestate != 0) {
         pline(lev, "</measure>\n");
         measurestate = 0;
      }
   } 
   minit++;

   if (sscanf(hfile[line][spine], "=%d", &measureno)) {
      pline(lev, "<measure number=\"");
      cout << minit << "\">\n";
      measurestate = 1;
   } else if (strncmp(hfile[line][spine], "=", 1) == 0) {
      if (hfile.getTotalDuration() > hfile[line].getAbsBeat()) {
         // don't start a new measure if we are at the end of the music
         pline(lev, "<measure number=\"");
         cout << minit << "\">\n";
         measurestate = 1;
      }
   }

   if ((ptr = strchr(hfile[line][spine], ':')) != NULL) {
      if ((ptr-1)[0] == '|' || (ptr-1)[0] == '!') {
         lev++;
         pline(lev, "<barline>\n"); 
         lev++;
         if (strstr(hfile[line][spine], ":|!") != NULL) {
            pline(lev, "<bar-style>light-heavy</bar-style>\n");
         } else if (strstr(hfile[line][spine], ":||") != NULL) {
            pline(lev, "<bar-style>light-light</bar-style>\n");
         } else if (strstr(hfile[line][spine], ":!") != NULL) {
            pline(lev, "<bar-style>heavy</bar-style>\n");
         } else if (strstr(hfile[line][spine], ":|") != NULL) {
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

void convertAttributeToXML(HumdrumFile& hfile, int line, int spine) {
   int length = strlen(hfile[line][spine]);

   if (strncmp(hfile[line][spine], "*M", 2) == 0 &&
         strchr(hfile[line][spine], '/') != NULL) {
      // meter marking
      int top = 0;
      int bottom = 0;

      int flag = sscanf(hfile[line][spine], "*M%d/%d", &top, &bottom);
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
   } else if (strncmp(hfile[line][spine], "*k[", 3) == 0 &&
         hfile[line][spine][length-1] == ']') {
      // key signature
      int pitch = 0;
      if (length > 4) {
         pitch = Convert::kernToBase40(&hfile[line][spine][length-3]);
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

   } else if (strcmp(hfile[line][spine], "*clefF4") == 0) {
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

   } else if (strcmp(hfile[line][spine], "*clefG2") == 0) {
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

void convertNoteToXML(HumdrumFile& hfile, int line, int spine) {
   static char buffer[128] = {0};
   int i;

   if (strcmp(hfile[line].getExInterp(spine), "**kern") != 0) {
      return;
   }

   musicstart = 1;

   // copy current accidentals into chord buffer
   for (i=0; i<7; i++) {
      v1chord[i] = v1lastmeasure[i];
      v1prechordstates[i] = v1states[i];
   }

   lev++;

   double cdur = 0;
   int xdur = 0;

   int tokencount = hfile[line].getTokenCount(spine);
   for (i=0; i<tokencount; i++) {
      hfile[line].getToken(buffer, spine, i);
      cdur = convertNoteEntryToXML(hfile, line, spine, buffer, i, 0);
   }

   if ((spine+1 < hfile[line].getFieldCount()) && 
         (hfile[line].getPrimaryTrack(spine) == 
         hfile[line].getPrimaryTrack(spine+1))) {

      xdur = (int)(cdur * divisions);
      direction = xdur;

      tokencount = hfile[line].getTokenCount(spine+1);
      for (i=0; i<tokencount; i++) {
         hfile[line].getToken(buffer, spine+1, i);
         cdur = convertNoteEntryToXML(hfile, line, spine+1, buffer, i, 1);
      }

      xdur = (int)(cdur * divisions) - 
            (int)(divisions * hfile[line].getDuration());
      direction += xdur;
   }

   lev--;
}



//////////////////////////////
//
// checkbackup -- 
//

void checkbackup (void) {

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

   direction = 0;

}



//////////////////////////////
//
// convertNoteEntryToXML --
//

double convertNoteEntryToXML(HumdrumFile& hfile, int line, int spine,
      const char* buffer, int chord, int vlevel) {
 
   double output = 0;
   if (strcmp(buffer, ".") == 0) {
      // nothing to do
      return 0.0;
   }
   int explicitz = 0;
   int altered = 0;
   double duration = Convert::kernToDuration(buffer);
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
   
   checkbackup();

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
      cout << duration*divisions << "</duration>\n";
      output = duration;
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

   int vcase = 0;
   if (strchr(hfile[line].getSpineInfo(spine), '(') == NULL) {
      vcase = 0;
      pline(lev, "<voice>1</voice>\n");
      pline(lev, "<type>");
   } else if (strchr(hfile[line].getSpineInfo(spine), 'a') != NULL) {
      vcase = 1;
      pline(lev, "<voice>1</voice>\n");
      pline(lev, "<type>");
   } else if (strchr(hfile[line].getSpineInfo(spine), 'b') != NULL) {
      vcase = 2;
      pline(lev, "<voice>2</voice>\n");
      pline(lev, "<type>");
   } else {
      vcase = 3;
      pline(lev, "<voice>1</voice>\n");
      pline(lev, "<type>");
   }

   char durstring[32] = {0};
   Convert::durationToKernRhythm(durstring, duration);
   printDurationType(durstring);
   cout << "</type>\n";
   printDots(durstring);

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

   if (duration >= 1.0) {
      // kill rogue beam marks.
      beamlevel[vlevel] = 0;
   }
   if (!chord) {
      processBeams(hfile, line, spine, buffer, vlevel);
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
      processTextUnderlay(hfile, line, spine);
   }

   lev--;
   pline(lev, "</note>\n");
   return output;
}



//////////////////////////////
//
// processBeams --
//

void processBeams(HumdrumFile& hfile, int line, int spine, const char* buffer,
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
         pline(lev, "<beam number=\"");
         cout << beamlevel[vlevel] - closebeam + i + 1 << "\">end</beam>\n";
         rcount++;
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

void processTextUnderlay(HumdrumFile& hfile, int line, int spine) {
   int fields = hfile[line].getFieldCount();
   if (spine >= fields-1) {
      return;
   }

   int tspine = spine+1;
   int verse = 1;
   while (tspine < fields && 
         strcmp(hfile[line].getExInterp(tspine), "**kern") != 0) {
      if (strcmp(hfile[line][tspine], ".") == 0) {
         // ignore null tokens
      } else if (strcmp(hfile[line].getExInterp(tspine), "**text") == 0) {
         displayLyrics(hfile, line, tspine, verse);
      } else {
         displayUnknownTextType(hfile, line, tspine, verse);
      }

      tspine++;
      verse++;
   }

}



//////////////////////////////
//
// displayLyrics --
//

void displayLyrics(HumdrumFile& hfile, int line, int spine, int verse) {
   displayUnknownTextType(hfile, line, spine, verse);
}



//////////////////////////////
//
// displayUnknownTextType --
//

void displayUnknownTextType(HumdrumFile& hfile, int line, int spine, 
      int verse) {

   pline(lev, "<lyric number=\"");
   cout << verse << "\" name=\"verse\">\n";
   lev++;
   pline(lev, "<syllabic>single</syllabic>\n");
   pline(lev, "<text>");

   displayHTMLText(hfile[line][spine]);

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

int makePartList(HumdrumFile& hfile) {
   lev++;
   pline(lev, "<part-list>\n");
   lev++;
   
   // find the start of the spine data and output a part for each
   // spine
   int i = 0;
   int count = 0;   // the number of **kern spines
   int j = 0;
   while (i<hfile.getNumLines() && hfile[i].getType() != 
         E_humrec_interpretation) {
      i++;
   }
   if (hfile[i].getType() == E_humrec_interpretation) {
      if (strncmp("**", hfile[i][0], 2) != 0) {
         cout << "Error on line " << i + 1 << " of file: No start of data" 
              << endl;
      }
   } else {
      cout << "Error: no data in file" << endl;
      exit(1);
   }

   if (reverseQ) {
      for (j=0; j<hfile[i].getFieldCount(); j++) {
         if (strcmp(hfile[i].getExInterp(j), "**kern") != 0) {
            continue;
         }
         count++;
         generatePartInfo(hfile, i, j, count);
      }
   } else {
      // doing parts in reverse order
      for (j=hfile[i].getFieldCount()-1; j>=0; j--) {
         if (strcmp(hfile[i].getExInterp(j), "**kern") != 0) {
            continue;
         }
         count++;
         generatePartInfo(hfile, i, j, count);
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

void generatePartInfo(HumdrumFile& hfile, int start, int spine, int count) {
   int i = start + 1;
   int j = 0;
   pline(lev, "<score-part id=\"P");
   cout << count << "\">\n";
   lev++;

   int done = 0;
   while (!done && i < hfile.getNumLines()) {
      if (hfile[i].getType() == E_humrec_interpretation) {
         for (j=0; j<hfile[i].getFieldCount(); j++) {
            if (hfile[i].getPrimaryTrack(j) == spine + 1) {
               if (strncmp(hfile[i][j], "*I", 2) == 0 &&
                   strncmp(hfile[i][j], "*IC", 3) != 0) {
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
      cout << &(hfile[i][j][2]) << "</part-name>\n";
   } else {
      pline(lev, "<part-name>XPart ");
      cout << spine << "</part-name>\n";
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




// md5sum: 3a9e006239c1dcc076eb860c0df45210 hum2xml.cpp [20090501]

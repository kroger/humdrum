//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Oct 24 10:08:24 PDT 2001
// Last Modified: Fri Mar 29 09:37:12 PST 2002 (fixed bib recs at end of data)
// Last Modified: Wed Jun 24 15:37:46 PDT 2009 (updated for GCC 4.4)
// Filename:      ...sig/examples/all/hum2gmn.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/hum2gmn.cpp
// Syntax:        C++; museinfo
//
// Description:   Converts a Humdrum file into Guido Music Notation.
//
// Reference:     http://www.salieri.org/GUIDO
// Reference:     http://guidolib.sourceforge.net
//
// Status:        Initial stages of development
//
// Things todo:   beams
//                articulations (except staccatos)
//                ties
//                lyrics
//                multiple voices on a staff
//                stem direction markers
//                double barline encodings?
//                tempo markings?
//                etc.
//
// Things done:   systems, notes, rhythms, slurs, clefs, time sigs, key sigs,
//		  grace notes, staccatos.
//
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

#ifndef VISUAL
   #include <time.h>
#endif

// function declarations:
void      checkOptions          (Options& opts, int argc, char** argv);
void      example               (void);
void      convertToGMN          (HumdrumFile& hfile);
void      convertPartToGMN      (HumdrumFile& hfile, int part, int startline);
void      convertMeasureToGMN   (HumdrumFile& hfile, int part, int line);
int       convertInterpretationToGMN(HumdrumFile& hfile, int part, int line);
void      convertNoteDataToGMN  (HumdrumFile& hfile, int part, int line);
void      convertNoteToGMN      (const char* note, int graceQ);
void      convertGlobalComment  (HumdrumRecord& globalcomment);
void      convertBibliography   (HumdrumRecord& bibrec);
void      processHeader         (HumdrumFile& hfile);
void      processFooter         (HumdrumFile& hfile);
void      print                 (const char* data);
void      usage                 (const char* command);
int       getPartSpine          (HumdrumFile& hfile, int part, int line);
void      resetGlobals          (void);
void      printTitleComposer    (HumdrumFile& hfile);


// User interface variables:
Options   options;
int       debugQ = 0;          // used with the --debug option
int       cautionaryQ = 1;     // used with the --nocaution option
int       reverseQ = 0;        // reverse the ordering of the parts
int       quietQ = 0;          // don't add extra information

// Global state information
int       Goctave = -99;       // for displaying octave after a note
double    Gduration = -99.0;   // for sticky rhythms
int       Gnoteinit = 0;       // for marking when the first note is written
char      lastchar = '\0';     // last printed character in print stream
int       Gline  =0;           // for error warnings
int       Gpartinit = 0;       // for identifying the first part in the file.

//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   checkOptions(options, argc, argv);  // process the command-line options
   HumdrumFile hfile(options.getArg(1));
   hfile.analyzeRhythm();
   convertToGMN(hfile);
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
           << "craig@ccrma.stanford.edu, Oct 2001" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 24 Oct 2001" << endl;
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
// convertToGMN -- 
//

void convertToGMN(HumdrumFile& hfile) {

   // find the start of the data and count how many parts there are
   int i, j;
   int parts = 0;
   Array<int> partspines;
   partspines.setSize(100);
   partspines.setSize(0);
   partspines.allowGrowth();
   int startline = 0;
   for (i=0; i<hfile.getNumLines(); i++) {
      if (strncmp(hfile[i][0], "**", 2) == 0) {
         startline = i;
         for (j=0; j<hfile[i].getFieldCount(); j++) {
            if (strcmp(hfile[i].getExInterp(j), "**kern") == 0) {
               parts++;
               partspines.append(j);
            }
         }
         break;
      }
   }

   
   processHeader(hfile);
   print("{ ");    // print a system start
  

   for (i=0; i<partspines.getSize(); i++) {
      resetGlobals();
      if (!reverseQ) {
         convertPartToGMN(hfile, partspines[partspines.getSize()-1-i], startline);
      } else {
         convertPartToGMN(hfile, partspines[i], startline);
      }
      if (i < partspines.getSize() - 1) {
         print(", ");
      }
   }

   print("} \n");    // print a system end
   processFooter(hfile);

   if (quietQ == 0) {
      print("\n\n");
      print("% Converted with hum2gmn v1.0.1 on ");
      #ifndef VISUAL
         time_t rawtime;
         struct tm *timeinfo;
         time(&rawtime);
         timeinfo = localtime(&rawtime);
         print(asctime(timeinfo));
      #else
         print("<time not available>");
      #endif
      print("\n");
      
   }
}



//////////////////////////////
//
// convertPartToGMN --
//

void convertPartToGMN(HumdrumFile& hfile, int part, int startline) {
   static int staffno = 1;
   static char buffer[1024] = {0};
   print("[ ");     // beginning of part marker

   sprintf(buffer, "\\staff<%d> ", staffno++);
   print(buffer);
   if (Gpartinit == 0) {
      printTitleComposer(hfile);
      print("\n\t\\systemFormat<dx=1.0cm> ");
      print("\n\t\\barFormat<style=\"system\"> ");
      print("\n\t\\set<autoEndBar=\"off\"> ");
   }

   int i;
   int done = 0;
   for (i=startline+1; i<hfile.getNumLines(); i++) {
      Gline = i+1;
      switch (hfile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
            break;
         case E_humrec_global_comment:
            convertGlobalComment(hfile[i]);
            break;
         case E_humrec_bibliography:
            if (Gpartinit == 0) {
               convertBibliography(hfile[i]);
            }
            break;
         case E_humrec_data_comment:
            break;
         case E_humrec_data_kern_measure:
            convertMeasureToGMN(hfile, part, i);
            break;
         case E_humrec_interpretation:
            done = convertInterpretationToGMN(hfile, part, i);
            break;
         case E_humrec_data:
            if (Gnoteinit == 0) {
               print("\n\t\t");
               Gnoteinit = 1;
            }
            convertNoteDataToGMN(hfile, part, i);
            break;
         default:
            break;
      }
      if (done) break;
   }


   print("\n] ");    // end of part marker

   Gpartinit = 1;    // set to 1 after first voice has been processed
}



//////////////////////////////
//
// printTitleComposer -- print the title and composer if available
//

void printTitleComposer(HumdrumFile& hfile) {
   int i;
   const char *cptr;
   char buffer[128] = {0};
   int titleQ = 0;
   for (i=0; i<hfile.getNumLines(); i++) {
      if (hfile[i].getType() == E_humrec_bibliography) {

         if (!titleQ && strncmp(hfile[i][0], "!!!OTL", 6) == 0) {
            titleQ = 1;
            cptr = strchr(hfile[i][0], ':');
            if (cptr == NULL) {
               continue;
            }
            cptr++;
            while (cptr[0] == ' ') {
               cptr++;
            }
            if (cptr[0] != '\0') {
               print("\n\t\\title<\"");
            } else {
               continue;
            }
            while (cptr[0] != '\0') {
               if (cptr[0] == '\"') {
                  print("\\\'");
               } else {
                  buffer[0] = cptr[0];
                  buffer[1] = '\0';
                  print(buffer);
               }
               cptr++;
            }
            print("\">");
         }

         if (strncmp(hfile[i][0], "!!!COM", 6) == 0) {
            cptr = strchr(hfile[i][0], ',');
            if (cptr == NULL) {
               cptr = strchr(hfile[i][0], ':');
            }
            if (cptr == NULL) {
               continue;
            }
            cptr++;
            while (cptr[0] == ' ') {
               cptr++;
            }
            if (cptr[0] != '\0') {
               print("\n\t\\composer<\"");
            } else {
               continue;
            }
            while (cptr[0] != '\0') {
               if (cptr[0] == '\"') {
                  print("\\\'");
               } else {
                  buffer[0] = cptr[0];
                  buffer[1] = '\0';
                  print(buffer);
               }
               cptr++;
            }
            cptr = strchr(hfile[i][0], ':');
            if (cptr != NULL) {
               cptr++;
               while (cptr[0] == ' ') {
                  cptr++;
               }
               if (cptr[0] == '\0') {
                  print("\">");
                  continue;
               }
               print(" ");
               while (cptr[0] != ',' && cptr[0] != '\0') {
                  if (cptr[0] == '\"') {
                     print("\\\'");
                  } else {
                     buffer[0] = cptr[0];
                     buffer[1] = '\0';
                     print(buffer);
                  }
                  cptr++;
               }

            }
            print("\">");
         }

      }
   }
}


//////////////////////////////
//
// getPartSpine -- get the spine number for the first
//    occurance of the part in the given line.
//

int getPartSpine(HumdrumFile& hfile, int part, int line) {
   int i;
   int output = -1;
   for (i=0; i<hfile[line].getFieldCount(); i++) {
      if (part+1 == hfile[line].getPrimaryTrack(i)) {
         output = i;
         break;
      } 
   }

   return output;
}



//////////////////////////////
//
// convertMeasureToGMN -- convert **kern measure to GMN values.
//

void convertMeasureToGMN(HumdrumFile& hfile, int part, int line) {
   char buffer[1024] = {0};
   int spine = getPartSpine(hfile, part, line);
   if (spine < 0) {
      cerr << "Warning: invalid humdrum syntax at line " << line+1 << endl;
   }
   int length = strlen(hfile[line][spine]);
   int repeatQ = 0;
   int startrep = 0;
   int endrep = 0;

   if (strchr(hfile[line][spine], '-') != NULL) {
      // don't display hidden measures
      return;
   }

   if (strchr(hfile[line][spine], ':') != NULL) {
      repeatQ = 1;
   }

   // process repeat signs
   if (repeatQ) {
      int i;
      for (i=1; i<length; i++) {
         if (hfile[line][spine][i] == ':') {
            if (hfile[line][spine][i-1] == '|' || hfile[line][spine][i-1]=='!') {
               startrep = 1;
            } else {
               endrep = 1;
            }
         }
      }
   
      if (endrep) {
         print("\\repeatEnd ");
      }
   }

   print("\n\t");

   int measurenum = 0;
   int measureQ = 0;
   int count = sscanf(hfile[line][spine], "=%d", &measurenum);
   if (count == 1) {
      measureQ = 1;
   }
   int doubleQ = 0;
   if (strncmp(hfile[line][spine], "==", 2) == 0) {
      doubleQ = 1;
   }

   if (!repeatQ) { 
      if (doubleQ) {
         print("\\endBar");
      } else {
         print("\\bar");
      }
      if (measureQ) {
         sprintf(buffer, "<%d> ", measurenum);
         print(buffer);
      } else {
         print(" ");
      }
   }

   if (startrep) {
      print("\\repeatBegin ");
   }

}



//////////////////////////////
//
// convertInterpretationToGMN -- convert **kern interpretation to GMN values.
//

int convertInterpretationToGMN(HumdrumFile& hfile, int part, int line) {
   static char buffer[1024] = {0};
   int spine = getPartSpine(hfile, part, line);
   if (spine < 0) {
      cerr << "Warning: invalid humdrum syntax at line " << line+1 << endl;
      return 0;
   }
   int length = strlen(hfile[line][spine]);

   if (strcmp(hfile[line][spine], "*-") == 0) {
      return 1;
   }

   // proces a clef
   if (strncmp(hfile[line][spine], "*clef", 5) == 0) {
      if (Gnoteinit == 0) {
         print("\n\t"); 
      }
      char clef = 'x';
      int  ctype = -99;
      if (length >= 5) {
         clef = hfile[line][spine][5];
      }
      if (length >= 6 && isdigit(hfile[line][spine][6])) {
         ctype = hfile[line][spine][6] - '0'; 
      }
      clef = tolower(clef);
      if (ctype != 99) {
         sprintf(buffer, "\\clef<\"%c%d\">", clef, ctype);
      } else {
         sprintf(buffer, "\\clef<\"%c\">", clef);
      }
      print(buffer);
      print(" ");
      return 0;
   }

   // proces a key signature
   if ((strncmp(hfile[line][spine], "*k[", 3) == 0)
         && (hfile[line][spine][length-1] == ']')) {
      if (Gnoteinit == 0) {
         print("\n\t"); 
      }
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

      sprintf(buffer, "\\key<%d>", keyinfo);
      print(buffer);
      print(" ");

      return 0;
   }

   // process time signatures
   if (strncmp(hfile[line][spine], "*M", 2) == 0 &&
         strchr(hfile[line][spine], '/') != NULL) {
      int top = 0;
      int bottom = 0;

      int flag = sscanf(hfile[line][spine], "*M%d/%d", &top, &bottom);
      if (flag == 2) {
         if (Gnoteinit == 0) {
            print("\n\t"); 
         }
         sprintf(buffer, "\\meter<\"%d/%d\">", top, bottom);
         print(buffer);
         print(" ");
      }
      return 0;
   } 


   return 0;
}



//////////////////////////////
//
// convertNoteDataToGMN -- Convert note data to GMN values.
//

void convertNoteDataToGMN(HumdrumFile& hfile, int part, int line) {
   int i;
   char kbuffer[1024] = {0};
   int spine = getPartSpine(hfile, part, line);
   int chordnotes = hfile[line].getTokenCount(spine);

   // check to see if just a null token
   if (strcmp(hfile[line][spine], ".") == 0) {
      return;
   }

   // check for the start of a slur
   if (strchr(hfile[line][spine], '(') != NULL) {
      print("\\slur( ");
   }

   // check for the start of a tie
   if (strchr(hfile[line][spine], '[') != NULL) {
      print("\\tieBegin ");
   }

   // check to see if this is a grace note/chord
   int graceQ = 0;
   if (strchr(hfile[line][spine], 'q') != NULL) {
      print("\\grace( ");
      graceQ = 1;
   }

   // check for accidentals
   int staccatoQ = 0;
   if (strchr(hfile[line][spine], '\'') != NULL) {
      print("\\stacc( ");
      staccatoQ = 1;
   }


   if (chordnotes > 1) {
      print("{");
   }

   for (i=0; i<chordnotes; i++) {
      hfile[line].getToken(kbuffer, spine, i);
      convertNoteToGMN(kbuffer, graceQ);
      if (chordnotes > 1) {
         if (i < chordnotes - 1) {
            print(",");
         }
      } else {
         print(" ");
      }
   }

   if (chordnotes > 1) {
      print("} ");
   }

   if (staccatoQ) {
      print(") ");
   }

   if (graceQ) {
      print(") ");
   }

   // check for the end of a tie
   if (strchr(hfile[line][spine], ']') != NULL) {
      print("\\tieEnd ");
   }

   // check for the end of a slur
   if (strchr(hfile[line][spine], ')') != NULL) {
      print(") ");
   }
}



//////////////////////////////
//
// convertNoteToGMN -- convert a note to GMN format.
//

void convertNoteToGMN(const char* note, int graceQ) {
   static char buffer[1024] = {0};

   int octave = 0;
   int base40 = 0;
   int restQ = 0;
   char name = 'x';
   int length = strlen(note);

   // get the accidentals
   int accidental = 0;
   int i;
   for (i=0; i<length; i++) {
      if (note[i] == '#') {
         accidental++;
      } else if (note[i] == '-') {
         accidental--;
      }
   }
 
   // process pitch
   if (strchr(note, 'r') != NULL) {
      restQ = 1;
      name = '_';
   } else {   // process a real note
      base40 = Convert::kernToBase40(note);
      octave = (base40 / 40) - 3;           // Middle C = C1 in Guido
      Convert::base40ToKern(buffer, base40);
      name = tolower(buffer[0]);
   }

   if (restQ) {
      print("_");
   } else {
      sprintf(buffer, "%c", name);
      print(buffer);
      if (accidental > 0) {
         for (i=0; i<accidental; i++) {
            print("#");
         }
      } else if (accidental < 0) {
         for (i=0; i<-accidental; i++) {
            print("&");
         }
      }
      if (octave != Goctave) {
         sprintf(buffer, "%d", octave);
         print(buffer);
         Goctave = octave;
      }
   }


   // process duration
   double duration = Convert::kernToDuration(note);
   Convert::durationToKernRhythm(buffer, duration);
   if (duration == Gduration) {
      // don't print duration, since it is alreay active.
   } else {
      Gduration = duration;
      print("/");
      print(buffer);
   }
   
}


//////////////////////////////
//
// processFooter --
//

void processFooter(HumdrumFile& hfile) {
   int i;
   int start = 0;
   for (i=hfile.getNumLines()-1; i>=0; i--) {
      if (strcmp(hfile[i][0], "*-") == 0) {
         start = i;
         break;
      }
   }

   if (start == 0) {
      // already printed the header
      return;
   }

   print("\n");
   for (i=start; i<hfile.getNumLines(); i++) {
      switch(hfile[i].getType()) {
         case E_humrec_global_comment:
            convertGlobalComment(hfile[i]);
            break;
         case E_humrec_bibliography:
            convertBibliography(hfile[i]);
            break;
      }
   }
}



/////////////////////////////
//
// processHeader -- print the header information in the file
//

void processHeader(HumdrumFile& hfile) {
   int i;
   int done = 0;
   for (i=0; i<hfile.getNumLines(); i++) {
      switch(hfile[i].getType()) {
         case E_humrec_global_comment:
            convertGlobalComment(hfile[i]);
            break;
         case E_humrec_bibliography:
            convertBibliography(hfile[i]);
            break;
         case E_humrec_interpretation:
            done = 1; 
            break;
      }
      if (done) break;
   }
   print("\n");
}



//////////////////////////////
//
// convertBibliography --  print a bibliographic record (!!! -> %%%) 
//

void convertBibliography(HumdrumRecord& bibrec) {
   if (lastchar != '\n') {
      print("\n");
   }
   print("%%%");
   print(&(bibrec.getLine()[3])); 
   print("\n");
}



//////////////////////////////
//
// convertBibliography --  print a bibliographic record (!!! -> %%%) 
//

void convertGlobalComment(HumdrumRecord& globalcomment) {
   if (lastchar != '\n') {
      print("\n");
   }
   print("%%");
   print(&(globalcomment.getLine()[2]));
   print("\n");
}




//////////////////////////////
//
// print -- print the data
//

void print(const char* data) {
  int length = strlen(data);
  lastchar = data[length-1];
  cout << data;
}



//////////////////////////////
//
// usage --
//

void usage(const char* command) {

}


//////////////////////////////
//
// resetGlobals -- global states for parsing into Guido notation
//

void resetGlobals(void) {
   Goctave   = -99;
   Gduration = -99.0;
   Gnoteinit = 0;
}






// md5sum: 86ef2895d810769328d903dd55105db3 hum2gmn.cpp [20090626]

//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat May 23 21:08:48 PDT 1998
// Last Modified: Fri Jul  3 14:18:04 PDT 1998
// Last Modified: Sat Oct 14 20:26:15 PDT 2000 (revised for museinfo 1.0)
// Last Modified: Wed Nov 29 12:14:41 PST 2000 (use internal analysis)
// Last Modified: Tue Apr 21 00:41:11 PDT 2009 (fixed spine manip printing)
// Last Modified: Thu May 14 20:25:17 PDT 2009 (-U option added)
// Last Modified: Mon Apr 26 06:21:58 PDT 2010 (-n, -s options added)
// Filename:      ...sig/examples/all/sonority2.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/sonority2.cpp
// Syntax:        C++; museinfo
//
// Description:   Analyzes **kern data for timeslice chord qualities
//

#include "humdrum.h"

#include <stdlib.h>
#include <string.h>

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif


// function declarations
void    checkOptions        (Options& opts, int argc, char* argv[]);
void    example             (void);
void    processRecords      (HumdrumFile& infile);
void    usage               (const char* command);
void    fillStringWithNotes (char* string, ChordQuality& quality, 
                             HumdrumFile& infile, int line);
int     identifyBassNote    (SigCollection<int>& notes, 
                             HumdrumFile& infile, int line,
                             Array<int>& sounding);
int     transitionalSonority(ChordQuality& quality, HumdrumFile& infile, 
                             int line);


// global variables
Options      options;            // database for command-line arguments
char         unknown[256] = {0}; // space for unknown chord simplification
int          chordinit;          // for initializing chord detection function
int          explicitQ = 0;      // used with -U option
int          notesQ    = 0;      // used with -n option
int          suppressQ = 0;      // used with -s option
int          parenQ    = 1;      // used with -P option
int          appendQ   = 0;      // used with -a option
int          octaveVal = -100;   // used with -o option
const char*  notesep   = " ";    // used with -N option


///////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[]) {
   HumdrumFile infile, outfile;

   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   for (int i=0; i<numinputs || i==0; i++) {
      chordinit = 1;
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
      } else {
         infile.read(options.getArg(i+1));
      }

      // analyze the input file according to command-line options
      processRecords(infile);
       
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("t|type=b",          "show only chord type");
   opts.define("i|inversion=b",     "show only chord inversion");
   opts.define("r|root=b",          "show only chord root");
   opts.define("a|assemble|append=b", "append analysis to input data");
   opts.define("f|format=s:t:i:r",  "control display style");
   opts.define("u|unknown=s:X",     "control display of unknowns");
   opts.define("U|unknown-pcs=b",   "print pcs of unknown sonrities");
   opts.define("d|debug=b",         "determine bad input line num");
   opts.define("n|notes=b",         "display pitch classes in sonority");
   opts.define("s|suppress=b",      "suppress data if overlapping sonority");
   opts.define("P|paren-off=b",     "suppress parentheses for overlapping");
   opts.define("N|separator=s: ",   "characters to separate pitch classes");
   opts.define("o|octave=i:4",      "characters to separate pitch classes");

   opts.define("author=b",          "author of program");
   opts.define("version=b",         "compilation info");
   opts.define("example=b",         "example usages");
   opts.define("h|help=b",          "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, May 1998" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: Nov 2000" << endl;
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

   if (opts.getBoolean("root")) {
      ChordQuality::setDisplay("r");
   } else if (opts.getBoolean("type")) {
      ChordQuality::setDisplay("t");
   } else if (opts.getBoolean("inversion")) {
      ChordQuality::setDisplay("i");
   } else {
      ChordQuality::setDisplay(opts.getString("format"));
   }

   Convert::chordType.setNullName(opts.getString("unknown"));
   Convert::chordInversion.setNullName(opts.getString("unknown"));
   Convert::kernPitchClass.setNullName(opts.getString("unknown"));

   strncpy(unknown, opts.getString("unknown"), 64);
   strcat(unknown, ":");
   strncat(unknown, opts.getString("unknown"), 64);
   strcat(unknown, ":");
   strncat(unknown, opts.getString("unknown"), 64);

   explicitQ =  opts.getBoolean("unknown-pcs");
   notesQ    =  opts.getBoolean("notes");
   suppressQ =  opts.getBoolean("suppress");
   parenQ    = !opts.getBoolean("paren-off");
   appendQ   =  opts.getBoolean("append");
   if (opts.getBoolean("separator")) {
      notesep   =  opts.getString("separator");
   }
   if (opts.getBoolean("octave")) {
      octaveVal =  opts.getInteger("octave");
   }
}



//////////////////////////////
//
// example -- example usage of the sonority program
//

void example(void) {
   cout <<
   "                                                                         \n"
   "# example usage of the sonority program.                                 \n"
   "# analyze a Bach chorale for chord qualities:                            \n"
   "     sonority chor217.krn                                                \n"
   "                                                                         \n"
   "# display the chord analysis with original data:                         \n"
   "     sonority -a chor217.krn                                             \n"
   "                                                                         \n"
   "# display only the roots of chords:                                      \n"
   "     sonority -r chor217.krn                                             \n"
   "                                                                         \n"
   << endl;
}



//////////////////////////////
//
// processRecords -- looks at humdrum records and determines chord
//	sonority quality;
//

void processRecords(HumdrumFile& infile) {
   Array<ChordQuality> cq;
   infile.analyzeSonorityQuality(cq);
   ChordQuality quality;

   int foundstart = 0;
   char aString[512] = {0};

   for (int i=0; i<infile.getNumLines(); i++) {
      if (options.getBoolean("debug")) {
         cout << "processing line " << (i+1) << " of input ..." << endl;
	 cout << "LINE IS: " << infile[i] << endl;
      }
      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_bibliography:
         case E_humrec_global_comment:
            cout << infile[i] << endl;
            break;
         case E_humrec_data_comment:
            if (appendQ) {
	       cout << infile[i] << "\t";
            } 
            if (infile[i].equalFieldsQ("**kern")) {
               cout << infile[i][0];
            } else {
               cout << "!";
            }
            cout << endl;
            break;
         case E_humrec_data_interpretation:
            if (appendQ) {
               cout << infile[i] << "\t";
            }
            if (!foundstart && infile[i].hasExclusiveQ()) {
               foundstart = 1;
               cout << "**qual";
            } else {
               if (infile[i].equalFieldsQ("**kern") && 
                   (!infile[i].isSpineManipulator(0))) {
                  cout << infile[i][0];
               } else {
                  cout << "*";
               }
            }
	    cout << endl;
            break;
         case E_humrec_data_kern_measure:
            if (appendQ) {
               cout << infile[i] << "\t";
            }
            cout << infile[i][0];
            cout << endl;
            break;
         case E_humrec_data:
            if (appendQ) {
               cout << infile[i] << "\t";
            }
            // handle null fields
            if (infile[i].equalFieldsQ("**kern", ".")) {
	       cout << "." << endl;
               break;
            }
            if (notesQ == 0) {
               quality = cq[i];
               quality.makeString(aString, explicitQ);
               if (strcmp(aString, "") != 0) {
                  if (strcmp(aString, unknown) == 0 || (
                        quality.getType()==E_chord_note && 
                              options.getBoolean("root"))
                        ) {
                     char tempbuffer[128] = {0};
	             strcpy(tempbuffer, aString);
                     strcpy(aString, options.getString("unknown"));
	             strcat(aString, tempbuffer);
                  }
		  if (suppressQ && transitionalSonority(quality, infile, i)) {
                     strcpy(aString, ".");
                  }
               } else {
                  strcpy(aString, "rest");
               }
            } else {
               quality = cq[i];
               fillStringWithNotes(aString, quality, infile, i);
            }
	    cout << aString << endl;

            break;
         default:
            cerr << "Error on line " << (i+1) << " of input" << endl;
            cerr << "record type = " << infile[i].getType() << endl;
            exit(1);
      }
   }

}



//////////////////////////////
//
// transitionalSonority --
//

int transitionalSonority(ChordQuality& quality, HumdrumFile& infile, int line) {
   SigCollection<int> notes;
   quality.getNotesInChord(notes);

   Array<int> octave;
   octave.setSize(notes.getSize());
   octave.allowGrowth(0);
   octave.setAll(0);

   Array<int> sounding;
   sounding.setSize(notes.getSize());
   sounding.allowGrowth(0);
   sounding.setAll(0);

   // int bassindex = identifyBassNote(notes, infile, line, sounding);

   int i;
   for (i=0; i<sounding.getSize(); i++) {
      if (sounding[i] == 0) {
         return 1;
      }
   }

   return 0;
}



//////////////////////////////
//
// fillStringWithNotes --
//

void fillStringWithNotes(char* string, ChordQuality& quality, 
      HumdrumFile& infile, int line) {

   string[0] = '\0';

   SigCollection<int> notes;
   quality.getNotesInChord(notes);

   Array<int> octave;
   octave.setSize(notes.getSize());
   octave.allowGrowth(0);
   octave.setAll(0);

   Array<int> sounding;
   sounding.setSize(notes.getSize());
   sounding.allowGrowth(0);
   sounding.setAll(0);

   int bassindex = identifyBassNote(notes, infile, line, sounding);
   if (bassindex >= 0) {
      octave[bassindex] = -1;
      if (notes[bassindex] >= 40) {
         octave[bassindex] += -2;
      }
   }

   int i;
   if (suppressQ) {
      for (i=0; i<sounding.getSize(); i++) {
         if (sounding[i] == 0) {
            strcpy(string, ".");
            return;
         }
      }
   }

   string[0] = '\0';
   char buffer[32] = {0};
   for (i=0; i<notes.getSize(); i++) {
      if (octaveVal >= 0) {
         Convert::base40ToKern(buffer, (notes[i]%40) + octaveVal * 40);
      } else {
         Convert::base40ToKern(buffer, notes[i] + ((octave[i]+4) * 40));
      }
      if (parenQ && (sounding[i] == 0)) {
         strcat(string, "(");
      }
      strcat(string, buffer);
      if (parenQ && (sounding[i] == 0)) {
         strcat(string, ")");
      }
      if (i < notes.getSize() - 1) {
         strcat(string, notesep);
      }
   }

}



//////////////////////////////
//
// identifyBassnote --
//

int identifyBassNote(SigCollection<int>& notes, HumdrumFile& infile, 
      int line, Array<int>& sounding) {
   int j, k;
   int output = -1;
   int minval = 1000000;
   int value;
   int tcount;
   char buffer[128] = {0};

   Array<int> soundQ(40);
   soundQ.setAll(0);

   sounding.setSize(notes.getSize());
   sounding.setAll(0);

   int pline;
   int pspine;

   int dotQ = 0;

   if (notes.getSize() == 0) {
      return -1;
   }

   for (j=0; j<infile[line].getFieldCount(); j++) {
      if (!infile[line].isExInterp(j, "**kern")) {
         continue;
      }
      dotQ = 0;
      if (strcmp(infile[line][j], ".") == 0) {
         pline  = infile[line].getDotLine(j);
         pspine = infile[line].getDotSpine(j);
         dotQ = 1;
      } else {
         pline = line;
         pspine = j;
      }
      tcount = infile[pline].getTokenCount(pspine);
      for (k=0; k<tcount; k++) {
         infile[pline].getToken(buffer, pspine, k);
         if (strchr(buffer, 'r') != NULL) {
            continue;
         }
	 if (strcmp(buffer, ".") == 0) {
            // shouldn't get here...
            continue;
         }
	 value = Convert::kernToMidiNoteNumber(buffer);
         if (value < 0) {
            continue;
         }
         if (value < minval) {
            minval = value;
         }
         if (dotQ) {
            continue;
         }
         if (strchr(buffer, '_') != NULL) {
            continue;
         }
         if (strchr(buffer, ']') != NULL) {
            continue;
         }
	 value = Convert::kernToBase40(buffer);
         if (value < 0) {
            continue;
         }
         soundQ[value % 40] = 1;
      }
   }

   if (minval > 100000) {
      return -1;
   }

   minval = minval % 12;
   int i;
   int tval;
   for (i=0; i<notes.getSize(); i++) {
      if (notes[i] >= 0) {
         if (soundQ[notes[i]%40]) {
            sounding[i] = 1;
         }
      }
      tval = Convert::base40ToMidiNoteNumber(notes[i]);
      if (tval < 0) {
         continue;
      }
      tval = tval % 12;
      if (tval == minval) {
         output = i;
         // break;  need to supress this because of sounding tests
      }
   }
   return output;
}



//////////////////////////////
//
// usage -- gives the usage statement for the sonority program
//

void usage(const char* command) {
   cout <<
   "                                                                         \n"
   "Analyzes **kern data and generates a **qual spine which gives the chord  \n"
   "quality of the **kern spines.  Currently, input spines cannot split or   \n"
   "join.                                                                    \n"
   "                                                                         \n"
   "Usage: " << command << " [-a][-i|-r|-t] [input1 [input2 ...]]            \n"
   "                                                                         \n"
   "Options:                                                                 \n"
   "   -a = assemble the **qual analysis spine with input data               \n"
   "   -i = displays the **qual chord inversion only                         \n"
   "   -r = displays the **qual chord root only                              \n"
   "   -t = displays the **qual chord type only                              \n"
   "   --options = list of all options, aliases and default values           \n"
   "                                                                         \n"
   << endl;
}



// md5sum: 71deb39b03840b1ae8917c862eae7648 sonority.cpp [20100905]

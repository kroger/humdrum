//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat May 23 21:08:48 PDT 1998
// Last Modified: Fri Jul  3 14:18:04 PDT 1998
// Last Modified: Sat Oct 14 20:26:15 PDT 2000 (revised for museinfo 1.0)
// Last Modified: Wed Nov 29 12:14:41 PST 2000 (use internal analysis)
// Last Modified: Tue Apr 21 00:41:11 PDT 2009 (fixed spine manip printing)
// Last Modified: Thu May 14 20:25:17 PDT 2009 (-U option added)
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
void         checkOptions(Options& opts, int argc, char* argv[]);
void         example(void);
void         processRecords(HumdrumFile& infile, HumdrumFile& outfile);
void         usage(const char* command);


// global variables
Options      options;            // database for command-line arguments
char         unknown[256] = {0}; // space for unknown chord simplification
int          chordinit;          // for initializing chord detection function
int          explicitQ = 0;      // for use with -U option


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
      outfile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
      } else {
         infile.read(options.getArg(i+1));
      }

      // analyze the input file according to command-line options
      processRecords(infile, outfile);

      // check to see if only the analysis spine is required
      if (!options.getBoolean("assemble")) {
         outfile = outfile.extract(-1);
      }

      outfile.write(cout);
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
   opts.define("a|assemble=b",      "assemble analysis with input");
   opts.define("f|format=s:t:i:r",  "control display style");
   opts.define("u|unknown=s:X",     "control display of unknowns");
   opts.define("U|unknown-pcs=b",   "print pcs of unknown sonrities");
   opts.define("d|debug=b",         "determine bad input line num");
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

   explicitQ = opts.getBoolean("unknown-pcs");
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

void processRecords(HumdrumFile& infile, HumdrumFile& outfile) {
   Array<ChordQuality> cq;
   infile.analyzeSonorityQuality(cq);
   ChordQuality quality;

   int foundstart = 0;
   char aString[256] = {0};

   for (int i=0; i<infile.getNumLines(); i++) {
      if (options.getBoolean("debug")) {
         cout << "processing line " << (i+1) << " of input ..." << endl;
      }
      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_bibliography:
         case E_humrec_global_comment:
            outfile.appendLine(infile[i]);
            break;
         case E_humrec_data_comment:
            if (infile[i].equalFieldsQ("**kern")) {
               infile[i].appendField(infile[i][0]);
            } else {
               infile[i].appendField("!");
            }
            outfile.appendLine(infile[i]);
            break;
         case E_humrec_data_interpretation:
            if (!foundstart && infile[i].hasExclusiveQ()) {
               foundstart = 1;
               infile[i].appendField("**qual");
            } else {
               if (infile[i].equalFieldsQ("**kern") && 
                   (!infile[i].isSpineManipulator(0))) {
                  infile[i].appendField(infile[i][0]);
               } else {
                  infile[i].appendField("*");
               }
            }
            outfile.appendLine(infile[i]);
            break;
         case E_humrec_data_kern_measure:
            if (infile[i].equalFieldsQ("**kern")) {
               infile[i].appendField(infile[i][0]);
            } else {
               infile[i].appendField("=");
            }
            outfile.appendLine(infile[i]);
            break;
         case E_humrec_data:
            // handle null fields
            if (infile[i].equalFieldsQ("**kern", ".")) {
               infile[i].appendField(".");
               outfile.appendLine(infile[i]);
               break;
            }
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
            } else {
               strcpy(aString, "rest");
            }
            infile[i].appendField(aString);
            outfile.appendLine(infile[i]);
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



// md5sum: 3ba44d5108fa6b105d672a9f307be510 sonority.cpp [20090518]

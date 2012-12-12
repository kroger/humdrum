//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Mar 13 21:37:28 PST 2004
// Last Modified: Sat Mar 13 21:37:30 PST 2004
// Last Modified: Thu May  6 00:42:02 PDT 2004 (added measure renumbering)
// Last Modified: Thu Jun  3 18:01:43 PDT 2004 (added -p option)
// Last Modified: Sat Jun 26 16:49:06 PDT 2010 (added middle syllable markers)
// Filename:      ...sig/examples/all/xml2hum.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/xml2hum.cpp
// Syntax:        C++; museinfo
//
// Description:   Converts a MusicXML 1.0 file into a Humdrum file.
//

#include "humdrum.h"
#include "MusicXmlFile.h"
#include "Options.h"
#include "PerlRegularExpression.h"

#include <iostream>

// user interface variables
Options options;
int     debugQ   = 0;       // used with --debug option
int     infoQ    = 0;       // used with -i option
int     midifixQ = 0;       // used with -x option
int     staff    = -1;      // used with -t option
int     stemQ    = 1;       // used with -S option
int     beamQ    = 1;       // used with -B option
int     dynamicsQ= 1;       // used with -D option
int     mfixQ    = 1;       // used with -M option
int     printQ   = 0;       // used with -p option
int     textQ    = 1;       // used with -T option

// function declarations:
void      checkOptions      (Options& opts, int argc, char** argv);
void      example           (void);
void      usage             (const char* command);
void      printTextString   (const char* string);

//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   checkOptions(options, argc, argv);

   MusicXmlFile xmlfile(options.getArg(1));

   if (printQ) {
      xmlfile.print();
      exit(0);
   }

   if (debugQ) {
      xmlfile.info();
   }

   xmlfile.setOption("stem", stemQ);         // set stem encoding option
   xmlfile.setOption("beam", beamQ);         // set beam encoding option
   xmlfile.setOption("lyric", textQ);        // set text encoding option
   xmlfile.setOption("dynamics", dynamicsQ); // set dynamics encoding option

   HumdrumFile  hfile;

   if (debugQ) {
      cout << "====================================================\n" << endl;
   }

   if (infoQ) {
      //cout << "Filename: " << options.getArg(1) << endl;
      //cout << "Parts:    " << xmlfile.getPartCount() << endl;
      xmlfile.info(cout);
      exit(0);
   }
   
   if (midifixQ) {
      xmlfile.fixMidiAccidentals();
   }

   if (mfixQ) {
      xmlfile.fixMeasureNumbers();
   }

   if (staff <= 0) {
      xmlfile.createHumdrumFile(hfile); 
   } else if (staff <= xmlfile.getStaffCount()) {
      xmlfile.humdrumPart(hfile, staff-1, debugQ);
   }

   int i, j;
   for (i=0; i<hfile.getNumLines(); i++) {
      // prevent blank lines from occuring:
      if (hfile[i].getFieldCount() > 0) {
         if (hfile[i].getType() != E_humrec_empty) {
            for (j=0; j<hfile[i].getFieldCount(); j++) {
            // cout << hfile[i] << '\n';
               if (strlen(hfile[i][j]) == 0) {
                  cout << '.';
               } else {
                  printTextString(hfile[i][j]);
               }
               if (j<hfile[i].getFieldCount()-1) {
                  cout << '\t';
               }
            }
            cout << '\n';
         }
      }
   }
   // cout << hfile << flush;

   return 0;
}



//////////////////////////////
//
// printTextString -- do some conversions from UTF-8.
//

void printTextString(const char* string) {
   Array<char> newstring;
   newstring.setSize(strlen(string)+1);
   strcpy(newstring.getBase(), string);
   //PerlRegularExpression pre;
   //pre.sar(newstring, "\xc3\xa9", "&eacute;", "g");
   //cout << string;
   int i;
   for (i=0; i<(int)strlen(string); i++) {
      if (string[i] == '\xC3') {

         if (string[i+1] == '\xA0') { cout << "&agrave;"; i+=1; continue; }
         if (string[i+1] == '\xA1') { cout << "&aacute;"; i+=1; continue; }
         if (string[i+1] == '\xA2') { cout << "&acirc;";  i+=1; continue; }
         if (string[i+1] == '\xA3') { cout << "&atilde;"; i+=1; continue; }
         if (string[i+1] == '\xA4') { cout << "&auml;";   i+=1; continue; }
         if (string[i+1] == '\xA5') { cout << "&aring;";  i+=1; continue; }
         if (string[i+1] == '\xA7') { cout << "&ccedil;"; i+=1; continue; }
         if (string[i+1] == '\xA8') { cout << "&egrave;"; i+=1; continue; }
         if (string[i+1] == '\xA9') { cout << "&eacute;"; i+=1; continue; }
         if (string[i+1] == '\xAA') { cout << "&ecirc;";  i+=1; continue; }
         if (string[i+1] == '\xAB') { cout << "&euml;";   i+=1; continue; }
         if (string[i+1] == '\xAC') { cout << "&igrave;"; i+=1; continue; }
         if (string[i+1] == '\xAD') { cout << "&iacute;"; i+=1; continue; }
         if (string[i+1] == '\xAE') { cout << "&icirc;";  i+=1; continue; }
         if (string[i+1] == '\xAF') { cout << "&iuml;";   i+=1; continue; }
         if (string[i+1] == '\xB1') { cout << "&ntilde;"; i+=1; continue; }
         if (string[i+1] == '\xB2') { cout << "&ograve;"; i+=1; continue; }
         if (string[i+1] == '\xB3') { cout << "&oacute;"; i+=1; continue; }
         if (string[i+1] == '\xB4') { cout << "&ocirc;";  i+=1; continue; }
         if (string[i+1] == '\xB5') { cout << "&otilde;"; i+=1; continue; }
         if (string[i+1] == '\xB6') { cout << "&ouml;";   i+=1; continue; }
         if (string[i+1] == '\xB9') { cout << "&ugrave;"; i+=1; continue; }
         if (string[i+1] == '\xBA') { cout << "&uacute;"; i+=1; continue; }
         if (string[i+1] == '\xBB') { cout << "&ucirc;";  i+=1; continue; }
         if (string[i+1] == '\xBB') { cout << "&uuml;";   i+=1; continue; }

      }
      cout << string[i];
   }

}



//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// checkOptions --
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("debug=b", "print debugging information program");
   opts.define("x|midifix|accidentals=b",  "fix accidentals from MIDI file");
   opts.define("s|staff=i:-1", "extract a particular staff");
   opts.define("i|info=b", "print information about the MusicXML file");
   opts.define("p|print=b", "print original file and exit");
   opts.define("S|no-stems=b", "do not print stem information of notes");
   opts.define("B|no-beams=b", "do not print beam information of notes");
   opts.define("D|no-dynamics=b", "do not print dynamics information");
   opts.define("N|no-notation=b", "do not print notation-specific information");
   opts.define("M|no-measure-number-fix=b", "do not renumber measures");
   opts.define("T|no-text=b", "do not convert lyrics to humdrum spines");

   opts.define("author=b",  "author of program");
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");
   opts.define("h|help=b",  "short description");
   opts.process(argc, argv);

   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, 10 March 2004" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: May 2004" << endl;
      cout << "compiled: " << __DATE__ << endl;
      exit(0);
   } else if (opts.getBoolean("help")) {
      usage(opts.getCommand());
      exit(0);
   } else if (opts.getBoolean("example")) {
      example();
      exit(0);
   }

   if (opts.getArgCount() != 1) {
      cout << "Error: need one filename specified" << endl;
      exit(1);
   }

   debugQ    =  opts.getBoolean("debug");
   infoQ     =  opts.getBoolean("info");
   stemQ     = !opts.getBoolean("no-stems");
   beamQ     = !opts.getBoolean("no-beams");
   dynamicsQ = !opts.getBoolean("no-dynamics");
   staff     =  opts.getInteger("staff");
   midifixQ  =  opts.getBoolean("midifix");
   mfixQ     = !opts.getBoolean("no-measure-number-fix");
   printQ    =  opts.getBoolean("print");
   textQ     = !opts.getBoolean("no-text");
   if (opts.getBoolean("no-notation")) {
      stemQ     = 0;
      beamQ     = 0;
      dynamicsQ = 0;
   }
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



// md5sum: 355276b61985f1c5dc1f2d6a0268dc30 xml2hum.cpp [20111215]

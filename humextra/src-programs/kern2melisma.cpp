//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Dec 13 13:35:37 PST 2000
// Last Modified: Wed Dec 13 13:35:41 PST 2000
// Filename:      ...sig/examples/all/kern2melisma.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/kern2melisma.cpp
// Syntax:        C++; museinfo
// Reference:     http://bobo.link.cs.cmu.edu/music-analysis
//
// Description:   Generates a list of pitches in a Humdrum file 
//                according to the Melisma Music Analyzer file format
//                for notes.  
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
void      checkOptions      (Options& opts, int argc, char** argv);
void      example           (void);
void      preparePitch      (char* buffer2, const char* buffer1);
void      printOutput       (HumdrumFile& hfile);
void      usage             (const char* command);

// User interface variables:
Options   options;
int       barlinesQ = 1;      // used with the -B option
int       midinoteQ = 0;      // used with the -m option
int       classQ    = 0;      // used with the -c option
double    tdefault  = 60.0;   // used with the -t option

//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   // process the command-line options
   checkOptions(options, argc, argv);

   HumdrumFile hfile(options.getArg(1));
   hfile.analyzeRhythm("4");
   printOutput(hfile);
   return 0;
}

//////////////////////////////////////////////////////////////////////////



//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("B|nobarlines=b", "don't display barlines");
   opts.define("m|midi|MIDI=b", "display pitches as MIDI note numbers");
   opts.define("c|class=b", "display pitches in pitch class notation");
   opts.define("t|tempo|default-tempo=d:60.0", "tempo if none specified");

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

   if (opts.getBoolean("nobarlines")) {
      barlinesQ = 0;
   } else {
      barlinesQ = 1;
   }

   if (opts.getBoolean("class")) {
      classQ = 1;
   } else {
      classQ = 0;
   }

   tdefault = opts.getDouble("default-tempo");
   if (tdefault == 0.0) {
      cout << "Error default tempo cannot be zero." << endl;
      exit(1);
   }
   if (tdefault < 0.0) {
      cerr << "Warning: converting tempo to be positive" << endl;
      tdefault = -tdefault;
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
// printOutput -- one small bug: the duration of the notes
//   are calculated under constant tempo.  A tempo change during
//   a tied note will not give the correct ending point of the
//   tied note.  This bug is minor and the work to fix it is major.
//

void printOutput(HumdrumFile& hfile) {
   Array<double> tempo;
   hfile.analyzeTempoMarkings(tempo, tdefault);

   double currentmillisecond = 0.0;
   double lastduration = 0.0;
   double endmillisecond;

   int i, j, k;
   char buffer1[1024] = {0};
   char buffer2[1024] = {0};
   double duration;
   for (i=0; i<hfile.getNumLines(); i++) {
      currentmillisecond = currentmillisecond + 
            lastduration * 60000.0 / tempo[i];
      lastduration = hfile[i].getDuration();

      if (hfile[i].getType() == E_humrec_global_comment) {
         cout << "Comment\t" << &(hfile[i].getLine()[3]) << endl;
      }

      if (hfile[i].getType() == E_humrec_bibliography) {
         cout << "Reference " << &(hfile[i].getLine()[3]) << endl;
      }

      if (hfile[i].getType() == E_humrec_interpretation) {
         for (j=0; j<hfile[i].getFieldCount(); j++) {
            if (hfile[i].getExInterpNum(j) != E_KERN_EXINT) {
               continue;
            }
            int direction = 0;
            int acount = 0;
            if (strncmp(hfile[i][j], "*k[", 3) == 0) {
               if (strchr(hfile[i][j], '-') != NULL) {
                  direction = -1;
               } else if (strchr(hfile[i][j], '#') != NULL) {
                  direction = 1;
               }
               acount = (strlen(hfile[i][j]) - 4)/2;
               if (direction == 0) {
                  cout << "Info keysig no accidentals" << endl;
                  break;
               } else {
                  cout << "Info keysig " << acount;
                  if (acount < 0) {
                     cout << " flat";
                  } else {
                     cout << " sharp";
                  }
                  if (acount > 1) {
                     cout << "s";
                  }
                  cout << endl;
                  break;
               }
            }
         }
      }

      // look for key interpretations
      if (hfile[i].getType() == E_humrec_interpretation) {
         for (j=0; j<hfile[i].getFieldCount(); j++) {
            if (hfile[i].getExInterpNum(j) != E_KERN_EXINT) {
               continue;
            }
            int length = strlen(hfile[i][j]);
            if (length < 3 || length > 4) {
               continue;
            }
            if (hfile[i][j][length-1] == ':') {
               cout << "Info key ";
               cout << (char)toupper(hfile[i][j][1]);
               if (hfile[i][j][2] == '-') {
                  cout << "-flat";
               }
               if (hfile[i][j][2] == '#') {
                  cout << "-sharp";
               }
               if (islower(hfile[i][j][1])) {
                  cout << " Minor";
               } else {
                  cout << " Major";
               }
               cout << endl;
               break;
            }
         }
      }

      // look for tempo markings
      if (hfile[i].getType() == E_humrec_interpretation) {
         for (j=0; j<hfile[i].getFieldCount(); j++) {
            if (hfile[i].getExInterpNum(j) != E_KERN_EXINT) {
               continue;
            }
            double tempomark = 0.0;
            if (strncmp(hfile[i][j], "*MM", 3) == 0 && 
                  isdigit(hfile[i][j][3])) {
               sscanf(hfile[i][j], "*MM%lf", &tempomark);
               cout << "Info Tempo " << tempomark << " MM per quarter note" 
                    << endl;
               break;
            }
         }
      }

      if (hfile[i].getType() != E_humrec_data) {
         continue;
      }

      for (j=0; j<hfile[i].getFieldCount(); j++) {
         if (hfile[i].getExInterpNum(j) != E_KERN_EXINT) {
            continue;
         }

         for (k=0; k<hfile[i].getTokenCount(j); k++) {
            if (strcmp(hfile[i][j], ".") == 0) {
               continue;
            }
            hfile[i].getToken(buffer1, j, k);
            if (strchr(buffer1, '_') != NULL) {
               // ignore notes which are tied
               continue;
            }
            if (strchr(buffer1, ']') != NULL) {
               // ignore notes which are tied
               continue;
            }

            preparePitch(buffer2, buffer1);
            duration = hfile.getTiedDuration(i, j, k);

            int note = Convert::kernToMidiNoteNumber(buffer2);
            if (classQ) {
               note = note % 12;
            }
            if (note < 0) {
               // don't display rests.
               continue;
            }

            cout << "Note\t";
            // cout << hfile.getAbsBeat(i) << "\t";
            cout << (int)(currentmillisecond+0.5) << "\t";

            // cout << duration << "\t";
            endmillisecond = currentmillisecond + duration * 60000 / tempo[i];
            if (int(endmillisecond+0.5) == int(currentmillisecond+0.5)) {
               // give grace notes 1 millisecond duration...
               cout << (int)(endmillisecond+0.5)+1 << "\t";
            } else {
               cout << (int)(endmillisecond+0.5) << "\t";
            }

            cout << note;

            cout << "\n";
         }
      }
   }
}



//////////////////////////////
//
// preparePitch --
//

void preparePitch(char* buffer2, const char* buffer1) {
   strcpy(buffer2, buffer1);
}


//////////////////////////////
//
// usage --
//

void usage(const char* command) {


}



// md5sum: 07eaaa60a484c3c4f66eb68cabcc9c8f kern2melisma.cpp [20080205]

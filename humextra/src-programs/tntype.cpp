//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Mar 17 07:46:28 PDT 2011
// Last Modified: Thu Mar 17 07:46:35 PDT 2011
// Filename:      ...sig/examples/all/tntype.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/tntype.cpp
// Syntax:        C++; museinfo
//
// Description:   Analyzes **kern data with serial descriptions.
// Reference: 	  http://solomonsmusic.net/pcsets.htm
//

#include "humdrum.h"
#include "PerlRegularExpression.h"

#include <stdlib.h>
#include <string.h>

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif

#define NORM_OPEN   "["
#define NORM_CLOSE  "]"
#define PRIME_OPEN  "("
#define PRIME_CLOSE ")"
#define TN_OPEN     "{"
#define TN_CLOSE    "}"
#define IV_OPEN     "<"
#define IV_CLOSE    ">"


// function declarations
void        checkOptions          (Options& opts, int argc, char* argv[]);
void        example               (void);
void        processRecords        (HumdrumFile& infile);
void        usage                 (const char* command);
void        fillStringWithNotes   (char* string, ChordQuality& quality, 
                                   HumdrumFile& infile, int line);
int         identifyBassNote      (SigCollection<int>& notes, 
                                   HumdrumFile& infile, int line,
                                   Array<int>& sounding);
int         transitionalSonority  (ChordQuality& quality, HumdrumFile& infile, 
                                   int line);
double      getMeasureSize        (HumdrumFile& infile, int width);
void        printAttackMarker     (HumdrumFile& infile, int line);
void        printRotation         (HumdrumFile& infile, int line);
const char* getDescription        (const char* tntype);

// global variables
Options      options;            // database for command-line arguments
char         unknown[256] = {0}; // space for unknown chord simplification
int          chordinit;          // for initializing chord detection function
int          notesQ    = 0;      // used with --notes option
int          suppressQ = 0;      // used with -s option
int          transQ    = 0;      // used with -T option
int          parenQ    = 1;      // used with -P option
int          ivQ       = 0;      // used with --iv option
int          infoQ     = 0;      // used with -D option
int          forteQ    = 0;      // used with --forte option
int          tnQ       = 0;      // used with --tn option
int          normQ     = 0;      // used with -n option
int          tnormQ    = 0;      // used with -t option
int          verboseQ  = 0;      // used with -v option
int          tniQ      = 0;      // used with --tni option
int          rotationQ = 0;      // used with -r option
int          attackQ   = 0;      // used with -x option
int          appendQ   = 0;      // used with -a option
const char*  notesep   = " ";    // used with -N option
const char* colorindex[26];


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
   opts.define("a|assemble|append=b", "append analysis to input data");
   opts.define("debug=b",      "determine bad input line num");
   opts.define("d|notes=b",    "display pitch classes in sonority");
   opts.define("n|norm=b",     "display normal form of pitch sets");
   opts.define("D|description=b", "display musical Description of Tn type");
   opts.define("f|form|tnorm=b", "display transposed normal form of pitch sets");
   opts.define("i|iv|ic=b",    "print interval vector");
   opts.define("x|sonor|suspension=b", "print marker if not all start attacks");
   opts.define("F|forte=b",      "print forte interval vector set name");
   opts.define("r|rotation=b", "mark which note is in the bass");
   opts.define("t|transpose=b", "indicate the tranposition value for Tn form");
   opts.define("Tn|tn=b",      "print forte set with subsets");
   opts.define("Tni|tni=b",    "print forte set with subsets/inversion");
   opts.define("v|verbose=b",  "print verbose label with extra info");
   opts.define("s|suppress=b", "suppress data if overlapping sonority");
   opts.define("S|paren-off=b","suppress parentheses for overlapping");

   opts.define("author=b",     "author of program");
   opts.define("version=b",    "compilation info");
   opts.define("example=b",    "example usages");
   opts.define("h|help=b",     "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Mar 2011" << endl;
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

   ivQ       =  opts.getBoolean("iv");
   transQ    =  opts.getBoolean("transpose");
   attackQ   =  opts.getBoolean("suspension");
   infoQ     =  opts.getBoolean("description");
   forteQ    =  opts.getBoolean("forte");
   rotationQ =  opts.getBoolean("rotation");
   tnQ       =  opts.getBoolean("Tn");
   verboseQ  =  opts.getBoolean("verbose");
   if (tnQ) {
      forteQ = 1;
   }
   tniQ      =  opts.getBoolean("Tni");
   if (tniQ) {
      tnQ = 1;
      forteQ = 1;
   }
   normQ     =  opts.getBoolean("norm");
   tnormQ    =  opts.getBoolean("tnorm");
   notesQ    =  opts.getBoolean("notes");
   suppressQ =  opts.getBoolean("suppress");
   parenQ    = !opts.getBoolean("paren-off");
   appendQ   =  opts.getBoolean("append");
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
   Array<char> data;
   PerlRegularExpression pre;

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
               if (tniQ) {
                  cout << "**Tni";
               } else if (tnQ) {
                  cout << "**Tn";
               } else if (forteQ) {
                  cout << "**forte";
               } else if (tnormQ) {
                  cout << "**tnf";
               } else if (notesQ) {
                  cout << "**dpc";
               } else if (normQ) {
                  cout << "**nf";
               } else if (ivQ) {
                  cout << "**iv";
               } else if (infoQ) {
                  cout << "**description";
               } else {
                  cout << "**tnt";
               }
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
            if (ivQ) {
               Array<int> iv;
               infile.getIntervalVector(iv, i);
               cout << IV_OPEN; 
               for (int ii=0; ii<iv.getSize(); ii++) {
                  if (iv[ii] < 9) {
                     cout << iv[ii];
                     continue;
                  }
                  if (iv[ii] < 36) {
                     cout << char(iv[ii]-10+'A');
                     continue;
                  }
                  if (ii > 0) {
                     cout << ",";
                  }
                  cout << iv[ii];
                  if (ii < 5) {
                     cout << ",";
                  }
               }
               cout << IV_CLOSE << endl;

            } else if (forteQ) {
               const char* name = infile.getForteSetName(i);
               cout << name;
               if (tnQ) {
                  if (strcmp(name, "3-11") == 0) {
                     if (strcmp(cq[i].getTypeName(), "min") == 0) {
                        cout << "A";
                     } else if (strcmp(cq[i].getTypeName(), "maj") == 0) {
                        cout << "B";
                     }
                  }
               }
               if (tniQ) {
                  int inversion = -1;
                  if (strcmp(name, "3-11") == 0) {
                     if ((strcmp(cq[i].getTypeName(), "min") == 0) || 
                        (strcmp(cq[i].getTypeName(), "maj") == 0)) {
                        inversion = cq[i].getInversion();
                     }
                     if (inversion >= 0) {
                        cout << char('a'+inversion);
                     }
                  }
               }
               if (attackQ) {
                  printAttackMarker(infile, i);
               }
               cout << endl;

            } else if (normQ) {

               Array<int> norm;
               infile.getNormalForm(norm, i);
               cout << NORM_OPEN;
               for (int ii=0; ii<norm.getSize(); ii++) {
                  if (norm[ii] < 10) {
                     cout << norm[ii];
                  } else {
                     cout << char('A' + norm[ii] - 10);
                  }
               }
               cout << NORM_CLOSE;
               if (rotationQ) {
                  printRotation(infile, i);
               }
               if (attackQ) {
                  printAttackMarker(infile, i);
               }

               cout << endl;

            } else if (tnormQ) {
               Array<int> norm;
               infile.getNormalForm(norm, i);
               cout << TN_OPEN;
               int value;
               for (int ii=0; ii<norm.getSize(); ii++) {
                  value = (norm[ii] - norm[0] + 144) % 12;
                  if (value < 10) {
                     cout << value;
                  } else {
                     cout << char('A' + value - 10);
                  }
               }
               cout << TN_CLOSE;
               if (rotationQ) {
                  printRotation(infile, i);
               }
               if (attackQ) {
                  printAttackMarker(infile, i);
               }
               if (transQ) {
                  if (norm.getSize() > 0) {
                     cout << "T";
                     if (norm[0] < 10) {
                        cout << "0";
                     }
                     cout << norm[0];
                  }
               }
               cout << endl;

            } else if (notesQ) {
               quality = cq[i];
               fillStringWithNotes(aString, quality, infile, i);
	       cout << aString << endl;
            } else if (infoQ) {
               const char* tnname;
               tnname = infile.getTnSetName(i);
               data.setSize(strlen(tnname)+1);
               strcpy(data.getBase(), tnname);
               pre.sar(data, "Z", "", "g");
               if (pre.search(data, "^(\\d+-\\d+[AB]?)", "")) {
                  cout << getDescription(pre.getSubmatch(1));
               } else {
                  cout << ".";
               }
               cout << endl;

            } else {
               const char* tnname;
               tnname = infile.getTnSetName(i);
               if (verboseQ) {
                  cout << tnname;
               } else {
                  data.setSize(strlen(tnname)+1);
                  strcpy(data.getBase(), tnname);
                  if (pre.search(data, "^(\\d+-\\d+[AB]?)", "")) {
                     cout << pre.getSubmatch(1);
                  } else if (pre.search(data, "^(\\d+-)Z(\\d+[AB]?)", "")) {
                     cout << pre.getSubmatch(1);
                     cout << pre.getSubmatch(2);
                  } else {
                     cout << tnname;
                  }
               }
               if (rotationQ) {
                  printRotation(infile, i);
               }
               if (attackQ) {
                  printAttackMarker(infile, i);
               }
               if (transQ) {
                  Array<int> norm;
                  infile.getNormalForm(norm, i);
                  if (norm.getSize() > 0) {
                     cout << "T";
                     if (norm[0] < 10) {
                        cout << "0";
                     }
                     cout << norm[0];
                  }
               }
               cout << endl;
            }

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
// printRotation --
//

void printRotation(HumdrumFile& infile, int line) {
   Array<int> base12;
   infile.getBase12PitchList(base12, line);
   int i;
   if (base12.getSize() == 0) {
      return;
   }
   int min = base12[0];
   for (i=1; i<base12.getSize(); i++) {
      if (base12[i] < min) {
         min = base12[i];
      }
   }
   min = min % 12;

   Array<int> norm;
   infile.getNormalForm(norm, line);

   for (i=0; i<norm.getSize(); i++) {
     if (norm[i] == min)  {
        cout << char('a' + i);
        return;
     }
   }
   cout << '?';
}



//////////////////////////////
//
// printAttackMarker -- print an "s" if the there is any note at
//    the start of the region which is not attacked (i.e., suspended
//    from a previous sonority; otherwise, print an "x" which means
//    that all notes in the sonority are attacked at the start of the
//    region.
//

void printAttackMarker(HumdrumFile& infile, int line) {
   int j, ii, jj;
   int& i = line;
   int dotQ;
   for (j=0; j<infile[i].getFieldCount(); j++) {
      if (!infile[i].isExInterp(j, "**kern")) {
         continue;
      }
      if (strcmp(infile[i][j], ".") == 0) {
        ii = infile[i].getDotLine(j);
        jj = infile[i].getDotSpine(j);
        if (ii < 0 || jj < 0) {
           continue;
        }
        dotQ = 1;
      } else {
         ii = i;
         jj = j;
         dotQ = 0;
      }
      if (strchr(infile[ii][jj], 'r') != NULL) {
         continue;
      } else if (dotQ) {
         cout << "s";
         return;
      }
      if (strchr(infile[ii][jj], '_') != NULL) {
         cout << "s";
         return;
      }
      if (strchr(infile[ii][jj], ']') != NULL) {
         cout << "s";
         return;
      }
   }
   cout << "x";

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
   octave.setAll(4);

   Array<int> sounding;
   sounding.setSize(notes.getSize());
   sounding.allowGrowth(0);
   sounding.setAll(0);

   int bassindex = identifyBassNote(notes, infile, line, sounding);
   if (bassindex >= 0) {
      octave[bassindex] = 3;
      //if (notes[bassindex] >= 40) {
      //   octave[bassindex] += -2;
      //}
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
      //if (octaveVal >= 0) {
      //   Convert::base40ToKern(buffer, (notes[i]%40) + octaveVal * 40);
      //} else {
      //   Convert::base40ToKern(buffer, notes[i] + ((octave[i]+4) * 40));
      //}
      Convert::base40ToKern(buffer, notes[i]%40 + (octave[i] * 40));
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
         if (pline < 0 || pspine < 0) {
            continue;
         }
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
// getDescription -- return a rough musical description of the Tn type
//

const char* getDescription(const char* tntype) {
   int cardinality = 0;
   if (!sscanf(tntype, "%d", &cardinality)) {
      return ".";
   }
   switch (cardinality) {
      case 0:
         if (strcmp(tntype, "0-1") == 0)	return "Rest";
         break;

      case 1:
         if (strcmp(tntype, "1-1") == 0)	return "Unison";
         break;

      case 2:
         if (strcmp(tntype, "2-1") == 0)	return "Semitone";
         if (strcmp(tntype, "2-2") == 0)	return "Whole-tone";
         if (strcmp(tntype, "2-3") == 0)	return "Minor Third";
         if (strcmp(tntype, "2-4") == 0)	return "Major Third";
         if (strcmp(tntype, "2-5") == 0)	return "Perfect Fourth";
         if (strcmp(tntype, "2-6") == 0)	return "Tritone";
         break;

      case 3:
         if (strcmp(tntype, "3-1") == 0)	return "BACH /Chromatic Trimirror";
         if (strcmp(tntype, "3-2A") == 0)	return "Phrygian Trichord";
         if (strcmp(tntype, "3-2B") == 0)	return "Minor Trichord";
         if (strcmp(tntype, "3-3A") == 0)	return "Major-minor Trichord.1";
         if (strcmp(tntype, "3-3B") == 0)	return "Major-minor Trichord.2";
         if (strcmp(tntype, "3-4A") == 0)	return "Incomplete Major-seventh Chord.1";
         if (strcmp(tntype, "3-4B") == 0)	return "Incomplete Major-seventh Chord.2";
         if (strcmp(tntype, "3-5A") == 0)	return "Rite chord.2, Tritone-fourth.1";
         if (strcmp(tntype, "3-5B") == 0)	return "Rite chord.1, Tritone-fourth.2";
         if (strcmp(tntype, "3-6") == 0)	return "Whole-tone Trichord";
         if (strcmp(tntype, "3-7A") == 0)	return "Incomplete Minor-seventh Chord";
         if (strcmp(tntype, "3-7B") == 0)	return "Incomplete Dominant-seventh Chord.2";
         if (strcmp(tntype, "3-8A") == 0)	return "Incomplete Dominant-seventh Chord.1/Italian-sixth";
         if (strcmp(tntype, "3-8B") == 0)	return "Incomplete Half-dim-seventh Chord";
         if (strcmp(tntype, "3-9") == 0)	return "Quartal Trichord";
         if (strcmp(tntype, "3-10") == 0)	return "Diminished Chord";
         if (strcmp(tntype, "3-11A") == 0)	return "Minor Chord";
         if (strcmp(tntype, "3-11B") == 0)	return "Major Chord";
         if (strcmp(tntype, "3-12") == 0)	return "Augmented Chord";
         break;

      case 4:
         if (strcmp(tntype, "4-1") == 0)	return "BACH /Chromatic Tetramirror";
         if (strcmp(tntype, "4-2A") == 0)	return "Major-second Tetracluster.2";
         if (strcmp(tntype, "4-2B") == 0)	return "Major-second Tetracluster.1";
         if (strcmp(tntype, "4-3") == 0)	return "Alternating Tetramirror";
         if (strcmp(tntype, "4-4A") == 0)	return "Minor Third Tetracluster.2";
         if (strcmp(tntype, "4-4B") == 0)	return "Minor Third Tetracluster.1";
         if (strcmp(tntype, "4-5A") == 0)	return "Major Third Tetracluster.2";
         if (strcmp(tntype, "4-5B") == 0)	return "Major Third Tetracluster.1";
         if (strcmp(tntype, "4-6") == 0)	return "Perfect Fourth Tetramirror";
         if (strcmp(tntype, "4-7") == 0)	return "Arabian Tetramirror";
         if (strcmp(tntype, "4-8") == 0)	return "Double Fourth Tetramirror";
         if (strcmp(tntype, "4-9") == 0)	return "Double Tritone Tetramirror";
         if (strcmp(tntype, "4-10") == 0)	return "Minor Tetramirror";
         if (strcmp(tntype, "4-11A") == 0)	return "Phrygian Tetrachord";
         if (strcmp(tntype, "4-11B") == 0)	return "Major Tetrachord";
         if (strcmp(tntype, "4-12A") == 0)	return "Harmonic-minor Tetrachord";
         if (strcmp(tntype, "4-12B") == 0)	return "Major-third Diminished Tetrachord";
         if (strcmp(tntype, "4-13A") == 0)	return "Minor-second Diminished Tetrachord";
         if (strcmp(tntype, "4-13B") == 0)	return "Perfect-fourth Diminished Tetrachord";
         if (strcmp(tntype, "4-14A") == 0)	return "Major-second Minor Tetrachord";
         if (strcmp(tntype, "4-14B") == 0)	return "Perfect-fourth Major Tetrachord";
         if (strcmp(tntype, "4-15A") == 0)	return "All-interval Tetrachord.1";
         if (strcmp(tntype, "4-15B") == 0)	return "All-interval Tetrachord.2";
         if (strcmp(tntype, "4-16A") == 0)	return "Minor-second Quartal Tetrachord";
         if (strcmp(tntype, "4-16B") == 0)	return "Tritone Quartal Tetrachord";
         if (strcmp(tntype, "4-17") == 0)	return "Major-minor Tetramirror";
         if (strcmp(tntype, "4-18A") == 0)	return "Major-diminished Tetrachord";
         if (strcmp(tntype, "4-18B") == 0)	return "Minor-diminished Tetrachord";
         if (strcmp(tntype, "4-19A") == 0)	return "Minor-augmented Tetrachord";
         if (strcmp(tntype, "4-19B") == 0)	return "Augmented-major Tetrachord";
         if (strcmp(tntype, "4-20") == 0)	return "Major-seventh Chord";
         if (strcmp(tntype, "4-21") == 0)	return "Whole-tone Tetramirror";
         if (strcmp(tntype, "4-22A") == 0)	return "Major-second Major Tetrachord";
         if (strcmp(tntype, "4-22B") == 0)	return "Perfect-fourth Minor Tetrachord";
         if (strcmp(tntype, "4-23") == 0)	return "Quartal Tetramirror";
         if (strcmp(tntype, "4-24") == 0)	return "Augmented Seventh Chord";
         if (strcmp(tntype, "4-25") == 0)	return "French-sixth Chord";
         if (strcmp(tntype, "4-26") == 0)	return "Minor-seventh Chord";
         if (strcmp(tntype, "4-27A") == 0)	return "Half-diminished Seventh Chord/Tristan Chord";
         if (strcmp(tntype, "4-27B") == 0)	return "Dominant-seventh/German-sixth Chord";
         if (strcmp(tntype, "4-28") == 0)	return "Diminished-seventh Chord";
         if (strcmp(tntype, "4-29A") == 0)	return "All-interval Tetrachord.3";
         if (strcmp(tntype, "4-29B") == 0)	return "All-interval Tetrachord.4";
         break;

      case 5:
         if (strcmp(tntype, "5-1") == 0)	return "Chromatic Pentamirror";
         if (strcmp(tntype, "5-2A") == 0)	return "Major-second Pentacluster.2";
         if (strcmp(tntype, "5-2B") == 0)	return "Major-second Pentacluster.1";
         if (strcmp(tntype, "5-3A") == 0)	return "Minor-second Major Pentachord";
         if (strcmp(tntype, "5-3B") == 0)	return "Spanish Pentacluster";
         if (strcmp(tntype, "5-4A") == 0)	return "Blues Pentacluster";
         if (strcmp(tntype, "5-4B") == 0)	return "Minor-third Pentacluster";
         if (strcmp(tntype, "5-5A") == 0)	return "Major-third Pentacluster.2";
         if (strcmp(tntype, "5-5B") == 0)	return "Major-third Pentacluster.1";
         if (strcmp(tntype, "5-6A") == 0)	return "Oriental Pentacluster.1, Raga Megharanji (13161)";
         if (strcmp(tntype, "5-6B") == 0)	return "Oriental Pentacluster.2";
         if (strcmp(tntype, "5-7A") == 0)	return "DoublePentacluster.1, Raga Nabhomani (11415)";
         if (strcmp(tntype, "5-7B") == 0)	return "Double Pentacluster.2";
         if (strcmp(tntype, "5-8") == 0)	return "Tritone-Symmetric Pentamirror";
         if (strcmp(tntype, "5-9A") == 0)	return "Tritone-Expanding Pentachord";
         if (strcmp(tntype, "5-9B") == 0)	return "Tritone-Contracting Pentachord";
         if (strcmp(tntype, "5-10A") == 0)	return "Alternating Pentachord.1";
         if (strcmp(tntype, "5-10B") == 0)	return "Alternating Pentachord.2";
         if (strcmp(tntype, "5-11A") == 0)	return "Center-cluster Pentachord.1";
         if (strcmp(tntype, "5-11B") == 0)	return "Center-cluster Pentachord.2";
         if (strcmp(tntype, "5-12") == 0)	return "Locrian Pentamirror";
         if (strcmp(tntype, "5-13A") == 0)	return "Augmented Pentacluster.1";
         if (strcmp(tntype, "5-13B") == 0)	return "Augmented Pentacluster.2";
         if (strcmp(tntype, "5-14A") == 0)	return "Double-seconds Triple-fourth Pentachord.1";
         if (strcmp(tntype, "5-14B") == 0)	return "Double-seconds Triple-fourth Pentachord.2";
         if (strcmp(tntype, "5-15") == 0)	return "Assymetric Pentamirror";
         if (strcmp(tntype, "5-16A") == 0)	return "Major-minor-dim Pentachord.1";
         if (strcmp(tntype, "5-16B") == 0)	return "Major-minor-dim Pentachord.2";
         if (strcmp(tntype, "5-17") == 0)	return "Minor-major Ninth Chord";
         if (strcmp(tntype, "5-18A") == 0)	return "Gypsy Pentachord.1";
         if (strcmp(tntype, "5-18B") == 0)	return "Gypsy Pentachord.2";
         if (strcmp(tntype, "5-19A") == 0)	return "Javanese Pentachord";
         if (strcmp(tntype, "5-19B") == 0)	return "Balinese Pentachord";
         if (strcmp(tntype, "5-20A") == 0)	return "Balinese Pelog Pentatonic (12414), Raga Bhupala, Raga Bibhas";
         if (strcmp(tntype, "5-20B") == 0)	return "Hirajoshi Pentatonic (21414), Iwato (14142), Sakura/Raga Saveri (14214)";
         if (strcmp(tntype, "5-21A") == 0)	return "Syrian Pentatonic/Major-augmented Ninth Chord, Raga Megharanji (13134)";
         if (strcmp(tntype, "5-21B") == 0)	return "Lebanese Pentachord/Augmented-minor Chord";
         if (strcmp(tntype, "5-22") == 0)	return "Persian Pentamirror, Raga reva/Ramkali (13314)";
         if (strcmp(tntype, "5-23A") == 0)	return "Minor Pentachord";
         if (strcmp(tntype, "5-23B") == 0)	return "Major Pentachord";
         if (strcmp(tntype, "5-24A") == 0)	return "Phrygian Pentachord";
         if (strcmp(tntype, "5-24B") == 0)	return "Lydian Pentachord";
         if (strcmp(tntype, "5-25A") == 0)	return "Diminished-major Ninth Chord";
         if (strcmp(tntype, "5-25B") == 0)	return "Minor-diminished Ninth Chord";
         if (strcmp(tntype, "5-26A") == 0)	return "Diminished-augmented Ninth Chord";
         if (strcmp(tntype, "5-26B") == 0)	return "Augmented-diminished Ninth Chord";
         if (strcmp(tntype, "5-27A") == 0)	return "Major-Ninth Chord";
         if (strcmp(tntype, "5-27B") == 0)	return "Minor-Ninth Chord";
         if (strcmp(tntype, "5-28A") == 0)	return "Augmented-sixth Pentachord.1";
         if (strcmp(tntype, "5-28B") == 0)	return "Augmented-sixth Pentachord.2";
         if (strcmp(tntype, "5-29A") == 0)	return "Kumoi Pentachord.2";
         if (strcmp(tntype, "5-29B") == 0)	return "Kumoi Pentachord.1";
         if (strcmp(tntype, "5-30A") == 0)	return "Enigmatic Pentachord.1";
         if (strcmp(tntype, "5-30B") == 0)	return "Enigmatic Pentachord.2, Altered Pentatonic (14223)";
         if (strcmp(tntype, "5-31A") == 0)	return "Diminished Minor-Ninth Chord";
         if (strcmp(tntype, "5-31B") == 0)	return "Ranjaniraga/Flat-Ninth Pentachord";
         if (strcmp(tntype, "5-32A") == 0)	return "Neapolitan Pentachord.1";
         if (strcmp(tntype, "5-32B") == 0)	return "Neapolitan Pentachord.2";
         if (strcmp(tntype, "5-33") == 0)	return "Whole-tone Pentamirror";
         if (strcmp(tntype, "5-34") == 0)	return "Dominant-ninth/major-minor/Prometheus Pentamirror, Dominant Pentatonic (22332)";
         if (strcmp(tntype, "5-35") == 0)	return "'Black Key' Pentatonic/Slendro/Bilahariraga/Quartal Pentamirror, Yo (23232)";
         if (strcmp(tntype, "5-36A") == 0)	return "Major-seventh Pentacluster.2";
         if (strcmp(tntype, "5-36B") == 0)	return "Minor-seventh Pentacluster.1";
         if (strcmp(tntype, "5-37") == 0)	return "Center-cluster Pentamirror";
         if (strcmp(tntype, "5-38A") == 0)	return "Diminished Pentacluster.1";
         if (strcmp(tntype, "5-38B") == 0)	return "Diminished Pentacluster.2";
         break;

      case 6:
         if (strcmp(tntype, "6-1") == 0)	return "Chromatic Hexamirror/1st ord. all-comb (P6, Ib, RI5)";
         if (strcmp(tntype, "6-2A") == 0)	return "comb I (b)";
         if (strcmp(tntype, "6-2B") == 0)	return "comb I (1)";
         if (strcmp(tntype, "6-4") == 0)	return "comb RI (6)";
         if (strcmp(tntype, "6-5A") == 0)	return "comb I (b)";
         if (strcmp(tntype, "6-5B") == 0)	return "comb I (3)";
         if (strcmp(tntype, "6-6") == 0)	return "Double-cluster Hexamirror";
         if (strcmp(tntype, "6-7") == 0)	return "Messiaen's mode 5 (114114), 2nd ord.all-comb(P3+9,I5,Ib,R6,RI2+8)";
         if (strcmp(tntype, "6-8") == 0)	return "1st ord.all-comb (P6, I1, RI7)";
         if (strcmp(tntype, "6-9A") == 0)	return "comb I (b)";
         if (strcmp(tntype, "6-9B") == 0)	return "comb I (3)";
         if (strcmp(tntype, "6-13") == 0)	return "Alternating Hexamirror/comb RI7)";
         if (strcmp(tntype, "6-14A") == 0)	return "comb P (6)";
         if (strcmp(tntype, "6-14B") == 0)	return "comb P (6)";
         if (strcmp(tntype, "6-15A") == 0)	return "comb I (b)";
         if (strcmp(tntype, "6-15B") == 0)	return "comb I (5)";
         if (strcmp(tntype, "6-16A") == 0)	return "comb I (3)";
         if (strcmp(tntype, "6-16B") == 0)	return "Megha or 'Cloud' Raga/comb.I (1)";
         if (strcmp(tntype, "6-18A") == 0)	return "comb I (b)";
         if (strcmp(tntype, "6-18B") == 0)	return "comb I (5)";
         if (strcmp(tntype, "6-20") == 0)	return "Augmented scale, Genus tertium, 3rd ord. all-comb (P2+6+10, I3+7+b, R4+8, RI1+5+9)";
         if (strcmp(tntype, "6-21A") == 0)	return "comb I (1)";
         if (strcmp(tntype, "6-21B") == 0)	return "comb I (3)";
         if (strcmp(tntype, "6-22A") == 0)	return "comb I (b)";
         if (strcmp(tntype, "6-22B") == 0)	return "comb I (5)";
         if (strcmp(tntype, "6-23") == 0)	return "Super-Locrian Hexamirror/comb RI (8)";
         if (strcmp(tntype, "6-24B") == 0)	return "Melodic-minor Hexachord";
         if (strcmp(tntype, "6-25A") == 0)	return "Locrian Hexachord/Suddha Saveriraga";
         if (strcmp(tntype, "6-25B") == 0)	return "Minor Hexachord";
         if (strcmp(tntype, "6-26") == 0)	return "Phrygian Hexamirror/comb RI (8)";
         if (strcmp(tntype, "6-27A") == 0)	return "comb I (b)";
         if (strcmp(tntype, "6-27B") == 0)	return "Pyramid Hexachord/comb I (1)";
         if (strcmp(tntype, "6-28") == 0)	return "Double-Phrygian Hexachord/comb RI (6)";
         if (strcmp(tntype, "6-29") == 0)	return "comb RI (9)";
         if (strcmp(tntype, "6-30A") == 0)	return "Minor-bitonal Hexachord/comb R (6), I (5,b)";
         if (strcmp(tntype, "6-30B") == 0)	return "Petrushka chord, Major-bitonal Hexachord, comb R (6), I (1,7)";
         if (strcmp(tntype, "6-31A") == 0)	return "comb I (7)";
         if (strcmp(tntype, "6-31B") == 0)	return "comb I (b)";
         if (strcmp(tntype, "6-32") == 0)	return "Arezzo major Diatonic (221223), major hexamirror, quartal hexamirror, 1st ord.all-comb P (6), I (3), RI (9)";
         if (strcmp(tntype, "6-33A") == 0)	return "Dorian Hexachord/comb I (1)";
         if (strcmp(tntype, "6-33B") == 0)	return "Dominant-11th/Lydian Hexachord/comb I (5)";
         if (strcmp(tntype, "6-34A") == 0)	return "Mystic Chord or Prometheus Hexachord/comb I (B)";
         if (strcmp(tntype, "6-34B") == 0)	return "Harmonic Hexachord/Augmented-11th/comb I (7)";
         if (strcmp(tntype, "6-35") == 0)	return "Wholetone scale/6th ord.all-comb.(P+IoddT, R+RIevenT)";
         if (strcmp(tntype, "6-37") == 0)	return "comb RI (4)";
         if (strcmp(tntype, "6-38") == 0)	return "comb RI (3)";
         if (strcmp(tntype, "6-42") == 0)	return "comb RI (3)";
         if (strcmp(tntype, "6-44A") == 0)	return "Schoenberg Anagram Hexachord";
         if (strcmp(tntype, "6-44B") == 0)	return "Bauli raga (133131)";
         if (strcmp(tntype, "6-45") == 0)	return "comb RI (6)";
         if (strcmp(tntype, "6-47B") == 0)	return "Blues mode.1 (321132)";
         if (strcmp(tntype, "6-48") == 0)	return "comb RI (2)";
         if (strcmp(tntype, "6-49") == 0)	return "Prometheus Neapolitan mode (132312), comb RI (4)";
         if (strcmp(tntype, "6-50") == 0)	return "comb RI (1)";
         break;

      case 7:
         if (strcmp(tntype, "7-1") == 0)	return "Chromatic Heptamirror";
         if (strcmp(tntype, "7-20A") == 0)	return "Chromatic Phrygian inverse (1123113)";
         if (strcmp(tntype, "7-20B") == 0)	return "Pantuvarali Raga (1321131), Chromatic Mixolydian (1131132), Chromatic Dorian/Mela Kanakangi (1132113)";
         if (strcmp(tntype, "7-21B") == 0)	return "Gypsy hexatonic (1312113)";
         if (strcmp(tntype, "7-22") == 0)	return "Persian, Major Gypsy, Hungarian Minor, Double Harmonic scale, Bhairav That, Mayamdavagaula Raga (all: 1312131), Oriental (1311312)";
         if (strcmp(tntype, "7-23B") == 0)	return "Tritone Major Heptachord";
         if (strcmp(tntype, "7-24B") == 0)	return "Enigmatic Heptatonic (1322211)";
         if (strcmp(tntype, "7-27B") == 0)	return "Modified Blues mode (2121132)";
         if (strcmp(tntype, "7-30A") == 0)	return "Neapolitan-Minor mode (1222131), Mela Dhenuka";
         if (strcmp(tntype, "7-31A") == 0)	return "Alternating Heptachord.1/Hungarian Major mode (3121212)";
         if (strcmp(tntype, "7-31B") == 0)	return "Alternating Heptachord.2";
         if (strcmp(tntype, "7-32A") == 0)	return "Harmonic-Minor mode (2122131), Spanish Gypsy, Mela Kiravani, Pilu That";
         if (strcmp(tntype, "7-32B") == 0)	return "Dharmavati Scale (2131221), Harmonic minor inverse (1312212), Mela Cakravana, Raga Ahir Bhairav";
         if (strcmp(tntype, "7-33") == 0)	return "Neapolitan-major mode (1222221)/Leading-Whole-tone mode (222211)";
         if (strcmp(tntype, "7-34") == 0)	return "Harmonic/Super-Locrian, Melodic minor ascending (1212222)/Aug.13th Heptamirror, Jazz Minor";
         if (strcmp(tntype, "7-35") == 0)	return "Major Diatonic Heptachord/Dominant-13th, Locrian (1221222), Phrygian (1222122), Major inverse";
         break;

      case 8:
         if (strcmp(tntype, "8-1") == 0)	return "Chromatic Octamirror";
         if (strcmp(tntype, "8-9") == 0)	return "Messiaen's mode 4 (11131113)";
         if (strcmp(tntype, "8-22B") == 0)	return "Spanish Octatonic Scale (r9) (12111222)";
         if (strcmp(tntype, "8-23") == 0)	return "Quartal Octachord, Diatonic Octad";
         if (strcmp(tntype, "8-25") == 0)	return "Messiaen mode 6 (11221122)";
         if (strcmp(tntype, "8-26") == 0)	return "Spanish Phrygian (r9) (12112122)/ Blues mode.2 (21211212)";
         if (strcmp(tntype, "8-28") == 0)	return "Alternating Octatonic or Diminished scale (12121212)";
         break;

      case 9:
         if (strcmp(tntype, "9-1") == 0)	return "Chromatic Nonamirror";
         if (strcmp(tntype, "9-7A") == 0)	return "Nonatonic Blues Scale (211111212)";
         if (strcmp(tntype, "9-9") == 0)	return "Raga Ramdasi Malhar (r2) (211122111)";
         if (strcmp(tntype, "9-11B") == 0)	return "Diminishing Nonachord";
         if (strcmp(tntype, "9-12") == 0)	return "Tsjerepnin/Messiaen mode 3 (112112112)";
         break;

      case 10:
         if (strcmp(tntype, "10-1") == 0)	return "Chromatic Decamirror";
         if (strcmp(tntype, "10-5") == 0)	return "Major-minor mixed (r7)";
         if (strcmp(tntype, "10-6") == 0)	return "Messiaen mode 7 (1111211112)";
         break;

      case 11:
         if (strcmp(tntype, "11-1") == 0)	return "Chromatic Undecamirror";
         break;

      case 12:
         if (strcmp(tntype, "12-1") == 0)	return "Chromatic Scale/Dodecamirror (111111111111)";
         break;
   }

   // no musical desciption of the Tn type
   return ".";

}



//////////////////////////////
//
// usage -- gives the usage statement for the sonority program
//

void usage(const char* command) {
   cout <<
   "                                                                         \n"
   << endl;
}



// md5sum: 81901c7ec2a38645891dc8ea30363c40 tntype.cpp [20110325]

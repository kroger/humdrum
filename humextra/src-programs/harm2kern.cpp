//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul 27 11:57:21 PDT 2003
// Last Modified: Thu Jul 31 22:00:31 PDT 2003
// Last Modified: Sun May  2 22:26:31 PDT 2010 (added **recip rhythm spec.)
// Filename:      ...sig/examples/all/harm2root.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/harm2root.cpp
// Syntax:        C++; museinfo
//
// Description:   Convert a Humdrum **harm spine into a **root spine
//

#include "humdrum.h"
#include "PerlRegularExpression.h"

#include <string.h>
#include <ctype.h>

// function declarations
void      checkOptions      (Options& opts, int argc, char* argv[]);
void      example           (void);
void      usage             (const char* command);
void      generateAnalysis  (HumdrumFile& infile, Array<double>& durs);
void      doRhythmAnalysis  (HumdrumFile& infile, Array<double>& durs);
int       makeRootInterval  (const char* harmdata, int keyroot, int keymode);
void      getChordPitches   (Array<int>& pitches, const char* token, int root, 
                             int keyroot, int keymode);
void      printChordInfo    (const char* token,  int rootinterval, 
                             int keyroot, int keymode, double duration);
int       getInversion      (const char* token);
int       getScaleDegree    (const char* harm);
int       adjustKeyMode     (int keymode, int keyroot, const char* lastpart);
int       getRecipField     (int line, HumdrumFile& infile, int recip);
double    getRecipDuration  (HumdrumFile& infile, int line, int primarySpine);
int       checkForKeyDesignation(HumdrumFile& infile, int line);
int       checkForTimeSignature(HumdrumFile& infile, int line);

// global variables
Options   options;           // database for command-line arguments
int       debugQ       = 0;  // used with --debug option
int       appendQ      = 0;  // used with -a option
int       octave       = 2;  // used with -o option
int       rhythmQ      = 1;  // used with --RR option
int       rootQ        = 1;  // used with -r option 
int       recip        = -1; // used with -R option
int       recipField   = -1; // used with -R option
const char *instrument = ""; // used with -I option

///////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[]) {
   HumdrumFile infile;

   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   for (int i=0; i<numinputs || i==0; i++) {
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
      } else {
         infile.read(options.getArg(i+1));
      }

      Array<double> durs;
      durs.setSize(0);
      doRhythmAnalysis(infile, durs);
      generateAnalysis(infile, durs);
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// doRhythmAnalysis --
//

void doRhythmAnalysis(HumdrumFile& infile, Array<double>& durs) {
   int i, j;
   infile.analyzeRhythm();
   durs.setSize(infile.getNumLines());
   durs.setSize(0);
   double lastbeat = 0.0;
   double beat = 0.0;
   double duration = 0.0;

   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isData()) {
         for (j=infile[i].getFieldCount()-1; j>=0; j--) {
            if (strcmp(infile[i].getExInterp(j), "**harm") == 0) {
               if (strcmp(infile[i][j], ".") != 0) {
                  lastbeat = beat; 
                  beat = infile[i].getAbsBeat();
                  duration = beat - lastbeat;
                  durs.append(duration);
               }
            }
         }
      }
   } 

   for (i=0; i<durs.getSize()-1; i++)  {
      durs[i] = durs[i+1];
   }
   if (durs.getSize() > 0) {
      duration = infile[infile.getNumLines()-1].getAbsBeat() - beat;
      durs[durs.getSize()-1] = duration;
   }
}



//////////////////////////////
//
// generateAnalysis --
//

void generateAnalysis(HumdrumFile& infile, Array<double>& durs) {
   int i, j;
   int length;
   int keyroot = 2;
   int keymode = 0;      // 0 = major, 1 = minor
   int rootinterval = 0;
   int testpitch = 0;
   char buffer[128] = {0};
   int rindex = 0;
   double rdur;

   for (i=0; i<infile.getNumLines(); i++) {
      if (appendQ) {
         cout << infile[i];
      }
      if (infile[i].isData()) {
         for (j=0; j<infile[i].getFieldCount(); j++) {
            if (strcmp(infile[i].getExInterp(j), "**harm") == 0) {
               if (appendQ) {
                  cout << "\t";
               }
               if (strcmp(infile[i][j], ".") != 0) {
                  rootinterval = makeRootInterval(infile[i][j], keyroot, 
                        keymode);
                  if (rootQ) {
                     if (durs.getSize() > 0) {
                        Convert::durationToKernRhythm(buffer, durs[rindex++]);
                        if (rhythmQ && buffer[0] != '-') {
                           cout << buffer;
                        }
                     }
                     if (rootinterval >= 0) {
                        cout << Convert::base40ToKern(buffer, 
                              (keyroot + rootinterval) % 40 + octave * 40); 
                     } else {
                        cout << "r";
                     }
                     if (strchr(infile[i][j], ';') != NULL) {
                        cout << ';';
                     }
                  } else {
                     // print entire chord
                     if ((recipField <= 0) && (durs.getSize() > 0)) {
                        printChordInfo(infile[i][j], rootinterval, keyroot, 
                           keymode, durs[rindex++]);
                     } else if (recipField > 0) { 
                        rdur = getRecipDuration(infile, i, recipField);
                        printChordInfo(infile[i][j], rootinterval, keyroot, 
                           keymode, rdur);
                     } else {
                        printChordInfo(infile[i][j], rootinterval, keyroot, 
                           keymode, -1);
                     }
                  }
               } else {
                  cout << ".";
               }
            }
         }
      } else if(infile[i].isInterpretation()) {
         if (appendQ) {
            cout << "\t";
         }
         length = strlen(infile[i][0]);
         if (infile[i][0][length-1] == ':') {
            testpitch = Convert::kernToBase40(infile[i][0]);
            if (testpitch >= 0) {
               cout << infile[i][0];
               keyroot = testpitch % 40;
               keymode = islower(infile[i][0][1]) ? 1 : 0;
            }
         } else if (strncmp(infile[i][0], "**", 2) == 0) {
            if (rootQ) {
               cout << "**root";
            } else {
               cout << "**kern";
            }
	    if (recip >= 0) {
               recipField = getRecipField(i, infile, recip);
            }
         } else if (strncmp(infile[i][0], "*I", 2) == 0) {
            if (instrument[0] == '\0') {
               cout << "*";
            } else {
               cout << "*I" << instrument;
            }
         } else if (infile[i].equalFieldsQ()) {
            if (strcmp(infile[i][0], "*+") == 0) {
               cout << "*";
            } else if (strcmp(infile[i][0], "*^") == 0) {
               cout << "*";
            } else if (strcmp(infile[i][0], "*v") == 0) {
               cout << "*";
            } else if (strcmp(infile[i][0], "*x") == 0) {
               cout << "*";
            } else {
               cout << infile[i][0];
            }
         } else {
            int status = 0;
            status = checkForKeyDesignation(infile, i);
            if (status == 0) {
               status = checkForTimeSignature(infile, i);
            } 
            if (status == 0) {
               cout << "*";
            }
         }
      } else if (infile[i].isMeasure()) {
         if (appendQ) {
            cout << "\t";
         }
         cout << infile[i][0];
      } else if (infile[i].isLocalComment()) {
         if (appendQ) {
            cout << "\t";
         }
         cout << '!';
      } else if (infile[i].isGlobalComment()) {
         cout << infile[i];
      } else if (infile[i].isBibliographic()) {
         cout << infile[i];
      }
      if (appendQ) {
         cout << "\n";
      } else {
         cout << "\n";
      }
   }
}



//////////////////////////////
//
// checkForKeyDesignation -- look for a key designation on the current
//   line, and echo it in the output spine.
//

int checkForKeyDesignation(HumdrumFile& infile, int line) {
   if (!infile[line].isInterpretation()) {
      return 0;
   }
   PerlRegularExpression pre;
   int j;
   for (j=0; j<infile[line].getFieldCount(); j++) {
      if (pre.search(infile[line][j], "^\\*[A-G]([#-]*):", "i")) {
         cout << infile[line][j];
         return 1;
      }
   }

   return 0;
}



//////////////////////////////
//
// checkForTimeSignature -- look for a time signature on the current
//   line in the input **recip data if it exists, and echo it in 
//   the output spine.
//

int checkForTimeSignature(HumdrumFile& infile, int line) {
   if (!infile[line].isInterpretation()) {
      return 0;
   }
   if (recipField <= 0) {
      return 0;
   }

   PerlRegularExpression pre;
   int j;
   for (j=0; j<infile[line].getFieldCount(); j++) {
      if (recipField != infile[line].getPrimaryTrack(j)) {
         continue;
      }
      if (pre.search(infile[line][j], "^\\*M\\d+/\\d+$")) {
         cout << infile[line][j];
         return 1;
      }
   }

   return 0;
}



//////////////////////////////
//
// getRecipDuration -- 
//

double getRecipDuration(HumdrumFile& infile, int line, int primarySpine) {
   int ii, jj;
   int j;
   for (j=0; j<infile[line].getFieldCount(); j++) {
      if (primarySpine != infile[line].getPrimaryTrack(j)) {
         continue;
      }
      if (!infile[line].isExInterp(j, "**recip")) {
         // maybe also allow **kern spines to define rhythm here someday.
         return -1;
      }
      if (strcmp(infile[line][j], ".") == 0) {
         ii = infile[line].getDotLine(j);
         jj = infile[line].getDotSpine(j);
      } else {
         ii = line;
         jj = j;
      }
      return Convert::kernToDuration(infile[ii][jj]);
   }

   return -1.0;
}



//////////////////////////////
//
// getRecipField -- get the primary spine number of the nth (offset from 1)
//     **recip spine from left to right in the file.
//

int getRecipField(int line, HumdrumFile& infile, int recip) {
   if (recip < 1) {
      return -1;
   }

   int output = -1;
   int counter = 0;
   int j;
   for (j=0; j<infile[line].getFieldCount(); j++) {
      if (infile[line].isExInterp(j, "**recip")) {
         counter++;
         if (counter == recip) {
            return j+1;
         }
      }
   }
   if (counter > 0) {
      output = 1;
   }
 
   return output;
}



//////////////////////////////
//
// printChordInfo --
//

void printChordInfo(const char* token,  int root, 
      int keyroot, int keymode, double duration) {
   int i;
   char dbuffer[128] = {0};
   char pbuffer[128] = {0};
   Convert::durationToKernRhythm(dbuffer, duration);
   if (!rhythmQ) {
      dbuffer[0] = '\0';
   }

   Array<int> pitches;
   if (root >= 0) {
      getChordPitches(pitches, token, root, keyroot, keymode);
      for (i=0; i<pitches.getSize(); i++) {
         if (duration > 0) {
            cout << dbuffer;
         }
         cout << Convert::base40ToKern(pbuffer, pitches[i]);
         if (strchr(token, ';') != NULL) {
            cout << ';';
         }
         if (i<pitches.getSize()-1) {
            cout << ' ';
         }
      }
   } else {
      if (duration > 0) {
         cout << dbuffer;
      }
      cout << "r";
      if (strchr(token, ';') != NULL) {
         cout << ';';
      }
   }
}




//////////////////////////////
//
// getChordPitches --
//

void getChordPitches(Array<int>& pitches, const char* token, int root, 
   int keyroot, int keymode) {

   int oct = octave + 2;
   int inversion = getInversion(token);

   pitches.setSize(0);

   // store the root pitch
   int pitch = (root + keyroot) % 40 + oct * 40;
   int base = pitch;
   if (inversion == 0) {
      pitch = pitch - 40;
   }
   pitches.append(pitch);

   char firstpart[128] = {0};
   char lastpart[128] = {0};
   strcpy(firstpart, token);
   int i;
   int length = strlen(firstpart);
   for (i=0; i<length; i++) {
      if (firstpart[i] == '/') {
         firstpart[i] = '\0';
         break;
      }
   }

   int offset = 0;

   // determine the third
   if (strchr(firstpart, 'i') != NULL) {
      pitch = base + 11;
      pitches.append(pitch);
   } else if (strchr(firstpart, 'I') != NULL) {
      pitch = base + 12;
      pitches.append(pitch);
   } else if (strchr(firstpart, 'v') != NULL) {
      pitch = base + 11;
      pitches.append(pitch);
   } else if (strchr(firstpart, 'V') != NULL) {
      pitch = base + 12;
      pitches.append(pitch);
   } else if (strchr(firstpart, 'N') != NULL) {
      pitch = base + 12;
      pitches.append(pitch);
   } else if (strstr(firstpart, "Lt") != NULL) {
      pitch = base + 12;
      pitches.append(pitch);
   } else if (strstr(firstpart, "Gn") != NULL) {
      pitch = base + 12;
      pitches.append(pitch);
   } else if (strstr(firstpart, "Fr") != NULL) {
      pitch = base + 12;
      pitches.append(pitch);
   }
   if (inversion == 1) {
      pitches[1] = pitches[1] % 40 + 40 * (oct - 1);
   }
   pitches[1] = pitches[1] + offset;

   // determine the fifth
   if (strchr(firstpart, 'o') != NULL) {
      pitch = base + 22;
      pitches.append(pitch);
   } else if (strchr(firstpart, '+') != NULL) {
      pitch = base + 24;
      pitches.append(pitch);
   } else {
      pitch = base + 23;
      pitches.append(pitch);
   }
   if (inversion == 2) {
      pitches[2] = pitches[2] % 40 + 40 * (oct - 1);
   }
   pitches[2] = pitches[2] + offset;


   // determine the seventh

   if (strchr(firstpart, '7') == NULL) {
      return;
   }

   if (strchr(token, '/') != NULL) {
      strcpy(lastpart, strchr(token, '/')+1);
      offset = makeRootInterval(lastpart, keyroot, keymode);
      keymode = adjustKeyMode(keymode, keyroot, lastpart);
   } 

   int rootscaledegree = getScaleDegree(firstpart);
   int seventhdegree = (rootscaledegree + 6) % 7;

   // int pitch = (root + keyroot) % 40 + oct * 40;
   // int base = pitch;
   
   pitches.append(pitch);
   if (strstr(firstpart, "D7") != NULL) {
      pitches[3] = base + 33;
      offset = 0;
   } else if (keymode == 0) {
      // major key
      int degrees[7] = {0, 6, 12, 17, 23, 29, 35};
      pitches[3] = (keyroot + degrees[seventhdegree]) % 40 + oct * 40;
      if (pitches[3] - base < 25) {
         pitches[3] += 40;
      } 
      //if (base % 40 > 25) {
      //   pitches[3] += 40;
      //}
   } else {
      // minor key (harmonic minor used)
      int degrees[7] = {0, 6, 11, 17, 23, 28, 35};
      pitches[3] = (keyroot + degrees[seventhdegree]) % 40 + oct * 40;
      if (base % 40 > 25) {
         pitches[3] += 40;
      }
   } 

   // fix the octave placement of the seventh (to be above the fifth)
   pitches[3] = pitches[3] + offset;
   if (pitches[3] - pitches[2] < 0) {
      pitches[3] += 40;
   }

   // put 3rd inversions in the octave below middle C
   if (inversion == 3) {
      pitches[3] = pitches[3] % 40 + 40 * (oct - 1);
   }

}



//////////////////////////////
//
// adjustKeyMode --
//

int adjustKeyMode(int keymode, int keyroot, const char* lastpart) {
   char string[128] = {0};
   strcpy(string, lastpart);
   const char* ptr = lastpart;
   if (strchr(string, '/') != NULL) {
      ptr = strchr(string, '/') + 1;
   } 
   if (strchr(ptr, 'v') != NULL) {
      keymode = 1;
   } else if (strchr(ptr, 'i') != NULL) {
      keymode = 1;
   } else if (strchr(ptr, 'V') != NULL) {
      keymode = 0;
   } else if (strchr(ptr, 'I') != NULL) {
      keymode = 0;
   } else { // don't know the key so assume major
      keymode = 0;
   }

   return keymode;
}



//////////////////////////////
//
// getScaleDegree -- 
//

int getScaleDegree(const char* harm) {
   if (strstr(harm, "VII") != NULL) { return 6; } 
   if (strstr(harm, "vii") != NULL) { return 6; } 
   if (strstr(harm, "VI")  != NULL) { return 5; } 
   if (strstr(harm, "vi")  != NULL) { return 5; } 
   if (strstr(harm, "IV")  != NULL) { return 3; } 
   if (strstr(harm, "iv")  != NULL) { return 3; } 
   if (strstr(harm, "V")   != NULL) { return 4; } 
   if (strstr(harm, "v")   != NULL) { return 4; } 
   if (strstr(harm, "III") != NULL) { return 2; } 
   if (strstr(harm, "iii") != NULL) { return 2; } 
   if (strstr(harm, "II")  != NULL) { return 1; } 
   if (strstr(harm, "ii")  != NULL) { return 1; } 
   if (strstr(harm, "I")   != NULL) { return 0; } 
   if (strstr(harm, "i")   != NULL) { return 0; } 
   if (strstr(harm, "Fr")  != NULL) { return 5; }
   if (strstr(harm, "Gn")  != NULL) { return 5; }
   if (strstr(harm, "Lt")  != NULL) { return 5; }
   if (strstr(harm, "N")   != NULL) { return 1; }
   return 0;
}



//////////////////////////////
//
// getInversion --
//

int getInversion(const char* token) {
   if (strchr(token, 'b') != NULL) { return 1; }
   if (strchr(token, 'c') != NULL) { return 2; }
   if (strchr(token, 'd') != NULL) { return 3; }
   return 0;
}



//////////////////////////////
//
// makeRootInterval --
//

int makeRootInterval(const char* harmdata, int keyroot, int keymode) {
   int slashcount = 0;
   int i;
   int output = 0;
   int length = strlen(harmdata);
   char abuffer[128] = {0};
   strcpy(abuffer, harmdata);

   for (i=0; i<length; i++) {
      if (abuffer[i] == '/') {
         slashcount++;
         abuffer[i] = '\0';
      }
   }
   int offset = 0;
   
   if (slashcount >= 1) {
      const char* ptr = strchr(harmdata, '/') + 1;
      offset = makeRootInterval(ptr, keyroot, keymode);
   } 

   if (keymode) {
      // minor mode (harmonic minor)
      if      (strstr(abuffer, "-vii") != 0) { output = E_base40_min7; }
      else if (strstr(abuffer, "vii")  != 0) { output = E_base40_maj7; }
      else if (strstr(abuffer, "-VII") != 0) { output = E_base40_min7; }
      else if (strstr(abuffer, "VII")  != 0) { output = E_base40_maj7; }
      else if (strstr(abuffer, "#vi")  != 0) { output = E_base40_maj6; }
      else if (strstr(abuffer, "vi")   != 0) { output = E_base40_min6; }
      else if (strstr(abuffer, "#VI")  != 0) { output = E_base40_maj6; }
      else if (strstr(abuffer, "VI")   != 0) { output = E_base40_min6; }
      else if (strstr(abuffer, "iv")   != 0) { output = E_base40_per4; }
      else if (strstr(abuffer, "IV")   != 0) { output = E_base40_per4; }
      else if (strstr(abuffer, "#iii") != 0) { output = E_base40_maj3; }
      else if (strstr(abuffer, "#III") != 0) { output = E_base40_maj3; }
      else if (strstr(abuffer, "iii")  != 0) { output = E_base40_min3; }
      else if (strstr(abuffer, "III")  != 0) { output = E_base40_min3; }
      else if (strstr(abuffer, "-ii")  != 0) { output = E_base40_min2; }
      else if (strstr(abuffer, "-II")  != 0) { output = E_base40_min2; }
      else if (strstr(abuffer, "ii")   != 0) { output = E_base40_maj2; }
      else if (strstr(abuffer, "II")   != 0) { output = E_base40_maj2; }
      else if (strstr(abuffer, "N")    != 0) { output = E_base40_min2; }
      else if (strstr(abuffer, "v")    != 0) { output = E_base40_per5; }
      else if (strstr(abuffer, "V")    != 0) { output = E_base40_per5; }
      else if (strstr(abuffer, "i")    != 0) { output = E_base40_per1; }
      else if (strstr(abuffer, "I")    != 0) { output = E_base40_per1; }
      else if (strstr(abuffer, "Lt")   != 0) { output = E_base40_aug4; }
      else if (strstr(abuffer, "Gn")   != 0) { output = E_base40_aug4; }
      else if (strstr(abuffer, "Fr")   != 0) { output = E_base40_aug4; }
      else { output = E_base40_rest; }
   } else {
      // major mode
      if      (strstr(abuffer, "-vii") != 0) { output = E_base40_min7; }
      else if (strstr(abuffer, "vii")  != 0) { output = E_base40_maj7; }
      else if (strstr(abuffer, "-VII") != 0) { output = E_base40_min7; }
      else if (strstr(abuffer, "VII")  != 0) { output = E_base40_maj7; }
      else if (strstr(abuffer, "-vi")  != 0) { output = E_base40_min6; }
      else if (strstr(abuffer, "vi")   != 0) { output = E_base40_maj6; }
      else if (strstr(abuffer, "-VI")  != 0) { output = E_base40_min6; }
      else if (strstr(abuffer, "VI")   != 0) { output = E_base40_maj6; }
      else if (strstr(abuffer, "iv")   != 0) { output = E_base40_per4; }
      else if (strstr(abuffer, "IV")   != 0) { output = E_base40_per4; }
      else if (strstr(abuffer, "-iii") != 0) { output = E_base40_min3; }
      else if (strstr(abuffer, "-III") != 0) { output = E_base40_min3; }
      else if (strstr(abuffer, "iii")  != 0) { output = E_base40_maj3; }
      else if (strstr(abuffer, "III")  != 0) { output = E_base40_maj3; }
      else if (strstr(abuffer, "-ii")  != 0) { output = E_base40_min2; }
      else if (strstr(abuffer, "-II")  != 0) { output = E_base40_min2; }
      else if (strstr(abuffer, "ii")   != 0) { output = E_base40_maj2; }
      else if (strstr(abuffer, "II")   != 0) { output = E_base40_maj2; }
      else if (strstr(abuffer, "N")    != 0) { output = E_base40_min2; }
      else if (strstr(abuffer, "v")    != 0) { output = E_base40_per5; }
      else if (strstr(abuffer, "V")    != 0) { output = E_base40_per5; }
      else if (strstr(abuffer, "i")    != 0) { output = E_base40_per1; }
      else if (strstr(abuffer, "I")    != 0) { output = E_base40_per1; }
      else if (strstr(abuffer, "Lt")   != 0) { output = E_base40_aug4; }
      else if (strstr(abuffer, "Gn")   != 0) { output = E_base40_aug4; }
      else if (strstr(abuffer, "Fr")   != 0) { output = E_base40_aug4; }
      else { output = E_base40_rest; }
   }

   if (output < 0) {
      return output;
   } else {
      return (output + offset) % 40;
   }
}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("a|append=b",         "append analysis data to input");
   opts.define("o|octave=i:2",       "octave to output root pitch");
   opts.define("RR|no-rhythm=b",     "don't try to do any rhythm analysis");
   opts.define("R|recip=i:-1",       "use **recip column for rhythm");
   opts.define("r|root=b",           "extract only root information");
   opts.define("b|bass|bass-line=b", "extract only bass-line information");
   opts.define("I|instrument=s",     "instrument to play music on");

   opts.define("debug=b",          "trace input parsing");   
   opts.define("author=b",         "author of the program");   
   opts.define("version=b",        "compilation information"); 
   opts.define("example=b",        "example usage"); 
   opts.define("help=b",           "short description"); 
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, July 2003" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: July 2003" << endl;
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

   debugQ     =  opts.getBoolean("debug");
   appendQ    =  opts.getBoolean("append");
   rhythmQ    = !opts.getBoolean("no-rhythm");
   octave     =  opts.getInteger("octave");
   rootQ      =  opts.getBoolean("root");
   instrument =  opts.getString("instrument");
   recip      =  opts.getInteger("recip");
}



//////////////////////////////
//
// example -- example usage of the program
//

void example(void) {
   cout <<
   "                                                                        \n"
   << endl;
}



//////////////////////////////
//
// usage -- gives the usage statement for the program
//

void usage(const char* command) {
   cout <<
   "                                                                        \n"
   << endl;
}


// md5sum: 89e0dd0ee8fac22256dac44d0f7d9bdb harm2kern.cpp [20100505]

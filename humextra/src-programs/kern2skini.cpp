//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 12 23:17:41 PDT 2001
// Last Modified: Fri Jun 12 22:58:34 PDT 2009 (renamed SigCollection class)
// Filename:      ...sig/examples/all/kern2skini.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/kern2skini.cpp
// Syntax:        C++; museinfo
//
// Description:   Generates a list of pitches in a Humdrum file 
//                according to the STK SKINI format.
//

#include "humdrum.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>

#ifndef OLDCPP
   #include <iostream>
   #include <iomanip>
#else
   #include <iostream.h>
   #include <iomanip.h>
#endif

typedef long TEMP64BITFIX;

class SKINI {
   public:
      int message;
      double time;
      TEMP64BITFIX chan;
      double note;
      double vel;
      double id;
};

#define UNKNOWN     -1
#define MEASURE      0
#define NOTEON       1
#define NOTEOFF      2
#define COMMENT      3
#define TEMPO        4
#define KEYSIG       5
#define INFO         6
#define INSTRUMENT   7

// function declarations:
void      checkOptions      (Options& opts, int argc, char** argv);
void      example           (void);
void      preparePitch      (char* buffer2, const char* buffer1);
void      generateSkini     (HumdrumFile& hfile, SigCollection<SKINI>& skini);
void      printSkini        (SigCollection<SKINI>& skini);
void      usage             (const char* command);
ostream&  operator<<        (ostream& out, SKINI skini);
int       skinicompare      (const void* a, const void* b);

// User interface variables:
Options   options;
int       barlinesQ = 1;      // used with the -B option
int       midinoteQ = 0;      // used with the -m option
int       classQ    = 0;      // used with the -c option
double    tdefault  = 60.0;   // used with the -t option

int       idcounter = 0;      // used to sort output data

//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   // process the command-line options
   checkOptions(options, argc, argv);

   SigCollection<SKINI> skini;
   skini.setGrowth(100000);
   skini.setSize(100000);
   skini.setSize(0);
   skini.allowGrowth(1);

   HumdrumFile hfile(options.getArg(1));
   hfile.analyzeRhythm();
   generateSkini(hfile, skini);
   printSkini(skini);
   return 0;
}

//////////////////////////////////////////////////////////////////////////



//////////////////////////////
//
// printSkini -- sort SKINI messages and output
//

void printSkini(SigCollection<SKINI>& skini) {
   int i;

   qsort(skini.getBase(), skini.getSize(), sizeof(SKINI), skinicompare);
 
   // convert to delta times:
   for (i=skini.getSize()-1; i>0; i--) {
      skini[i].time = skini[i].time - skini[i-1].time; 
      if (skini[i].time < 0.00001) {
         skini[i].time  = 0.0;
      }
   }
   skini[0].time = 0.0;

   for (i=0; i<skini.getSize(); i++) {
      cout << skini[i] << endl;
   }
}



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
// generateSkini -- one small bug: the duration of the notes
//   are calculated under constant tempo.  A tempo change during
//   a tied note will not give the currect ending point of the
//   tied note.  This bug is minor and the work to fix it is major.
//

void generateSkini(HumdrumFile& hfile, SigCollection<SKINI>& skini) {
   Array<double> tempo;
   hfile.analyzeTempoMarkings(tempo, tdefault);

   double currentmillisecond = 0.0;
   double lastduration = 0.0;
   double endmillisecond;
   SKINI tempskini;

   int i, j, k;
   char buffer1[1024] = {0};
   char buffer2[1024] = {0};
   double duration;
   for (i=0; i<hfile.getNumLines(); i++) {
      currentmillisecond = currentmillisecond + 
            lastduration * 60000.0 / tempo[i];
      lastduration = hfile[i].getDuration();

      if (hfile[i].getType() == E_humrec_global_comment) {
         tempskini.message = COMMENT;
         tempskini.time = currentmillisecond/1000.0;
         if (hfile[i][0][2] == ' ') {
            tempskini.chan = (TEMP64BITFIX)(&hfile[i][0][3]);
         } else {
            tempskini.chan = (TEMP64BITFIX)(&hfile[i][0][2]);
         }
         tempskini.note = 0;
         tempskini.vel  = 0;
         tempskini.id   = idcounter;
         skini.append(tempskini);
         idcounter++;
         // cout << "// " << &(hfile[i].getLine()[3]) << endl;
      }

      if (hfile[i].getType() == E_humrec_bibliography) {
         tempskini.message = INFO;
         tempskini.time = currentmillisecond/1000.0;
         if (hfile[i][0][3] == ' ') {
            tempskini.chan = (TEMP64BITFIX)(&hfile[i][0][4]);
         } else {
            tempskini.chan = (TEMP64BITFIX)(&hfile[i][0][3]);
         }
         tempskini.note = 0;
         tempskini.vel  = 0;
         tempskini.id   = idcounter;
         skini.append(tempskini);
         idcounter++;
         // cout << "///" << &(hfile[i].getLine()[3]) << endl;
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
               tempskini.message = KEYSIG;
               tempskini.time = currentmillisecond/1000.0;
               tempskini.chan = acount;
               tempskini.note = 0;
               tempskini.vel  = 0;
               tempskini.id   = idcounter;
               idcounter++;
               if (direction == 0) {
                  skini.append(tempskini);
                  break;
               } else {
                  if (direction < 0) {
                     tempskini.chan *= -1;
                  }
                  skini.append(tempskini);
                  break;
               }
            }
         }
      }

      // look for key interpretations
      /*
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
               cout << "// key ";
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
      */

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
               // cout << "// Tempo " << tempomark << " MM per quarter note" 
               //      << endl;
               tempskini.message = TEMPO;
               tempskini.time = currentmillisecond/1000.0;
               tempskini.chan = 0;
               tempskini.note = 0;
               tempskini.vel  = tempomark;
               tempskini.id   = idcounter;
               skini.append(tempskini);
               idcounter++;

               break;
            }
         }
      }

      // look for instrument names
      if (hfile[i].getType() == E_humrec_interpretation) {
         for (j=0; j<hfile[i].getFieldCount(); j++) {
            if (hfile[i].getExInterpNum(j) != E_KERN_EXINT) {
               continue;
            }
            if (strncmp(hfile[i][j], "*I", 2) == 0 && 
                  islower(hfile[i][j][2])) {
               tempskini.message = INSTRUMENT;
               tempskini.time = currentmillisecond/1000.0;
               tempskini.chan = (TEMP64BITFIX)&hfile[i][j][2];
               tempskini.note = hfile[i].getPrimaryTrack(j);
               tempskini.vel  = tempskini.time;
               tempskini.id   = idcounter;
               skini.append(tempskini);
               idcounter++;
            }
         }
      }


      // look for Barlines
      if (hfile[i].getType() == E_humrec_data_measure) {
         int mnum = -1;
         int flag = 0;
         flag = sscanf(hfile[i][0], "=%d", &mnum);
         if (flag == 1) {
            tempskini.message = MEASURE;
            tempskini.time = currentmillisecond/1000.0;
            tempskini.chan = mnum;
            tempskini.note = 0;
            tempskini.vel  = tempskini.time;
            tempskini.id   = idcounter;
            skini.append(tempskini);
            idcounter++;
         } else {
            tempskini.message = MEASURE;
            tempskini.time = currentmillisecond/1000.0;
            tempskini.chan = -1;
            tempskini.note = 0;
            tempskini.vel  = tempskini.time;
            tempskini.id   = idcounter;
            skini.append(tempskini);
            idcounter++;
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

            tempskini.message = NOTEON;
            tempskini.time = currentmillisecond/1000.0;
            tempskini.chan = hfile[i].getPrimaryTrack(j);
            tempskini.note = note;
            tempskini.vel  = 64;
            tempskini.id   = idcounter;
            skini.append(tempskini);
 

            endmillisecond = currentmillisecond + duration * 60000 / tempo[i];
            tempskini.message = NOTEOFF;
            tempskini.time = endmillisecond/1000.0;
            tempskini.chan = hfile[i].getPrimaryTrack(j);
            tempskini.note = note;
            tempskini.vel  = 64;
            tempskini.id   = idcounter;
            skini.append(tempskini);

            idcounter++;
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



//////////////////////////////
//
// operator<<SKINI --
//

ostream& operator<<(ostream& out, SKINI skini) {

   if (skini.message == MEASURE) {
      out << "\n";
      out << "// Measure ";
      if (skini.chan > 0) {
         out << "number " << skini.chan; 
      } 
      out << "\t=" << skini.vel;

      return out;
   }

   if (skini.message == COMMENT) {
      out << "// " << (char*)(skini.chan);
      return out;
   }

   if (skini.message == INFO) {
      out << "///" << (char*)(skini.chan);
      return out;
   }

   if (skini.message == TEMPO) {
      out << "// Tempo " << skini.vel << " MM per quarter note";
      return out;
   }

   if (skini.message == INSTRUMENT) {
      out << "// Instrument <" << (char*)skini.chan << "> for channel "
          << skini.note;
      return out;
   }

   if (skini.message == KEYSIG) {
      out << "// Keysig " << labs(skini.chan);
      if (skini.chan == 0) {
         out << " accidentals";
      } else if (skini.chan < 0) {
         out << " flat";
      } else {
         out << " sharp";
      }
      if (labs(skini.chan) > 1) {
         out << "s";
      }
      return out;
   }

   switch (skini.message) {
      case NOTEON:  out << "NoteOn   \t"; break;
      case NOTEOFF: out << "NoteOff  \t"; break;
      default:      out << "UNKNOWN  \t";
   }
   // send time value:
   out.setf(ios::left, ios::adjustfield);
   out << setw(10) << skini.time << "   ";

   // send channel:
   out << skini.chan << "\t";

   // send note number:
   out << "  " << skini.note << "\t";

   // send attack/release velocity:
   out << "  " << skini.vel;

   // out << "\t[" << skini.id << "]";

   return out;
}



///////////////////////////////
//
// skinicompare -- for sorting the times of SKINI messages
//

int skinicompare(const void* a, const void* b) {
   SKINI& aa = *((SKINI*)a);
   SKINI& bb = *((SKINI*)b);

   if (fabs(aa.time - bb.time) < 0.0001) {
      // break ties by the id number
      if (aa.id < bb.id) {
         return -1;
      } else if (aa.id > bb.id) {
         return 1;
      } else {
         return 0;
      }
   } else if (aa.time < bb.time) {
      return -1;
   } else if (aa.time > bb.time) {
      return 1;
   }

   return 1;
}
    


// md5sum: 810dfce9fe34da8fae775599337dfbb3 kern2skini.cpp [20100905]

//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Dec 13 13:35:37 PST 2000
// Last Modified: Sun Oct  9 02:06:07 PDT 2005 (converted from kern2melisma)
// Last Modified: Tue Dec 13 22:16:14 PST 2005 (small fixes)
// Filename:      ...sig/examples/all/time2matlab.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/time2matlab.cpp
// Syntax:        C++; museinfo
//
// Description:   Extracts performance times of Humdrum **kern data according
//                to information found in a **time spine on the line.
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
double    getEndTime        (HumdrumFile& hfile, int startindex, 
                             double duration);
int       getMetricLevel    (HumdrumFile& hfile, int index);
int       getMeasureNum     (HumdrumFile& hfile, int index);
double    getTimeToEnd      (HumdrumFile& infile, double starttime, 
                             int startindex);
void      printSaccid       (ostream& out, const char* string);
void      comment           (ostream& out, int count, int style);

// User interface variables:
Options   options;
int       debugQ    = 0;      // used with --debug option
int       barlinesQ = 1;      // used with -B option
int       midinoteQ = 0;      // used with -m option
int       classQ    = 0;      // used with -c option
int       auxdataQ  = 0;      // used with -a option
double    tdefault  = 60.0;   // used with -t option
int       humdrumQ  = 0;      // used with --humdrum option

char      humComment = '!';
char      matComment = '%';

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
   opts.define("a|auxiliary-data=b", "output extra notation info");
   opts.define("B|nobarlines=b", "don't display barlines");
   opts.define("m|midi|MIDI=b", "display pitches as MIDI note numbers");
   opts.define("c|class=b", "display pitches in pitch class notation");
   opts.define("t|tempo|default-tempo=d:60.0", "tempo if none specified");
   opts.define("humdrum=b", "print data in Humdrum file format");

   opts.define("debug=b", "Debugging flag");
   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("h|help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Oct 2005" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 8 Oct 2005" << endl;
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

   debugQ = opts.getBoolean("debug");

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
   auxdataQ = opts.getBoolean("auxiliary-data");
   humdrumQ = opts.getBoolean("humdrum");
}



//////////////////////////////
//
// example --
//

void example(void) {
   // someone is lazy and has not put an example here.
}



//////////////////////////////
//
// comment --
//

void comment(ostream& out, int count, int style) {
   int i;
   for (i=0; i<count; i++) {
      switch (style) {
         case 0:  out << matComment; break;
         default: out << humComment;
      }
   }

}



//////////////////////////////
//
// printOutput -- 
//

void printOutput(HumdrumFile& hfile) {
   Array<double> tempo;
   hfile.analyzeTempoMarkings(tempo, tdefault);

   double currentmillisecond = 0.0;
   double lastduration = 0.0;

   double linestarttime = 0.0;
   double lineendtime = 0.0;

   int founddata = 0;

   int metlev;
   int i, j, k;
   char buffer1[1024] = {0};
   char buffer2[1024] = {0};
   double duration;
   for (i=0; i<hfile.getNumLines(); i++) {
      currentmillisecond = currentmillisecond + 
            lastduration * 60000.0 / tempo[i];
      lastduration = hfile[i].getDuration();

      if (hfile[i].getType() == E_humrec_global_comment) {
         cout << "%% " << &(hfile[i].getLine()[3]) << endl;
      }

      if (hfile[i].getType() == E_humrec_bibliography) {
         cout << "%%%" << &(hfile[i].getLine()[3]) << endl;
      }


      if (hfile[i].getType() != E_humrec_data) {
         continue;
      }

      if (debugQ) {
         cout << "RECORD: " << hfile[i] << endl;
      }

      if (founddata == 0) {
         founddata = 1;

         comment(cout, 2, humdrumQ);
         cout << " Data column information:\n";
         comment(cout, 3, humdrumQ);
         cout << "col01: abstime\t" << 
                 "(average absolute time in milliseconds of human "
              << "beats)\n";
         comment(cout, 3, humdrumQ);
         cout << "col02: duration\t" <<
                 "(expected duration in ms based "
              << "on score duration)\n";
         comment(cout, 3, humdrumQ);
         cout << "col03: note\t\t" << 
                 "(MIDI note number of pitch)\n";
         comment(cout, 3, humdrumQ);
         cout << "col04: metlev\t" << 
                 "(metric level: 1 = downbeat; "
              <<  "0 = beat; -1 = offbeat)\n";
         comment(cout, 3, humdrumQ);
         cout << "col05: measure\t" <<
                  "(measure number in which note occurs)\n";
         comment(cout, 3, humdrumQ);
         cout << "col06: absbeat\t" <<
                 "(absolute beat from starting beat at 0)\n";
         comment(cout, 3, humdrumQ);
         cout << "col07: trackno\t" << 
                 "(hand which plays the note: 1 = left; 2=right)\n";
	 if (!auxdataQ) {
            comment(cout, 3, humdrumQ);
            cout << "col08: mintime\t" << 
                    "(minimum absolute time of human beat for this note)\n";
            comment(cout, 3, humdrumQ);
            cout << "col09: maxtime\t" << 
                    "(maximum absolute time of human beat for this note)\n";
            comment(cout, 3, humdrumQ);
            cout << "col10: sd\t\t" << 
                    "(standard deviation of human beat time in ms.)\n";
	 } else {
            //comment(cout, 3, humdrumQ);
            //cout << "col08: paccid\t" <<
	    //	    "(printed accidental 0=none, -1=flat, +1=sharp, 10=nat)\n";
            comment(cout, 3, humdrumQ);
            cout << "col08: saccid\t" <<
		    "(sounding accidental 0=natural, -1=flat, +1=sharp)\n";
         }
		   
         if (humdrumQ) {
            cout << "**start\t**dur\t**key\t**metr\t**meas\t**absb\t**track\t**saccid\n";
         }
      }
	        

      linestarttime = -1.0;
      // find current time value and save 
      for (j=0; j<hfile[i].getFieldCount(); j++) {
         if (strcmp(hfile[i].getExInterp(j), "**time") == 0) {
            sscanf(hfile[i][j], "%lf", &linestarttime);
            break;
         }
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
            lineendtime = getEndTime(hfile, i, duration);

            int note = Convert::kernToMidiNoteNumber(buffer2);
            if (classQ) {
               note = note % 12;
            }
            if (note < 0) {
               // don't display rests.
               continue;
            }
            if (note < 0 || note > 127) {
               cerr << "Error reading MIDI pitch number from string: " 
                    << buffer2 << endl;
               exit(1);
            }

            //cout << "Note\t";
            // cout << hfile.getAbsBeat(i) << "\t";
            // cout << (int)(currentmillisecond+0.5) << "\t";
            cout << linestarttime << "\t";

            if (debugQ && (lineendtime - linestarttime < 0)) {
               cerr << "Error duration of note on line: " << hfile[i] << endl;
               cerr << "Starttime: " << linestarttime << endl;
               cerr << "Endtime:   " << lineendtime << endl;
               cerr << "Line Index:     " << i << endl;
               exit(1);
            } 
            if (lineendtime != -1) {
               cout << lineendtime - linestarttime << "\t";
            } else {
               cout << (int)(getTimeToEnd(hfile, linestarttime, i) + 0.5) 
                    << "\t";
            }

            cout << note;

            cout << "\t";
            
            metlev = getMetricLevel(hfile, i);
            cout << metlev;

            cout << "\t" << getMeasureNum(hfile, i);

            cout << "\t" << hfile[i].getAbsBeat();

            // you must make sure that the spine order is
            // correct or this data will be bad
            cout << "\t" << hfile[i].getPrimaryTrack(j)-1;
   
            cout << "\t"; 
	    printSaccid(cout, buffer2);
 
            cout << "\n";
         }
      }
   }

   if (humdrumQ) {
      cout << "*-\t*-\t*-\t*-\t*-\t*-\t*-\t*-\n";
   }
}



//////////////////////////////
//
// printSaccid -- print the souding accidental of the note.
//    0 = natural without an accidental sign, 10 = natural with
//    a natural sign, -1 is flat, -2 double flat, +1 = sharp, 
//    +2 = double sharp.  10 is added (subtracted) to sharps (flats)
//    values if the sharp or flat is required to be printed as
//    a cautionary sign.
//

void printSaccid(ostream& out, const char* string) {
   if (strchr(string, 'n') != NULL) {
      out << 10;
      return;
   }

   int i;
   int accid = 0;
   int length = strlen(string);
   int literal = 0;
   for (i=0; i<length; i++) {
      if (string[i] == '-') {
         accid--;
      }
      if (string[i] == '#') {
         accid++;
      }
      if (string[i] == 'X') {
         literal = 1;
      }
   }
   if (literal) {
      if (accid > 0) {
         accid = accid+10;
      } else if (accid < 0) {
         accid = accid-10;
      }
   }
   out << accid;
   return;
}



//////////////////////////////
//
// getTimeToEnd --
//

double getTimeToEnd(HumdrumFile& infile, double starttime, int startindex) {


   double ctime = starttime;
   double cbeat = infile[startindex].getAbsBeat();
   double nbeat = infile[infile.getNumLines()-1].getAbsBeat();

   int preindex = -1;

   int i;
   for (i=startindex-1; i>=0; i--) {
      if (infile[i].getType() != E_humrec_data) {
         continue;
      }

      preindex = i;
      break;
   }

   if (preindex < 0) {
       return -1;
   }

   double pbeat = infile[preindex].getAbsBeat();
   double ptime = -1.0;

   for (i=0; i<infile[preindex].getFieldCount(); i++) {
      if (strcmp("**time", infile[preindex].getExInterp(i)) != 0) {
         continue;
      }

      sscanf(infile[preindex][i], "%lf", &ptime);
      break;
   }

   if (ptime < 0.0) {
      return -1;
   }

   double db2 = nbeat - cbeat;
   double db1 = cbeat - pbeat;
   double dt1 = ctime - ptime;

   // cout << "<< DB1 = " << db1 << " >> ";
   // cout << "<< DB2 = " << db2 << " >> ";
   // cout << "<< DT1 = " << dt1 << " >> ";
   
   return db2 * dt1 / db1;
}



//////////////////////////////
//
// getMeasureNum --
//

int getMeasureNum(HumdrumFile& hfile, int index) {
   int output = -1;

   int i;
   for (i=index; i>=0; i--) {
      if (hfile[i][0][0] == '=' && isdigit(hfile[i][0][1])) {
         sscanf(hfile[i][0], "=%d", &output);
         break;
      }
   }

   if (i <= 0) {
      output = 0;
   }


   return output;
}



//////////////////////////////
//
// getEndTime -- return the **time value at the given duration point after
//     the given index.
//

double getEndTime(HumdrumFile& hfile, int startindex, double duration) {
   double stopbeat = duration + hfile[startindex].getAbsBeat();
   int i, j;
   double output = -1.0;

   for (i=startindex+1; i<hfile.getNumLines(); i++) {
      if (hfile[i].getType() != E_humrec_data) {
         continue;
      }
      if (hfile[i].getAbsBeat() >= (stopbeat-0.0002)) {
         for (j=0; j<hfile[i].getFieldCount(); j++) {
            if (strcmp(hfile[i].getExInterp(j), "**time") == 0) {
               sscanf(hfile[i][j], "%lf", &output);
               break;
            } 
         }
         break;
      }
   }

   return output;
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
// getMetricLevel --  only checked with quarter-note beats.
//

int getMetricLevel(HumdrumFile& hfile, int index) {
   double fraction = hfile[index].getAbsBeat();
   fraction = fraction - (int)fraction;
   if (fraction < 0.0001 || fraction > 0.9999) {
      fraction = hfile[index].getAbsBeat() / 3.0;
      fraction = fraction - (int)fraction;
      if (fraction < 0.0001 || fraction > 0.9999) {
         return 1; // on a measure downbeat
      } else {
         return 0; // on the (quarter note) beat
      }
   } else {
      return -1; // on an offbeat
   }
}




// md5sum: edad163cf9797b125c2fd80500199c0f time2matlab.cpp [20090406]

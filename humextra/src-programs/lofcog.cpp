//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Nov 19 15:25:16 PST 2003
// Last Modified: Thu Nov 20 10:10:42 PST 2003
// Filename:      ...sig/examples/all/lo5.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/lofcog.cpp
// Syntax:        C++; museinfo
//
// Description:   Line of 5ths median (center of gravity) calculations.
//
// Reference:     David Temperley, "The Cognition of Basic Musical Structures"
//		  MIT Press; 2001, Chapter 5: Pitch Spelling and the 
//                Tonal-Pitch-Class, pp. 114-136.
// 

#include "humdrum.h"
#include "CircularBuffer.h"
#include "PlotFigure.h"

#include <math.h>

// function declarations
void   checkOptions      (Options& opts, int argc, char* argv[]);
void   example           (void);
void   usage             (const char* command);
void   analyzeFile       (HumdrumFile& infile, Array<double>& fifthmean);
void   printAnalysis     (HumdrumFile& infile, Array<double>& fifthmeean);
int    lo5ToBase40       (double lineval);
int    base40ToLo5       (int base40);
double getFifthMean      (HumdrumFile& infile, int line, double beats, 
                          CircularBuffer<int>& notes, 
                          CircularBuffer<double>& absbeat, double& meansum,
                          int measure);
void   printXfig         (HumdrumFile& infile, Array<double>& fifthmean);
int    chooseLineNumber  (int base12, double average);
void   convertBase12ToBase40(int base12, int& x, int& y, int& z);
void   getDeviation      (HumdrumFile& infile, Array<double>& cog,
                          Array<Array<double> >& deviation);
double getAverage        (HumdrumFile& infile, 
                          Array<Array<double> >& pastdist, double abeat,
                          int index);

// global variables
Options      options;            // database for command-line arguments
int          debugQ     = 0;     // used with --debug option
int          errorQ     = 0;     // used with -e option
int          appendQ    = 0;     // used with -a option
double       beats      = 8.0;   // used with -b option
int          basemode   = 0;     // used with -m option
int          pitchQ     = 0;     // used with -p option
int          xfigQ      = 0;     // used with -x option
int          startbranch= 0;     // used with -f and -s options
int          keysig     = 0;     // used with -k option
int          keysigQ    = 0;     // used with -k option
int          deviationQ = 0;     // used with -d option
int          averageQ   = 0;     // used with -g option
double       avgbeat    = 4.0;   // used with -g option

const char* CurrentFile = ".";



///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   HumdrumFile infile;
   Array<double> fifthmean;

   // process the command-line options
   checkOptions(options, argc, argv);

   // build up the chord table from the input file(s):
   int i;
   for (i=1; i<=options.getArgCount() || options.getArgCount()==0; i++) {
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (options.getArgCount() < 1) {
         infile.read(cin);
      } else {
         infile.read(options.getArg(i));
         CurrentFile = options.getArg(i);
         if (strrchr(options.getArg(i), '/') != NULL) {
            CurrentFile = strrchr(options.getArg(i), '/') + 1;
         }
      }
     
      infile.analyzeRhythm();
      analyzeFile(infile, fifthmean);
      if (xfigQ) {
         printXfig(infile, fifthmean);
      } else {
         printAnalysis(infile, fifthmean);
      }
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////



//////////////////////////////
//
// printXfig -- print data in xfig format
//

void printXfig(HumdrumFile& infile, Array<double>& fifthmean) {
   int i;
   double xmin = 10000;
   double xmax = 0;
   double ymin = 100;
   double ymax = -100;
   PlotData   plotdata;
   PlotFigure plot;
   plot.allocatePoints(infile.getNumLines());
   for (i=0; i<infile.getNumLines(); i++) {
      if (fifthmean[i] < -100) {
         continue;
      }
      if (infile[i].getType() == E_humrec_data) {
         if (fifthmean[i] < ymin) {
            ymin = fifthmean[i];
         }
            if (fifthmean[i] > ymax) {
            ymax = fifthmean[i];
         }
         if (infile[i].getAbsBeat() < xmin) {
            xmin = infile[i].getAbsBeat();
         }
         if (infile[i].getAbsBeat() > xmax) {
            xmax = infile[i].getAbsBeat();
         }
      }
   }

   plot.setXRangeAutoOff();
   plot.setYRangeAutoOff();
   // plot.setXMin(xmin);
   // plot.setXMax(xmax);
   plot.setXMin(0.0);
   plot.setXMax(infile[infile.getNumLines()-1].getAbsBeat());
   plot.setYMin(ymin-0.5);
   plot.setYMax(ymax+0.5);
   int mnum;
   int count;
   char buffer[1024] = {0};
   for (i=0; i<infile.getNumLines(); i++) {
      if (fifthmean[i] < -100) {
         continue;
      }
      if (infile[i].getType() == E_humrec_data) {
         plotdata.addPoint(infile[i].getAbsBeat(), fifthmean[i]);
      } else if (infile[i].getType() == E_humrec_data_measure) {
         count = sscanf(infile[i][0], "=%d", &mnum);
         sprintf(buffer, "%d", mnum);
         plot.addVLine(infile[i].getAbsBeat(), "");
         if ((mnum > 0) && (mnum % 10 == 0)) {
            plot.addText(buffer, infile[i].getAbsBeat(), ymin-0.75, 14, 0, 0);
         }
      }
   }

   plot.addPlot(plotdata);
   plot.yTicksOn();
   plot.setYTicksDelta(1.0);
   plot.printXfig(cout);
}



//////////////////////////////
//
// analyzeFile -- analyze the line of fifth mean for each line in
//     the file.
//

void analyzeFile(HumdrumFile& infile, Array<double>& fifthmean) {
   int i;
   int measure = 0;
   int linecount = infile.getNumLines();
   fifthmean.setSize(linecount);
   fifthmean.allowGrowth(0);
   fifthmean.setAll(0);

   CircularBuffer<int>    notes(10000);
   CircularBuffer<double> absbeat(10000);
   CircularBuffer<double> meanline(10000);

   notes.reset();
   absbeat.reset();
   meanline.reset();

   double meansum = 0.0;

   int length, ii, jj;
   for (i=1; i<linecount; i++) {
      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
         case E_humrec_data_comment:
            fifthmean[i] = fifthmean[i-1];
            break;
         case E_humrec_interpretation:
            if (keysigQ) {
               for (ii=0; ii<infile[i].getFieldCount(); ii++) {
                  if (strcmp(infile[i].getExInterp(ii), "**kern") != 0) {
                     continue;
                  }
                  if (strncmp(infile[i][ii], "*k[", 3) == 0) {
                     keysig = 0;
                     length = strlen(infile[i][ii]);
                     for (jj=3; jj<length; jj++) {
                        if (infile[i][ii][jj] == '-') {
                           keysig--;
                        } else if (infile[i][ii][jj] == '#') {
                           keysig++;
                        }
                     }
                     if (keysig > 3) {
                        startbranch = 1;
                     } else if (keysig < -3) {
                        startbranch = -1;
                     } else {
                        startbranch = 0;
                     }
                  }
               }
            }
            fifthmean[i] = fifthmean[i-1];
            break;
         case E_humrec_data_kern_measure:
            sscanf(infile[i][0], "=%d", &measure);
            fifthmean[i] = fifthmean[i-1];
            break;
         case E_humrec_data:
            fifthmean[i] = 
               getFifthMean(infile, i, beats, notes, absbeat, meansum, measure);
            break;
      }
   }

}



//////////////////////////////
//
// getFifthMean -- process the notes in a line.
//

double getFifthMean(HumdrumFile& infile, int line, double beats, 
      CircularBuffer<int>& notes, CircularBuffer<double>& absbeat, 
      double& meansum, int measure) {
   char buffer[1024] = {0};
   int tnote;
   int base12;
   double bias;
   double tbeat;
   int base40;
   int linenum;
   int tlinenum;
   int i, j;
   for (i=0; i<infile[line].getFieldCount(); i++) {
      if (strcmp(infile[line].getExInterp(i), "**kern") != 0) {
         // ignore non-kern spines
         continue;
      }
      if (strcmp(infile[line][i], ".") == 0) {
         // null token, so ignore
         continue;
      }
      for (j=0; j<infile[line].getTokenCount(i); j++) {
         infile[line].getToken(buffer, i, j);
         if (buffer[0] == '\0') {
            // invalid chord note
            continue;
         }
         base40 = Convert::kernToBase40(buffer);
         if (base40 < -10) {
            // ignore rests
            continue;
         }
         if (basemode == 0) {
            linenum = base40ToLo5(base40);
         } else {
            // forget the spelling and try to figure it out from the
            // line of fifths mean 
            base12 = Convert::base40ToMidiNoteNumber(base40) % 12;
            bias = 0.0;
            if (infile[line].getAbsBeat() <= beats) {
               bias = startbranch * 3;   // shift to sharps, flats or naturals
               // gradually remove effect of bias as more notes are added
               bias = bias - bias/(notes.getCount()+10);  
            }
            if (notes.getCount() > 0) {
               linenum = chooseLineNumber(base12, 
                     bias+meansum/notes.getCount());
            } else {
               linenum = chooseLineNumber(base12, bias);
            }
            tlinenum = base40ToLo5(base40);
            if (errorQ && (tlinenum != linenum)) {
               cout << "!! naming error";

               if (measure > 0) { 
                  cout << " in bar=" << measure 
                       << ", beat=" << infile[line].getBeat();
               } else {
                  cout << " on original line=" << line;
               }
               cout << ", spine=" << i+1;
               if (infile[line].getTokenCount(i) > 1) {
                  cout << " token=" << j+1;
               }
               cout << " (" << Convert::base40ToKern(buffer, 
                       lo5ToBase40(tlinenum)+4*40) 
                    << " is changed to ";

               cout << Convert::base40ToKern(buffer, lo5ToBase40(linenum)+4*40)
                    << ")";
               cout << " [" << tlinenum << " to " << linenum << "]";
               cout << endl;
            }
            
         }

         notes.insert(linenum);         
         absbeat.insert(infile[line].getAbsBeat());
         meansum += linenum;

         // remove any old notes which are outside of the analysis window
         while (infile[line].getAbsBeat() - absbeat[absbeat.getCount()-1] > 
               beats) {
            tnote = notes.extract();
            tbeat = absbeat.extract();
            meansum -= tnote;
         }
      }
   }

   if (notes.getCount() > 0) {
      return meansum/notes.getCount();
   } else {
      return -1000;
   }
}



//////////////////////////////
//
// chooseLineNumber --  Choose the pitch spelling which is closest to 
//   the average pitch value.
//

int chooseLineNumber(int base12, double average) {
   int x, y, z;
   convertBase12ToBase40(base12, x, y, z);
   x = base40ToLo5(x);
   y = base40ToLo5(y);
   z = base40ToLo5(z);

   double diffx = fabs(average-x);
   double diffy = fabs(average-y);
   double diffz = fabs(average-z);

   if ((diffx < diffy) && (diffx < diffz)) {
      return x;
   }
   if (diffy < diffz) {
      return y;
   }
   return z;
}



///////////////////////////////
//
// convertBase12ToBase40 -- give a list of the 

void convertBase12ToBase40(int base12, int& x, int& y, int& z) {
   switch (base12) {
      case 0:   x=38; y=2;  z=6;     break;  // B#/C/D--
      case 1:   x=39; y=3;  z=7;     break;  // B##/C#/D-
      case 2:   x= 4; y=8;  z=12;    break;  // C##/D/E--
      case 3:   x= 9; y=13; z=17;    break;  // D#/E-/F--
      case 4:   x=10; y=14; z=18;    break;  // D##/E/F-
      case 5:   x=15; y=19; z=23;    break;  // E#/F/G--
      case 6:   x=16; y=20; z=24;    break;  // E##/F#/G-
      case 7:   x=21; y=25; z=29;    break;  // F##/G/A--
      case 8:   x=26; y=30; z=-1000; break;  // G#/A-
      case 9:   x=27; y=31; z=35;    break;  // G##/A/B--
      case 10:  x=32; y=36; z=0;     break;  // A#/B-/C--
      case 11:  x=33; y=37; z=1;     break;  // A##/B/C-
      default:  x=-1000; y=-1000; z=-1000; break;  // unknown
   }
}



//////////////////////////////
//
// getDeviation -- calculate the deviation of notes from the
//    fifth mean.
//

void getDeviation(HumdrumFile& infile, Array<double>& cog,
      Array<Array<double> >& deviation) {

   Array<Array<int> > notes;

   deviation.setSize(infile.getNumLines());
   notes.setSize(infile.getNumLines());

   int i, j, k;
   for (i=0; i<deviation.getSize(); i++) {
      deviation[i].setSize(32);
      deviation[i].setGrowth(32);
      deviation[i].setSize(0);
      notes[i].setSize(32);
      notes[i].setGrowth(32);
      notes[i].setSize(0);
   }

   char buffer[1024] = {0};
   int tokencount;
   int base40;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() != E_humrec_data) {
         continue;
      }

      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strcmp(infile[i].getExInterp(j), "**kern") != 0) {
            continue;
         }
         if (strcmp(infile[i][j], ".") == 0) {
            continue;
         }
         
         tokencount = infile[i].getTokenCount(j);
         for (k=0; k<tokencount; k++) {
            infile[i].getToken(buffer, j, k);
            if (strchr(buffer, ']') != NULL) {  // ignore tied notes
               continue;
            }
            if (strchr(buffer, '_') != NULL) {  // ignore tied notes
               continue;
            }
            base40 = Convert::kernToBase40(buffer);
            if (base40 < 0) {
               continue;
            }
            // have an interesting note, so store it
            notes[i].append(base40);
         }
      }
   }

   // notes are extracted so calculate the deviations

   double dev;
   for (i=0; i<infile.getNumLines(); i++) {
      if (notes[i].getSize() < 1) {
         continue;
      }
      deviation[i].setSize(notes[i].getSize());
      for (j=0; j<notes[i].getSize(); j++) {
         dev = base40ToLo5(notes[i][j]) - cog[i];
         deviation[i][j] = dev;
      }
   }

}



//////////////////////////////
//
// printAnalysis -- print the analysis of the mean.
//

void printAnalysis(HumdrumFile& infile, Array<double>& fifthmean) {
   int i;
   int linecount = infile.getNumLines();
   char buffer[1024] = {0};
   int printbeat = 0;
   int ii, kk;

   Array<Array<double> > deviation;
   if (deviationQ) {
      getDeviation(infile, fifthmean, deviation);
   }

   double tempval;
   for (i=0; i<linecount; i++) {
      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
            cout << infile[i] << endl;
            break;
         case E_humrec_data_comment:
            if (appendQ) {
               cout << infile[i] << "\t" << "!" << endl;
            }
            break;
         case E_humrec_data_kern_measure:
            if (appendQ) {
               cout << infile[i] << "\t" << infile[i][0] << endl;
            } else {
               cout << infile[i][0] << endl;
            }
            break;
         case E_humrec_interpretation:
            if (appendQ) {
               cout << infile[i] << "\t";
            }
            if (strcmp(infile[i][0], "*-") == 0) {
               cout << "*-";
            } else if (strncmp(infile[i][0], "**", 2) == 0) {
               cout << "**lo5";
               if (appendQ == 0) {
                  cout << "\n*b=" << beats;
                  printbeat = 1;
               }
            } else {
               cout << "*";
               if (printbeat == 0) {
                  cout << "b=" << beats;
                  printbeat = 1;
               }
            }
            cout << endl;
            break;
         case E_humrec_data:
            if (appendQ) {
               if (printbeat == 0) {
                  for (ii=0; ii<infile[i].getFieldCount(); ii++) {
                     cout << "*\t";
                  }
                  cout << "*b=" << beats << endl;
                  printbeat = 1;
               } 
               cout << infile[i] << "\t";
            } 
            if (deviationQ) {
               if (lo5ToBase40(fifthmean[i]) < -100) {
                  cout << ".";
               } else {
                  if (averageQ) {
                     cout << getAverage(infile, deviation, avgbeat, i);
                  } else {
                     for (kk=0; kk<deviation[i].getSize(); kk++) {
                        tempval = deviation[i][kk];
                        tempval = (int)(100 * tempval + 0.5)/100.0;
                        cout << tempval;
                        if (kk < deviation[i].getSize() - 1) {
                           cout << " ";
                        }
                     }
                  }
               }
            } else if (pitchQ) {
               if (lo5ToBase40(fifthmean[i]) < -100) {
                  cout << ".";
               } else {
                  cout << Convert::base40ToKern(buffer, 
                           lo5ToBase40(fifthmean[i]) + 4 * 40);
               }
            } else {
               if (fifthmean[i] < -100) {
                  cout << ".";
               } else {
                  cout << fifthmean[i];
               }
            }
            cout << endl;
            break;
      
      }
   }
}



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("b|beats=d:8.0",   "number of quarter-notes in past to average");
   opts.define("a|append=b",      "append analysis to input file");
   opts.define("x|xfig=b",        "print output in xfig format");
   opts.define("p|pitch=b",       "print lo5 pitch name instead of number");
   opts.define("m|midi=b",        "simiulate MIDI pitch names");
   opts.define("e|error=b",       "print branch errors");
   opts.define("s|sharp=b",       "start with a sharp bias");
   opts.define("f|flat=b",        "start with a flat bias");
   opts.define("k|keysig=b",      "start with a bias related to key signature");
   opts.define("d|deviation=b",   "deviation of a note from the lof cog");
   opts.define("g|average=d:4.0", "average the deviations of individual notes");

   opts.define("debug=b",         "trace input parsing");   
   opts.define("author=b",        "author of the program");   
   opts.define("version=b",       "compilation information"); 
   opts.define("example=b",       "example usage"); 
   opts.define("h|help=b",        "short description"); 
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Nov 2003" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 19 Nov 2003" << endl;
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

   debugQ       = opts.getBoolean("debug");
   keysigQ      = opts.getBoolean("keysig");
   errorQ       = opts.getBoolean("error");
   xfigQ        = opts.getBoolean("xfig");
   appendQ      = opts.getBoolean("append");
   pitchQ       = opts.getBoolean("pitch");
   beats        = opts.getDouble("beats");
   basemode     = opts.getBoolean("midi");
   startbranch += opts.getBoolean("sharp");
   startbranch -= opts.getBoolean("flat");
   deviationQ   = (opts.getBoolean("deviation") || opts.getBoolean("average"));
   averageQ     = opts.getBoolean("average");
   avgbeat      = opts.getDouble("average");

}



//////////////////////////////
//
// example -- example usage of the lo5 program
//

void example(void) {
   cout <<
   "                                                                        \n"
   << endl;
}



//////////////////////////////
//
// usage -- gives the usage statement for the quality program
//

void usage(const char* command) {
   cout <<
   "                                                                        \n"
   << endl;
}



//////////////////////////////
//
// lo5ToBase40 -- convert a line of fifth pitch-class into a
//      base-40 pitch class.
//

int lo5ToBase40(double lineval) {
   int lv = (int)(lineval+0.49);
   if (lv < -15)  {  // can't go lower than F--
      return -1000;
   }
   if (lv > 19)  {  // can't go higher than B##
      return -1000;
   }

   if (lv >= 0) {
      return ((lv * 23 + 2) + 4000) % 40;
   } else { 
      return ((-(lv-1) * 17 + 2) + 4000) % 40;
   }
}



///////////////////////////////
//
// base40ToLo5 -- convert base40 to line of fifths
//

int base40ToLo5(int base40) {
   base40 = (base40 + 4000) % 40; // remove any octave information;

   switch (base40) {
      case  0: return -14;   // C--
      case  1: return  -7;   // C-
      case  2: return   0;   // C
      case  3: return   7;   // C#
      case  4: return  14;   // C##
      case  5: return -1000; // unknown
      case  6: return -12;   // D--
      case  7: return  -5;   // D-
      case  8: return   2;   // D
      case  9: return   9;   // D#
      case 10: return  16;   // D##
      case 11: return -1000; // unknown
      case 12: return -10;   // E--
      case 13: return  -3;   // E-
      case 14: return   4;   // E
      case 15: return  11;   // E#
      case 16: return  18;   // E##
      case 17: return -15;   // F--
      case 18: return  -8;   // F-
      case 19: return  -1;   // F
      case 20: return   6;   // F#
      case 21: return  13;   // F##
      case 22: return -1000; // unknown
      case 23: return -13;   // G--
      case 24: return  -6;   // G-
      case 25: return   1;   // G
      case 26: return   8;   // G#
      case 27: return  15;   // G##
      case 28: return -1000; // unknown
      case 29: return -11;   // A--
      case 30: return  -4;   // A-
      case 31: return   3;   // A
      case 32: return  10;   // A#
      case 33: return  17;   // A##
      case 34: return -1000; // unknown
      case 35: return  -9;   // B--
      case 36: return  -2;   // B-
      case 37: return   5;   // B
      case 38: return  12;   // B#
      case 39: return  19;   // B##
   }
   return -1000;  // unknown or rest
}



//////////////////////////////
//
// getAverage --
//

double getAverage(HumdrumFile& infile, Array<Array<double> >& data,
      double abeat, int index) {
   int i, j;

   int count = 0;
   double sum = 0.0;
   double startdur = infile[index].getAbsBeat();

   for (i=index; i>=0; i--) {
      if (startdur - infile[i].getAbsBeat() > abeat) {
         break;
      }
      for (j=0; j<data[i].getSize(); j++) {
         if (data[i][j] == -1.0) {
            sum += infile[i].getAbsBeat();
         } else {
            sum += data[i][j];
         }
         count++;
      }
   }

   if (count > 0) {
      return sum/count;
   }
   return 0;
}



// md5sum: 5a02410e288a4f8168cc943cde1ace7c lofcog.cpp [20050403]

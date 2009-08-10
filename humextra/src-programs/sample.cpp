//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jul  9 13:20:28 PDT 1998
// Last Modified: Tue Dec  5 15:11:50 PST 2000 (enable polyphonic sampling)
// Filename:      ...sig/examples/all/sample.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/sample.cpp
// Syntax:        C++; museinfo
//
// Description:   Samples pitches according to the given pattern
//

#include "humdrum.h"
#include "CircularBuffer.h"

#include <math.h>
#include <string.h>
#include <ctype.h>


// function declarations
void         checkOptions(Options& opts, int argc, char* argv[]);
void         createDataLine(char* buffer, HumdrumFile& infile, int line, 
                   double duration, int style);
void         example(void);
void         processRecords(HumdrumFile& infile, HumdrumFile& outfile);
void         usage(const char* command);

// interface variables
Options                options;          // database for command-line arguments
CircularBuffer<double> durations(1000);
CircularBuffer<int>    styles(1000);

///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
   HumdrumFile infile, outfile;

   // process the command-line options
   checkOptions(options, argc, argv);

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   for (int i=0; i<numinputs || i==0; i++) {
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
   opts.define("r|rhythm|rhythm-cycle|=s:4");  // rhythm to sample notes at
   opts.define("m|metric=s");                 // metric cycle to sample

   opts.define("d|divide=b");             // determine if to split input notes
   opts.define("debug=b");                // determine bad input line num
   opts.define("author=b");               // author of program
   opts.define("version=b");              // compilation info
   opts.define("example=b");              // example usages
   opts.define("h|help=b");               // short description
   opts.process(argc, argv);

   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, July 1998" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 5 December 2000" << endl;
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

   double duration = 0.0;
   int style = 0;

   int length;
   length = strlen(opts.getString("rhythm"));
   char buffer[length + 1];
   strcpy(buffer, opts.getString("rhythm"));

   durations.reset();
   styles.reset();

   char *durstr;
   durstr = strtok(buffer, " ,:\t\n;");
   while (durstr != NULL) {
      duration = Convert::kernToDuration(durstr);
      if (strchr(durstr, 'r') != 0) {
         style = 'r';
      } else if (strchr(durstr, 'x') != 0) {
         style = 'x';
      } else {
         style = 0;
      }
      durations.insert(duration);
      styles.insert(style);
      durstr = strtok(NULL, " ,:\t\n;");
   }

}



//////////////////////////////
//
// example -- example usage of the sample program
//

void example(void) {
   cout <<
   "                                                                         \n"
   "# example usage of the sample program.                                   \n"
   "                                                                         \n"
   << endl;
}



//////////////////////////////
//
// processRecords -- 
//

void processRecords(HumdrumFile& infile, HumdrumFile& outfile) {
   infile.analyzeRhythm();
   char buffer[10000] = {0};

   int style;
   double duration;
   style = styles.extract();
   duration = durations.extract();

   double currbeat = 0;

   double targetbeat = 0;
   int lastline = 0;
   double currdur;
   int state = 0;

   for (int i=0; i<infile.getNumLines(); i++) {
      if (options.getBoolean("debug")) {
         cout << "processing line " << (i+1) << " of input ..." << endl;
      }

      if (infile[i].getType() != E_humrec_data) {
         outfile.appendLine(infile[i]);
         continue;
      }
 
      state = 0;
      currbeat = infile[i].getAbsBeat();
      currdur = infile[i].getDuration();
      while ((currbeat+currdur > targetbeat) || 
            (fabs(currbeat-targetbeat) < 0.001)) {

         if (fabs(currbeat - targetbeat) < 0.0001) {
            createDataLine(buffer, infile, i, duration, style);
            outfile.appendLine(buffer);
            styles.insert(style);
            durations.insert(duration);
            style = styles.extract();
            duration = durations.extract();
            targetbeat += duration;
         } else if (currbeat+currdur > targetbeat) {
            if (state == 1) {
               createDataLine(buffer, infile, lastline, duration, style);
            } else {
               createDataLine(buffer, infile, i, duration, style);
            }
            outfile.appendLine(buffer);
            styles.insert(style);
            durations.insert(duration);
            style = styles.extract();
            duration = durations.extract();
            targetbeat += duration;
         } else {
            break;
         }
      }

      lastline = i;

   }
}



//////////////////////////////
//
// createDataLine -- 
//

void createDataLine(char* buffer, HumdrumFile& infile, int line, 
      double duration, int style) { 
   buffer[0] = '\0';
   if (style == 'x') {
      return;
   }

   char dstring[100] = {0};
   char pbuffer[100] = {0};
   char tbuffer[100] = {0};
   int tokencount;

   Convert::durationToKernRhythm(dstring, duration);
   int pitch;

   int i, k;
   for (i=0; i<infile[line].getFieldCount(); i++) {
      if (infile[line].getExInterpNum(i) == E_KERN_EXINT) {
         if (strcmp(infile[line][i], ".") == 0) {
            int dotline  = infile[line].getDotLine(i);
            int dotspine = infile[line].getDotSpine(i);
            tokencount   = infile[dotline].getTokenCount(dotspine);
            for (k=0; k<tokencount; k++) {
               infile[dotline].getToken(tbuffer, dotspine, k);
               pitch = Convert::kernToBase40(tbuffer);
               Convert::base40ToKern(pbuffer, pitch);
               strcat(buffer, dstring);
               if (style == 'r') {
                  strcat(buffer, "r");
               } else {
                  strcat(buffer, pbuffer);
               }
               if (k < tokencount - 1) {
                  strcat(buffer, " ");
               }
            }
         } else {
            tokencount = infile[line].getTokenCount(i);
            for (k=0; k<tokencount; k++) {
               infile[line].getToken(tbuffer, i, k);
               pitch = Convert::kernToBase40(tbuffer);
               Convert::base40ToKern(pbuffer, pitch);
               strcat(buffer, dstring);
               if (style == 'r') {
                  strcat(buffer, "r");
               } else {
                  strcat(buffer, pbuffer);
               }
               if (k < tokencount - 1) {
                  strcat(buffer, " ");
               }
            }
         }
         strcat(buffer, "\t");
      } else {
         continue;
      }
   }

   // remove any extra tab character that might have been added.
   int length = strlen(buffer);
   if (buffer[length-1] == '\t') {
      buffer[length-1] = '\0';
   }
}



//////////////////////////////
//
// usage -- gives the usage statement for the sample program
//

void usage(const char* command) {
   cout <<
   "                                                                         \n"
   "Analyzes **kern data and generates a rhythmic analysis which gives       \n"
   "the beat location of **kern data records in the measure.  Currently,     \n"
   "input spines cannot split or join.                                       \n"
   "                                                                         \n"
   "Usage: " << command << " [-a][-b base-rhythm][-s|-d][input1 [input2 ...]]\n"
   "                                                                         \n"
   "Options:                                                                 \n"
   "   -b = set the base rhythm for analysis to specified kern rhythm value. \n"
   "   -d = gives the duration of each kern record in beat measurements.     \n"
   "   -s = sum the beat count in each measure.                              \n"
   "   --options = list of all options, aliases and default values           \n"
   "                                                                         \n"
   << endl;
}


// md5sum: 6e41ca61d5e538864e16c6c86db90137 sample.cpp [20050403]

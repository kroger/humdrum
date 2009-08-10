//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Nov 15 08:55:35 PST 2001
// Last Modified: Thu Nov 15 08:55:40 PST 2001
// Filename:      ...sig/examples/all/spinemv.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/spinemv.cpp
// Syntax:        C++; museinfo
//
// Description:   For reorganizing spines in a humdrum file.
//

#include "humdrum.h"

#include <string.h>


// function declarations:
void      checkOptions      (Options& opts, int argc, char** argv);
void      example           (void);
void      usage             (const char* command);

void      getTransformation (HumdrumFile& hfile, Array<int>& transform);
void      printTransformation (HumdrumFile& hfile, Array<int>& transform);
void      printLine         (HumdrumFile& hfile,Array<int>& transform,int line);
double    getAveragePitch   (HumdrumFile& hfile, int primaryTrack);

// User interface variables:
Options     options;
int         reverseQ = 0;
int         determineQ = 0;
int         normalQ = 0;
const char* primaryspine = "**kern";


//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   HumdrumFile hfile;

   checkOptions(options, argc, argv);  // process the command-line options

   // figure out the number of input files to process
   int numinputs = options.getArgCount();

   for (int i=0; i<numinputs || i==0; i++) {
      hfile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         hfile.read(cin);
      } else {
         hfile.read(options.getArg(i+1));
      }
      // hfile.analyzeRhythm();

      Array<int> transform;
      getTransformation(hfile, transform);
      if (!determineQ) {
         printTransformation(hfile, transform);
      }

   }

   return 0;
}


//////////////////////////////////////////////////////////////////////////



//////////////////////////////
//
// printTransformation -- 
//

void printTransformation(HumdrumFile& hfile, Array<int>& transform) {
   int i;

   for (i=0; i<hfile.getNumLines(); i++) {
      switch (hfile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
            cout << hfile[i] << "\n";
            break;
         case E_humrec_data_comment:
         case E_humrec_data_kern_measure:
         case E_humrec_interpretation:
         case E_humrec_data:
            printLine(hfile, transform, i);
            break;
         default:
            cout << hfile[i] << "\n";
            break;
      }
   }

}



//////////////////////////////
//
// printLine -- 
//

void printLine(HumdrumFile& hfile, Array<int>& transform, int line) {
   int i, j;
   int target = 0;
   int count = 0;

   for (j=0; j<transform.getSize(); j++) {
      target = transform[j];
      for (i=0; i<hfile[line].getFieldCount(); i++) {
         if (hfile[line].getPrimaryTrack(i) == target) {
            if (count != 0) {
               cout << "\t";
            }
            count++;
            cout << hfile[line][i];
         }
      }
   }
   cout << "\n";

}



//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("f|field-order=s",      "reordering scheme of output"); 
   opts.define("r|reverse=b",          "reverse the order of the spines"); 
   opts.define("n|normal=b",           "convert score to normal order"); 
   opts.define("p|primary=s:**kern",   "main spine for group reversals"); 
   opts.define("d|determine|detect=b", "determine score ordering only"); 

   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("h|help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Nov 2001" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: Nov 2001" << endl;
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

   reverseQ     = opts.getBoolean("reverse");
   normalQ      = opts.getBoolean("normal");
   determineQ   = opts.getBoolean("determine");
   if (determineQ) {
      normalQ = 1;
   }
   primaryspine = options.getString("primary");

}



//////////////////////////////
//
// example --
//

void example(void) { }



//////////////////////////////
//
// getTransformation -- 
//

void getTransformation(HumdrumFile& hfile, Array<int>& transform) {
   transform.setSize(1000);
   transform.setSize(0);
   transform.allowGrowth(1);

   int maxspine = hfile.getMaxTracks();
   int i, j;

   if (reverseQ) {
      transform.setSize(maxspine);
      for (i=0; i<maxspine; i++) {
         transform[i] = maxspine - i;
      }
      return;
   }

   if (options.getBoolean("field-order")) {
      // const char* order = options.getString("field-order");
      cout << "Not yet written" << endl;
      return;
   }

   if (normalQ) {
      int findex = -1;
      int lindex = -1;
      int i;

      for (i=0; i<hfile.getMaxTracks(); i++) {
         if (strcmp(hfile.getTrackExInterp(i+1), "**kern") == 0) {
            if (findex == -1) {
               findex = i;
            }
            lindex = i;
         }
      }

      double favg = 0.0;
      double lavg = 0.0;
      if (findex != lindex) {
         favg = getAveragePitch(hfile, findex + 1);         
         lavg = getAveragePitch(hfile, lindex + 1);         
      }
      if (favg > lavg) {
         if (determineQ) {
            cout << "Score is in reverse order.\tpitch avgs:"
                 << favg << "\tcompared to " << lavg << endl;
            return;
         }
         // reverse data by kern (or primary spine type) blocks
         for (i=maxspine; i>0; i--) {
            if (strcmp(hfile.getTrackExInterp(i), "**kern") == 0) {
               transform.append(i);
               j = i+1;
               while ((j <= maxspine) && 
                      (strcmp(hfile.getTrackExInterp(j), "**kern") != 0)) {
                  transform.append(j);
                  j++;
               }
            }
         }
         transform.allowGrowth(0);
         return;
      } else {
         // score is already in normal form
         if (determineQ) {
            cout << "Score is in normal order.\tpictch avgs.:"
                 << favg << " compared to " << lavg << endl;
            return;
         }
         for (i=1; i<=maxspine; i++) {
            transform.append(i);
         }
         transform.allowGrowth(0);
         return;
      }
   }


   // reverse data by kern (or primary spine type) blocks
   char buffer[1024] = {0};
   if (strncmp(primaryspine, "**", 2) != 0) {
      strcpy(buffer, "**");
      strcat(buffer, primaryspine);
   } else {
      strcat(buffer, primaryspine);
   }
   for (i=maxspine; i>0; i--) {
      if (strcmp(hfile.getTrackExInterp(i), buffer) == 0) {
         transform.append(i);
         j = i+1;
         while ((j <= maxspine) && 
                (strcmp(hfile.getTrackExInterp(j), buffer) != 0)) {
            transform.append(j);
            j++;
         }
      }
   }



}


//////////////////////////////
//
// getAveragePitch -- get the base-12 pitch average.
//

double getAveragePitch(HumdrumFile& hfile, int primaryTrack) {
   int i, j, k;
   char buffer[1024] = {0};
   double sum = 0.0;
   int count = 0;
   int tokencount = 0;
   int pitch = 0;

   for (i=0; i<hfile.getNumLines(); i++) {
      if (hfile[i].getType() == E_humrec_data) {
         for (j=0; j<hfile[i].getFieldCount(); j++) {
            if (hfile[i].getPrimaryTrack(j) != primaryTrack) {
               continue;
            }
            tokencount = hfile[i].getTokenCount(j); 
            for (k=0; k<tokencount; k++) {
               hfile[i].getToken(buffer, j, k);
               if (strcmp(buffer, ".") == 0) {
                  continue;
               }
               if (strcmp(buffer, ".") == 0) {
                  continue;
               }
               if (strchr(buffer, 'r') != NULL) {
                  continue;
               }
               if (strchr(buffer, '_') != NULL) {  
                  // don't bother with tied notes
                  continue;
               }
               if (strchr(buffer, ']') != NULL) {  
                  // don't bother with tied notes
                  continue;
               }
               pitch = Convert::kernToMidiNoteNumber(buffer);
               if (pitch > 0) {
                  sum += pitch;
                  count++;
               }
            }
         }
      }
   }

   return sum/count;
}



//////////////////////////////
//
// usage --
//

void usage(const char* command) { }



// md5sum: 8809beb49cafe8c95630fb2b8b5432ad mvspine.cpp [20050403]

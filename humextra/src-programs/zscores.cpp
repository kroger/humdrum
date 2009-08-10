//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jan 13 08:14:22 PST 2008
// Last Modified: Sun Jan 13 13:40:14 PST 2008
// Filename:      ...sig/examples/all/zscores.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/zscores.cpp
// Syntax:        C++; museinfo
//
// Description:   Calculate normalized scores (z-scores) for specified spine.
//

#include <math.h>
#include "humdrum.h"

#ifndef OLDCPP
   #include <iostream>
#else
   #include <iostream.h>
#endif

class Datum {
   public:
      double data;
      int line;
      int spine;
      int token;
};

// function declarations:
void      checkOptions          (Options& opts, int argc, char** argv);
void      example               (void);
void      usage                 (const char* command);
int       findBestField         (HumdrumFile& infile);
void      printRawNumbers       (Array<Datum> data, int style);
void      extractNumbers        (HumdrumFile& infile, int field, 
                                 Array<Datum>& data);
double    getMean               (Array<Datum>& data);
double    getStandardDeviation  (double mean, Array<Datum>& data);
double    getSampleSD           (double mean, Array<Datum>& data);
void      adjustData            (Array<Datum>& data, double mean, double sd,
                                 int reverse);
void      printDataSingle       (Array<Datum>& data, int field, 
		                 HumdrumFile& infile, double mean, double sd);
void      printDataReplace      (Array<Datum>& data, int field, 
		                 HumdrumFile& infile, double mean, double sd);
void      printDataPrepend      (Array<Datum>& data, int field, 
		                 HumdrumFile& infile, double mean, double sd);
void      printDataAppend       (Array<Datum>& data, int field, 
		                 HumdrumFile& infile, double mean, double sd);

// User interface variables:
Options   options;
int       field     = 0;         // used with -f option
int       inputQ    = 0;         // used with -i option
int       fullQ     = 0;         // used with --full option
int       statQ     = 0;         // used with -s option
int       rawQ      = 0;         // used with -r option
int       appendQ   = 0;         // used with -a option
int       prependQ  = 0;         // used with -p option
int       replaceQ  = 0;         // used with --replace option
int       suppressQ = 0;         // used with -S option
int       reverseQ  = 0;         // used with --reverse option
int       meanQ     = 0;         // used with -m option
int       sampleQ   = 0;         // used with --sample
int       sdQ       = 0;         // used with -d option
double    myMean    = 0;         // used with -m option
double    mySd      = 0;         // used with -d option


//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   // process the command-line options
   checkOptions(options, argc, argv);
   HumdrumFile infile;

   if (options.getArgCount() < 1) {
      infile.read(cin);
   } else {
      infile.read(options.getArg(1));
   }

   if (field <= 0) { // field counter starts at 1
      field = findBestField(infile);
   }
   if (field > infile.getMaxTracks()) {
      field = infile.getMaxTracks();
   }

   Array<Datum>  data;
   extractNumbers(infile, field, data);

   if (inputQ) {
      printRawNumbers(data, fullQ);
   }

   double mean;
   double sd;


   
   if (meanQ) {
      mean = myMean;
   } else {
      mean = getMean(data);
   }
   
   if (sdQ) {
      sd = mySd;
   } else {
      if (sampleQ) {
         sd = getSampleSD(mean, data);
      } else {
         sd = getStandardDeviation(mean, data);
      }
   }


   if (statQ) {
      cout << "Mean:\t" << mean << "\n";
      cout << "SD:\t" << sd << "\n";
   }
   if (inputQ) {
      exit(0);
   }
   if (statQ && !rawQ) {
      exit(0);
   }
   
   adjustData(data, mean, sd, reverseQ);

   if (rawQ) {
      printRawNumbers(data, fullQ);
      exit(0);
   } else if (replaceQ) {
      printDataReplace(data, field, infile, mean, sd);
   } else if (prependQ) {
      printDataPrepend(data, field, infile, mean, sd);
   } else if (appendQ) {
      printDataAppend(data, field, infile, mean, sd);
   } else {
      printDataSingle(data, field, infile, mean, sd);
   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// printDataReplace --
//

void printDataReplace(Array<Datum>& data, int field, HumdrumFile& infile, 
      double mean, double sd) {

   int z = 0;
   int scount;

   int i, j, k;
   int exinterp;
   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
         case E_humrec_data_comment:
         case E_humrec_data_kern_measure:
            cout << infile[i];
            break;
         case E_humrec_interpretation:
            exinterp = 0;
	    for (j=0; j<infile[i].getFieldCount(); j++) {
               if (j > 0) {
                  cout << '\t';
               }
               cout << infile[i][j];
               if (infile[i].getPrimaryTrack(j) == field) {
                  if (!suppressQ && (strncmp(infile[i][j], "**", 2) == 0)) {
                     exinterp = 1;
                  }
               } 
            }
            if (exinterp && !suppressQ) {
               cout << '\n';
               for (j=0; j<infile[i].getFieldCount(); j++) {
                  if (j > 0) {
                     cout << '\t';
                  }
                  if (infile[i].getPrimaryTrack(j) == field) {
                     cout << "!shift=" << mean;
                  } else {
                     cout << '!';
                  }
               }
               cout << '\n';
               for (j=0; j<infile[i].getFieldCount(); j++) {
                  if (j > 0) {
                     cout << '\t';
                  }
                  if (infile[i].getPrimaryTrack(j) == field) {
                     cout << "!scale=" << sd;
                  } else {
                     cout << '!';
                  }
               }
            }
            break;
         case E_humrec_data:
	    for (j=0; j<infile[i].getFieldCount(); j++) {
               if (j > 0) {
                  cout << '\t';
               }
               if (infile[i].getPrimaryTrack(j) == field) {
                  if (strcmp(infile[i][j], ".") == 0) {
                     cout << ".";
                  } else {
                     scount = infile[i].getTokenCount(j);
                     for (k=0; k<scount; k++) {
                        if ((data[z].line != i) ||
                            (data[z].spine != j) ||
                            (data[z].token != k) ) {
                           cerr << "STRANGE ERROR on line " << i+1;
                           cerr << " in spine " << j+1;
                           cerr << " at token " << k+1;
                           cerr << endl;
                           exit(1);
                        }
                        cout << data[z++].data;
                        if (k < scount - 1) {
                           cout << " ";
                        }
                     }
                  }
               } else {
                  cout << infile[i][j];
               }
            }
            break;
         default:
            cout << infile[i];
            break;
      }
      cout << '\n';
   }

}



//////////////////////////////
//
// printDataAppend --
//

void printDataAppend(Array<Datum>& data, int field, HumdrumFile& infile, 
      double mean, double sd) {

   int z = 0;
   int scount;

   int i, j, k;
   int exinterp = 0;
   int collapse = 0;
   int counter = 0;
   int counter2 = 0;
   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
            cout << infile[i];
            break;
         case E_humrec_interpretation:
            counter = 0;
            exinterp = 0;
	    collapse = 0;
            counter2 = 0;

	    for (j=0; j<infile[i].getFieldCount(); j++) {
               if (infile[i].getPrimaryTrack(j) == field) {
                  counter2++;
                  if (strcmp(infile[i][j], "*v") == 0) {
                     collapse++;
		     break;
                  }
               }
            }
	    if (collapse) {
               for (j=0; j<infile[i].getFieldCount(); j++) {
                  cout << "*\t";
               }
	       counter = 0;
               for (j=0; j<infile[i].getFieldCount(); j++) {
                  if (infile[i].getPrimaryTrack(j) == field) {
                     if (counter++ > 0) {
                        cout << '\t';
                     }
                     cout << infile[i][j];
                  }
               }
               cout << "\n";
               ////////////////////////////////////////////
	       cout << infile[i] << "\t";
               counter = 0;
               for (j=0; j<counter2 - collapse + 1; j++) {
                  if (j > 0) {
                     cout << "\t";
                  }
                  cout << "*";
               }
            } else {
               cout << infile[i];
	       cout << "\t";
               counter = 0;
	       for (j=0; j<infile[i].getFieldCount(); j++) {
                  if (infile[i].getPrimaryTrack(j) == field) {
                     if (counter++ > 0) {
                        cout << '\t';
                     }
                     cout << infile[i][j];
                     if (!suppressQ && (strncmp(infile[i][j], "**", 2) == 0)) {
                        exinterp = 1;
                     }
                  } 
               }
               if (exinterp && !suppressQ) {
                  counter = 0;
		  cout << "\n";
                  for (j=0; j<infile[i].getFieldCount(); j++) {
                     cout << "!\t";
                  }
                  for (j=0; j<infile[i].getFieldCount(); j++) {
                     if (infile[i].getPrimaryTrack(j) == field) {
                        if (counter++ > 0) {
                           cout << '\t';
                        }
                        cout << "!shift=" << mean;
                     } 
                  }
                  cout << '\n';
   
                  counter = 0;
                  for (j=0; j<infile[i].getFieldCount(); j++) {
                     cout << "!\t";
                  }
                  for (j=0; j<infile[i].getFieldCount(); j++) {
                     if (infile[i].getPrimaryTrack(j) == field) {
                        if (counter++ > 0) {
                           cout << '\t';
                        }
                        cout << "!scale=" << sd;
                     } 
                  }
               } else {
                  // cout << "\t" << infile[i];
               }
            }
            break;
         case E_humrec_data_kern_measure:
            counter = 0;
	    cout << infile[i] << "\t";
	    for (j=0; j<infile[i].getFieldCount(); j++) {
               if (infile[i].getPrimaryTrack(j) == field) {
                  if (counter > 0) {
                     cout << '\t';
                  }
                  counter++;
                  cout << infile[i][j];
               }
            }
            break;
         case E_humrec_data:
	    cout << infile[i] << "\t";
            counter = 0;
	    for (j=0; j<infile[i].getFieldCount(); j++) {
               if (infile[i].getPrimaryTrack(j) == field) {
                  if (counter > 0) {
                     cout << '\t';
                  }
                  counter++;
                  if (strcmp(infile[i][j], ".") == 0) {
                     cout << ".";
                  } else {
                     scount = infile[i].getTokenCount(j);
                     for (k=0; k<scount; k++) {
                        if ((data[z].line != i) ||
                            (data[z].spine != j) ||
                            (data[z].token != k) ) {
                           cerr << "STRANGE ERROR on line " << i+1;
                           cerr << " in spine " << j+1;
                           cerr << " at token " << k+1;
                           cerr << endl;
                           exit(1);
                        }
                        cout << data[z++].data;
                        if (k < scount - 1) {
                           cout << " ";
                        }
                     }
                  }
               }
            }
            break;
         default:
            cout << infile[i];
            break;
      }
      cout << '\n';
   }
}



//////////////////////////////
//
// printDataPrepend --
//

void printDataPrepend(Array<Datum>& data, int field, HumdrumFile& infile, 
      double mean, double sd) {

   int z = 0;
   int scount;

   int i, j, k;
   int exinterp = 0;
   int collapse = 0;
   int counter = 0;
   int counter2 = 0;
   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
            cout << infile[i];
            break;
         case E_humrec_interpretation:
            counter = 0;
            exinterp = 0;
	    collapse = 0;
            counter2 = 0;

	    for (j=0; j<infile[i].getFieldCount(); j++) {
               if (infile[i].getPrimaryTrack(j) == field) {
                  counter2++;
                  if (strcmp(infile[i][j], "*v") == 0) {
                     collapse++;
		     break;
                  }
               }
            }
	    if (collapse) {
               for (j=0; j<infile[i].getFieldCount(); j++) {
                  if (infile[i].getPrimaryTrack(j) == field) {
                     if (counter++ > 0) {
                        cout << '\t';
                     }
                     cout << infile[i][j];
                  }
               }
               for (j=0; j<infile[i].getFieldCount(); j++) {
                  cout << "\t*";
               }
               cout << "\n";
               ////////////////////////////////////////////
               counter = 0;
               for (j=0; j<counter2 - collapse + 1; j++) {
                  if (j > 0) {
                     cout << "\t";
                  }
                  cout << "*";
               }
            } else {
               counter = 0;
	       for (j=0; j<infile[i].getFieldCount(); j++) {
                  if (infile[i].getPrimaryTrack(j) == field) {
                     if (counter++ > 0) {
                        cout << '\t';
                     }
                     cout << infile[i][j];
                     if (!suppressQ && (strncmp(infile[i][j], "**", 2) == 0)) {
                        exinterp = 1;
                     }
                  } 
               }
               if (exinterp && !suppressQ) {
                  cout << '\t';
                  cout << infile[i];
                  cout << '\n';
                  counter = 0;
                  for (j=0; j<infile[i].getFieldCount(); j++) {
                     if (infile[i].getPrimaryTrack(j) == field) {
                        if (counter++ > 0) {
                           cout << '\t';
                        }
                        cout << "!shift=" << mean;
                     } 
                  }
                  for (j=0; j<infile[i].getFieldCount(); j++) {
                     cout << "\t!";
                  }
                  cout << '\n';
   
                  counter = 0;
                  for (j=0; j<infile[i].getFieldCount(); j++) {
                     if (infile[i].getPrimaryTrack(j) == field) {
                        if (counter++ > 0) {
                           cout << '\t';
                        }
                        cout << "!scale=" << sd;
                     } 
                  }
                  for (j=0; j<infile[i].getFieldCount(); j++) {
                     cout << "\t!";
                  }
               } else {
                  //cout << "\t" << infile[i];
               }
            }
            break;
         case E_humrec_data_kern_measure:
            counter = 0;
	    for (j=0; j<infile[i].getFieldCount(); j++) {
               if (infile[i].getPrimaryTrack(j) == field) {
                  if (counter > 0) {
                     cout << '\t';
                  }
                  counter++;
                  cout << infile[i][j];
               }
            }
            break;
         case E_humrec_data:
            counter = 0;
	    for (j=0; j<infile[i].getFieldCount(); j++) {
               if (infile[i].getPrimaryTrack(j) == field) {
                  if (counter > 0) {
                     cout << '\t';
                  }
                  counter++;
                  if (strcmp(infile[i][j], ".") == 0) {
                     cout << ".";
                  } else {
                     scount = infile[i].getTokenCount(j);
                     for (k=0; k<scount; k++) {
                        if ((data[z].line != i) ||
                            (data[z].spine != j) ||
                            (data[z].token != k) ) {
                           cerr << "STRANGE ERROR on line " << i+1;
                           cerr << " in spine " << j+1;
                           cerr << " at token " << k+1;
                           cerr << endl;
                           exit(1);
                        }
                        cout << data[z++].data;
                        if (k < scount - 1) {
                           cout << " ";
                        }
                     }
                  }
               }
            }
            break;
         default:
            cout << infile[i];
            break;
      }
      if (!exinterp) {
         cout << '\t' << infile[i];
      } else {
         exinterp = 0;
      }
      cout << '\n';
   }
}



//////////////////////////////
//
// printDataSingle --
//

void printDataSingle(Array<Datum>& data, int field, HumdrumFile& infile, 
      double mean, double sd) {

   int z = 0;
   int scount;

   int i, j, k;
   int interp;
   int counter = 0;
   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
            cout << infile[i];
            break;
         case E_humrec_interpretation:
            interp = 0;
            counter = 0;
	    for (j=0; j<infile[i].getFieldCount(); j++) {
               if (infile[i].getPrimaryTrack(j) == field) {
                  if (counter > 0) {
                     cout << '\t';
                  }
                  counter++;
                  cout << infile[i][j];
                  if (!suppressQ && (strncmp(infile[i][j], "**", 2) == 0)) {
                     cout << "\n!shift=" << mean << '\n';
                     cout << "!scale=" << sd;
                  }
               }
            }
            break;
         case E_humrec_data_comment:
         case E_humrec_data_kern_measure:
            counter = 0;
	    for (j=0; j<infile[i].getFieldCount(); j++) {
               if (infile[i].getPrimaryTrack(j) == field) {
                  if (counter > 0) {
                     cout << '\t';
                  }
                  counter++;
                  cout << infile[i][j];
               }
            }
            break;
         case E_humrec_data:
            counter = 0;
	    for (j=0; j<infile[i].getFieldCount(); j++) {
               if (infile[i].getPrimaryTrack(j) == field) {
                  if (counter > 0) {
                     cout << '\t';
                  }
                  counter++;
                  if (strcmp(infile[i][j], ".") == 0) {
                     cout << ".";
                  } else {
                     scount = infile[i].getTokenCount(j);
                     for (k=0; k<scount; k++) {
                        if ((data[z].line != i) ||
                            (data[z].spine != j) ||
                            (data[z].token != k) ) {
                           cerr << "STRANGE ERROR on line " << i+1;
                           cerr << " in spine " << j+1;
                           cerr << " at token " << k+1;
                           cerr << endl;
                           exit(1);
                        }
                        cout << data[z++].data;
                        if (k < scount - 1) {
                           cout << " ";
                        }
                     }
                  }
               }
            }
            break;
         default:
            cout << infile[i];
            break;
      }
      cout << '\n';
   }

}



//////////////////////////////
//
// adjustData -- apply mean shift and standard deviation scaling.
//

void adjustData(Array<Datum>& data, double mean, double sd, int reverse) {
   int i;
   int size = data.getSize();
   if (reverse) {
      for (i=0; i<size; i++) {
         data[i].data = data[i].data * sd + mean;
      }
   } else {
      for (i=0; i<size; i++) {
         data[i].data = (data[i].data - mean) / sd;
      }
   }
}



//////////////////////////////
//
// getMean --
//

double getMean(Array<Datum>& data) {
   int size = data.getSize();
   if (size <= 0) {
      return 0.0;
   }

   int i;
   double sum = 0.0;
   for (i=0; i<size; i++) {
      sum += data[i].data;
   }

   return sum / size;
}



//////////////////////////////
//
// getStandardDeviation --
//

double getStandardDeviation(double mean, Array<Datum>& data) {
   int size = data.getSize();
   double sum = 0.0;
   double value;
   int i;
   for (i=0; i<size; i++) {
      value = data[i].data - mean;
      sum += value * value;
   }

   return sqrt(sum / size);
}



//////////////////////////////
//
// getSampleSD --
//

double getSampleSD(double mean, Array<Datum>& data) {
   int size = data.getSize();
   double sum = 0.0;
   double value;
   int i;
   for (i=0; i<size; i++) {
      value = data[i].data - mean;
      sum += value * value;
   }

   return sqrt(sum / (size - 1.0));
}



//////////////////////////////
//
// printRawNumbers --
//

void printRawNumbers(Array<Datum> data, int style) {
   int i;
   for (i=0; i<data.getSize(); i++) {
      cout << data[i].data;
      if (style) {
         cout << '\t' << data[i].line;
         cout << '\t' << data[i].spine;
         cout << '\t' << data[i].token;
      }
      cout << '\n';
   }
}


//////////////////////////////
//
// extractNumbers --
//

void extractNumbers(HumdrumFile& infile, int field, Array<Datum>& data) {
   data.setSize(infile.getNumLines()*2);
   data.setGrowth(infile.getNumLines()*2);
   data.setSize(0);

   Datum datum;
   double value;

   int i, j, k;
   int scount;
   char buffer[10000] = {0};

   for (i=0; i<infile.getNumLines(); i++) {
// cout << endl << "	--> LINE " << i+1 << ": ";
      if (infile[i].getType() != E_humrec_data) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
// cout << j << ",";
         if (field == infile[i].getPrimaryTrack(j)) {
            if (strcmp(infile[i][j], ".") == 0) {
               continue; // ignore null tokens
            }
            scount = infile[i].getTokenCount(j);
            for (k=0; k<scount; k++) {
               infile[i].getToken(buffer, j, k);
               if (strcmp(buffer, ".") == 0) {
                  continue;  // ignore null subtokens
               }
               if (sscanf(buffer, "%lf", &value) == 1) {
// cout << endl <<  "FOUND " << value << " at " << i << "," << j << "," << k << endl;
                  datum.data  = value;
                  datum.line  = i;
                  datum.spine = j;
                  datum.token = k;
                  data.append(datum);
               }
            }
         } 
      }
   }


   data.allowGrowth(0);
}



//////////////////////////////
//
// findBestField -- skip over the first **kern columns
//

int findBestField(HumdrumFile& infile) {
   int output = 0;
   int i, j;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() != E_humrec_interpretation) {
         continue;
      }
      if (strncmp(infile[i][0], "**", 2) != 0) {
         continue;   // data file is bad if this case occurs
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strcmp(infile[i][j], "**kern") == 0) {
            continue;
         }
         output = j;
         break;
      }
      break;
   }

   return output + 1;  // index from 1 to N
}





//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("m|mean=d:0.0", "use this mean rather than calculated one");
   opts.define("d|sd=d:0.0",   "use this SD rather than calculated one");
   opts.define("a|append=b",   "append output data to input");
   opts.define("p|prepend=b",  "prepend output data to input");
   opts.define("r|raw=b",      "raw output display");
   opts.define("sample=b",     "use sample rather than population SD");
   opts.define("replace=b",    "raw output display");
   opts.define("s|stat=b",     "display analyzed statistics");
   opts.define("S|suppress=b", "suppress display of statistics in output");
   opts.define("reverse=b",    "reverse z-score calculation process");
   opts.define("f|field=i:0",  "spine number to process");
   opts.define("i|input=b",    "display only extract values");
   opts.define("full=b",       "display only extract values with locations");

   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Jan 2008" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 12 Jan 2002" << endl;
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
   
   field     = opts.getInteger("field");
   inputQ    = opts.getBoolean("input");
   fullQ     = opts.getBoolean("full");
   statQ     = opts.getBoolean("stat");
   rawQ      = opts.getBoolean("raw");
   appendQ   = opts.getBoolean("append");
   prependQ  = opts.getBoolean("prepend");
   replaceQ  = opts.getBoolean("replace");
   suppressQ = opts.getBoolean("suppress");
   reverseQ  = opts.getBoolean("reverse");
   sampleQ   = opts.getBoolean("sample");

   meanQ     = opts.getBoolean("mean");
   myMean    = opts.getDouble("mean");
   sdQ       = opts.getBoolean("sd");
   mySd      = opts.getDouble("sd");
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



// md5sum: b17f9fee811c0f042b1d011ef0b71bfa zscores.cpp [20080202]

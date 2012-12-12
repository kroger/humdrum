//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 23 05:24:14 PST 2009
// Last Modified: Mon Nov 23 05:24:18 PST 2009
// Last Modified: Thu Dec 22 11:50:31 PST 2011 (added -V and -k options)
// Filename:      ...sig/examples/all/ridx.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/ridx.cpp
// Syntax:        C++; museinfo
//
// Description:   Remove various components of Humdrum file data structures.
//

#include "humdrum.h"
#include "PerlRegularExpression.h"

#include <string.h>
#include <stdio.h>

#ifndef OLDCPP
   #include <iostream>
   #include <fstream>
#else
   #include <iostream.h>
   #include <fstream.h>
#endif


// function declarations:
void      checkOptions          (Options& opts, int argc, char** argv);
void      example               (void);
void      usage                 (const char* command);
void      processFile           (HumdrumFile& infile);


// User interface variables:
Options   options;
int       option_D = 0;   // used with -D option
int       option_d = 0;   // used with -d option
int       option_G = 0;   // used with -G option
int       option_g = 0;   // used with -g option
int       option_I = 0;   // used with -I option
int       option_i = 0;   // used with -i option
int       option_L = 0;   // used with -L option
int       option_l = 0;   // used with -l option
int       option_T = 0;   // used with -T option
int       option_U = 0;   // used with -U and -u option

int       option_M = 0;   // used with -M option
int       option_C = 0;   // used with -C option
int       option_c = 0;   // used with -c option
int       option_k = 0;   // used with -k option
int       option_V = 0;   // used with -V option
 
//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   // process the command-line options
   checkOptions(options, argc, argv);
   HumdrumFile infile;
   infile.setAllocation(1123123);   // allow for very large inputs up to 
                                    // million lines.
   int numinputs = options.getArgumentCount();

   for (int i=0; i<numinputs || i==0; i++) {
      infile.clear();

      // if no command-line arguments read data file from standard input
      if (numinputs < 1) {
         infile.read(cin);
      } else {
         infile.read(options.getArg(i+1));
      }

      processFile(infile);

   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile) {
   int i;
   PerlRegularExpression pre;

   int revQ = option_V;

   for (i=0; i<infile.getNumLines(); i++) {

      if (option_D && (infile[i].isMeasure() || infile[i].isData())) {
         // remove data lines if -D is specified
         if (revQ) {
            cout << infile[i] << "\n";
         }
         continue;
      }
      if (option_d) {
         // remove null data lines if -d is specified
         if (option_k && infile[i].isData() && 
               infile[i].equalFieldsQ("**kern", ".")) {
            // remove if only all **kern spines are null.
            if (revQ) {
               cout << infile[i] << "\n";
            }
            continue;
         } else if (!option_k && infile[i].isData() && 
               infile[i].equalDataQ(".")) {
            // remove null data lines if all spines are null.
            if (revQ) {
               cout << infile[i] << "\n";
            }
            continue;
         }
      }
      if (option_G && (infile[i].isGlobalComment() || 
            infile[i].isBibliographic())) {
         // remove global comments if -G is specified
         if (revQ) {
            cout << infile[i] << "\n";
         }
         continue;
      }
      if (option_g && pre.search(infile[i][0], "^!!+\\s*$", "")) {
         // remove empty global comments if -g is specified
         if (revQ) {
            cout << infile[i] << "\n";
         }
         continue;
      }
      if (option_I && infile[i].isInterpretation()) {
         // remove all interpretation records
         if (revQ) {
            cout << infile[i] << "\n";
         }
         continue;
      }
      if (option_i && infile[i].isInterpretation() && 
            infile[i].equalDataQ("*")) {
         // remove null interpretation records
         if (revQ) {
            cout << infile[i] << "\n";
         }
         continue;
      }
      if (option_L && infile[i].isLocalComment()) {
         // remove all local comments
         if (revQ) {
            cout << infile[i] << "\n";
         }
         continue;
      }
      if (option_l && infile[i].isLocalComment() && 
            infile[i].equalDataQ("!")) {
         // remove null local comments
         if (revQ) {
            cout << infile[i] << "\n";
         }
         continue;
      }
      if (option_T && infile[i].isTandem()) {
         // remove tandem (non-manipulator) interpretations
         if (revQ) {
            cout << infile[i] << "\n";
         }
         continue;
      }
      if (option_U) {
         // remove unnecessary (duplicate exclusive) interpretations
         // HumdrumFile class does not allow duplicate ex. interps.
         // continue;
      }

      // non-classical options:

      if (option_M && infile[i].isMeasure()) {
         // remove all measure lines
         if (revQ) {
            cout << infile[i] << "\n";
         }
         continue;
      }
      if (option_C && infile[i].isComment()) {
         // remove all comments (local & global)
         if (revQ) {
            cout << infile[i] << "\n";
         }
         continue;
      }
      if (option_c && (infile[i].isLocalComment() || 
            infile[i].isGlobalComment())) {
         // remove all comments (local & global)
         if (revQ) {
            cout << infile[i] << "\n";
         }
         continue;
      }

      // got past all test, so print the current line:
      if (!revQ) {
         cout << infile[i] << "\n";
      }
   }

}



//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   // Humdrum Toolkit classic rid options:
   opts.define("D=b",          "remove all data records");
   opts.define("d=b",          "remove null data records");
   opts.define("G=b",          "remove all global comments");
   opts.define("g=b",          "remove null global comments");
   opts.define("I=b",          "remove all interpretation records");
   opts.define("i=b",          "remove null interpretation records");
   opts.define("L=b",          "remove all local comments");
   opts.define("l|1=b",        "remove null local comments");
   opts.define("T=b",          "remove all tandem interpretations");
   opts.define("U|u=b",        "remove unnecessary (duplicate ex. interps.");
   opts.define("k=b",          "for -d, only consider **kern spines.");
   opts.define("V=b",          "negate filtering effect of program.");

   // additional options
   opts.define("M=b",          "remove measure lines");
   opts.define("C=b",          "remove all comment lines");
   opts.define("c=b",          "remove global and local comment lines");


   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Nov 2009" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 23 Nov 2009" << endl;
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

   option_D = opts.getBoolean("D");
   option_d = opts.getBoolean("d");
   option_G = opts.getBoolean("G");
   option_g = opts.getBoolean("g");
   option_I = opts.getBoolean("I");
   option_i = opts.getBoolean("i");
   option_L = opts.getBoolean("L");
   option_l = opts.getBoolean("l");
   option_T = opts.getBoolean("T");
   option_U = opts.getBoolean("U");
   option_M = opts.getBoolean("M");
   option_C = opts.getBoolean("C");
   option_c = opts.getBoolean("c");
   option_k = opts.getBoolean("k");
   option_V = opts.getBoolean("V");
   
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



// md5sum: fa0b17b2b7bee282159c7cff84b67c36 ridx.cpp [20120404]

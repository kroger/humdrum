//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Oct 14 23:58:44  2002
// Last Modified: Mon Apr 25 11:30:20 PDT 2005
// Filename:      ...sig/examples/all/humcat.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/humcat.cpp
// Syntax:        C++; museinfo
//
// Description:   Concatenate multiple humdrum files into one continuous
//                data stream.
//

#include "humdrum.h"

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
void      printFile             (HumdrumFile& infile, int index, int count);
int       getIdTags             (Array<Array<char> >& idtags, 
                                 HumdrumFile& infile);
void      printFileID           (HumdrumFile& infile, int index, int count, 
                                 Array<Array<char> >& primaryids);
int       findTag               (Array<Array<char> >& primaryids, 
                                 Array<char>& idtags);
void      printLineID           (HumdrumFile& infile, int index, 
                                 Array<Array<char> >& primaryids, 
				 Array<Array<char> >& idtags);

// User interface variables:
Options   options;
HumdrumFile firstinput;

//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
   // process the command-line options
   checkOptions(options, argc, argv);
   HumdrumFile infile;
   int i;

   Array<Array<char> > idtags;
   int idtagQ = 0;
   firstinput.read(options.getArg(1));

   for (i=1; i<=options.getArgCount(); i++) {
      infile.clear();
      infile.read(options.getArg(i));
      if (i ==1) {
         idtagQ = getIdTags(idtags, infile);
      }

      if (idtagQ && i > 1) {
         printFileID(infile, i-1, options.getArgCount(), idtags);
      } else {
         printFile(infile, i-1, options.getArgCount());
      }
   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// getIdTags --
//

int getIdTags(Array<Array<char> >& idtags, HumdrumFile& infile) {
   int i, j;
   idtags.setSize(infile.getMaxTracks());
   for (i=0; i<idtags.getSize(); i++) {
      idtags[i].setSize(128);
      idtags[i][0] = '\0';
   }
   int foundids = 0;
   char tag[128] = {0};
   
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].getType() != E_humrec_data_comment) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strncmp(infile[i][j], "!ID=", 4) == 0) {
            strcpy(tag, &(infile[i][j][4]));
            if (strlen(tag)) {
               foundids++;
               strcpy(idtags[infile[i].getPrimaryTrack(j)-1].getBase(), tag);
            }
         }
      }
   }

   return foundids;
}


//////////////////////////////
//
// printFile --
//

void printFile(HumdrumFile& infile, int index, int count) {

   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isInterpretation()) {
         if ((strcmp(infile[i][0], "*-") == 0) && (index == count - 1)) {
            cout << infile[i] << "\n";
         } else if ((strncmp(infile[i][0], "**", 2) == 0) && (index == 0)) {
            cout << infile[i] << "\n";
         } else {
            if ((strcmp(infile[i][0], "*-") == 0) && (index != count - 1)) {
               // print nothing
            } else if ((strncmp(infile[i][0], "**", 2) == 0) && (index != 0)) {
               // print nothing
            } else {
               cout << infile[i] << "\n";
            }
         }
      } else {
         cout << infile[i] << "\n";
      }
   }

}



//////////////////////////////
//
// printFileID --
//

void printFileID(HumdrumFile& infile, int index, int count, 
      Array<Array<char> >& primaryids) {

   Array<Array<char> > idtags;
   int idtagQ = getIdTags(idtags, infile);
   if (!idtagQ) {
      cout << "ERROR: no ID tags found in file: " << endl;
      cout << infile;
      exit(1);
   }

   int i;
   for (i=0; i<infile.getNumLines(); i++) {
      if (infile[i].isInterpretation()) {
         if ((strcmp(infile[i][0], "*-") == 0) && (index == count - 1)) {
            cout << infile[i] << "\n";
         } else if ((strncmp(infile[i][0], "**", 2) == 0) && (index == 0)) {
            cout << infile[i] << "\n";
         } else {
            if ((strcmp(infile[i][0], "*-") == 0) && (index != count - 1)) {
               // print nothing
            } else if ((strncmp(infile[i][0], "**", 2) == 0) && (index != 0)) {
               // print nothing
            } else {
               printLineID(infile, i, primaryids, idtags);
               // cout << infile[i] << "\n";
            }
         }
      } else {
         printLineID(infile, i, primaryids, idtags);
         // cout << infile[i] << "\n";
      }
   }

}



//////////////////////////////
//
// printLineID --
//

void printLineID(HumdrumFile& infile, int index, 
      Array<Array<char> >& primaryids, Array<Array<char> >& idtags) {
   Array<Array<int> > spines;
   spines.setSize(primaryids.getSize());
   int i;
   for (i=0; i<spines.getSize(); i++) {
      spines[i].setSize(32);
      spines[i].setSize(0);
   }

   int mapping;
   for (i=0; i<idtags.getSize(); i++) {
      mapping = findTag(primaryids, idtags[i]);
      spines[mapping].append(i);
   }

   int j;
   for (i=0; i<spines.getSize(); i++) {
      if (spines[i].getSize() == 0) {
         cout << "X";
      } else {
         for (j=0; j<spines[i].getSize(); j++) {
            cout << "Y";
         }
      }
      cout << "\t";
   }

}



//////////////////////////////
//
// findTag --
//

int findTag(Array<Array<char> >& primaryids, Array<char>& idtags) {
   int i;
   for (i=0; i<primaryids.getSize(); i++) {
      if (strcmp(primaryids[i].getBase(), idtags.getBase()) == 0) {
         return i;
      }
   }

   return -1;
}




//////////////////////////////
//
// checkOptions -- 
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("h|header=s:",  "Header filename for placement in output");
   opts.define("t|trailer=s:", "Trailer filename for placement in output");

   opts.define("author=b",  "author of program"); 
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");   
   opts.define("help=b",  "short description");
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Oct 2002" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 14 Oct 2002" << endl;
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



// md5sum: c8f34a7f6c04aed3445355b1f4deef3a humcat.cpp [20080518]

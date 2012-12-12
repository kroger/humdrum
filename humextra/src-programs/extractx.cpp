//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 18 11:23:42 PDT 2005
// Last Modified: Tue Sep  1 13:54:42 PDT 2009 (added -f, and -p options)
// Last Modified: Wed Sep  2 13:26:58 PDT 2009 (added -t option)
// Last Modified: Sat Sep  5 15:21:29 PDT 2009 (added sub-spine features)
// Last Modified: Tue Sep  8 11:43:46 PDT 2009 (added co-spine extraction)
// Filename:      ...sig/examples/all/extractx.cpp
// Web Address:   http://sig.sapp.org/examples/museinfo/humdrum/extractx.cpp
// Syntax:        C++; museinfo
//
// Description:   C++ implementation of the Humdrum Toolkit extract command.
//

#include "humdrum.h"
#include "PerlRegularExpression.h"

#include <stdlib.h>
#include <string.h>

#ifndef OLDCPP
   #include <iostream>
   #include <fstream>
#else
   #include <iostream.h>
   #include <fstream.h>
#endif


// function declarations
void    checkOptions            (Options& opts, int argc, char* argv[]);
void    excludeFields           (HumdrumFile& infile, Array<int>& field, 
                                 Array<int>& subfield, Array<int>& model);
void    extractFields           (HumdrumFile& infile, Array<int>& field, 
                                 Array<int>& subfield, Array<int>& model);
void    extractTrace            (HumdrumFile& infile, const char* tracefile);
void    getInterpretationFields (Array<int>& field, Array<int>& subfield,
                                 Array<int>& model, HumdrumFile& infile,
                                 const char* interps, int state);
//void    extractInterpretations  (HumdrumFile& infile, const char* interps);
void    example                 (void);
void    usage                   (const char* command);
void    fillFieldData           (Array<int>& field, Array<int>& subfield, 
                                 Array<int>& model, const char* fieldstring, 
                                 HumdrumFile& infile);
void    processFieldEntry       (Array<int>& field, Array<int>& subfield, 
                                 Array<int>& model, const char* string, 
                                 HumdrumFile& infile);
void    removeDollarsFromString (Array<char>& buffer, int maxtrack);
int     isInList                (int number, Array<int>& listofnum);
void    getTraceData            (Array<int>& startline, 
                                 Array<Array<int> >& fields, 
                                 const char* tracefile, HumdrumFile& infile);
void    printTraceLine          (HumdrumFile& infile, int line, 
                                 Array<int>& field);
void    dealWithSpineManipulators(HumdrumFile& infile, int line, 
                                 Array<int>& field, Array<int>& subfield,
                                 Array<int>& model);
void    storeToken              (Array<Array<char> >& storage, 
                                 const char* string);
void    storeToken              (Array<Array<char> >& storage, int index, 
                                 const char* string);
void    printMultiLines         (Array<int>& vsplit, Array<int>& vserial, 
                                 Array<Array<char> >& tempout);
void    reverseSpines           (Array<int>& field, Array<int>& subfield, 
                                 Array<int>& model, HumdrumFile& infile, 
                                 const char* exinterp);
void    getSearchPat            (Array<char>& spat, int target, 
                                 const char* modifier);
void    expandSpines            (Array<int>& field, Array<int>& subfield, 
                                 Array<int>& model, HumdrumFile& infile, 
                                 const char* interp);
void    dealWithSecondarySubspine(Array<int>& field, Array<int>& subfield, 
                                 Array<int>& model, int targetindex, 
                                 HumdrumFile& infile, int line, int spine, 
                                 int submodel);
void    dealWithCospine         (Array<int>& field, Array<int>& subfield, 
                                 Array<int>& model, int targetindex, 
                                 HumdrumFile& infile, int line, int cospine, 
                                 int comodel, int submodel, 
                                 const char* cointerp);
void    printCotokenInfo        (int& start, HumdrumFile& infile, int line, 
                                 int spine, Array<Array<char> >& cotokens, 
                                 Array<int>& spineindex, 
                                 Array<int>& subspineindex);
void    fillFieldDataByGrep     (Array<int>& field, Array<int>& subfield, 
                                 Array<int>& model, const char* grepString, 
                                 HumdrumFile& infile, int state);

// global variables
Options      options;            // database for command-line arguments
int          excludeQ = 0;       // used with -x option
int          expandQ  = 0;       // used with -e option
const char*  expandInterp = "";  // used with -E option
int          interpQ  = 0;       // used with -i option
const char*  interps  = "";      // used with -i option
int          debugQ   = 0;       // used with --debug option
int          fieldQ   = 0;       // used with -f or -p option
const char*  fieldstring = "";   // used with -f or -p option
Array<int>   field;              // used with -f or -p option
Array<int>   subfield;           // used with -f or -p option
Array<int>   model;              // used with -p, or -e options and similar
int          countQ   = 0;       // used with -C option
int          traceQ   = 0;       // used with -t option
const char*  tracefile = "";     // used with -t option
int          reverseQ = 0;       // used with -r option
const char*  reverseInterp = "**kern"; // used with -r and -R options.
// sub-spine "b" expansion model: how to generate data for a secondary
// spine if the primary spine is not divided.  Models are:
//    'd': duplicate primary spine (or "a" subspine) data (default)
//    'n': null = use a null token
//    'r': rest = use a rest instead of a primary spine note (in **kern)
//         data.  'n' will be used for non-kern spines when 'r' is used.
int          submodel = 'd';     // used with -m option
const char* editorialInterpretation = "yy";
const char* cointerp = "**kern";   // used with -c option
int         comodel  = 0;          // used with -M option
const char* subtokenseparator = " "; // used with a future option
int         interpstate = 0;       // used -I or with -i
int         grepQ       = 0;       // used with -g option
const char* grepString  = "";      // used with -g option



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
      if (countQ) {
         cout << infile.getMaxTracks() << endl;
         exit(0);
      }
      if (expandQ) {
         expandSpines(field, subfield, model, infile, expandInterp);
      } else if (interpQ) {
         getInterpretationFields(field, subfield, model, infile, interps, 
               interpstate);
      } else if (reverseQ) {
         reverseSpines(field, subfield, model, infile, reverseInterp);
      } else if (fieldQ || excludeQ) {
         fillFieldData(field, subfield, model, fieldstring, infile);
      } else if (grepQ) {
         fillFieldDataByGrep(field, subfield, model, grepString, infile, 
            interpstate);
      }
      
      if (debugQ && !traceQ) {
         cout << "!! Field Expansion List:";
         for (int j=0; j<field.getSize(); j++) {
            cout << " " << field[j];
	    if (subfield[j]) {
               cout << (char)subfield[j];
            }
	    if (model[j]) {
               cout << (char)model[j];
            }
         }
         cout << endl;
      }

      // analyze the input file according to command-line options
      if (fieldQ || grepQ) {
         extractFields(infile, field, subfield, model);
      } else if (excludeQ) {
         excludeFields(infile, field, subfield, model);
      } else if (traceQ) {
         extractTrace(infile, tracefile);
      } else {
         cout << infile;
      }
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// fillFieldDataByGrep --
//

void fillFieldDataByGrep(Array<int>& field, Array<int>& subfield, 
      Array<int>& model, const char* searchstring, HumdrumFile& infile, 
      int state) {

   field.setSize(infile.getMaxTracks()+1);
   subfield.setSize(infile.getMaxTracks()+1);
   model.setSize(infile.getMaxTracks()+1);
   field.setSize(0);
   subfield.setSize(0);
   model.setSize(0);

   Array<int> tracks;
   tracks.setSize(infile.getMaxTracks()+1);
   tracks.setAll(0);
   PerlRegularExpression pre;
   int track;

   int i, j;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isSpineLine()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (pre.search(infile[i][j], searchstring, "")) {
            track = infile[i].getPrimaryTrack(j);
            tracks[track] = 1;
         }
      }
   }

   int zero = 0;
   for (i=1; i<tracks.getSize(); i++) {
      if (state != 0) {
         tracks[i] = !tracks[i];
      }
      if (tracks[i]) {
         field.append(i);
         subfield.append(zero);
         model.append(zero);
      }
   }
}



//////////////////////////////
//
// getInterpretationFields --
//

void getInterpretationFields(Array<int>& field, Array<int>& subfield, 
      Array<int>& model, HumdrumFile& infile, const char* interps, 
      int state) {
   Array<Array<char> > sstrings; // search strings
   sstrings.setSize(100);
   sstrings.setSize(0);
   sstrings.setGrowth(1000);

   int i, j, k;
   Array<char> buffer;
   buffer.setSize(strlen(interps)+1);
   strcpy(buffer.getBase(), interps);

   PerlRegularExpression pre;
   pre.sar(buffer, "\\s+", "", "g");  // remove spaces from the search string.

   int index;
   while (pre.search(buffer, "^([^,]+)")) {
      sstrings.setSize(sstrings.getSize()+1);
      index = sstrings.getSize()-1;
      sstrings[index].setSize(strlen(pre.getSubmatch(1))+1);
      strcpy(sstrings[index].getBase(), pre.getSubmatch());
      pre.sar(buffer, "^[^,]+,?", "", "");
   }

   if (debugQ) {
      cout << "!! Interpretation strings to search for: " << endl;
      for (i=0; i<sstrings.getSize(); i++) {
         cout << "!!\t" << sstrings[i].getBase() << endl;
      }
   }

   Array<int> tracks;
   tracks.setSize(infile.getMaxTracks()+1);
   tracks.setAll(0);
   tracks.allowGrowth(0);

   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isInterpretation()) {
         continue;
      }
      for (j=0; j<infile[i].getFieldCount(); j++) {
         for (k=0; k<sstrings.getSize(); k++) {
            if (strcmp(sstrings[k].getBase(), infile[i][j]) == 0) {
               tracks[infile[i].getPrimaryTrack(j)] = 1;
            }
         }
      }
   }

   field.setSize(tracks.getSize());
   field.setSize(0);
   subfield.setSize(tracks.getSize());
   subfield.setSize(0);
   model.setSize(tracks.getSize());
   model.setSize(0);


   int zero = 0;
   for (i=1; i<tracks.getSize(); i++) {
      if (state == 0) {
         tracks[i] = !tracks[i];
      }
      if (tracks[i]) {
         field.append(i);
         subfield.append(zero);
         model.append(zero);
      }
   }

}



//////////////////////////////
//
// expandSpines --
//

void expandSpines(Array<int>& field, Array<int>& subfield, Array<int>& model,
      HumdrumFile& infile, const char* interp) {

   Array<int> splits;
   splits.setSize(infile.getMaxTracks()+1);
   splits.allowGrowth(0);
   splits.setAll(0);

   int i, j;
   for (i=0; i<infile.getNumLines(); i++) {
      if (!infile[i].isSpineManipulator()) {
         continue;
      }

      for (j=0; j<infile[i].getFieldCount(); j++) {
         if (strchr(infile[i].getSpineInfo(j), '(') != NULL) {
            splits[infile[i].getPrimaryTrack(j)] = 1;
         }
      }
   }

   field.setSize(infile.getMaxTracks()*2);
   field.setSize(0);
   field.allowGrowth(1);
   subfield.setSize(infile.getMaxTracks()*2);
   subfield.setSize(0);
   subfield.allowGrowth(1);
   model.setSize(infile.getMaxTracks()*2);
   model.setSize(0);
   model.allowGrowth(1);
   int allQ = 0;
   if (strcmp(interp, "") == 0) {
      allQ = 1;
   }

   // ggg
   Array<int> dummyfield;
   Array<int> dummysubfield;
   Array<int> dummymodel;
   getInterpretationFields(dummyfield, dummysubfield, model, infile, interp, 1);

   Array<int> interptracks;

   interptracks.setSize(infile.getMaxTracks()+1);
   interptracks.allowGrowth(0);
   interptracks.setAll(0);
   for (i=0; i<dummyfield.getSize(); i++) {
      interptracks[dummyfield[i]] = 1;
   }

   int aval = 'a';
   int bval = 'b';
   int zero = 0;
   for (i=1; i<splits.getSize(); i++) {
      if (splits[i] && (allQ || interptracks[i])) {
         field.append(i);
         subfield.append(aval);
         model.append(zero);
         field.append(i);
         subfield.append(bval);
         model.append(zero);
      } else {
         field.append(i);
         subfield.append(zero);
         model.append(zero);
      }
   }

   field.allowGrowth(0);
   subfield.allowGrowth(0);
   model.allowGrowth(0);

   if (debugQ) {
      cout << "!!expand: ";
      for (i=0; i<field.getSize(); i++) {
         cout << field[i];
         if (subfield[i]) {
            cout << (char)subfield[i];
         }
         if (i < field.getSize()-1) {
            cout << ",";
         }
      }
      cout << endl;
   }
}



//////////////////////////////
//
// reverseSpines -- reverse the order of spines, grouped by the
//   given exclusive interpretation.
//

void reverseSpines(Array<int>& field, Array<int>& subfield, Array<int>& model,
      HumdrumFile& infile, const char* exinterp) {

   Array<int> target;
   target.setSize(infile.getMaxTracks()+1);
   target.allowGrowth(0);
   target.setAll(0);

   int t;

   for (t=1; t<=infile.getMaxTracks(); t++) {
      if (strcmp(infile.getTrackExInterp(t), exinterp) == 0) {
         target[t] = 1;
      }
   }

   field.setSize(infile.getMaxTracks()*2);
   field.setSize(0);

   int i, j;
   int lasti = target.getSize();
   for (i=target.getSize()-1; i>0; i--) {
      if (target[i]) {
         lasti = i;
         field.append(i);
         for (j=i+1; j<target.getSize(); j++) {
            if (!target[j]) {
               field.append(j);
            } else {
               break;
            }
         }
      }
   }

   // if the grouping spine is not first, then preserve the
   // locations of the pre-spines.
   int extras = 0;
   if (lasti != 1) {
      extras = lasti - 1;
      field.setSize(field.getSize()+extras);
      for (i=0; i<field.getSize()-extras; i++) {
         field[field.getSize()-1-i] = field[field.getSize()-1-extras-i];
      }
      for (i=0; i<extras; i++) {
         field[i] = i+1;
      }
   }

   if (debugQ) {
      cout << "!!reverse: ";
      for (i=0; i<field.getSize(); i++) {
         cout << field[i] << " ";
      }
      cout << endl;
   }
	     

   subfield.setSize(field.getSize());
   subfield.allowGrowth(0);
   subfield.setAll(0);
   model.setSize(field.getSize());
   model.allowGrowth(0);
   model.setAll(0);
   field.allowGrowth(0);
}



//////////////////////////////
//
// fillFieldData --
//

void fillFieldData(Array<int>& field, Array<int>& subfield, Array<int>& model,
      const char* fieldstring, HumdrumFile& infile) {

   int maxtrack = infile.getMaxTracks();

   field.setSize(maxtrack);
   field.setGrowth(maxtrack);
   field.setSize(0);
   subfield.setSize(maxtrack);
   subfield.setGrowth(maxtrack);
   subfield.setSize(0);
   model.setSize(maxtrack);
   model.setGrowth(maxtrack);
   model.setSize(0);

   PerlRegularExpression pre;
   Array<char> buffer;
   buffer.setSize(strlen(fieldstring)+1);
   strcpy(buffer.getBase(), fieldstring);
   pre.sar(buffer, "\\s", "", "gs");
   int start = 0;
   int value = 0;
   value = pre.search(buffer.getBase(), "^([^,]+,?)");
   while (value != 0) {
      start += value - 1;
      start += strlen(pre.getSubmatch(1));
      processFieldEntry(field, subfield, model, pre.getSubmatch(), infile);
      value = pre.search(buffer.getBase() + start, "^([^,]+,?)");
   }
}



//////////////////////////////
//
// processFieldEntry -- 
//   3-6 expands to 3 4 5 6
//   $   expands to maximum spine track
//   $-1 expands to maximum spine track minus 1, etc.
//

void processFieldEntry(Array<int>& field, Array<int>& subfield, 
      Array<int>& model, const char* string, HumdrumFile& infile) {

   int maxtrack = infile.getMaxTracks();
   int modletter;
   int subletter;

   PerlRegularExpression pre;
   Array<char> buffer;
   buffer.setSize(strlen(string)+1);
   strcpy(buffer.getBase(), string);

   // remove any comma left at end of input string (or anywhere else)
   pre.sar(buffer, ",", "", "g");

   // first remove $ symbols and replace with the correct values
   removeDollarsFromString(buffer, infile.getMaxTracks());

   int zero = 0;
   if (pre.search(buffer.getBase(), "^(\\d+)-(\\d+)$")) {
      int firstone = strtol(pre.getSubmatch(1), NULL, 10);
      int lastone  = strtol(pre.getSubmatch(2), NULL, 10);

      if ((firstone < 1) && (firstone != 0)) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains too small a number at start: " << firstone << endl;
         cerr << "Minimum number allowed is " << 1 << endl;
         exit(1);
      }
      if ((lastone < 1) && (lastone != 0)) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains too small a number at end: " << lastone << endl;
         cerr << "Minimum number allowed is " << 1 << endl;
         exit(1);
      }
      if (firstone > maxtrack) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains number too large at start: " << firstone << endl;
         cerr << "Maximum number allowed is " << maxtrack << endl;
         exit(1);
      }
      if (lastone > maxtrack) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains number too large at end: " << lastone << endl;
         cerr << "Maximum number allowed is " << maxtrack << endl;
         exit(1);
      }

      int i;
      if (firstone > lastone) {
         for (i=firstone; i>=lastone; i--) {
            field.append(i);
            subfield.append(zero);
            model.append(zero);
         }
      } else {
         for (i=firstone; i<=lastone; i++) {
            field.append(i);
            subfield.append(zero);
            model.append(zero);
         }
      }
   } else if (pre.search(buffer.getBase(), "^(\\d+)([a-z]*)")) {
      int value = strtol(pre.getSubmatch(1), NULL, 10);
      modletter = 0;
      subletter = 0;
      if (strchr(pre.getSubmatch(2), 'a') != NULL) {
         subletter = 'a';
      }
      if (strchr(pre.getSubmatch(), 'b') != NULL) {
         subletter = 'b';
      }
      if (strchr(pre.getSubmatch(), 'c') != NULL) {
         subletter = 'c';
      }
      if (strchr(pre.getSubmatch(), 'd') != NULL) {
         modletter = 'd';
      }
      if (strchr(pre.getSubmatch(), 'n') != NULL) {
         modletter = 'n';
      }
      if (strchr(pre.getSubmatch(), 'r') != NULL) {
         modletter = 'r';
      }

      if ((value < 1) && (value != 0)) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains too small a number at end: " << value << endl;
         cerr << "Minimum number allowed is " << 1 << endl;
         exit(1);
      }
      if (value > maxtrack) {
         cerr << "Error: range token: \"" << string << "\"" 
              << " contains number too large at start: " << value << endl;
         cerr << "Maximum number allowed is " << maxtrack << endl;
         exit(1);
      }
      field.append(value);
      if (value == 0) {
         subfield.append(zero);
         model.append(zero);
      } else {
         subfield.append(subletter);
         model.append(modletter);
      }
   }
}



//////////////////////////////
//
// removeDollarsFromString -- substitute $ sign for maximum track count.
//

void removeDollarsFromString(Array<char>& buffer, int maxtrack) {
   PerlRegularExpression pre;
   char buf2[128] = {0};
   int value2;

   if (pre.search(buffer.getBase(), "\\$$")) {
      sprintf(buf2, "%d", maxtrack);
      pre.sar(buffer, "\\$$", buf2);
   }

   if (pre.search(buffer.getBase(), "\\$(?![\\d-])")) {
      // don't know how this case could happen, however...
      sprintf(buf2, "%d", maxtrack);
      pre.sar(buffer, "\\$(?![\\d-])", buf2, "g");
   }

   if (pre.search(buffer.getBase(), "\\$0")) {
      // replace $0 with maxtrack (used for reverse orderings)
      sprintf(buf2, "%d", maxtrack);
      pre.sar(buffer, "\\$0", buf2, "g");
   }

   while (pre.search(buffer.getBase(), "\\$(-?\\d+)")) {
      value2 = maxtrack - (int)fabs(strtol(pre.getSubmatch(1), NULL, 10));
      sprintf(buf2, "%d", value2);
      pre.sar(buffer, "\\$-?\\d+", buf2);
   }

}



//////////////////////////////
//
// excludeFields -- print all spines except the ones in the list of fields.
//

void excludeFields(HumdrumFile& infile, Array<int>& field,
      Array<int>& subfield, Array<int>& model) {
   int i;
   int j;
   int start = 0;

   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
            cout << infile[i] << '\n';
            break;
         case E_humrec_data_comment:
         case E_humrec_data_kern_measure:
         case E_humrec_interpretation:
         case E_humrec_data:
            start = 0;
            for (j=0; j<infile[i].getFieldCount(); j++) {
               if (isInList(infile[i].getPrimaryTrack(j), field)) {
                  continue;
               }
               if (start != 0) {
                  cout << '\t';
               }
               start = 1;
               cout << infile[i][j];
            }
            if (start != 0) {
               cout << endl;
            }
            break;
         default:
            cout << "!!Line = UNKNOWN:" << infile[i] << endl;
            break;
      }
   }
}



//////////////////////////////
//
// extractFields -- print all spines except the ones in the list of fields.
//

void extractFields(HumdrumFile& infile, Array<int>& field,
      Array<int>& subfield, Array<int>& model) {

   PerlRegularExpression pre;
   int i;
   int j;
   int t;
   int start = 0;
   int target;
   int subtarget;
   int modeltarget;
   Array<char> spat;

   for (i=0; i<infile.getNumLines(); i++) {
      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
            cout << infile[i] << '\n';
            break;
         case E_humrec_data_comment:
         case E_humrec_data_kern_measure:
         case E_humrec_interpretation:
         case E_humrec_data:
            if (infile[i].isSpineManipulator()) {
               dealWithSpineManipulators(infile, i, field, subfield, model);
               break;
            }
            start = 0;
            for (t=0; t<field.getSize(); t++) {
               target = field[t];
	       subtarget = subfield[t];
               modeltarget = model[t];
               if (modeltarget == 0) {
                  switch (subtarget) {
                     case 'a':
                     case 'b':
                        modeltarget = submodel;
                        break;
                     case 'c':
                        modeltarget = comodel;
                  }
               }
               if (target == 0) {
                  if (start != 0) {
                     cout << '\t';
                  }
                  start = 1;
                  switch (infile[i].getType()) {
                     case E_humrec_data_comment:
                        cout << "!"; break;
                     case E_humrec_data_kern_measure:
                        cout << infile[i][0]; break;
                     case E_humrec_data:
                        cout << "."; break;
                     // interpretations handled in dealwithSpineManipulators()
                  }
               } else {
                  for (j=0; j<infile[i].getFieldCount(); j++) {
                     if (infile[i].getPrimaryTrack(j) != target) {
                        continue;
                     } 
                     switch (subtarget) {
                     case 'a':

                        getSearchPat(spat, target, "a");
                        if (pre.search(infile[i].getSpineInfo(j), 
                              spat.getBase()) ||
                              !pre.search(infile[i].getSpineInfo(j), "\\(")) {
                           if (start != 0) {
                              cout << '\t';
                           }
                           start = 1;
                           cout << infile[i][j];
                        }

                        break;
                     case 'b':

                        getSearchPat(spat, target, "b");
                        if (pre.search(infile[i].getSpineInfo(j), 
                              spat.getBase())) { 
                           if (start != 0) {
                              cout << '\t';
                           }
                           start = 1;
                           cout << infile[i][j];
                        } else if (!pre.search(infile[i].getSpineInfo(j), 
                              "\\(")) {
                           if (start != 0) {
                              cout << '\t';
                           }
                           start = 1;
			   dealWithSecondarySubspine(field, subfield, model, t, 
                                 infile, i, j, modeltarget);
                        }

                        break;
                     case 'c':
                        if (start != 0) {
                           cout << '\t';
                        }
			start = 1;
			dealWithCospine(field, subfield, model, t, infile, i, j,
                           modeltarget, modeltarget, cointerp);
                        break;
                     default:
                        if (start != 0) {
                           cout << '\t';
                        }
                        start = 1;
                        cout << infile[i][j];
                     }
                  }
               }
            }
            if (start != 0) {
               cout << endl;
            }
            break;
         default:
            cout << "!!Line = UNKNOWN:" << infile[i] << endl;
            break;
      }
   }
}



//////////////////////////////
//
// dealWithCospine -- extract the required token(s) from a co-spine.
//

void dealWithCospine(Array<int>& field, Array<int>& subfield, Array<int>& model,
      int targetindex, HumdrumFile& infile, int line, int cospine, 
      int comodel, int submodel, const char* cointerp) {

   Array<Array<char> > cotokens;
   cotokens.setSize(50);
   cotokens.setSize(0);
   cotokens.setGrowth(200);

   char buffer[1024];
   int i, j, k;
   int index;

   if (infile[line].isInterpretation()) {
      cout << infile[line][cospine];
      return;
   }

   if (infile[line].isMeasure()) {
      cout << infile[line][cospine];
      return;
   }

   if (infile[line].isLocalComment()) {
      cout << infile[line][cospine];
      return;
   }

   int count = infile[line].getTokenCount(cospine);
   for (k=0; k<count; k++) {
      infile[line].getToken(buffer, cospine, k, 1000);
      cotokens.setSize(cotokens.getSize()+1);
      index = cotokens.getSize()-1;
      cotokens[index].setSize(strlen(buffer)+1);
      strcpy(cotokens[index].getBase(), buffer);
   }

   Array<int> spineindex;
   Array<int> subspineindex;
   spineindex.setSize(infile.getMaxTracks()*2);
   subspineindex.setSize(infile.getMaxTracks()*2);
   spineindex.setSize(0);
   subspineindex.setSize(0);
   spineindex.setGrowth(1000);
   subspineindex.setGrowth(1000);

   for (j=0; j<infile[line].getFieldCount(); j++) {
      if (strcmp(infile[line].getExInterp(j), cointerp) != 0) {
         continue;
      }
      if (strcmp(infile[line][j], ".") == 0) {
         continue;
      }
      count = infile[line].getTokenCount(j);
      for (k=0; k<count; k++) {
         infile[line].getToken(buffer, j, k, 1000);
         if (comodel == 'r') {
            if (strchr(buffer, 'r') != NULL) {
               continue;
            }
         }
         spineindex.append(j);
         subspineindex.append(k);
      }
   }

   if (debugQ) {
      cout << "\n!!codata:\n";
      for (i=0; i<cotokens.getSize(); i++) {
         cout << "!!\t" << i << "\t" << cotokens[i].getBase();
         if (i < spineindex.getSize()) {
            cout << "\tspine=" << spineindex[i];
            cout << "\tsubspine=" << subspineindex[i];
         } else {
            cout << "\tspine=.";
            cout << "\tsubspine=.";
         }
         cout << endl;
      }
   }

   Array<char> buff;

   int start = 0;
   for (i=0; i<field.getSize(); i++) {
      if (strcmp(infile.getTrackExInterp(field[i]), cointerp) != 0) {
         continue;
      }

      for (j=0; j<infile[line].getFieldCount(); j++) {
         if (infile[line].getPrimaryTrack(j) != field[i]) {
            continue;
         }
         if (subfield[i] == 'a') {
            getSearchPat(buff, field[i], "a");
            if ((strchr(infile[line].getSpineInfo(j), '(') == NULL) ||
               (strstr(infile[line].getSpineInfo(j), buff.getBase()) != NULL)) {
               printCotokenInfo(start, infile, line, j, cotokens, spineindex, 
                     subspineindex);
            }
         } else if (subfield[i] == 'b') {
            // this section may need more work...
            getSearchPat(buff, field[i], "b");
            if ((strchr(infile[line].getSpineInfo(j), '(') == NULL) ||
               (strstr(infile[line].getSpineInfo(j), buff.getBase()) != NULL)) {
               printCotokenInfo(start, infile, line, j, cotokens, spineindex, 
                     subspineindex);
            }
         } else {
            printCotokenInfo(start, infile, line, j, cotokens, spineindex, 
               subspineindex);
         }
      }
   }
}



//////////////////////////////
//
// printCotokenInfo --
//

void printCotokenInfo(int& start, HumdrumFile& infile, int line, int spine, 
      Array<Array<char> >& cotokens, Array<int>& spineindex, 
      Array<int>& subspineindex) {
   int i;
   int found = 0;
   for (i=0; i<spineindex.getSize(); i++) {
      if (spineindex[i] == spine) {
         if (start == 0) {
            start++;
         } else {
            cout << subtokenseparator;
         }
         if (i<cotokens.getSize()) {
            cout << cotokens[i].getBase();
         } else {
            cout << ".";
         }
	 found = 1;
      }
   }
   if (!found) {
      if (start == 0) {
         start++;
      } else {
         cout << subtokenseparator;
      }
      cout << ".";
   }
}



//////////////////////////////
//
// dealWithSecondarySubspine -- what to print if a secondary spine
//     does not exist on a line.
//

void dealWithSecondarySubspine(Array<int>& field, Array<int>& subfield, 
      Array<int>& model, int targetindex, HumdrumFile& infile, int line, 
      int spine, int submodel) {

   int& i = line;
   int& j = spine;

   PerlRegularExpression pre;
   Array<char> buffer;

   switch (infile[line].getType()) {
      case E_humrec_data_comment:
         if ((submodel == 'n') || (submodel == 'r')) {
            cout << "!";
         } else {
            cout << infile[i][j];
         }
         break;

      case E_humrec_data_kern_measure:
         cout << infile[i][j];
         break;

      case E_humrec_interpretation:
         if ((submodel == 'n') || (submodel == 'r')) {
            cout << "*";
         } else {
            cout << infile[i][j];
         }
         break;

      case E_humrec_data:
         if (submodel == 'n') {
            cout << ".";
         } else if (submodel == 'r') {
            if (strcmp(infile[i][j], ".") == 0) {
               cout << ".";
            } else if (strchr(infile[i][j], 'q') != NULL) {
               cout << ".";
            } else if (strchr(infile[i][j], 'Q') != NULL) {
               cout << ".";
            } else {
               buffer.setSize(strlen(infile[i][j])+1);
               strcpy(buffer.getBase(), infile[i][j]);
               if (pre.search(buffer, "{")) {
                  cout << "{";
               }
               // remove secondary chord notes:
               pre.sar(buffer, " .*", "", "");
               // remove unnecessary characters (such as stem direction):
               pre.sar(buffer, "[^}pPqQA-Ga-g0-9.;%#n-]", "", "g");
               // change pitch to rest:
               pre.sar(buffer, "[A-Ga-g#n-]+", "r", "");
               // add editorial marking unless -Y option is given:
               if (strcmp(editorialInterpretation, "") != 0) {
                  if (pre.search(buffer, "rr")) {
                     pre.sar(buffer, "(?<=rr)", editorialInterpretation, "");
                  } else {
                     pre.sar(buffer, "(?<=r)", editorialInterpretation, "");
                  }
               }
               cout << buffer.getBase();
            }
         } else {
            cout << infile[i][j];
         }
         break;

      default:
         cerr << "Should not get to this line of code" << endl;
         exit(1);
   }
}




//////////////////////////////
//
// getSearchPat -- 
//

void getSearchPat(Array<char>& spat, int target, const char* modifier) {
   if (strlen(modifier) > 20) {
      cerr << "Error in GetSearchPat" << endl;
      exit(1);
   }
   spat.setSize(32);
   strcpy(spat.getBase(), "\\(");
   char buffer[32] = {0};
   sprintf(buffer, "%d", target);
   strcat(spat.getBase(), buffer);
   strcat(spat.getBase(), "\\)");
   strcat(spat.getBase(), modifier);
   spat.setSize(strlen(spat.getBase())+1);
}



//////////////////////////////
//
// dealWithSpineManipulators -- check for proper Humdrum syntax of
//     spine manipulators (**, *-, *x, *v, *^) when creating the output.
//

void dealWithSpineManipulators(HumdrumFile& infile, int line, 
      Array<int>& field, Array<int>& subfield, Array<int>& model) {

   Array<int> vmanip;  // counter for *v records on line
   vmanip.setSize(infile[line].getFieldCount());
   vmanip.allowGrowth(0);
   vmanip.setAll(0);

   Array<int> xmanip; // counter for *x record on line
   xmanip.setSize(infile[line].getFieldCount());
   xmanip.allowGrowth(0);
   xmanip.setAll(0);

   int i = 0;
   int j;
   for (j=0; j<vmanip.getSize(); j++) {
      if (strcmp(infile[line][j], "*v") == 0) {
         vmanip[j] = 1;
      }
      if (strcmp(infile[line][j], "*x") == 0) {
         xmanip[j] = 1;
      }
   }

   int counter = 1;
   for (i=1; i<xmanip.getSize(); i++) {
      if ((xmanip[i] == 1) && (xmanip[i-1] == 1)) {
         xmanip[i] = counter;
         xmanip[i-1] = counter;
         counter++;
      }
   }

   counter = 1;
   i = 0;
   while (i < vmanip.getSize()) {
      if (vmanip[i] == 1) {
         while ((i < vmanip.getSize()) && (vmanip[i] == 1)) {
            vmanip[i] = counter;
            i++;
         }
         counter++;
      }
      i++;
   }

   Array<int> fieldoccur;  // nth occurance of an input spine in the output
   fieldoccur.setSize(field.getSize());
   fieldoccur.allowGrowth(0);
   fieldoccur.setAll(0);
 
   Array<int> trackcounter; // counter of input spines occurances in output
   trackcounter.setSize(infile.getMaxTracks()+1);
   trackcounter.allowGrowth(0);
   trackcounter.setAll(0);

   for (i=0; i<field.getSize(); i++) {
      if (field[i] != 0) {
         trackcounter[field[i]]++;
         fieldoccur[i] = trackcounter[field[i]];
      }
   }

   Array<Array<char> > tempout;
   Array<int> vserial;  
   Array<int> xserial;
   Array<int> fpos;     // input column of output spine

   tempout.setSize(1000);
   tempout.setGrowth(1000);
   tempout.setSize(0);

   vserial.setSize(1000);
   vserial.setGrowth(1000);
   vserial.setSize(0);

   xserial.setSize(1000);
   xserial.setGrowth(1000);
   xserial.setSize(0);

   fpos.setSize(1000);
   fpos.setGrowth(1000);
   fpos.setSize(0);

   Array<char> spat;
   Array<char> spinepat;
   PerlRegularExpression pre;
   int subtarget;
   int modeltarget;
   int xdebug = 0;
   int vdebug = 0;
   int suppress = 0;
   int t;
   int target;
   int tval;
   for (t=0; t<field.getSize(); t++) {
      target = field[t];
      subtarget = subfield[t];
      modeltarget = model[t];
      if (modeltarget == 0) {
         switch (subtarget) {
            case 'a':
            case 'b':
               modeltarget = submodel;
               break;
            case 'c':
               modeltarget = comodel;
         }
      }
      suppress = 0;
      if (target == 0) {
         if (strncmp(infile[line][0], "**", 2) == 0) {
            storeToken(tempout, "**blank"); 
            tval = 0;
            vserial.append(tval);
            xserial.append(tval);
            fpos.append(tval);
         } else if (strcmp(infile[line][0], "*-") == 0) {
            storeToken(tempout, "*-"); 
            tval = 0;
            vserial.append(tval);
            xserial.append(tval);
            fpos.append(tval);
         } else {
            storeToken(tempout, "*"); 
            tval = 0;
            vserial.append(tval);
            xserial.append(tval);
            fpos.append(tval);
         }
      } else {
         for (j=0; j<infile[line].getFieldCount(); j++) {
            if (infile[line].getPrimaryTrack(j) != target) {
               continue;
            }
	    // filter by subfield
	    if (subtarget == 'a') {
               getSearchPat(spat, target, "b");
	       if (pre.search(infile[line].getSpineInfo(j), spat.getBase())) {
                  continue;
               }
	    } else if (subtarget == 'b') {
               getSearchPat(spat, target, "a");
	       if (pre.search(infile[line].getSpineInfo(j), spat.getBase())) {
                  continue;
               }
            }

            switch (subtarget) {
            case 'a':

               if (!pre.search(infile[line].getSpineInfo(j), "\\(")) {
                  if (strcmp(infile[line][j], "*^") == 0) {
                     storeToken(tempout, "*");
                  } else {
                     storeToken(tempout, infile[line][j]);
                  }
               } else {
                  getSearchPat(spat, target, "a");
		  spinepat.setSize(strlen(infile[line].getSpineInfo(j))+1);
		  strcpy(spinepat.getBase(), infile[line].getSpineInfo(j));
		  pre.sar(spinepat, "\\(", "\\(", "g");
		  pre.sar(spinepat, "\\)", "\\)", "g");

		  if ((strcmp(infile[line][j], "*v") == 0) &&
                        (strcmp(spinepat.getBase(), spat.getBase()) == 0)) {
                     storeToken(tempout, "*");
                  } else {
                     getSearchPat(spat, target, "b");
                     if ((strcmp(spinepat.getBase(), spat.getBase())==0) && 
                           (strcmp(infile[line][j], "*v") == 0)) {
                        // do nothing
			suppress = 1;
                     } else {
                        storeToken(tempout, infile[line][j]);
                     }
                  }
               }

               break;
            case 'b':

               if (!pre.search(infile[line].getSpineInfo(j), "\\(")) {
                  if (strcmp(infile[line][j], "*^") == 0) {
                     storeToken(tempout, "*");
                  } else {
                     storeToken(tempout, infile[line][j]);
                  }
               } else {
                  getSearchPat(spat, target, "b");
		  spinepat.setSize(strlen(infile[line].getSpineInfo(j))+1);
		  strcpy(spinepat.getBase(), infile[line].getSpineInfo(j));
		  pre.sar(spinepat, "\\(", "\\(", "g");
		  pre.sar(spinepat, "\\)", "\\)", "g");

		  if ((strcmp(infile[line][j], "*v") == 0) &&
                        (strcmp(spinepat.getBase(), spat.getBase()) == 0)) {
                     storeToken(tempout, "*");
                  } else {
                     getSearchPat(spat, target, "a");
                     if ((strcmp(spinepat.getBase(), spat.getBase())==0) && 
                           (strcmp(infile[line][j], "*v") == 0)) {
                        // do nothing
			suppress = 1;
                     } else {
                        storeToken(tempout, infile[line][j]);
                     }
                  }
               }

               break;
            case 'c':
               // work on later
               storeToken(tempout, infile[line][j]);
               break;
            default:
               storeToken(tempout, infile[line][j]);
            }

	    if (suppress) {
               continue;
            }

            if (strcmp(tempout[tempout.getSize()-1].getBase(), "*x") == 0) {
               tval = fieldoccur[t] * 1000 + xmanip[j];
               xserial.append(tval);
	       xdebug = 1;
            } else {
               tval = 0;
               xserial.append(tval);
            }

            if (strcmp(tempout[tempout.getSize()-1].getBase(), "*v") == 0) {
               tval = fieldoccur[t] * 1000 + vmanip[j];
               vserial.append(tval);
	       vdebug = 1;
            } else {
               tval = 0;
               vserial.append(tval);
            }

            fpos.append(j);

         }
      }
   }

   vserial.allowGrowth(0);
   xserial.allowGrowth(0);

   if (debugQ && xdebug) {
      cout << "!! *x serials = ";
      for (int ii=0; ii<xserial.getSize(); ii++) {
         cout << xserial[ii] << " ";
      }
      cout << "\n";
   }

   if (debugQ && vdebug) {
      cout << "!!LINE: " << infile[line].getLine() << endl;
      cout << "!! *v serials = ";
      for (int ii=0; ii<vserial.getSize(); ii++) {
         cout << vserial[ii] << " ";
      }
      cout << "\n";
   }

   // check for proper *x syntax /////////////////////////////////
   for (i=0; i<xserial.getSize()-1; i++) {
      if (!xserial[i]) {
         continue;
      }
      if (xserial[i] != xserial[i+1]) {
         if (strcmp(tempout[i].getBase(), "*x") == 0) {
            xserial[i] = 0;
            tempout[i].setSize(tempout[i].getSize()-1);
            strcpy(tempout[i].getBase(), "*");
         }
      } else {
         i++;
      }
   }

   if ((tempout.getSize() == 1) || (xserial.getSize() == 1)) {
      // get rid of *x if there is only one spine in output
      if (xserial[0]) {
         xserial[0] = 0;
         tempout[0].setSize(tempout[0].getSize()-1);
         strcpy(tempout[0].getBase(), "*");
      }
   } else if (xserial.getSize() > 1) {
      // check the last item in the list
      int index = xserial.getSize()-1;
      if (strcmp(tempout[index].getBase(), "*x") == 0) {
         if (xserial[index] != xserial[index-1]) {
            xserial[index] = 0;
            tempout[index].setSize(tempout[index].getSize()-1);
            strcpy(tempout[index].getBase(), "*");
         }
      }
   }
	     
   // check for proper *v syntax /////////////////////////////////
   Array<int> vsplit;
   vsplit.setSize(vserial.getSize());
   vsplit.allowGrowth(0);
   vsplit.setAll(0);

   // identify necessary line splits
   for (i=0; i<vserial.getSize()-1; i++) {
      if (!vserial[i]) {
         continue;
      }
      while ((i<vserial.getSize()-1) && (vserial[i]==vserial[i+1])) {
         i++;
      }
      if ((i<vserial.getSize()-1) && vserial[i]) {
         if (vserial.getSize() > 1) {
            if (vserial[i+1]) {
               vsplit[i+1] = 1;
            }
         }
      } 
   }

   // remove single *v spines:

   for (i=0; i<vsplit.getSize()-1; i++) {
      if (vsplit[i] && vsplit[i+1]) {
         if (strcmp(tempout[i].getBase(), "*v") == 0) {
            tempout[i].setSize(2);
            strcpy(tempout[i].getBase(), "*");
            vsplit[i] = 0;
         }
      }
   }

   if (debugQ) {
      cout << "!!vsplit array: ";
      for (i=0; i<vsplit.getSize(); i++) {
         cout << " " << vsplit[i];
      }
      cout << endl;
   }
        
   if (vsplit.getSize() > 0) {
      if (vsplit[vsplit.getSize()-1]) {
         if (strcmp(tempout[tempout.getSize()-1].getBase(), "*v") == 0) {
            tempout[tempout.getSize()-1].setSize(2);
            strcpy(tempout[tempout.getSize()-1].getBase(), "*");
            vsplit[vsplit.getSize()-1] = 0;
         }
      }
   }

   int vcount = 0;
   for (i=0; i<vsplit.getSize(); i++) {
      vcount += vsplit[i];
   }
   
   if (vcount) {
      printMultiLines(vsplit, vserial, tempout);
   }

   int start = 0;
   for (i=0; i<tempout.getSize(); i++) {
      if (strcmp(tempout[i].getBase(), "") != 0) {
         if (start != 0) {
            cout << "\t";
         }
         cout << tempout[i].getBase();
         start++;
      }
   }
   if (start) {
      cout << '\n';
   }
}



//////////////////////////////
//
// printMultiLines -- print separate *v lines.
//

void printMultiLines(Array<int>& vsplit, Array<int>& vserial, 
      Array<Array<char> >& tempout) {
   int i;

   int splitpoint = -1;
   for (i=0; i<vsplit.getSize(); i++) {
      if (vsplit[i]) {
         splitpoint = i;
         break;
      }
   }

   if (debugQ) {
      cout << "!!tempout: ";
      for (i=0; i<tempout.getSize(); i++) {
         cout << tempout[i].getBase() << " ";
      }
      cout << endl;
   }

   if (splitpoint == -1) {
      return;
   }

   int start = 0;
   int printv = 0;
   for (i=0; i<splitpoint; i++) {
      if (strcmp(tempout[i].getBase(), "") != 0) {
         if (start) {
            cout << "\t";
         }
         cout << tempout[i].getBase();
         start = 1;
         if (strcmp(tempout[i].getBase(), "*v") == 0) {
            if (printv) {
               tempout[i].setSize(1);               
               tempout[i][0] = '\0';
            } else {
               tempout[i].setSize(2);               
               strcpy(tempout[i].getBase(), "*");
               printv = 1;
            }
         } else {
            tempout[i].setSize(2);
            strcpy(tempout[i].getBase(), "*");
         }
      }
   }

   for (i=splitpoint; i<vsplit.getSize(); i++) {
      if (strcmp(tempout[i].getBase(), "") != 0) {
         if (start) {
            cout << "\t";
         }
         cout << "*";
      }
   }

   if (start) {
      cout << "\n";
   }

   vsplit[splitpoint] = 0;

   printMultiLines(vsplit, vserial, tempout);
}



//////////////////////////////
//
// storeToken --
//

void storeToken(Array<Array<char> >& storage, const char* string) {
   storage.setSize(storage.getSize()+1);
   int index = storage.getSize()-1;
   storeToken(storage, index, string);
}

void storeToken(Array<Array<char> >& storage, int index, const char* string) {
   storage[index].setSize(strlen(string)+1);
   strcpy(storage[index].getBase(), string);
}



//////////////////////////////
//
// isInList -- returns true if first number found in list of numbers.
//     returns the matching index plus one.
//

int isInList(int number, Array<int>& listofnum) {
   int i;
   for (i=0; i<listofnum.getSize(); i++) {
      if (listofnum[i] == number) {
         return i+1;
      }
   }
   return 0;

}



//////////////////////////////
//
// getTraceData -- 
//

void getTraceData(Array<int>& startline, Array<Array<int> >& fields, 
      const char* tracefile, HumdrumFile& infile) {
   char buffer[1024] = {0};
   PerlRegularExpression pre;
   int linenum;
   startline.setSize(10000);
   startline.setGrowth(10000);
   startline.setSize(0);
   fields.setSize(10000);
   fields.setGrowth(10000);
   fields.setSize(0);

   ifstream input;
   input.open(tracefile);
   if (!input.is_open()) {
      cerr << "Error: cannot open file for reading: " << tracefile << endl;
      exit(1);
   }

   Array<char> temps;
   Array<int> field;
   Array<int> subfield;
   Array<int> model;

   input.getline(buffer, 1024);
   while (!input.eof()) {
      if (pre.search(buffer, "^\\s*$")) {
         continue;
      }
      if (!pre.search(buffer, "(\\d+)")) {
         continue;
      }
      linenum = strtol(pre.getSubmatch(1), NULL, 10);
      linenum--;  // adjust so that line 0 is the first line in the file
      temps.setSize(strlen(buffer)+1);
      strcpy(temps.getBase(), buffer);
      pre.sar(temps, "\\d+", "", "");
      pre.sar(temps, "[^,\\s\\d\\$\\-].*", "");  // remove any possible comments
      pre.sar(temps, "\\s", "", "g");
      if (pre.search(temps.getBase(), "^\\s*$")) {
         // no field data to process online
         continue;
      }
      startline.append(linenum);
      fillFieldData(field, subfield, model, temps.getBase(), infile);
      fields.append(field);
      input.getline(buffer, 1024);
   }

}



//////////////////////////////
//
// extractTrace --
//

void extractTrace(HumdrumFile& infile, const char* tracefile) {
   Array<int> startline;
   Array<Array<int> > fields;
   getTraceData(startline, fields, tracefile, infile);
   int i, j;

   if (debugQ) {
      for (i=0; i<startline.getSize(); i++) {
         cout << "!!TRACE " << startline[i]+1 << ":\t";
         for (j=0; j<fields[i].getSize(); j++) {
            cout << fields[i][j] << " ";
         }
         cout << "\n";
      }
   }
             

   if (startline.getSize() == 0) {
      for (i=0; i<infile.getNumLines(); i++) {
         switch (infile[i].getType()) {
            case E_humrec_none:
            case E_humrec_empty:
            case E_humrec_global_comment:
            case E_humrec_bibliography:
               cout << infile[i] << '\n';
         }
      }
      return;
   }

   for (i=0; i<startline[0]; i++) {
      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
            cout << infile[i] << '\n';
      }
   }

   int endline;
   for (j=0; j<startline.getSize(); j++) {
      if (j == startline.getSize()-1) {
         endline = infile.getNumLines()-1;
      } else {
         endline = startline[j+1]-1;
      }
      for (i=startline[j]; i<endline; i++) {
         switch (infile[i].getType()) {
            case E_humrec_none:
            case E_humrec_empty:
            case E_humrec_global_comment:
            case E_humrec_bibliography:
               cout << infile[i] << '\n';
            default:
               printTraceLine(infile, i, fields[j]);
         }
      }
   }
}



//////////////////////////////
//
// printTraceLine --
//

void printTraceLine(HumdrumFile& infile, int line, Array<int>& field) {
   int j;
   int t;
   int start = 0;
   int target;

   start = 0;
   for (t=0; t<field.getSize(); t++) {
      target = field[t];
      for (j=0; j<infile[line].getFieldCount(); j++) {
         if (infile[line].getPrimaryTrack(j) != target) {
            continue;
         }
         if (start != 0) {
            cout << '\t';
         }
         start = 1;
         cout << infile[line][j];
      }
   }
   if (start != 0) {
      cout << endl;
   }
}


/*

//////////////////////////////
//
// extractInterpretation -- extract interpretations which match the
//    given list.  Not used any more: generalized to allow for
//    other interpretation matching other than exclusive interpretations.
//

void extractInterpretations(HumdrumFile& infile, const char* interps) {
   int i;
   int j;
   int column = 0;
   PerlRegularExpression pre;
   Array<char> buffer;
   buffer.setSize(1024);
   buffer.setSize(1);
   strcpy(buffer.getBase(), "");

   for (i=0; i<infile.getNumLines(); i++) {
      if (debugQ) {
         cout << "!!Processing line " << i+1 << ": " << infile[i] << endl;
      }
      switch (infile[i].getType()) {
         case E_humrec_none:
         case E_humrec_empty:
         case E_humrec_global_comment:
         case E_humrec_bibliography:
            cout << infile[i] << '\n';
            break;
         case E_humrec_data_comment:
         case E_humrec_data_kern_measure:
         case E_humrec_interpretation:
         case E_humrec_data:
            column = 0;
            for (j=0; j<infile[i].getFieldCount(); j++) {
               buffer.setSize(strlen(infile[i].getExInterp(j))+1);
               strcpy(buffer.getBase(), infile[i].getExInterp(j));
               buffer.setSize(buffer.getSize()+strlen("\\b"));
               strcat(buffer.getBase(), "\\b"); // word boundary marker
	       pre.sar(buffer, "\\*", "\\\\*", "g");
               if (pre.search(interps, buffer.getBase()) == 0) {
               // if (strstr(interps, infile[i].getExInterp(j)) == NULL) {
                  continue;
               }
               if (column != 0) {
                  cout << '\t';
               }
               column++;
               cout << infile[i][j];
            }
            if (column != 0) {
               cout << endl;
            }
            break;
         default:
            cout << "!!Line is UNKNOWN:" << infile[i] << endl;
            break;
      }
   }
}

*/



//////////////////////////////
//
// checkOptions -- validate and process command-line options.
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("P|F|S|x|exclude=s:", "Remove listed spines from output");
   opts.define("i=s:", "Exclusive interpretation list to extract from input");
   opts.define("I=s:", "Exclusive interpretation exclusion list");
   opts.define("f|p|s|field|path|spine=s:", 
		   "for extraction of particular spines");
   opts.define("C|count=b", "print a count of the number of spines in file");
   opts.define("c|cointerp=s:**kern", "Exclusive interpretation for cospines");
   opts.define("g|grep=s:", "Extract spines which match a given regex.");
   opts.define("r|reverse=b", "reverse order of spines by **kern group");
   opts.define("R=s:**kern", "reverse order of spine by exinterp group");
   opts.define("t|trace=s:", "use a trace file to extract data");
   opts.define("e|expand=b", "expand spines with subspines");
   opts.define("E|expand-interp=s:", "expand subspines limited to exinterp");
   opts.define("m|model|method=s:d", "method for extracting secondary spines");
   opts.define("M|cospine-model=s:d", "method for extracting cospines");
   opts.define("Y|no-editoral-rests=b", 
		   "do not display yy marks on interpreted rests");

   opts.define("debug=b", "print debugging information");
   opts.define("author=b");                     // author of program
   opts.define("version=b");                    // compilation info
   opts.define("example=b");                    // example usages
   opts.define("h|help=b");                     // short description
   opts.process(argc, argv);
   
   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, Feb 2008" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: Feb 2008" << endl;
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

   excludeQ    = opts.getBoolean("x");
   interpQ     = opts.getBoolean("i");
   interps     = opts.getString("i");
   interpstate = 1;
   if (!interpQ) {
      interpQ = opts.getBoolean("I");
      interpstate = 0;
      interps = opts.getString("I");
   }
	     
   fieldQ      = opts.getBoolean("f");
   debugQ      = opts.getBoolean("debug");
   countQ      = opts.getBoolean("count");
   traceQ      = opts.getBoolean("trace");
   tracefile   = opts.getString("trace");
   reverseQ    = opts.getBoolean("reverse");
   expandQ     = opts.getBoolean("expand") || opts.getBoolean("E");
   submodel    = opts.getString("model")[0];
   cointerp    = opts.getString("cointerp");
   comodel     = opts.getString("cospine-model")[0];

   if (opts.getBoolean("no-editoral-rests")) {
      editorialInterpretation = "";
   }

   if (interpQ) {
      fieldQ = 1;
   }
	    
   if (expandQ) {
      fieldQ = 1;
      expandInterp = opts.getString("expand-interp");
   }

   if (!reverseQ) {
      reverseQ = opts.getBoolean("R");
      if (reverseQ) {
         reverseInterp = opts.getString("R");
      }
   }

   if (reverseQ) {
      fieldQ = 1;
   }

   if (excludeQ) {
      fieldstring = opts.getString("x");
   } else if (fieldQ) {
      fieldstring = opts.getString("f");
   }

   grepQ = opts.getBoolean("grep");
   grepString = opts.getString("grep");

}



//////////////////////////////
//
// example -- example usage of the sonority program
//

void example(void) {
   cout <<
   "                                                                         \n"
   << endl;
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



// md5sum: 07c88288c90a26c8102e6708e4b202d7 extractx.cpp [20121016]
